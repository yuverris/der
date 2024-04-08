#ifndef DER_TYPECHECKER_HH
#define DER_TYPECHECKER_HH
#include "debug.hpp"
#include "parser.hpp"
#include "types.hpp"
#include "lexer.hpp"
#include "der_ir.hpp"
#include <map>
#include <string>
#include <memory>
namespace der
{
    namespace typechecker
    {

        struct TypeChecker
        {
            struct Pair
            {
                std::shared_ptr<types::TypeHandle> ty;
                std::shared_ptr<ast::Expr> expr;
                bool usable = true;
            };
            std::vector<parser::AstInfo> m_input{};
            unsigned int m_index = 0;
            // this is the only approach to name mangling I can think of, is to first statically analyze and store into a scope
            // then go over that scope and update it as soon as a call to a generic function is found.
            // as for actually replacing the caller to the new mangled name, it's better to modify the next input
            // will it be a performance hit?.... I'm sure it is we'll see.
            std::map<std::string, std::shared_ptr<types::TypeHandle>> local_scope = {};
            std::map<std::string, std::shared_ptr<types::TypeHandle>> generics_scope = {};
            std::vector<std::unique_ptr<der::ir::Expr>> m_output{};
            bool is_in_fn = false;
            std::shared_ptr<types::TypeHandle> ret_fn_ty = nullptr;

            TypeChecker(const std::vector<parser::AstInfo> &in) : m_input(in) {}

            parser::AstInfo m_current()
            {
                return m_input.at(m_index);
            }
            void m_advance()
            {
                if (m_index < m_input.size())
                    m_index += 1;
            }
            void do_the_thing()
            {
                for (auto &x : m_input)
                {
                    get_stmt_type(x.expr->get_ty()->clone(), x.loc);
                    m_advance();
                }
                for (auto &x : m_input)
                {
                    der_debug("converting to ir.....");
                    der_debug_e(x.expr == nullptr);
                    der_debug(std::format("value: {}", x.expr->debug()));
                    m_output.push_back(convert_to_ir(x.expr->clone()));
                    // der_debug_e(x.expr->debug());
                }
            }
            std::string get_output()
            {
                std::string out;
                for (auto &&a : m_output)
                {
                    out += std::format("{};\n", a->value());
                }
                return out;
            }
            // got confused lol, but I convert directly from AST -> IR after the typechecker is successfully done
            std::unique_ptr<der::ir::Expr> convert_to_ir(const std::shared_ptr<der::ast::Expr> &expr)
            {
                der_debug("start");
                der_debug_e(expr->debug());
                if (expr->get_ty()->get_ty() == types::TYPES::INTEGER)
                {
                    der_debug("recognized INTEGER.");
                    return std::make_unique<der::ir::Integer>(dynamic_cast<der::ast::Integer *>(expr.get())->value);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::STRING)
                {
                    der_debug("recognized STR");
                    return std::make_unique<der::ir::String>(dynamic_cast<der::ast::String *>(expr.get())->value);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::CHAR)
                {
                    der_debug("recognized CHAR");
                    return std::make_unique<der::ir::Char>(dynamic_cast<der::ast::Character *>(expr.get())->val);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::BOOL)
                {
                    der_debug("recognized BOOL");
                    return std::make_unique<der::ir::Bool>(dynamic_cast<der::ast::Bool *>(expr.get())->value);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::BINARY_OP)
                {
                    ast::BinaryOper *binop = dynamic_cast<ast::BinaryOper *>(expr.get());
                    der_debug("recognized BIN_OP IR.");
                    std::unique_ptr<ir::Expr> lfs = convert_to_ir(std::move(binop->left));
                    std::unique_ptr<ir::Expr> rfs = convert_to_ir(std::move(binop->right));
                    return std::make_unique<der::ir::Binary>(std::move(lfs), lexer::tokens_to_str[binop->op], std::move(rfs));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::LOGICAL_OP)
                {
                    der_debug("recognized LOG_OP type.");
                    ast::LogicalBinaryOper *logop = dynamic_cast<ast::LogicalBinaryOper *>(expr.get());
                    std::unique_ptr<ir::Expr> lfs = convert_to_ir(std::move(logop->left));
                    std::unique_ptr<ir::Expr> rfs = convert_to_ir(std::move(logop->right));
                    return std::make_unique<der::ir::Logical>(std::move(lfs), lexer::tokens_to_str[logop->op], std::move(rfs));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::IDENT)
                {
                    der_debug("recognized IDENT IR.");
                    return std::make_unique<der::ir::Ident>(dynamic_cast<ast::Identifier *>(expr.get())->ident);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::FCALL)
                {
                    der_debug("aha fcallllll!!!!");
                    ast::FunctionCall *callee = dynamic_cast<ast::FunctionCall *>(expr.get());
                    std::vector<std::unique_ptr<der::ir::Expr>> args;
                    for (auto &a : callee->args)
                        args.push_back(convert_to_ir(a->clone()));
                    return std::make_unique<der::ir::FunctionCall>(convert_to_ir(std::move(callee->callee)), args);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::VAR)
                {
                    ast::Variable *var = dynamic_cast<ast::Variable *>(expr.get());
                    std::string var_name = var->name;
                    auto var_value = convert_to_ir(std::move(var->value));
                    if (var->ty->get_ty() == types::TYPES::ARRAY)
                    {
                        types::Array *array_ty = dynamic_cast<types::Array *>(var->ty.get());
                        return std::make_unique<der::ir::ArrayVariable>(convert_c_type(std::move(array_ty->ty)), var_name, array_ty->size, std::move(var_value));
                    }
                    else
                    {
                        if (var->ty->get_ty() == types::TYPES::IDENT)
                        {
                            std::string id = dynamic_cast<types::Identifier *>(var->ty.get())->ident;
                            return std::make_unique<der::ir::Variable>(convert_c_type(std::move(local_scope.at(id))), var_name, std::move(var_value));
                        }
                        else
                        {
                            return std::make_unique<der::ir::Variable>(convert_c_type(std::move(var->ty)), var_name, std::move(var_value));
                        }
                    }
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::FUNCTION)
                {
                    ast::Function<parser::AstInfo> *fnc = dynamic_cast<ast::Function<parser::AstInfo> *>(expr.get());
                    der_debug(std::format("fname {}", fnc->name));
                    std::vector<ir::CArgTy> c_args = {};
                    std::vector<std::unique_ptr<ir::Expr>> body = {};
                    for (auto &arg : fnc->args)
                    {
                        c_args.push_back({.ty = convert_c_type(std::move(arg.ty)), .name = arg.ident});
                    }
                    for (auto &s : fnc->body)
                    {
                        body.push_back(convert_to_ir(std::move(s.expr)));
                    }

                    return std::make_unique<ir::Function>(convert_c_type(std::move(fnc->ret_ty)), fnc->name, c_args, body);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::RETURN)
                {
                    return std::make_unique<ir::Return>(convert_to_ir(std::move(dynamic_cast<ast::Return *>(expr.get())->ret_expr)));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::IF)
                {
                    ast::IfStmt<parser::AstInfo> *ifs = dynamic_cast<ast::IfStmt<parser::AstInfo> *>(expr.get());
                    std::unique_ptr<ir::Expr> cond = convert_to_ir(std::move(ifs->cond));
                    std::vector<std::unique_ptr<ir::Expr>> body = {};
                    std::vector<std::unique_ptr<ir::Expr>> else_ = {};
                    for (auto &a : ifs->body)
                        body.push_back(convert_to_ir(std::move(a.expr)));
                    for (auto &a : ifs->else_block)
                        else_.push_back(convert_to_ir(std::move(a.expr)));
                    return std::make_unique<ir::If>(std::move(cond), std::move(body), std::move(else_));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::PIPE_OP)
                {
                    ast::PipeOper *pipe = dynamic_cast<ast::PipeOper *>(expr.get());
                    std::unique_ptr<ir::Expr> lfs = convert_to_ir(std::move(pipe->left));
                    std::unique_ptr<ir::Expr> rfs = convert_to_ir(std::move(pipe->right));
                    ir::FunctionCall *fnc = dynamic_cast<ir::FunctionCall *>(rfs.get());
                    fnc->args.push_back(std::move(lfs));
                    // auto reduced = ir::FunctionCall(std::move(fnc->callee), );
                    return std::make_unique<ir::FunctionCall>(*fnc);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::SMOL_IF)
                {
                    ast::SmolIfStmt *ifst = dynamic_cast<ast::SmolIfStmt *>(expr.get());
                    return std::make_unique<ir::SmolIf>(std::move(convert_to_ir(std::move(ifst->cond))), std::move(convert_to_ir(std::move(ifst->expr))));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::ARRAY)
                {
                    ast::Array *array = dynamic_cast<ast::Array *>(expr.get());
                    std::vector<std::unique_ptr<ir::Expr>> values = {};
                    for (auto &a : array->values)
                        values.push_back(convert_to_ir(std::move(a)));
                    return std::make_unique<ir::Array>(std::move(values));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::STRUCT)
                {
                    ast::Struct *_struct = dynamic_cast<ast::Struct *>(expr.get());
                    std::vector<ir::StructMember> members = {};
                    for (auto &a : _struct->members)
                    {
                        members.push_back(ir::StructMember(a.name, convert_c_type(std::move(a.type))));
                    }
                    return std::make_unique<ir::Struct>(_struct->name, members);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::ENUM)
                {
                    ast::Enum *_enum = dynamic_cast<ast::Enum *>(expr.get());
                    return std::make_unique<ir::Enum>(_enum->name, _enum->members);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::RANGED_FOR)
                {
                    ast::RangedFor<parser::AstInfo> *ranged_for = dynamic_cast<ast::RangedFor<parser::AstInfo> *>(expr.get());
                    // std::cout << ranged_for->debug() << '\n';
                    std::unique_ptr<ir::Expr> init = convert_to_ir(std::move(ranged_for->f_start));
                    std::unique_ptr<ir::Expr> goal = convert_to_ir(std::move(ranged_for->f_end));
                    std::string ident = ranged_for->ident;
                    std::vector<std::unique_ptr<ir::Expr>> body = {};
                    for (auto &e : ranged_for->body)
                        body.push_back(convert_to_ir(e.expr->clone()));
                    return std::make_unique<ir::RangedFor>(std::move(init), std::move(goal), ident, body);
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::SUBSCRIPT)
                {
                    ast::Subscript *ex = dynamic_cast<ast::Subscript *>(expr.get());
                    return std::make_unique<ir::Subscript>(convert_to_ir(std::move(ex->target)), convert_to_ir(std::move(ex->inner)));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::SET_OP)
                {
                    ast::SetOper *set_op = dynamic_cast<ast::SetOper *>(expr.get());
                    return std::make_unique<ir::SetOp>(convert_to_ir(std::move(set_op->left)), convert_to_ir(std::move(set_op->right)));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::DOT_OP)
                {
                    ast::DotOper *dot_op = dynamic_cast<ast::DotOper *>(expr.get());
                    return std::make_unique<ir::Dot>(convert_to_ir(std::move(dot_op->left)), convert_to_ir(std::move(dot_op->right)));
                }
                else if (expr->get_ty()->get_ty() == types::TYPES::STRUCT_INSTANCE)
                {
                    ast::StructInstance *instance = dynamic_cast<ast::StructInstance *>(expr.get());
                    std::vector<ir::StructInitializer> inits = {};
                    for (auto &x : instance->inits)
                    {
                        inits.push_back(ir::StructInitializer{.ident = x.ident, .value = convert_to_ir(std::move(x.value))});
                    }
                    return std::make_unique<ir::StructInstance>(inits);
                }
                else
                {
                    throw 44;
                }
            }

            // need to handle much more complicated types, array and shit as well
            std::string convert_c_type(const std::shared_ptr<types::TypeHandle> &type)
            {
                der_debug_e(type->debug());
                switch (type->get_ty())
                {
                case types::TYPES::BOOL:
                    return "int";
                case types::TYPES::INTEGER:
                    return "int";
                case types::TYPES::STRING:
                    return "const char*";
                case types::TYPES::DOUBLE:
                    return "double";
                case types::TYPES::VOID:
                    return "void";
                case types::TYPES::CHAR:
                    return "char";
                case types::TYPES::STRUCT:
                    return std::format("struct {}", dynamic_cast<types::Struct *>(type.get())->name);
                case types::TYPES::ENUM:
                    return std::format("enum {}", dynamic_cast<types::Enum *>(type.get())->name);
                default:
                {
                    der_debug_e(type->debug());
                    return "wiwi";
                }
                }
            }

            void get_stmt_type(const std::shared_ptr<types::TypeHandle> &type, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug_e(type->debug());
                if (type->get_ty() == types::TYPES::VAR)
                {
                    der_debug("recognized VAR type.");
                    check_var(dynamic_cast<types::Variable *>(type.get()), loc);
                }

                else if (type->get_ty() == types::TYPES::IF)
                {
                    der_debug("recognized IF type.");
                    types::If *ifs = dynamic_cast<types::If *>(type.get());
                    for (std::unique_ptr<types::TypeHandle> &t : ifs->body)
                    {
                        der_debug("typechecking the if-then.");
                        der_debug_e(t->debug());
                        get_stmt_type(t->clone(), loc);
                    }
                    for (std::unique_ptr<types::TypeHandle> &t : ifs->else_stmt)
                    {
                        der_debug("typechecking the if-else.");
                        der_debug_e(t->debug());
                        get_stmt_type(t->clone(), loc);
                    }
                    if (get_expr_type(std::move(ifs->cond), loc)->get_ty() != types::TYPES::BOOL)
                        throw types::CompilationErr("if statement condition must return a boolean.", loc);
                }
                else if (type->get_ty() == types::TYPES::FUNCTION)
                {
                    der_debug("inner fnc def");
                    check_fn(dynamic_cast<types::Function *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::FCALL)
                {
                    der_debug("aha fcallllll!!!!");
                    // get_expr_type(type, expr, loc);
                    check_fncall(dynamic_cast<types::Fcall *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::STRUCT)
                {
                    der_debug("struct encounter.");
                    check_struct(dynamic_cast<types::Struct *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::RANGED_FOR)
                {
                    der_debug("ranged for visit");
                    check_ranged_for(dynamic_cast<types::RangedFor *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::ENUM)
                {
                    der_debug("enum encounter.");
                    check_enum(dynamic_cast<types::Enum *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::SET_OP)
                {
                    der_debug("set op encounter.");
                    check_set_op(dynamic_cast<types::SetOp *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::RETURN)
                {
                    // if (is_in_fn && ret_fn_ty != nullptr)
                    // {
                    //     types::Return *ret = dynamic_cast<types::Return *>(type.get());
                    //     auto rty = get_expr_type(std::move(ret->ty), loc);
                    //     auto frty = get_expr_type(std::move(ret_fn_ty->clone()), loc);
                    //     if (!rty->is_same(frty.get()))
                    //         throw types::CompilationErr("return type doesnt match return expr type", loc);
                    // }
                    types::Return *ret = dynamic_cast<types::Return *>(type.get());
                    ret_fn_ty = get_expr_type(std::move(ret->ty), loc);
                }
                else if (type->get_ty() == types::TYPES::SMOL_IF)
                {
                    der_debug("smol if encounter.");
                    types::SmolIf *smol_if = dynamic_cast<types::SmolIf *>(type.get());
                    if (get_expr_type(smol_if->lfs->clone(), loc)->get_ty() != types::TYPES::BOOL)
                    {
                        throw types::CompilationErr("expected bool expr in smol if stmt", loc);
                    }
                }
                else
                {
                    der_debug("shit");
                    der_debug("falling back to calling get_expr_ty anyway");
                    der_debug_e(type->debug());
                    get_expr_type(type->clone(), loc);
                }
            }
            std::shared_ptr<types::TypeHandle> get_expr_type(const std::shared_ptr<types::TypeHandle> &type, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug_e(type->debug());
                if (type->get_ty() == types::TYPES::INTEGER)
                {
                    der_debug("recognized INTEGER.");
                    return type;
                }
                else if (type->get_ty() == types::TYPES::STRING)
                {
                    der_debug("recognized STR");
                    return type;
                }
                else if (type->get_ty() == types::TYPES::BOOL)
                {
                    der_debug("recognized BOOL");
                    return type;
                }
                else if (type->get_ty() == types::TYPES::ARRAY)
                {
                    der_debug("recognized ARRAY");
                    return type;
                }
                else if (type->get_ty() == types::TYPES::CHAR)
                {
                    der_debug("recognized CHAR");
                    return type;
                }
                else if (type->get_ty() == types::TYPES::BINARY_OP)
                {
                    der_debug("recognized BIN_OP type.");
                    return check_binary(dynamic_cast<types::BinaryOp *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::LOGICAL_OP)
                {
                    der_debug("recognized LOG_OP type.");
                    return check_logical_binary(dynamic_cast<types::LogicalBinaryOp *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::IDENT)
                {
                    return check_identifier(dynamic_cast<types::Identifier *>(type.get())->ident, loc);
                }
                else if (type->get_ty() == types::TYPES::FCALL)
                {
                    der_debug("aha fcallllll!!!!");
                    return check_fncall(dynamic_cast<types::Fcall *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::DOT_OP)
                {
                    der_debug("dot op encounter");
                    return check_dot_op(dynamic_cast<types::DotOp *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::SUBSCRIPT)
                {
                    der_debug("sdsdsdsdsd");
                    return check_subscript(dynamic_cast<types::Subscript *>(type.get()), loc);
                }
                else if (type->get_ty() == types::TYPES::PIPE_OP)
                {
                    types::PipeOp *pipe = dynamic_cast<types::PipeOp *>(type.get());
                    std::shared_ptr<types::TypeHandle> lfs = get_expr_type(std::move(pipe->lfs), loc);
                    std::shared_ptr<types::TypeHandle> rfs = std::move(pipe->rfs); /*-*/
                    if (rfs->get_ty() != types::TYPES::FCALL)
                    {
                        throw types::CompilationErr("right hand of the pipe operator '|>' should be a function call.", loc);
                    }
                    types::Fcall *fnc = dynamic_cast<types::Fcall *>(rfs.get());
                    fnc->args.push_back(lfs->clone());
                    return check_fncall(fnc, loc);
                }
                else if (type->get_ty() == types::TYPES::STRUCT_INSTANCE)
                {
                    der_debug("ara ara struct instance");
                    return check_struct_instance(dynamic_cast<types::StructInstance *>(type.get()), loc);
                }
                else
                {
                    der_debug("shit");
                    der_debug_e(type->debug());
                    throw 99;
                    return std::unique_ptr<types::TypeHandle>(new types::Dummy());
                }
            }
            void check_var(types::Variable *var, const SourceLoc &loc)
            {
                der_debug("start");
                if (local_scope.find(var->name) != local_scope.end())
                    throw types::CompilationErr(std::format("identifier {} is already defined.", var->name), loc);
                std::shared_ptr<types::TypeHandle> expected = std::move(var->expected_ty);
                der_debug_e(expected->debug());
                der_debug_e(var->actual_ty->debug());
                std::shared_ptr<types::TypeHandle> actual = get_expr_type(std::move(var->actual_ty), loc);

                // if (actual->get_ty() == types::TYPES::FCALL)
                // {
                //     types::Fcall* fc = dynamic_cast<types::Fcall*>(actual.get());
                //     actual = fc->
                // }
                if (expected->get_ty() == types::TYPES::IDENT)
                {
                    types::Identifier *id = dynamic_cast<types::Identifier *>(expected.get());
                    if (local_scope.find(id->ident) == local_scope.end())
                        throw types::CompilationErr(std::format("type '{}' is not defined.", id->ident), loc);
                    else
                    {
                        auto ident = local_scope.at(id->ident);
                        if (ident->get_ty() != types::TYPES::STRUCT && ident->get_ty() != types::TYPES::ENUM)
                        {
                            throw types::CompilationErr(std::format("'{}' is not a type.", id->ident), loc);
                        }
                        else if (!ident->is_same(actual.get()))
                        {
                            throw types::CompilationErr(std::format("variable is type of: '{}', value is type of: {}", ident->debug(), actual->debug()), loc);
                        }
                    }
                    local_scope[var->name] = local_scope.at(id->ident);
                }
                else if (expected->is_same(actual.get()))
                    local_scope[var->name] = std::move(expected);
                else
                    throw types::CompilationErr(std::format("inconsistent variable type. var is {}, value is {}", expected->debug(), actual->debug()), loc);
            }
            void check_set_op(types::SetOp *op, const SourceLoc &loc)
            {
                der_debug("start");
                if (op->lfs->get_ty() == types::TYPES::IDENT)
                {
                    types::Identifier *ident = dynamic_cast<types::Identifier *>(op->lfs.get());
                    if (local_scope.find(ident->ident) == local_scope.end())
                    {
                        throw types::CompilationErr(std::format("identifier '{}' is not defined.", ident->ident), loc);
                    }
                    else
                    {
                        if (local_scope.at(ident->ident)->is_same(op->rfs.get()))
                        {
                            local_scope[ident->ident] = op->lfs->clone();
                        }
                        else
                        {
                            throw types::CompilationErr(std::format("identifier '{}' is type {}, you are trying to assign it with a {} instead.", ident->ident,
                                                                    local_scope.at(ident->ident)->debug(), op->rfs->debug()),
                                                        loc);
                        }
                    }
                }
                else
                {
                    throw types::CompilationErr("invalid left hand of reassign.", loc);
                }
            }
            std::shared_ptr<types::TypeHandle> check_dot_op(types::DotOp *dotop, const SourceLoc &loc)
            {
                // std::string left_ty_deb = dotop->lfs->debug();
                der_debug_e(dotop->lfs->debug());
                auto left = get_expr_type(std::move(dotop->lfs), loc);
                if (dotop->rfs->get_ty() != types::TYPES::IDENT)
                    throw types::CompilationErr("dot operator expected an identifier on the right hand.", loc);
                der_debug_e(left->debug());
                types::Identifier *right_ident = dynamic_cast<types::Identifier *>(dotop->rfs.get());
                if (left->get_ty() == types::TYPES::STRUCT)
                {
                    types::Struct *actual_struct = dynamic_cast<types::Struct *>(left.get());
                    for (auto &x : actual_struct->members)
                    {
                        if (x.name == right_ident->ident)
                            return std::move(x.type);
                    }
                    throw types::CompilationErr("gaygaygyagyagaygaygaygaygaygaygaygay", loc);
                    //  types::Struct* _struct = dynamic_cast<types::Struct*>(left.get());
                }
                else if (left->get_ty() == types::TYPES::ENUM)
                {
                    types::Enum *_enum = dynamic_cast<types::Enum *>(left.get());
                    for (auto &x : _enum->members)
                    {
                        der_debug("woah is that an enum?");
                        if (x == right_ident->ident)
                        {
                            return std::make_shared<types::EnumInstance>(*_enum, right_ident->ident);
                        }
                    }
                    throw types::CompilationErr(std::format("{} is not a member of enum {}", right_ident->ident, _enum->name), loc);
                }
                else
                {
                    throw types::CompilationErr("dot operator only valable on structs and enums", loc);
                }
            }
            // REMINDER NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS
            // NO FOKING IMPLICIT CONVERSIONS NO FOKING IMPLICIT CONVERSIONS NO FOKING IMPLICIT CONVERSIONS
            // NO FOKING IMPLICIT CONVERSIONS NO FOKING IMPLICIT CONVERSIONS NO FOKING IMPLICIT CONVERSIONS
            std::shared_ptr<types::TypeHandle> check_binary(types::BinaryOp *bin, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug("lfs check");
                auto t = bin->clone();
                auto lfs = get_expr_type(std::move(bin->lfs), loc);
                der_debug("rfs check");
                auto rfs = get_expr_type(std::move(bin->rfs), loc);
                if (lfs->get_ty() != rfs->get_ty())
                    throw types::CompilationErr("binary operation not supported by different operand types.", loc);
                return std::shared_ptr<types::TypeHandle>(std::move(t));
            }
            std::shared_ptr<types::TypeHandle> check_logical_binary(types::LogicalBinaryOp *bin, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug("lfs check");
                auto lfs = get_expr_type(std::move(bin->lfs), loc);
                der_debug("rfs check");
                auto rfs = get_expr_type(std::move(bin->rfs), loc);
                if (lfs->get_ty() != rfs->get_ty())
                    throw types::CompilationErr("logical binary operation not supported by different operand types.", loc);
                return std::shared_ptr<types::Bool>(new types::Bool());
            }
            std::shared_ptr<types::TypeHandle> check_identifier(const std::string &ident, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug_e(ident);
                if (local_scope.find(ident) != local_scope.end())
                {
                    der_debug("identifier found");
                    return local_scope.at(ident)->clone();
                }
                else
                {
                    der_debug_e(std::to_string(local_scope.size()));
                    throw types::CompilationErr(std::format("{} shit aint shitting", ident), loc);
                }
            }
            std::shared_ptr<types::TypeHandle> check_subscript(types::Subscript *sub, const SourceLoc &loc)
            {
                auto outer = get_expr_type(std::move(sub->target), loc);
                auto inner = get_expr_type(std::move(sub->inner), loc);
                der_debug_e(types::ty_to_str[outer->get_ty()]);
                if ((outer->get_ty() != types::TYPES::ARRAY) && (outer->get_ty() != types::TYPES::STRING))
                    throw types::CompilationErr("subscript valabe ssdfqksdqkds dure les arrays and strings uwu", loc);
                if (inner->get_ty() != types::TYPES::INTEGER)
                    throw types::CompilationErr("array index only works with ints bruv dude", loc);
                if (outer->get_ty() == types::TYPES::ARRAY)
                    return std::move(dynamic_cast<types::Array *>(outer.get())->ty);
                else
                    return std::make_shared<types::Character>();
            }
            std::shared_ptr<types::TypeHandle> check_struct_instance(types::StructInstance *init, const SourceLoc &loc)
            {
                if (local_scope.find(init->name) == local_scope.end())
                    throw types::CompilationErr(std::format("{} is not defined.", init->name), loc);
                auto parent = dynamic_cast<types::Struct *>(local_scope[init->name].get());
                if (parent->get_ty() != types::TYPES::STRUCT)
                    throw types::CompilationErr(std::format("{} is not a struct.", init->name), loc);
                if (init->inits.size() != parent->members.size())
                    throw types::CompilationErr(std::format("struct {} requires {} members, you supplied {}.", init->name, parent->members.size(), init->inits.size()), loc);
                for (size_t i = 0; i < init->inits.size(); ++i)
                {
                    if (init->inits.at(i).ident != parent->members.at(i).name)
                        throw types::CompilationErr(std::format("member '{}' doesn't exist in struct '{}'", init->inits.at(i).ident, init->name), loc);
                    if (!init->inits.at(i).value->is_same(parent->members.at(i).type.get()))
                        throw types::CompilationErr(std::format("mismatched types in struct {} initialization", init->name), loc);
                }
                return std::make_shared<types::StructInstance>(*init);
            }
            void check_ranged_for(types::RangedFor *ranged_for, const SourceLoc &loc)
            {
                auto left = get_expr_type(std::move(ranged_for->f_start), loc);
                if (left->get_ty() != types::TYPES::INTEGER)
                {
                    throw types::CompilationErr("for loop init must be integers.", loc);
                }
                auto right = get_expr_type(std::move(ranged_for->f_end), loc);
                if (right->get_ty() != types::TYPES::INTEGER)
                {
                    throw types::CompilationErr("for loop init must be integers.", loc);
                }
                local_scope[ranged_for->ident] = std::make_shared<types::Integer>();
                auto old = local_scope;
                for (std::unique_ptr<types::TypeHandle> &e : ranged_for->stmts)
                {
                    der_debug_e(e->debug());
                    get_stmt_type(std::move(e), loc);
                }
                local_scope = old;
            }
            void check_fn(types::Function *fnc, const SourceLoc &loc)
            {
                // der_debug_e(std::to_string(fnc->body.size()));
                // der_debug_e(expr.get());
                std::string t = fnc->name;
                auto ret = fnc->clone_ret();
                std::shared_ptr<types::Function> miata = std::shared_ptr<types::Function>(new types::Function(*fnc));
                local_scope[t] = miata;
                auto old = local_scope;
                is_in_fn = true;

                // apparently having .expr = expr was causing a segault cuz the return type ptr was 0 aka nullptr
                // why tho? idk but somehow it works, took a while to figure out.
                // der_debug_e(expr->debug());
                // apparently go fuck yourself C++

                for (size_t i = 0; i < fnc->args.size(); ++i)
                {
                    der_debug_e(fnc->args.at(i).ident);
                    der_debug_e(fnc->args.at(i).ty->debug());
                    local_scope[fnc->args.at(i).ident] = fnc->args.at(i).ty->clone();
                }

                for (size_t i = 0; i < fnc->body.size(); ++i)
                {
                    der_debug_e(fnc->body.at(i)->debug());
                    get_stmt_type(std::move(fnc->body.at(i)), loc);
                }
                der_debug_e(std::to_string(ret == nullptr));
                if (ret_fn_ty != nullptr && ret != nullptr)
                    if (!ret->is_same(ret_fn_ty.get()))
                        throw types::CompilationErr("ezezeze", loc);
                for (auto &a : fnc->args)
                    local_scope.erase(a.ident);

                ret_fn_ty = nullptr;
                local_scope = old;
            }
            std::shared_ptr<types::TypeHandle> check_fncall(types::Fcall *fcall, const SourceLoc &loc)
            {
                auto old = local_scope;
                auto callee = get_expr_type(std::move(fcall->callee), loc);
                if (callee->get_ty() != types::TYPES::FUNCTION)
                {
                    throw types::CompilationErr(std::format("'{}' machi fonction bach tcalliha hhhhhhhh.", types::ty_to_str[callee->get_ty()]), loc);
                }
                types::Function *fn_callee = dynamic_cast<types::Function *>(callee.get());
                {
                    auto old_scope = local_scope;
                    der_debug(std::format("start fncall to: {}, generics: {}", fn_callee->name, fn_callee->generics.size()));
                    if (fcall->args.size() != fn_callee->args.size())
                    {
                        throw types::CompilationErr(std::format("function call to '{}' arguments don't match, you supplied {} {}, function have {} {}", fn_callee->name, fcall->args.size(), fcall->args.size() > 1 ? "arguments" : "argument", fn_callee->args.size(), fn_callee->args.size() > 1 ? "arguments" : "argument"), loc);
                    }
                    if (fn_callee->generics.size() == 0)
                    {
                        for (size_t i = 0; i < fn_callee->args.size(); ++i)
                        {
                            types::ArgType arg_ty = std::move(fn_callee->args.at(i));
                            auto call_ty = get_expr_type(std::move(fcall->args.at(i)), loc);
                            der_debug(call_ty->debug());
                            der_debug_e(arg_ty.ident);
                            if (arg_ty.ty->get_ty() != call_ty->get_ty())
                            {
                                throw types::CompilationErr(std::format("mismatched argument type, argument '{}' is {}.", arg_ty.ident, arg_ty.ty->debug()), loc);
                            }
                        }
                    }
                    // else
                    // {

                    //     std::string fncname = fn_callee->name;
                    //     std::vector<types::ArgType> newargs = {};
                    //     // as for generics, well I just iterate over the arguments, then find if the callee's argument is a generic then store it in the generics scope
                    //     // and basically see if other arguments use the same generic. push everything into newargs
                    //     for (size_t i = 0; i < fn_callee->args.size(); ++i)
                    //     {
                    //         types::ArgType arg_ty = std::move(fn_callee->args[i]);
                    //         auto call_ty = get_expr_type(std::move(fcall->args[i]), expr, loc);
                    //         der_debug(call_ty->debug());
                    //         der_debug_e(arg_ty.ident);
                    //         if (arg_ty.ty->get_ty() == types::TYPES::IDENT)
                    //         {
                    //             der_debug("it's generic!");
                    //             auto argty = arg_ty.ty->get_ty();
                    //             types::Identifier *g = dynamic_cast<types::Identifier *>(arg_ty.ty.get());
                    //             if (generics_scope.find(g->ident) == generics_scope.end())
                    //             {

                    //                 generics_scope[g->ident] = call_ty->clone();
                    //                 fncname += "_e";
                    //                 newargs.push_back(types::ArgType(arg_ty.ident, loc, call_ty->clone()));
                    //             }
                    //             else if (auto l = generics_scope.at(g->ident); l->get_ty() != call_ty->get_ty())
                    //             {
                    //                 der_debug_e(l->get_ty() == argty);
                    //                 throw types::CompilationErr(std::format("generic '{}' deduced to '{}', but you supplied a '{}'.", g->ident, types::ty_to_str[l->get_ty()], types::ty_to_str[call_ty->get_ty()]), loc);
                    //             }
                    //             else
                    //             {
                    //                 local_scope[arg_ty.ident] = Pair{.ty = generics_scope.at(arg_ty.ident)->clone(), .expr = std::make_shared<ast::Bool>(true)};
                    //             }
                    //         }

                    //         else if (arg_ty.ty->get_ty() != call_ty->get_ty())
                    //         {
                    //             throw types::CompilationErr(std::format("mismatched argument type, argument '{}' is {}.", arg_ty.ident, arg_ty.ty->debug()), loc);
                    //         }
                    //         else
                    //         {

                    //             newargs.push_back(fn_callee->args.at(i));
                    //             local_scope[arg_ty.ident] = Pair{.ty = call_ty->clone(), .expr = std::make_shared<ast::Bool>(true)};
                    //         }
                    //     }
                    //     // auto newfexpr = std::shared_ptr<ast::Function<parser::AstInfo>>(new ast::Function<parser::AstInfo>(fncname, fn_callee_expr->body, newargs, {}, fn_callee_expr->ret_ty->clone()));
                    //     // der_debug("hit generic function.");
                    //     // for (size_t i = 0; i < fn_callee->body.size(); ++i)
                    //     // {
                    //     //     der_debug("typechecking the fnc inner shit.");
                    //     //     der_debug_e(fn_callee->body.at(i)->debug());
                    //     //     get_stmt_type(fn_callee->body.at(i)->clone(), fn_callee_expr->body.at(i).expr->clone(), loc);
                    //     // }
                    //     // local_scope.insert_or_assign(fncname, Pair{.ty = std::shared_ptr<types::Function>(new types::Function(fncname, {}, fn_callee->body, newargs)), .expr = newfexpr});

                    //     local_scope = old_scope;
                    //     der_debug(fncname);
                    // }
                }
                local_scope = old;
                return fn_callee->ret_ty->clone();
            }
            void check_struct(types::Struct *_struct, const SourceLoc &loc)
            {
                if (local_scope.find(_struct->name) != local_scope.end())
                    throw types::CompilationErr(std::format("identifier {} is already defined.", _struct->name), loc);
                local_scope[_struct->name] = _struct->clone();
            }
            void check_enum(types::Enum *_enum, const SourceLoc &loc)
            {
                if (local_scope.find(_enum->name) != local_scope.end())
                    throw types::CompilationErr(std::format("identifier {} is already defined.", _enum->name), loc);
                local_scope[_enum->name] = _enum->clone();
            }
        };
    }
}
#endif