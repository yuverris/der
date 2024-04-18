#ifndef DER_TYPES_HH
#define DER_TYPES_HH
#include <string>
#include <memory>
#include <map>
#include "source_loc.hpp"
namespace der
{
    namespace types
    {

        enum class TYPES
        {
            INTEGER,
            DOUBLE,
            STRING,
            CHAR,
            BOOL,
            FUNCTION,
            FCALL,
            BINARY_OP,
            UNARY_OP,
            IDENT,
            VAR,
            ARRAY_VAR,
            NADA,
            GENERIC,
            LOGICAL_OP,
            IF,
            VOID,
            RETURN,
            DOT_OP,
            PIPE_OP,
            SMOL_IF,
            ARRAY,
            TEMPLATE_PARAM,
            STRUCT,
            ENUM,
            SET_OP,
            RANGED_FOR,
            POINTER,
            SUBSCRIPT,
            STRUCT_INSTANCE,
            ENUM_INSTANCE,
            POINTER_DEREF,
            GET_ADDRESS,
            CAST,
            DUMMY
        };

        std::map<TYPES, std::string> ty_to_str{
            {TYPES::BOOL, "bool"},
            {TYPES::INTEGER, "ra9m"},
            {TYPES::STRING, "ktba"},
            {TYPES::VOID, "walo"},
            {TYPES::CHAR, "harf"},

        };

        struct TypeHandle
        {
            virtual std::string debug() const = 0;
            virtual TYPES get_ty() const = 0;
            virtual std::unique_ptr<TypeHandle> clone() const = 0;

            virtual bool is_same(TypeHandle *) const = 0;

            virtual ~TypeHandle() = default;
        };

        struct Generic : TypeHandle
        {
            std::string name;
            Generic(const std::string &s) : name(s) {}
            Generic(const Generic &gn) : name(gn.name) {}
            TYPES get_ty() const override
            {
                return TYPES::GENERIC;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Generic>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Generic";
            }

            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct ArgType
        {
            std::string ident;
            SourceLoc loc;
            std::unique_ptr<TypeHandle> ty;

            ArgType(const std::string &ident, SourceLoc loc, std::unique_ptr<TypeHandle> ty) : ident(ident), loc(loc), ty(std::move(ty)) {}
            ArgType(const ArgType &other) : ident(other.ident), loc(other.loc), ty(other.ty->clone()) {}
        };
        struct CompilationErr
        {
            std::string msg;
            SourceLoc loc;
            CompilationErr(const std::string &m, const SourceLoc &loc) : msg(m), loc(loc) {}
        };

        struct Integer : TypeHandle
        {
            TYPES get_ty() const override
            {
                return TYPES::INTEGER;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::unique_ptr<Integer>(new Integer());
            }
            std::string debug() const override
            {
                return "Ty.Integer";
            }
            bool is_same(TypeHandle *other) const override
            {
                return other->get_ty() == TYPES::INTEGER;
            }
        };

        struct String : TypeHandle
        {
            TYPES get_ty() const override
            {
                return TYPES::STRING;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::unique_ptr<String>(new String());
            }
            std::string debug() const override
            {
                return "Ty.String";
            }
            bool is_same(TypeHandle *other) const override
            {
                return other->get_ty() == TYPES::STRING;
            }
        };

        struct Bool : TypeHandle
        {
            TYPES get_ty() const override
            {
                return TYPES::BOOL;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Bool>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Bool";
            }
            bool is_same(TypeHandle *other) const override
            {
                return other->get_ty() == TYPES::BOOL;
            }
        };

        struct Pointer : TypeHandle
        {
            std::unique_ptr<TypeHandle> victim;
            Pointer(std::unique_ptr<TypeHandle> vic) : victim(std::move(vic)) {}
            Pointer(const Pointer &ptr) : victim(ptr.victim->clone()) {}
            TYPES get_ty() const override
            {
                return TYPES::POINTER;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Pointer>(*this);
            }
            std::string debug() const override
            {
                return std::format("Ty.Pointer<{}>", victim->debug());
            }
            bool is_same(TypeHandle *other) const override
            {
                if(other->get_ty() == TYPES::POINTER)
                    return victim->is_same(dynamic_cast<Pointer*>(other)->victim.get());
                return false;
            }
        };
        struct PointerDeref : TypeHandle
        {
            std::unique_ptr<TypeHandle> victim;
            PointerDeref(std::unique_ptr<TypeHandle> vic) : victim(std::move(vic)) {}
            PointerDeref(const PointerDeref &ptr) : victim(ptr.victim->clone()) {}
            TYPES get_ty() const override
            {
                return TYPES::POINTER_DEREF;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<PointerDeref>(*this);
            }
            std::string debug() const override
            {
                return "Ty.PointerDeref";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };
        struct Cast: TypeHandle {
            std::unique_ptr<types::TypeHandle> to_ty;
            std::unique_ptr<types::TypeHandle> victim;
            Cast(std::unique_ptr<types::TypeHandle> to_ty, std::unique_ptr<types::TypeHandle> victim): to_ty(std::move(to_ty)), victim(std::move(victim)) {}
            Cast(const Cast& other): to_ty(other.to_ty->clone()), victim(other.victim->clone()) {}

            std::string debug() const override {
                return std::format("Ty.Cast<to: {}, from: {}>", to_ty->debug(), victim->debug());
            }
            std::unique_ptr<TypeHandle> clone() const override {
                return std::make_unique<Cast>(*this);
            }
            bool is_same(TypeHandle *) const override {
                return false;
            }

            types::TYPES get_ty() const override {
                return types::TYPES::CAST;
            }
        };
        struct GetAddress : TypeHandle
        {
            std::unique_ptr<TypeHandle> victim;
            GetAddress(std::unique_ptr<TypeHandle> vic) : victim(std::move(vic)) {}
            GetAddress(const GetAddress &ptr) : victim(ptr.victim->clone()) {}
            TYPES get_ty() const override
            {
                return TYPES::GET_ADDRESS;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<GetAddress>(*this);
            }
            std::string debug() const override
            {
                return "Ty.GetAddress";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };
        struct BinaryOp : TypeHandle
        {
            std::unique_ptr<TypeHandle> lfs;
            std::unique_ptr<TypeHandle> rfs;

            BinaryOp(std::unique_ptr<TypeHandle> lfs, std::unique_ptr<TypeHandle> rfs) : lfs(std::move(lfs)), rfs(std::move(rfs)) {}

            BinaryOp(const BinaryOp &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()) {}

            TYPES get_ty() const override
            {
                return TYPES::BINARY_OP;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<BinaryOp>(*this);
            }
            std::string debug() const override
            {
                return "Ty.BinOp";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct DotOp : TypeHandle
        {
            std::unique_ptr<TypeHandle> lfs;
            std::unique_ptr<TypeHandle> rfs;

            DotOp(std::unique_ptr<TypeHandle> lfs, std::unique_ptr<TypeHandle> rfs) : lfs(std::move(lfs)), rfs(std::move(rfs)) {}

            DotOp(const DotOp &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()) {}

            TYPES get_ty() const override
            {
                return TYPES::DOT_OP;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<DotOp>(*this);
            }
            std::string debug() const override
            {
                return "Ty.DotOp";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct PipeOp : TypeHandle
        {
            std::unique_ptr<TypeHandle> lfs;
            std::unique_ptr<TypeHandle> rfs;

            PipeOp(std::unique_ptr<TypeHandle> lfs, std::unique_ptr<TypeHandle> rfs) : lfs(std::move(lfs)), rfs(std::move(rfs)) {}

            PipeOp(const PipeOp &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()) {}

            TYPES get_ty() const override
            {
                return TYPES::PIPE_OP;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<PipeOp>(*this);
            }
            std::string debug() const override
            {
                return "Ty.PipeOp";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct SmolIf : TypeHandle
        {
            std::unique_ptr<TypeHandle> lfs;
            std::unique_ptr<TypeHandle> rfs;

            SmolIf(std::unique_ptr<TypeHandle> lfs, std::unique_ptr<TypeHandle> rfs) : lfs(std::move(lfs)), rfs(std::move(rfs)) {}

            SmolIf(const SmolIf &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()) {}

            TYPES get_ty() const override
            {
                return TYPES::SMOL_IF;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<SmolIf>(*this);
            }
            std::string debug() const override
            {
                return "Ty.SmolIf";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct LogicalBinaryOp : TypeHandle
        {
            std::unique_ptr<TypeHandle> lfs;
            std::unique_ptr<TypeHandle> rfs;

            LogicalBinaryOp(std::unique_ptr<TypeHandle> lfs, std::unique_ptr<TypeHandle> rfs) : lfs(std::move(lfs)), rfs(std::move(rfs)) {}

            LogicalBinaryOp(const LogicalBinaryOp &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()) {}

            TYPES get_ty() const override
            {
                return TYPES::LOGICAL_OP;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<LogicalBinaryOp>(*this);
            }
            std::string debug() const override
            {
                return "Ty.LogBinOp";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct Character : TypeHandle
        {

            TYPES get_ty() const override
            {
                return TYPES::CHAR;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Character>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Char";
            }
            bool is_same(TypeHandle *other) const override
            {
                return other->get_ty() == TYPES::CHAR;
            }
        };

        struct RangedFor : TypeHandle
        {
            std::unique_ptr<TypeHandle> f_start;
            std::unique_ptr<TypeHandle> f_end;
            std::vector<std::unique_ptr<TypeHandle>> stmts;
            std::string ident;

            RangedFor(const std::string &ident, std::unique_ptr<TypeHandle> f1, std::unique_ptr<TypeHandle> f2, const std::vector<std::unique_ptr<TypeHandle>> &stmt) : ident(ident), f_start(std::move(f1)), f_end(std::move(f2))
            {
                for (auto &e : stmt)
                {
                    stmts.push_back(e->clone());
                }
            }
            RangedFor(const RangedFor &other) : ident(other.ident), f_start(other.f_start->clone()), f_end(other.f_end->clone())
            {
                for (auto &e : other.stmts)
                {
                    stmts.push_back(e->clone());
                }
            }

            TYPES get_ty() const override
            {
                return TYPES::RANGED_FOR;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<RangedFor>(*this);
            }

            std::string debug() const override
            {
                return "Ty.RangedFor";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct Subscript : TypeHandle
        {
            std::unique_ptr<TypeHandle> target;
            std::unique_ptr<TypeHandle> inner;

            Subscript(std::unique_ptr<TypeHandle> t, std::unique_ptr<TypeHandle> i) : target(std::move(t)), inner(std::move(i)) {}
            Subscript(const Subscript &other) : target(other.target->clone()), inner(other.inner->clone()) {}

            TYPES get_ty() const override
            {
                return TYPES::SUBSCRIPT;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Subscript>(*this);
            }

            std::string debug() const override
            {
                return "Ty.Subscript";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct SetOp : TypeHandle
        {
            std::unique_ptr<TypeHandle> lfs;
            std::unique_ptr<TypeHandle> rfs;

            SetOp(std::unique_ptr<TypeHandle> lfs, std::unique_ptr<TypeHandle> rfs) : lfs(std::move(lfs)), rfs(std::move(rfs)) {}

            SetOp(const SetOp &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()) {}

            TYPES get_ty() const override
            {
                return TYPES::SET_OP;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<SetOp>(*this);
            }
            std::string debug() const override
            {
                return "Ty.SetOp";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct Array : TypeHandle
        {
            std::unique_ptr<TypeHandle> ty;
            size_t size;

            Array(std::unique_ptr<TypeHandle> ty, size_t size) : ty(std::move(ty)), size(size) {}

            Array(const Array &other) : ty(other.ty->clone()), size(other.size) {}

            TYPES get_ty() const override
            {
                return TYPES::ARRAY;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Array>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Array";
            }
            bool is_same(TypeHandle *other) const override
            {
                std::cout << "ababababababa\n";
                Array *x = dynamic_cast<Array *>(other);
                std::cout << "bbbbbbbbbbbbbbbb\n";
                return (ty->get_ty() == x->ty->get_ty()) && (size == x->size);
            }
        };

        struct TemplateParam : TypeHandle
        {
            std::string ident;
            std::vector<std::unique_ptr<TypeHandle>> elements;

            TemplateParam(const std::string &ident, const std::vector<std::unique_ptr<TypeHandle>> &el) : ident(ident)
            {
                for (auto &a : el)
                    elements.push_back(a->clone());
            }
            TemplateParam(const TemplateParam &other) : ident(other.ident)
            {
                for (auto &a : other.elements)
                    elements.push_back(a->clone());
            }

            TYPES get_ty() const override
            {
                return TYPES::TEMPLATE_PARAM;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<TemplateParam>(*this);
            }
            std::string debug() const override
            {
                return "Ty.TemplateParam";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct UnaryOp : TypeHandle
        {
            std::unique_ptr<TypeHandle> victim;

            UnaryOp(std::unique_ptr<TypeHandle> victim) : victim(std::move(victim)) {}

            UnaryOp(const UnaryOp &other) : victim(other.victim->clone()) {}

            TYPES get_ty() const override
            {
                return TYPES::UNARY_OP;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<UnaryOp>(*this);
            }
            std::string debug() const override
            {
                return "Ty.UnOp";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct Function : TypeHandle
        {
            std::string name;
            std::vector<Generic> generics;
            std::vector<ArgType> args;
            std::vector<std::unique_ptr<TypeHandle>> body;
            std::unique_ptr<TypeHandle> ret_ty;

            Function(const std::string &name, const std::vector<Generic> &generics, const std::vector<std::unique_ptr<TypeHandle>> &body, const std::vector<ArgType> &args, std::unique_ptr<TypeHandle> ret_ty) : name(name), generics(generics), ret_ty(std::move(ret_ty))
            {
                for (auto &k : args)
                    this->args.push_back(k);
                for (auto &k : body)
                    this->body.push_back(k->clone());
            }

            Function(const Function &fn) : name(fn.name), generics(fn.generics), args(fn.args), ret_ty(fn.ret_ty->clone())
            {
                for (auto &k : fn.body)
                    this->body.push_back(k->clone());
            }

            std::unique_ptr<TypeHandle> clone_ret()
            {
                return ret_ty->clone();
            }

            TYPES get_ty() const override
            {
                return TYPES::FUNCTION;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Function>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Function";
            }
            bool is_same(TypeHandle *other) const override
            {
                return other->get_ty() == TYPES::FUNCTION;
            }
        };

        struct Fcall : TypeHandle
        {
            std::unique_ptr<TypeHandle> callee;
            std::vector<std::unique_ptr<TypeHandle>> args;

            Fcall(std::unique_ptr<TypeHandle> callee, const std::vector<std::unique_ptr<TypeHandle>> &args) : callee(std::move(callee))
            {
                for (auto &a : args)
                    this->args.push_back(a->clone());
            }
            Fcall(const Fcall &other) : callee(other.callee->clone())
            {
                for (auto &a : other.args)
                    this->args.push_back(a->clone());
            }

            TYPES get_ty() const override
            {
                return TYPES::FCALL;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Fcall>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Fcall";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct Variable : TypeHandle
        {
            std::string name;
            std::unique_ptr<TypeHandle> expected_ty;
            std::unique_ptr<TypeHandle> actual_ty;
            bool is_const;

            Variable(const std::string &name, std::unique_ptr<TypeHandle> expected_ty, std::unique_ptr<TypeHandle> actual_ty, bool is_const = false) : name(name), expected_ty(std::move(expected_ty)), actual_ty(std::move(actual_ty)), is_const(is_const) {}

            Variable(const Variable &other) : name(other.name), expected_ty(other.expected_ty->clone()), actual_ty(other.actual_ty->clone()), is_const(other.is_const) {}

            TYPES get_ty() const override
            {
                return TYPES::VAR;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Variable>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Var";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct StructMember
        {
            std::string name;
            std::unique_ptr<types::TypeHandle> type;
            StructMember(const std::string &s, std::unique_ptr<types::TypeHandle> ty) : name(s), type(std::move(ty)) {}
            StructMember(const StructMember &sm) : name(sm.name), type(sm.type->clone()) {}
        };
        struct Struct : TypeHandle
        {
            std::string name;
            std::vector<StructMember> members;

            Struct(const std::string &name, const std::vector<StructMember> &vecs) : name(name), members(vecs) {}

            Struct(const Struct &other) : name(other.name), members(other.members) {}

            TYPES get_ty() const override
            {
                return TYPES::STRUCT;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Struct>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Struct";
            }
            bool is_same(TypeHandle *other) const override
            {
                return true;
            }
        };

        struct Enum : TypeHandle
        {
            std::string name;
            std::vector<std::string> members;

            Enum(const std::string &n, const std::vector<std::string> &members) : name(n), members(members) {}
            Enum(const Enum &other) : name(other.name), members(other.members) {}

            TYPES get_ty() const override
            {
                return TYPES::ENUM;
            }

            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Enum>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Enum";
            }
            bool is_same(TypeHandle *other) const override
            {
                if (other->get_ty() == TYPES::ENUM)
                {
                    Enum *tr = dynamic_cast<Enum *>(other);
                    if (name != tr->name)
                        return false;
                    if (members.size() != tr->members.size())
                        return false;
                    for (size_t i = 0; i < members.size(); ++i)
                        if (members.at(i) != tr->members.at(i))
                            return false;
                    return true;
                }
                else if (other->get_ty() == TYPES::ENUM_INSTANCE)
                {
                    return other->is_same(this->clone().get());
                }
                else
                {
                    return false;
                }
            }
        };

        // struct ArrayVariable : TypeHandle
        // {
        //     std::string name;
        //     std::unique_ptr<TypeHandle> expected_ty;
        //     std::unique_ptr<TypeHandle> actual_ty;

        //     ArrayVariable(const std::string &name, std::unique_ptr<TypeHandle> expected_ty, std::unique_ptr<TypeHandle> actual_ty) : name(name), expected_ty(std::move(expected_ty)), actual_ty(std::move(actual_ty)) {}

        //     ArrayVariable(const Variable &other) : name(other.name), expected_ty(other.expected_ty->clone()), actual_ty(other.actual_ty->clone()) {}

        //     TYPES get_ty() const override
        //     {
        //         return TYPES::ARRAY_VAR;
        //     }

        //     std::unique_ptr<TypeHandle> clone() const override
        //     {
        //         return std::make_unique<ArrayVariable>(*this);
        //     }
        //     std::string debug() const override
        //     {
        //         return "Ty.ArrayVar";
        //     }
        // };

        struct Identifier : TypeHandle
        {
            std::string ident;
            Identifier(const std::string &ident) : ident(ident) {}
            TYPES get_ty() const override
            {
                return TYPES::IDENT;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Identifier>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Ident";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct If : TypeHandle
        {
            std::unique_ptr<TypeHandle> cond;
            std::vector<std::unique_ptr<TypeHandle>> body;
            std::vector<std::unique_ptr<TypeHandle>> else_stmt;

            If(std::unique_ptr<TypeHandle> cond, std::vector<std::unique_ptr<TypeHandle>> &&body, std::vector<std::unique_ptr<TypeHandle>> &&else_stmt) : cond(std::move(cond))
            {
                for (auto &&a : body)
                    this->body.push_back(std::move(a));
                for (auto &&a : else_stmt)
                    this->else_stmt.push_back(std::move(a));
            }

            If(const If &other) : cond(other.cond->clone())
            {
                for (const auto &a : other.body)
                    body.push_back(a->clone());
                for (const auto &a : other.else_stmt)
                    else_stmt.push_back(a->clone());
            }

            TYPES get_ty() const override
            {
                return TYPES::IF;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<If>(*this);
            }
            std::string debug() const override
            {
                return "Ty.If";
            }

            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct Dummy : TypeHandle
        {
            TYPES get_ty() const override
            {
                return TYPES::DUMMY;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Dummy>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Lmao";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };
        struct Void : TypeHandle
        {
            TYPES get_ty() const override
            {
                return TYPES::VOID;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Void>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Void";
            }
            bool is_same(TypeHandle *other) const override
            {
                return other->get_ty() == TYPES::VOID;
            }
        };

        struct StructInitializer
        {
            std::string ident;
            std::unique_ptr<TypeHandle> value;
        };
        struct StructInstance : TypeHandle
        {
            std::string name;
            std::vector<StructInitializer> inits;
            StructInstance(const std::string &s, const std::vector<StructInitializer> &i) : name(s)
            {
                for (auto &x : i)
                    inits.push_back(StructInitializer{.ident = x.ident, .value = x.value->clone()});
            }
            StructInstance(const StructInstance &other) : name(other.name)
            {
                for (auto &x : other.inits)
                    inits.push_back(StructInitializer{.ident = x.ident, .value = x.value->clone()});
            }
            std::string debug() const override
            {
                return std::format("Ty.StructInit {}", name);
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<StructInstance>(*this);
            }
            TYPES get_ty() const override
            {
                return TYPES::STRUCT_INSTANCE;
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };

        struct EnumInstance : TypeHandle
        {
            Enum en;
            std::string value;
            EnumInstance(const Enum &en, const std::string &c) : en(en), value(c) {}
            EnumInstance(const EnumInstance &e) : en(e.en), value(e.value) {}

            TYPES get_ty() const override
            {
                return TYPES::ENUM_INSTANCE;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<EnumInstance>(*this);
            }
            std::string debug() const override
            {
                return "Ty.EnumInst";
            }
            bool is_same(TypeHandle *other) const override
            {
                if (other->get_ty() == TYPES::ENUM_INSTANCE)
                {
                    return en.is_same(dynamic_cast<EnumInstance *>(other));
                }
                else if (other->get_ty() == TYPES::ENUM)
                {
                    return en.is_same(dynamic_cast<Enum *>(other));
                }
                else
                {
                    return false;
                }
            }
        };

        struct Return : TypeHandle
        {
            std::unique_ptr<TypeHandle> ty;
            Return(std::unique_ptr<TypeHandle> t) : ty(std::move(t)) {}
            Return(const Return &other) : ty(other.ty->clone()) {}
            TYPES get_ty() const override
            {
                return TYPES::RETURN;
            }
            std::unique_ptr<TypeHandle> clone() const override
            {
                return std::make_unique<Return>(*this);
            }
            std::string debug() const override
            {
                return "Ty.Return";
            }
            bool is_same(TypeHandle *other) const override
            {
                return false;
            }
        };
        // std::unique_ptr<types::TypeHandle> str_to_ty(const std::string &ty, const SourceLoc &loc)
        // {
        //     if (ty == "ra9m")
        //         return std::unique_ptr<TypeHandle>(new Integer());
        //     else if (ty == "ktba")
        //         return std::unique_ptr<TypeHandle>(new String());
        //     else if (ty == "bool")
        //         return std::unique_ptr<TypeHandle>(new Bool());
        //     else if (ty == "walo")
        //         return std::unique_ptr<TypeHandle>(new Void());
        //     else if (ty[0] == '[')
        //     {

        //         return std::unique_ptr<TypeHandle>(new Array());
        //     }
        //     else
        //         return std::unique_ptr<TypeHandle>(new Generic(ty));
        // }
    }
}
#endif