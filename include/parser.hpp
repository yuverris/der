#ifndef DER_PARSER
#define DER_PARSER
#include <optional>
#include "ast.hpp"
#include "lexer.hpp"
#include "source_loc.hpp"
#include "debug.hpp"

namespace der
{
    namespace parser
    {
        struct SyntaxErr
        {
            std::string msg;
            SourceLoc loc;
            SyntaxErr(const std::string &m, const SourceLoc &loc) : msg(m), loc(loc) {}
        };

        unsigned short get_precedence(lexer::TOKENS tok)
        {
            using namespace lexer;
            switch (tok)
            {
            case TOKENS::TOKEN_OR:
                return 11;
            case TOKENS::TOKEN_AND:
                return 12;

            case TOKENS::TOKEN_BIT_OR:
                return 13;
            case TOKENS::TOKEN_BIT_AND:
                return 15;

            case TOKENS::TOKEN_EQUALITY:
                return 16;

            case TOKENS::TOKEN_PLUS:
            case TOKENS::TOKEN_MINUS:
                return 19;
            case TOKENS::TOKEN_DIVIDE:
            case TOKENS::TOKEN_MULTIPLY:
                return 20;
            case TOKENS::TOKEN_PIPE:
                return 50;
            case TOKENS::TOKEN_DOT:
                return 60;
            default:
                return 5;
            }
        }

        struct AstInfo
        {
            ast::ptr<ast::Expr> expr;
            SourceLoc loc;
            AstInfo(ast::ptr<ast::Expr> expr, SourceLoc loc) : expr(std::move(expr)), loc(loc) {}
            constexpr AstInfo(const AstInfo &a) : expr(a.expr->clone()), loc(a.loc) {}
            constexpr const AstInfo &operator=(const AstInfo &a)
            {
                expr = a.expr->clone();
                loc = a.loc;
                return *this;
            }
        };
        struct Parser
        {
            std::vector<lexer::TokenHandle> m_input;
            std::vector<AstInfo> m_output;
            size_t m_index = 0;
            Parser(const std::vector<lexer::TokenHandle> &inp) : m_input(inp), m_output() {}

            void parse()
            {
                der_debug("called");
                std::vector<AstInfo> parsed = {};
                while (m_index < m_input.size())
                {
                    der_debug("inner loop called");
                    auto expr = parse_expr(0);
                    der_debug_e(m_current().raw_value);
                    m_expect_or(lexer::TOKENS::TOKEN_SEMICOLON, m_current(), "Expected ';' after expression.");
                    m_advance();
                    m_output.push_back(expr);
                    der_debug("success");
                }
            }

            template <class... T>
            bool m_match(T... toks)
            {
                if (m_current().is(toks...) && !at_end())
                {
                    m_advance();
                    return true;
                }
                return false;
            }

            bool at_end()
            {
                return m_current().is(lexer::TOKENS::TOKEN_EOF);
            }

            AstInfo m_parse_variable()
            {
                der_debug("start");
                lexer::TokenHandle current = m_current();
                m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, current, "khass ikon identifier mn b3d 'dir'.");
                std::string name = current.raw_value;
                der_debug(name);
                der_debug_e(m_current().raw_value);
                m_advance();
                auto nexc = m_current();
                der_debug_e(nexc.raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_COLON, nexc, "nsiti ':'.");
                m_advance();
                auto ty = parse_type();
                m_advance();
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_EQUAL, m_current(), "nsiti '='.");
                m_advance();
                AstInfo value = parse_expr(0);
                return AstInfo(ast::ptr<ast::Expr>(new ast::Variable(name, std::move(value.expr), std::move(ty))), value.loc);
            }

            AstInfo m_parse_enum()
            {
                der_debug("start");
                m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, m_current(), "Expected identifier after enum declaration.");
                std::string enum_name = m_current().raw_value;
                m_advance();
                m_expect_or(lexer::TOKENS::TOKEN_OPEN_BRACE, m_current(), "Expected '{' after enum identifier.");
                std::vector<std::string> members{};
                m_advance();
                do
                {
                    if (m_current().is(lexer::TOKENS::TOKEN_CLOSE_BRACE))
                        break;
                    der_debug_e(m_current().raw_value);
                    m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, m_current(), "Expected identifier in enum member");
                    members.push_back(m_current().raw_value);
                    m_advance();
                } while (m_match(lexer::TOKENS::TOKEN_COMMA));
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACE, m_current(), "Expected '}' after enum definition");
                m_advance();
                return AstInfo(std::make_unique<ast::Enum>(enum_name, members), m_current().source_loc);
            }
            AstInfo m_parse_struct()
            {
                der_debug("start");
                m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, m_current(), "Expected identifier after struct definition");
                std::string struct_name = m_current().raw_value;
                m_advance();
                m_expect_or(lexer::TOKENS::TOKEN_OPEN_BRACE, m_current(), "Expected '{' after identifier.");
                std::vector<ast::StructMember> members{};
                m_advance();
                do
                {
                    if (m_current().is(lexer::TOKENS::TOKEN_CLOSE_BRACE))
                        break;
                    der_debug_e(m_current().raw_value);
                    m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, m_current(), "Expected identifier in struct member");
                    std::string name = m_current().raw_value;
                    m_advance();
                    m_expect_or(lexer::TOKENS::TOKEN_COLON, m_current(), "Expected colon ':' after identifier.");
                    m_advance();
                    auto type = parse_type();
                    m_advance();
                    members.push_back(ast::StructMember(name, std::move(type)));
                } while (m_match(lexer::TOKENS::TOKEN_SEMICOLON));
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACE, m_current(), "Expected '}' after struct definition");
                m_advance();
                return AstInfo(std::make_unique<ast::Struct>(struct_name, members), m_current().source_loc);
            }

            AstInfo parse_for_loop()
            {
                m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, m_current(), "Expected identifier after for loop");
                std::string ident = m_current().raw_value;
                m_advance();
                m_expect_or(lexer::TOKENS::TOKEN_COLON, m_current(), "Expected colon ':' after identifier, in for loop.");
                m_advance();
                auto _begin = parse_expr(0);
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_RANGE, m_current(), "Expected range '...' in for loop.");
                m_advance();
                auto _end = parse_expr(0);
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_OPEN_BRACE, m_current(), "Expected '{' after for loop statement.");
                auto body = parse_mult_stmt();
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACE, m_current(), "Expected '}' after for loop statement.");
                m_advance();
                return AstInfo(std::make_unique<ast::RangedFor<AstInfo>>(ident, _begin.expr->clone(), _end.expr->clone(), body), m_current().source_loc);
            }
            std::vector<ast::ptr<ast::Expr>> m_parse_function_args()
            {
                der_debug("start");
                m_advance();
                std::vector<ast::ptr<ast::Expr>> arg_list = {};
                if (m_current().is_not(lexer::TOKENS::TOKEN_CLOSE_PAREN))
                {
                    do
                    {
                        arg_list.push_back(parse_expr(0).expr->clone());
                    } while (m_match(lexer::TOKENS::TOKEN_COMMA));
                }
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_PAREN, m_current(), "nsiti ')' mn b3d argument list.");
                m_advance();
                der_debug_m("fnc call end", m_current().raw_value);
                return arg_list;
            }

            AstInfo parse_struct_init()
            {
                der_debug("start");
                std::string name = m_current().raw_value;
                der_debug_e(m_current().raw_value);
                m_advance();
                m_expect_or(lexer::TOKENS::TOKEN_OPEN_BRACE, m_current(), "Expected '{' in struct initialization.");
                m_advance();
                std::vector<ast::StructInitializer> inits = {};
                do
                {
                    m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, m_current(), "Expected identifier in struct initialization.");
                    std::string ident = m_current().raw_value;
                    m_advance();
                    m_expect_or(lexer::TOKENS::TOKEN_COLON, m_current(), "Expected ':' after identifier in struct initializaiton.");
                    m_advance();
                    auto expr = parse_expr(0).expr;
                    inits.push_back(ast::StructInitializer{.ident = ident, .value = std::move(expr)});
                } while (m_match(lexer::TOKENS::TOKEN_COMMA));
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACE, m_current(), "Expected '}' after struct initialization.");
                m_advance();
                return AstInfo(std::make_unique<ast::StructInstance>(name, inits), m_current().source_loc);
            }

            AstInfo parse_stmt()
            {
                auto ret = parse_expr(0);
                der_debug_e(ret.expr->debug());
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_SEMICOLON, m_current(), "nsiti ';' mn b3d expression.");
                m_advance();
                return ret;
            }

            std::vector<AstInfo> parse_mult_stmt()
            {
                std::vector<AstInfo> out{};
                der_debug("start");
                m_advance();
                while (m_current().is_not(lexer::TOKENS::TOKEN_CLOSE_BRACE))
                {
                    der_debug("inner loop called");
                    auto temp = parse_stmt();
                    der_debug_e(temp.expr->debug());
                    der_debug_e(m_current().raw_value);
                    out.push_back(temp);
                }
                der_debug_e(m_current().raw_value);
                return out;
            }

            AstInfo m_parse_array()
            {
                m_advance();
                std::vector<std::unique_ptr<ast::Expr>> values = {};
                do
                {
                    values.push_back(parse_expr(0).expr);
                } while (m_match(lexer::TOKENS::TOKEN_COMMA));
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACKET, m_current(), "Expected ']' after array expression.");
                m_advance();
                return AstInfo(std::make_unique<ast::Array>(std::move(values)), m_current().source_loc);
            }

            std::unique_ptr<types::TypeHandle> parse_type()
            {
                using namespace lexer;
                switch (m_current().token)
                {
                case TOKENS::TOKEN_IDENTIFIER:
                {

                    std::string v = m_current().raw_value;
                    if (m_input[m_index + 1].token == TOKENS::TOKEN_LESS_THAN)
                    {
                        std::vector<std::unique_ptr<types::TypeHandle>> ss{};
                        m_advance();
                        do
                        {
                            ss.push_back(parse_type());
                            m_advance();

                        } while (m_match(TOKENS::TOKEN_COMMA));

                        return std::make_unique<types::TemplateParam>(v, ss);
                    }
                    else
                    {
                        if (v == "ra9m")
                            return std::make_unique<types::Integer>();
                        else if (v == "ktba")
                            return std::make_unique<types::String>();
                        else if (v == "bool")
                            return std::make_unique<types::Bool>();
                        else if (v == "walo")
                            return std::make_unique<types::Void>();
                        else if (v == "harf")
                            return std::make_unique<types::Character>();
                        else
                            return std::make_unique<types::Identifier>(v);
                    }
                }
                case TOKENS::TOKEN_OPEN_BRACKET:
                {
                    m_advance();
                    m_expect_or(TOKENS::TOKEN_IDENTIFIER, m_current(), "Expected identifier in array type.");
                    auto ty = parse_type();
                    m_advance();
                    m_expect_or(TOKENS::TOKEN_SEMICOLON, m_current(), "Expected ';' semicolon after array type");
                    m_advance();
                    m_expect_or(TOKENS::TOKEN_INTEGER, m_current(), "Expected integer after ; in array type.");
                    size_t size = std::stoul(m_current().raw_value);
                    m_advance();
                    m_expect_or(TOKENS::TOKEN_CLOSE_BRACKET, m_current(), "Expected ']' after array type.");
                    return std::make_unique<types::Array>(std::move(ty), size);
                }
                }
            }

            AstInfo parse_if_stmt()
            {
                /*
                ila expr {
                    expr...;
                } awla? {
                    expr...;
                }
                */
                auto head = parse_expr(0);
                der_debug_e(head.expr->debug());
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_OPEN_BRACE, m_current(), "expected '{' after if statement head.");
                auto body = parse_mult_stmt();
                std::vector<AstInfo> else_stmt = {};
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACE, m_current(), "expected '}'.");
                m_advance();
                if (m_current().is(lexer::TOKENS::KEYWORD_AWLA))
                {
                    m_advance();
                    m_expect_or(lexer::TOKENS::TOKEN_OPEN_BRACE, m_current(), "expected '{' after else statement head.");
                    else_stmt = parse_mult_stmt();
                    der_debug_m("else block", m_current().raw_value);
                    m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACE, m_current(), "expected '}'.");
                    m_advance();
                }
                return AstInfo(ast::ptr<ast::Expr>(new ast::IfStmt(std::move(head.expr), std::move(body), std::move(else_stmt))), m_current().source_loc);
            }
            AstInfo parse_primary()
            {
                der_debug("start");
                using namespace lexer;
                TokenHandle th = m_current();
                switch (th.token)
                {
                case TOKENS::TOKEN_STRING:
                    der_debug("recognized string");
                    m_advance();
                    return AstInfo(ast::ptr<ast::Expr>(new ast::String(th.raw_value)), th.source_loc);
                case TOKENS::TOKEN_INTEGER:
                    der_debug("recognized integer");
                    m_advance();
                    return AstInfo(ast::ptr<ast::Expr>(new ast::Integer(th.raw_value)), th.source_loc);
                case TOKENS::TOKEN_BOOL:
                    der_debug("recognized boolean");
                    m_advance();
                    return AstInfo(ast::ptr<ast::Expr>(new ast::Bool(th.raw_value == "sa7i7" ? true : false)), th.source_loc);
                case TOKENS::TOKEN_STRUCT:
                {
                    der_debug("recognized struct");
                    m_advance();
                    return m_parse_struct();
                }
                case TOKENS::TOKEN_ENUM:
                {
                    der_debug("recognized enum");
                    m_advance();
                    return m_parse_enum();
                }
                case TOKENS::TOKEN_CHAR:
                {
                    der_debug("char ssss");
                    m_advance();
                    return AstInfo(ast::ptr<ast::Expr>(new ast::Character(th.raw_value.at(0))), th.source_loc);
                }
                case TOKENS::TOKEN_FOR:
                {
                    der_debug("recognized for loop");
                    m_advance();
                    return parse_for_loop();
                }
                case TOKENS::TOKEN_IDENTIFIER:
                {
                    der_debug("recognized identifier");
                    m_advance();
                    return AstInfo(ast::ptr<ast::Expr>(new ast::Identifier(th.raw_value)), th.source_loc);
                }
                case TOKENS::TOKEN_OPEN_BRACKET:
                    der_debug("recognized static array");
                    return m_parse_array();
                case TOKENS::KEYWORLD_DIR:
                {
                    der_debug("recognized keyword DIR");
                    m_advance();
                    auto var = m_parse_variable();
                    return var;
                }
                case TOKENS::KEYWORD_ILA:
                {
                    der_debug("recognized keyword ILA");
                    m_advance();
                    return parse_if_stmt();
                }
                case TOKENS::KEYWORD_DALATON:
                    der_debug("recognized keyword DALATON");
                    return parse_function();
                case TOKENS::KEYWORD_JADID:
                    der_debug("recognized keyword JADID");
                    m_advance();
                    return parse_struct_init();
                case TOKENS::TOKEN_RETURN:
                {

                    der_debug("recognized tok return");
                    if (!allow_return)
                    {
                        throw SyntaxErr("return statements aren't allowed outside of functions.", th.source_loc);
                    }
                    m_advance();
                    auto e = parse_expr(0).expr;
                    return AstInfo(ast::ptr<ast::Expr>(new ast::Return(std::move(e))), th.source_loc);
                }
                default:
                    throw SyntaxErr("invalid token " + th.raw_value, th.source_loc);
                }
            }

            AstInfo parse_expr(unsigned short prec)
            {
                der_debug("start");
                AstInfo lfs = parse_primary();
                der_debug_e(m_current().raw_value);
                while (m_current().is_not(lexer::TOKENS::TOKEN_SEMICOLON))
                {
                    lexer::TokenHandle current = m_current();
                    using lexer::TOKENS;
                    if (current.is(TOKENS::TOKEN_PLUS, TOKENS::TOKEN_MINUS, TOKENS::TOKEN_DIVIDE, TOKENS::TOKEN_MULTIPLY))
                    {
                        der_debug_e(prec);
                        der_debug_e(get_precedence(current.token));
                        if (get_precedence(current.token) <= prec)
                        {
                            break;
                        }
                        m_advance();
                        AstInfo rfs = parse_expr(get_precedence(current.token));
                        lfs = AstInfo(ast::ptr<ast::Expr>(new ast::BinaryOper(std::move(lfs.expr), current.token, std::move(rfs.expr))), rfs.loc);
                    }
                    else if (current.is(TOKENS::TOKEN_AND, TOKENS::TOKEN_OR, TOKENS::TOKEN_LESS_THAN, TOKENS::TOKEN_LESS_THAN_OR_EQUAL, TOKENS::TOKEN_GREATER_THAN, TOKENS::TOKEN_GREATER_THAN_OR_EQUAL, TOKENS::TOKEN_EQUALITY))
                    {
                        der_debug_e(prec);
                        der_debug_e(get_precedence(current.token));
                        if (get_precedence(current.token) <= prec)
                        {
                            break;
                        }
                        m_advance();
                        AstInfo rfs = parse_expr(get_precedence(current.token));
                        lfs = AstInfo(ast::ptr<ast::Expr>(new ast::LogicalBinaryOper(std::move(lfs.expr), current.token, std::move(rfs.expr))), rfs.loc);
                    }
                    else if (current.is(TOKENS::TOKEN_OPEN_PAREN))
                    {
                        der_debug("calling function call parse...");
                        auto temp = m_parse_function_args();
                        lfs = AstInfo(ast::ptr<ast::Expr>(new ast::FunctionCall(std::move(lfs.expr), temp)), lfs.loc);
                    }
                    else if (current.is(TOKENS::TOKEN_OPEN_BRACKET))
                    {
                        if (get_precedence(current.token) <= prec)
                        {
                            break;
                        }
                        der_debug("calling subscript parse...");
                        m_advance();
                        auto t = std::make_unique<ast::Subscript>(std::move(lfs.expr), parse_expr(0).expr);
                        m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACKET, m_current(), "Expected ']' after subscript operator.");
                        lfs = AstInfo(std::move(t), lfs.loc);
                        m_advance();
                        der_debug_e(lfs.expr->debug());
                    }
                    else if (current.is(TOKENS::TOKEN_PIPE))
                    {
                        der_debug("PIPE OP encounter.");
                        if (get_precedence(current.token) <= prec)
                        {
                            break;
                        }
                        der_debug_e(lfs.expr->debug());
                        m_advance();
                        lfs = AstInfo(ast::ptr<ast::Expr>(new ast::PipeOper(std::move(lfs.expr), parse_expr(get_precedence(current.token)).expr)), lfs.loc);
                    }
                    else if (current.is(TOKENS::TOKEN_EQUAL))
                    {
                        der_debug("SET OP encounter.");
                        if (get_precedence(current.token) <= prec)
                        {
                            break;
                        }
                        der_debug_e(lfs.expr->debug());
                        m_advance();
                        lfs = AstInfo(ast::ptr<ast::Expr>(new ast::SetOper(std::move(lfs.expr), parse_expr(get_precedence(current.token)).expr)), lfs.loc);
                    }
                    else if (current.is(TOKENS::TOKEN_DOUBLE_QST))
                    {
                        if (get_precedence(current.token) <= prec)
                        {
                            break;
                        }
                        der_debug("DOUBLE QST if stmt.");
                        m_advance();
                        lfs = AstInfo(ast::ptr<ast::Expr>(new ast::SmolIfStmt(std::move(lfs.expr), parse_expr(get_precedence(current.token)).expr)), lfs.loc);
                    }
                    else if (current.is(TOKENS::TOKEN_DOT))
                    {
                        if (get_precedence(current.token) <= prec)
                        {
                            break;
                        }
                        der_debug("DotOper expr.");
                        der_debug_e(m_current().raw_value);
                        m_advance();
                        lfs = AstInfo(ast::ptr<ast::Expr>(new ast::DotOper(std::move(lfs.expr), parse_expr(get_precedence(current.token)).expr)), lfs.loc);
                        der_debug(std::format("shit happens: {}", lfs.expr->debug()));
                    }
                    else
                    {
                        break;
                    }
                }
                der_debug("end");
                return lfs;
            }

            void m_advance()
            {
                if (m_index < m_input.size())
                    m_index += 1;
                der_debug("new idx: " + std::to_string(m_index));
            }

            void m_recover()
            {
                if (m_index >= 1)
                    m_index -= 1;
            }

            lexer::TokenHandle m_current()
            {
                return m_input.at(m_index);
            }

            // adv and return
            lexer::TokenHandle m_nexurrent()
            {
                m_advance();
                return m_input.at(m_index);
            }

            void m_expect_or(lexer::TOKENS expected_tok, lexer::TokenHandle actual_tok, const std::string &error_msg)
            {
                der_debug("called");
                if (actual_tok.token != expected_tok)
                    throw SyntaxErr(error_msg, actual_tok.source_loc);
            }

            AstInfo parse_function()
            {
                der_debug("called");
                auto current = m_nexurrent();
                m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, current, "Expected an identifier after keyword 'dalaton'.");
                std::string name = current.raw_value;
                std::vector<std::string> generics{};
                std::vector<types::ArgType> arg_list = {};
                m_advance();
                if (m_current().is(lexer::TOKENS::TOKEN_LESS_THAN))
                {
                    // throw SyntaxErr("generics are not yet supported (and definitely not because of a bug I couldn't figure out)", m_current().source_loc);
                    m_advance();
                    do
                    {
                        if (m_current().is(lexer::TOKENS::TOKEN_GREATER_THAN))
                        {
                            break;
                        }
                        m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, m_current(), "Expected an identifier in generics list.");
                        generics.push_back(m_current().raw_value);
                        m_advance();
                    } while (m_match(lexer::TOKENS::TOKEN_COMMA));
                    m_advance();
                }
                m_expect_or(lexer::TOKENS::TOKEN_OPEN_PAREN, m_current(), "Expected a '(' after function name identifier.");
                m_advance();
                // ah shit here we go again
                if (m_current().is_not(lexer::TOKENS::TOKEN_CLOSE_PAREN))
                {
                    do
                    {
                        der_debug(m_current().raw_value);
                        m_expect_or(lexer::TOKENS::TOKEN_IDENTIFIER, m_current(), "arguments dyal fonction khass ikon identifiers.");
                        std::string id = m_current().raw_value;
                        m_advance();
                        m_expect_or(lexer::TOKENS::TOKEN_COLON, m_current(), "Expected ':' after argument.");
                        m_advance();
                        auto ty = parse_type();
                        arg_list.push_back(types::ArgType(id, m_current().source_loc, std::move(ty)));
                        m_advance();
                    } while (m_match(lexer::TOKENS::TOKEN_COMMA));
                }
                der_debug_e(m_current().raw_value);
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_PAREN, m_current(), "nsiti ')' mn b3d argument list.");
                m_advance();
                m_expect_or(lexer::TOKENS::TOKEN_COLON, m_current(), "expected a colon ':' after.");
                m_advance();
                auto ret_ty = parse_type();
                m_advance();
                m_expect_or(lexer::TOKENS::TOKEN_OPEN_BRACE, m_current(), "khass '{' mn b3d list d'arguments.");
                allow_return = true;
                auto body = parse_mult_stmt();
                der_debug_e(m_current().raw_value);
                allow_return = false;
                m_expect_or(lexer::TOKENS::TOKEN_CLOSE_BRACE, m_current(), "expected '}'.");
                der_debug_m("fnc def end", m_current().raw_value);
                m_advance();
                return AstInfo(ast::ptr<ast::Expr>(new ast::Function<AstInfo>(name, body, arg_list, generics, std::move(ret_ty))), m_current().source_loc);
            }

            auto get_output()
            {
                return m_output;
            }

            bool allow_return = false;
        };
    }
}
#endif
