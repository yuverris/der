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
            std::map<std::string, Pair> local_scope = {};
            std::map<std::string, std::shared_ptr<types::TypeHandle>> generics_scope = {};
            std::vector<std::unique_ptr<der::ir::Expr>> m_output{};

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
                    get_stmt_type(x.expr->get_ty(), x.expr->clone(), x.loc);
                    m_advance();
                }
                for (auto &[key, x] : local_scope)
                {
                    der_debug("type checking.....");
                    der_debug(std::format("key: {}, value: {}", key, x.expr->debug()));
                    der_debug_e(x.expr == nullptr);
                    if (x.usable)
                        m_output.push_back(convert_to_ir(x.expr));
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
                        return std::make_unique<der::ir::Variable>(convert_c_type(std::move(var->ty)), var_name, std::move(var_value));
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
                default:
                    return "aaa";
                }
            }

            void get_stmt_type(std::shared_ptr<types::TypeHandle> type, const std::shared_ptr<ast::Expr> &expr, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug_e(type->debug());
                if (type->get_ty() == types::TYPES::VAR)
                {
                    der_debug("recognized VAR type.");
                    check_var(dynamic_cast<types::Variable *>(type.get()), expr, loc);
                }

                else if (type->get_ty() == types::TYPES::IF)
                {
                    der_debug("recognized IF type.");
                    types::If *ifs = dynamic_cast<types::If *>(type.get());
                    for (std::unique_ptr<types::TypeHandle> &t : ifs->body)
                    {
                        der_debug("typechecking the if-then.");
                        der_debug_e(t->debug());
                        get_expr_type(t->clone(), expr, loc);
                    }
                    for (std::unique_ptr<types::TypeHandle> &t : ifs->else_stmt)
                    {
                        der_debug("typechecking the if-else.");
                        der_debug_e(t->debug());
                        get_expr_type(t->clone(), expr, loc);
                    }
                    if (get_expr_type(std::move(ifs->cond), expr, loc)->get_ty() != types::TYPES::BOOL)
                        throw types::CompilationErr("if statement condition must return a boolean.", loc);
                }
                else if (type->get_ty() == types::TYPES::FUNCTION)
                {
                    der_debug("inner fnc def");
                    check_fn(dynamic_cast<types::Function *>(type.get()), expr, loc);
                }
                else if (type->get_ty() == types::TYPES::FCALL)
                {
                    der_debug("aha fcallllll!!!!");
                    // get_expr_type(type, expr, loc);
                    check_fncall(dynamic_cast<types::Fcall *>(type.get()), expr, loc);
                }
                else if (type->get_ty() == types::TYPES::RETURN)
                {
                    return;
                }
            }
            std::shared_ptr<types::TypeHandle> get_expr_type(const std::shared_ptr<types::TypeHandle> &type, const std::shared_ptr<ast::Expr> &expr, const SourceLoc &loc)
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
                else if (type->get_ty() == types::TYPES::BINARY_OP)
                {
                    der_debug("recognized BIN_OP type.");
                    return check_binary(dynamic_cast<types::BinaryOp *>(type.get()), expr, loc);
                }
                else if (type->get_ty() == types::TYPES::LOGICAL_OP)
                {
                    der_debug("recognized LOG_OP type.");
                    return check_logical_binary(dynamic_cast<types::LogicalBinaryOp *>(type.get()), expr, loc);
                }
                else if (type->get_ty() == types::TYPES::IDENT)
                {
                    return check_identifier(dynamic_cast<types::Identifier *>(type.get())->ident, expr, loc);
                }
                else if (type->get_ty() == types::TYPES::FCALL)
                {
                    der_debug("aha fcallllll!!!!");
                    der_debug(std::format("ffff: {}", expr->debug()));
                    return check_fncall(dynamic_cast<types::Fcall *>(type.get()), expr, loc);
                }
                else if (type->get_ty() == types::TYPES::PIPE_OP)
                {
                    types::PipeOp *pipe = dynamic_cast<types::PipeOp *>(type.get());
                    std::shared_ptr<types::TypeHandle> lfs = get_expr_type(std::move(pipe->lfs), expr, loc);
                    std::shared_ptr<types::TypeHandle> rfs = std::move(pipe->rfs); /*-*/
                    if (rfs->get_ty() != types::TYPES::FCALL)
                    {
                        throw types::CompilationErr("right hand of the pipe operator '|>' should be a function call.", loc);
                    }
                    types::Fcall *fnc = dynamic_cast<types::Fcall *>(rfs.get());
                    fnc->args.push_back(std::unique_ptr<types::TypeHandle>(lfs.get()));
                    return check_fncall(fnc, expr, loc);
                }
                else
                {
                    return std::unique_ptr<types::TypeHandle>(new types::Dummy());
                }
            }
            void check_var(types::Variable *var, const std::shared_ptr<ast::Expr> &expr, const SourceLoc &loc)
            {
                der_debug("start");
                std::shared_ptr<types::TypeHandle> expected = std::move(var->expected_ty);
                der_debug_e(expected->debug());
                der_debug_e(var->actual_ty->debug());
                std::shared_ptr<types::TypeHandle> actual = get_expr_type(std::move(var->actual_ty), expr, loc);

                // if (actual->get_ty() == types::TYPES::FCALL)
                // {
                //     types::Fcall* fc = dynamic_cast<types::Fcall*>(actual.get());
                //     actual = fc->
                // }
                if (expected->is_same(actual.get()))
                    local_scope[var->name] = Pair{.ty = std::move(expected), .expr = expr};
                else
                    throw types::CompilationErr(std::format("inconsistent variable type. var is {}, value is {}", expected->debug(), actual->debug()), loc);
            }
            // REMINDER NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS
            // NO FOKING IMPLICIT CONVERSIONS NO FOKING IMPLICIT CONVERSIONS NO FOKING IMPLICIT CONVERSIONS
            // NO FOKING IMPLICIT CONVERSIONS NO FOKING IMPLICIT CONVERSIONS NO FOKING IMPLICIT CONVERSIONS
            std::shared_ptr<types::TypeHandle> check_binary(types::BinaryOp *bin, const std::shared_ptr<ast::Expr> &expr, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug("lfs check");
                auto lfs = get_expr_type(std::move(bin->lfs), expr, loc);
                der_debug("rfs check");
                auto rfs = get_expr_type(std::move(bin->rfs), expr, loc);
                if (lfs->get_ty() != rfs->get_ty())
                    throw types::CompilationErr("binary operation not supported by different operand types.", loc);
                return std::shared_ptr<types::TypeHandle>(bin);
            }
            std::shared_ptr<types::TypeHandle> check_logical_binary(types::LogicalBinaryOp *bin, const std::shared_ptr<ast::Expr> &expr, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug("lfs check");
                auto lfs = get_expr_type(std::move(bin->lfs), expr, loc);
                der_debug("rfs check");
                auto rfs = get_expr_type(std::move(bin->rfs), expr, loc);
                if (lfs->get_ty() != rfs->get_ty())
                    throw types::CompilationErr("logical binary operation not supported by different operand types.", loc);
                return std::unique_ptr<types::Bool>(new types::Bool());
            }
            std::shared_ptr<types::TypeHandle> check_identifier(const std::string &ident, const std::shared_ptr<ast::Expr> &expr, const SourceLoc &loc)
            {
                der_debug("start");
                der_debug_e(ident);
                if (local_scope.find(ident) != local_scope.end())
                {
                    der_debug("identifier found");
                    return local_scope.at(ident).ty->clone();
                }
                else
                {
                    der_debug_e(std::to_string(local_scope.size()));
                    throw types::CompilationErr(std::format("{} shit aint shitting", ident), loc);
                }
            }
            void check_fn(types::Function *fnc, const std::shared_ptr<ast::Expr> &expr, const SourceLoc &loc)
            {
                der_debug("hhhh");
                der_debug_e(std::to_string(fnc->body.size()));
                der_debug(std::format("fn name: {}, has generics: {}", fnc->name, fnc->generics.size()));
                // apparently having .expr = expr was causing a segault cuz the return type ptr was 0 aka nullptr
                // why tho? idk but somehow it works, took a while to figure out.
                der_debug_e(expr->debug());
                ast::Function<parser::AstInfo> *fn = dynamic_cast<ast::Function<parser::AstInfo> *>(expr.get());
                local_scope[fnc->name] = Pair{.ty = std::make_shared<types::Function>(fnc->name, fnc->generics, fnc->body, fnc->args), .expr = std::shared_ptr<ast::Function<parser::AstInfo>>(fn), .usable = fnc->generics.size() == 0};
                if(fnc->generics.size() == 0){
                    for (auto &arg : fnc->args)
                    {
                        der_debug_e(arg.ident);
                        local_scope[arg.ident] = Pair{.ty = std::move(arg.ty), .expr = expr};
                    }

                    for (size_t i = 0; i < fn->body.size(); ++i)
                    {
                        der_debug_e(fn->body.at(i).expr->debug());
                        get_stmt_type(std::move(fnc->body.at(i)), fn->body.at(i).expr->clone(), loc);
                    }
                    for (auto &arg : fnc->args)
                    {
                        der_debug_e(arg.ident);
                        local_scope.erase(arg.ident);
                    }
                }
            }
            std::shared_ptr<types::TypeHandle> check_fncall(types::Fcall *fcall, const std::shared_ptr<ast::Expr> &expr, const SourceLoc &loc)
            {
                ast::Function<parser::AstInfo> *fn_callee_expr = dynamic_cast<ast::Function<parser::AstInfo> *>(expr.get());
                auto callee = get_expr_type(std::move(fcall->callee), std::shared_ptr<ast::Function<parser::AstInfo>>(fn_callee_expr), loc);
                if (callee->get_ty() != types::TYPES::FUNCTION)
                {
                    throw types::CompilationErr(std::format("'{}' machi fonction bach tcalliha hhhhhhhh.", types::ty_to_str[callee->get_ty()]), loc);
                }
                else
                {
                    types::Function *fn_callee = dynamic_cast<types::Function *>(callee.get());
                    der_debug_e(fn_callee_expr->debug());
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
                            types::ArgType arg_ty = std::move(fn_callee->args[i]);
                            auto call_ty = get_expr_type(std::move(fcall->args[i]), expr, loc);
                            der_debug(call_ty->debug());
                            der_debug_e(arg_ty.ident);
                            if (arg_ty.ty->get_ty() != call_ty->get_ty())
                            {
                                throw types::CompilationErr(std::format("mismatched argument type, argument '{}' is {}.", arg_ty.ident, arg_ty.ty->debug()), loc);
                            }
                            else
                            {
                                // newargs.push_back(fn_callee->args[i]);
                                local_scope[arg_ty.ident] = Pair{.ty = call_ty->clone()};
                            }
                        }
                    }
                    else
                    {

                        std::string fncname = fn_callee->name;
                        std::vector<types::ArgType> newargs = {};
                        // as for generics, well I just iterate over the arguments, then find if the callee's argument is a generic then store it in the generics scope
                        // and basically see if other arguments use the same generic. push everything into newargs
                        for (size_t i = 0; i < fn_callee->args.size(); ++i)
                        {
                            types::ArgType arg_ty = std::move(fn_callee->args[i]);
                            auto call_ty = get_expr_type(std::move(fcall->args[i]), expr, loc);
                            der_debug(call_ty->debug());
                            der_debug_e(arg_ty.ident);
                            if (arg_ty.ty->get_ty() == types::TYPES::IDENT)
                            {
                                der_debug("it's generic!");
                                auto argty = arg_ty.ty->get_ty();
                                types::Identifier *g = dynamic_cast<types::Identifier *>(arg_ty.ty.get());
                                if (generics_scope.find(g->ident) == generics_scope.end())
                                {

                                    generics_scope[g->ident] = call_ty->clone();
                                    fncname += "_e";
                                    newargs.push_back(types::ArgType(arg_ty.ident, loc, call_ty->clone()));
                                }
                                else if (auto l = generics_scope.at(g->ident); l->get_ty() != call_ty->get_ty())
                                {
                                    der_debug_e(l->get_ty() == argty);
                                    throw types::CompilationErr(std::format("generic '{}' deduced to '{}', but you supplied a '{}'.", g->ident, types::ty_to_str[l->get_ty()], types::ty_to_str[call_ty->get_ty()]), loc);
                                }
                                else
                                {
                                    local_scope[arg_ty.ident] = Pair{.ty = generics_scope.at(arg_ty.ident)->clone(), .expr = std::make_shared<ast::Bool>(true)};
                                }
                            }

                            else if (arg_ty.ty->get_ty() != call_ty->get_ty())
                            {
                                throw types::CompilationErr(std::format("mismatched argument type, argument '{}' is {}.", arg_ty.ident, arg_ty.ty->debug()), loc);
                            }
                            else
                            {

                                newargs.push_back(fn_callee->args.at(i));
                                local_scope[arg_ty.ident] = Pair{.ty = call_ty->clone(), .expr = std::make_shared<ast::Bool>(true)};
                            }
                        }
                        auto newfexpr = std::shared_ptr<ast::Function<parser::AstInfo>>(new ast::Function<parser::AstInfo>(fncname, fn_callee_expr->body, newargs, {}, fn_callee_expr->ret_ty->clone()));
                        der_debug("hit generic function.");
                        for (size_t i = 0; i < fn_callee->body.size(); ++i)
                        {
                            der_debug("typechecking the fnc inner shit.");
                            der_debug_e(fn_callee->body.at(i)->debug());
                            get_stmt_type(fn_callee->body.at(i)->clone(), fn_callee_expr->body.at(i).expr->clone(), loc);
                        }
                        local_scope = old_scope;
                        local_scope.insert_or_assign(fncname, Pair{.ty = std::shared_ptr<types::Function>(new types::Function(fncname, {}, fn_callee->body, newargs)), .expr = newfexpr});

                        der_debug(fncname);
                    }
                }
                return std::move(dynamic_cast<ast::Function<parser::AstInfo> *>(expr.get())->ret_ty);
            }
        };
    }
}
#endif