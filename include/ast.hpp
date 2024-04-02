#ifndef DER_AST
#define DER_AST
#include <string>
#include <memory>
#include <optional>
#include <format>
#include "lexer.hpp"
#include "types.hpp"
#include "source_loc.hpp"
#include "debug.hpp"
namespace der
{
    namespace ast
    {

        template <class T>
        using ptr = std::unique_ptr<T>;
        struct Expr
        {
            virtual std::string debug() const = 0;
            virtual ptr<Expr> clone() const = 0;
            virtual std::unique_ptr<types::TypeHandle> get_ty() const = 0;
            virtual ~Expr() = default;
        };

        struct BinaryOper : Expr
        {
            ptr<Expr> left;
            lexer::TOKENS op;
            ptr<Expr> right;

            BinaryOper(ptr<Expr> l, lexer::TOKENS op, ptr<Expr> r) : left(std::move(l)), op(op), right(std::move(r)) {}

            BinaryOper(const BinaryOper &bin) : left(bin.left->clone()), op(bin.op), right(bin.right->clone()) {}

            std::string debug() const override
            {
                return std::format("[Binary Op: {} {} {}]", left->debug(), lexer::tokens_to_str.at(op), right->debug());
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<BinaryOper>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::BinaryOp(left->get_ty(), right->get_ty()));
            }
        };

        struct DotOper : Expr
        {
            ptr<Expr> left;
            ptr<Expr> right;

            DotOper(ptr<Expr> l, ptr<Expr> r) : left(std::move(l)), right(std::move(r)) {}

            DotOper(const DotOper &bin) : left(bin.left->clone()), right(bin.right->clone()) {}

            std::string debug() const override
            {
                return std::format("[Dot Op: {} {}]", left->debug(), right->debug());
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<DotOper>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::DotOp(left->get_ty(), right->get_ty()));
            }
        };

        struct PipeOper : Expr
        {
            ptr<Expr> left;
            ptr<Expr> right;

            PipeOper(ptr<Expr> l, ptr<Expr> r) : left(std::move(l)), right(std::move(r)) {}

            PipeOper(const PipeOper &bin) : left(bin.left->clone()), right(bin.right->clone()) {}

            std::string debug() const override
            {
                return std::format("[Pipe Op: {} {}]", left->debug(), right->debug());
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<PipeOper>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::PipeOp(left->get_ty(), right->get_ty()));
            }
        };

        struct LogicalBinaryOper : Expr
        {
            ptr<Expr> left;
            lexer::TOKENS op;
            ptr<Expr> right;

            LogicalBinaryOper(ptr<Expr> l, lexer::TOKENS op, ptr<Expr> r) : left(std::move(l)), op(op), right(std::move(r)) {}

            LogicalBinaryOper(const LogicalBinaryOper &bin) : left(bin.left->clone()), op(bin.op), right(bin.right->clone()) {}

            std::string debug() const override
            {
                return std::format("[Logical Binary Op: {} {} {}]", left->debug(), lexer::tokens_to_str.at(op), right->debug());
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<LogicalBinaryOper>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::LogicalBinaryOp(left->get_ty(), right->get_ty()));
            }
        };

        struct UnaryOper : Expr
        {
            std::string op;
            ptr<Expr> victim;

            UnaryOper(const UnaryOper &un) : op(un.op), victim(un.victim->clone()) {}

            std::string debug() const override
            {
                return std::format("{}{}", op, victim->debug());
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<UnaryOper>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::UnaryOp(victim->get_ty()));
            }
        };

        struct Bool : Expr
        {
            bool value;

            Bool(bool value) : value(value) {}

            std::string debug() const override
            {
                return std::to_string(value);
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<Bool>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::Bool());
            }
        };

        struct Integer : Expr
        {
            long long int value;
            Integer(long long int v) : value(v) {}
            Integer(const std::string &str) : value(std::stoi(str)) {}
            std::string debug() const override
            {
                return std::to_string(value);
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<Integer>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::Integer());
            }
        };

        struct String : Expr
        {
            std::string value;

            String(const std::string &v) : value(v) {}
            std::string debug() const override
            {
                return value;
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<String>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::String());
            }
        };

        struct Identifier : Expr
        {
            std::string ident;

            Identifier(const std::string &str) : ident(str) {}

            std::string debug() const override
            {
                return std::format("ident: {}", ident);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::Identifier(ident));
            }
            ptr<Expr> clone() const override
            {
                return std::make_unique<Identifier>(*this);
            }
        };

        struct Variable : Expr
        {
            std::string name;
            std::unique_ptr<types::TypeHandle> ty;
            ptr<Expr> value;

            Variable(const std::string &name, ptr<Expr> value, ptr<types::TypeHandle> ty) : name(name), ty(std::move(ty)), value(std::move(value)) {}

            Variable(const Variable &var) : name(var.name), ty(var.ty->clone()), value(var.value->clone()) {}

            std::string debug() const override
            {
                return std::format("[Variable: {}: {} = {}]", name, ty->debug(), value->debug());
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::Variable(name, ty->clone(), value->get_ty()));
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<Variable>(*this);
            }
        };

        struct FunctionCall : Expr
        {
            ptr<Expr> callee;
            std::vector<ptr<Expr>> args;

            FunctionCall(ptr<Expr> callee, const std::vector<ptr<Expr>> &_args) : callee(std::move(callee))
            {
                for (auto &&a : _args)
                {
                    args.emplace_back(a->clone());
                }
            }

            FunctionCall(const FunctionCall &fc) : callee(fc.callee->clone())
            {
                for (auto &a : fc.args)
                {
                    args.emplace_back(a->clone());
                }
            }

            std::string debug() const override
            {
                std::string args_str;
                for (auto &&a : args)
                    args_str += a->debug();
                return "functioncall: " + callee->debug() + " argscount: " + std::to_string(args.size()) + " args: " + args_str;
            }
            ptr<Expr> clone() const override
            {
                return std::make_unique<FunctionCall>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                std::vector<std::unique_ptr<types::TypeHandle>> __args{};
                for (auto &a : args)
                    __args.push_back(a->get_ty());
                return std::unique_ptr<types::TypeHandle>(new types::Fcall(callee->get_ty(), __args));
            }
        };

        template <class T>
        struct IfStmt : Expr
        {
            ptr<Expr> cond;
            std::vector<T> body;
            std::vector<T> else_block;

            IfStmt(ptr<Expr> cond, std::vector<T> &&body, std::vector<T> &&else_block) : cond(std::move(cond)), body(body), else_block(else_block)
            {
            }

            IfStmt(const IfStmt &other) : cond(other.cond->clone())
            {
                for (auto &&a : other.body)
                    body.push_back(a);
                for (auto &&a : other.else_block)
                    else_block.push_back(a);
            }

            std::string debug() const override
            {
                std::string out = "ila(";
                out += "cond: " + cond->debug() + ") {";
                for (const auto &it : body)
                {
                    out += "\n\t";
                    out += it.expr->debug();
                }
                out += "\n}";
                if (else_block.size() > 0)
                {
                    out += " awla {";
                    for (const auto &it : else_block)
                    {
                        out += "\n\t";
                        out += it.expr->debug();
                    }
                    out += "\n}";
                }
                return out;
            }
            ptr<Expr> clone() const override
            {
                return std::make_unique<IfStmt>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                std::vector<std::unique_ptr<types::TypeHandle>> bodybody{};
                std::vector<std::unique_ptr<types::TypeHandle>> elselse{};
                for (auto &&a : body)
                {
                    bodybody.push_back(std::move(a.expr->get_ty()));
                }
                for (auto &&a : else_block)
                {
                    elselse.push_back(std::move(a.expr->get_ty()));
                }
                return std::unique_ptr<types::TypeHandle>(new types::If(cond->get_ty(), std::move(bodybody), std::move(elselse)));
            };
        };

        struct SmolIfStmt : Expr
        {
            std::unique_ptr<Expr> cond;
            std::unique_ptr<Expr> expr;

            SmolIfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Expr> expr) : cond(std::move(cond)), expr(std::move(expr)) {}
            SmolIfStmt(const SmolIfStmt &other) : cond(other.cond->clone()), expr(other.expr->clone()) {}

            ptr<Expr> clone() const override
            {
                return std::make_unique<SmolIfStmt>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::make_unique<types::SmolIf>(cond->get_ty(), expr->get_ty());
            }

            std::string debug() const override
            {
                return std::format("[SmolIfStmt {} {}]", cond->debug(), expr->debug());
            }
        };

        struct Array : Expr
        {
            std::vector<std::unique_ptr<Expr>> values;

            Array(std::vector<std::unique_ptr<Expr>> &&values)
            {
                for (auto &a : values)
                    this->values.push_back(std::move(a));
            }

            Array(const Array &other)
            {
                for (auto &a : other.values)
                    values.push_back(a->clone());
            }

            std::string debug() const override
            {
                return std::format("[Array len {}]", values.size());
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<Array>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
    
                return std::make_unique<types::Array>(values.at(0)->get_ty(), values.size());
            }
        };

        struct StructMember {
            std::string name;
            std::unique_ptr<types::TypeHandle> type;
        };

        struct Struct: Expr {
            std::vector<StructMember> members;
            std::string debug() const override
            {
                return std::format("[Array len {}]", members.size());
            }

            ptr<Expr> clone() const override
            {
                return std::make_unique<Array>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
    
                return std::make_unique<types::Struct>();
            }
        };

        template <class T>
        struct Function : Expr
        {
            std::string name;
            std::vector<T> body;
            std::vector<types::ArgType> args;
            std::vector<std::string> generics;
            std::unique_ptr<types::TypeHandle> ret_ty;

            Function(const std::string &name, const std::vector<T> &body, const std::vector<types::ArgType> &args, const std::vector<std::string> &generics, std::unique_ptr<types::TypeHandle> ret_ty) : name(name), body(body), generics(generics), ret_ty(std::move(ret_ty))
            {
                for (auto &&arg : args)
                {
                    this->args.push_back(arg);
                }
            }

            Function(const Function &other) : name(other.name), args(other.args), generics(other.generics), ret_ty(other.ret_ty->clone())
            {
                for (auto &&a : other.body)
                    body.push_back(a);
            }

            std::string debug() const override
            {
                std::string out = "dalaton " + name + "(";
                for (const types::ArgType &a : args)
                {
                    out += std::format("[{}:{}] ", a.ident, a.ty->debug());
                }
                out += ") {";
                for (const auto &it : body)
                {
                    out += "\n\t";
                    out += it.expr->debug();
                }
                out += "\n}";
                return out;
            }
            ptr<Expr> clone() const override
            {
                return std::make_unique<Function>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                std::vector<types::ArgType> __args{};
                std::vector<std::unique_ptr<types::TypeHandle>> __body{};
                std::vector<types::Generic> __generics{};

                for (auto &arg : args)
                    __args.emplace_back(types::ArgType(arg.ident, arg.loc, arg.ty->clone()));

                for (auto &e : body)
                    __body.push_back(e.expr->get_ty());

                for (auto &e : generics)
                    __generics.push_back(types::Generic(e));
                der_debug_e(std::to_string(body.size()));
                return std::unique_ptr<types::TypeHandle>(new types::Function(name, __generics, __body, __args));
            }
        };

        // struct Nada : Expr
        // {
        //     std::string debug() const override
        //     {
        //         return "nada";
        //     }

        //     ptr<Expr> clone() const override
        //     {
        //         return std::make_unique<Nada>(*this);
        //     }

        //     std::unique_ptr<types::TypeHandle> get_ty() const override
        //     {
        //         return std::unique_ptr<types::TypeHandle>(new types::Nada());
        //     }
        // };

        struct Return : Expr
        {
            std::unique_ptr<Expr> ret_expr;
            Return(std::unique_ptr<Expr> r) : ret_expr(std::move(r)) {}
            Return(const Return &other) : ret_expr(other.ret_expr->clone()) {}
            std::string debug() const override
            {
                return std::format("[Return {}]", ret_expr->debug());
            }
            ptr<Expr> clone() const override
            {
                return std::make_unique<Return>(*this);
            }

            std::unique_ptr<types::TypeHandle> get_ty() const override
            {
                return std::unique_ptr<types::TypeHandle>(new types::Return(ret_expr->get_ty()));
            }
        };
    }
}
#endif
