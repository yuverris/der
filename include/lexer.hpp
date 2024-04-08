#ifndef DER_LEXER_HPP
#define DER_LEXER_HPP
#include <cctype>
#include <string>
#include <vector>
#include <map>
#include "source_loc.hpp"

namespace der
{
    namespace lexer
    {
        enum class TOKENS
        {
            KEYWORD_ILA,
            KEYWORLD_DIR,
            KEYWORD_AWLA,
            KEYWORD_MOTA7ARIK,
            KEYWORD_CHOUF,
            KEYWORD_LAWALO,
            KEYWORD_DALATON,
            KEYWORD_KANT,
            KEYWORD_TABIT,
            TOKEN_EOF,
            TOKEN_IDENTIFIER,
            TOKEN_INTEGER,
            TOKEN_FLOAT,
            TOKEN_STRING,
            TOKEN_BOOL,
            TOKEN_PLUS,
            TOKEN_MINUS,
            TOKEN_MULTIPLY,
            TOKEN_DIVIDE,
            TOKEN_EQUAL,
            TOKEN_LESS_THAN,
            TOKEN_GREATER_THAN,
            TOKEN_LESS_THAN_OR_EQUAL,
            TOKEN_GREATER_THAN_OR_EQUAL,
            TOKEN_NOT_EQUAL,
            TOKEN_AND,
            TOKEN_OR,
            TOKEN_NOT,
            TOKEN_OPEN_PAREN,
            TOKEN_CLOSE_PAREN,
            TOKEN_OPEN_BRACE,
            TOKEN_CLOSE_BRACE,
            TOKEN_OPEN_BRACKET,
            TOKEN_CLOSE_BRACKET,
            TOKEN_SEMICOLON,
            TOKEN_COMMA,
            TOKEN_COLON,
            TOKEN_DOT,
            TOKEN_ASSIGN,
            TOKEN_EQUALITY,
            TOKEN_BIT_AND,
            TOKEN_BIT_OR,
            TOKEN_PIPE,
            TOKEN_IMPORT,
            TOKEN_RETURN,
            TOKEN_STRUCT,
            TOKEN_ENUM,
            TOKEN_RANGE,
            TOKEN_CHAR,
            TOKEN_FOR,
            KEYWORD_JADID,
            TOKEN_DOUBLE_QST
        };

        std::map<TOKENS, std::string> tokens_to_str{
            {TOKENS::TOKEN_PLUS, "+"},
            {TOKENS::TOKEN_MULTIPLY, "*"},
            {TOKENS::TOKEN_AND, "&&"},
            {TOKENS::TOKEN_DIVIDE, "/"},
            {TOKENS::TOKEN_EQUAL, "="},
            {TOKENS::TOKEN_MINUS, "-"},
            {TOKENS::TOKEN_LESS_THAN, "<"},
            {TOKENS::TOKEN_GREATER_THAN, ">"},
            {TOKENS::TOKEN_EQUALITY, "=="},
            {TOKENS::TOKEN_DOUBLE_QST, "??"},
            {TOKENS::TOKEN_RANGE, "..."},
            {TOKENS::TOKEN_DOT, "."}};
        struct TokenHandle
        {
            TOKENS token;
            std::string raw_value;
            SourceLoc source_loc;

            template <class... tok>
            bool is(tok... t)
            {
                return (... || (token == t));
            }

            template <class... tok>
            bool is_not(tok... t)
            {
                return !is(t...);
            }
        };

        class Lexer
        {
            std::vector<TokenHandle> m_output;
            std::string m_input;
            size_t m_index = 0;

            void m_advance(size_t n = 1)
            {
                m_index += n;
            }

            char m_current()
            {
                if (m_index >= m_input.size())
                {
                    return -1;
                }
                else
                {
                    return m_input[m_index];
                }
            }

            char m_consume()
            {
                char c = m_current();
                m_advance();
                return c;
            }

        public:
            Lexer(const std::string &input, const SourceLoc &loc = {}) : m_output({}), m_input(input) {}

            void lex()
            {
                SourceLoc local_loc{};

                char current = m_consume();
                while (current != -1)
                {
                    if (std::isdigit(current))
                    {
                        std::string temp{current};
                        char b = m_consume();
                        while (std::isdigit(b))
                        {
                            temp.push_back(b);
                            b = m_consume();
                        }
                        m_index -= 1;
                        local_loc.column += temp.size();
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_INTEGER, .raw_value = temp, .source_loc = local_loc});
                    }
                    else if (std::isalpha(current))
                    {
                        std::string temp{current};
                        char b = m_consume();
                        while (std::isalpha(b) || b == '_' || std::isdigit(b) && b != ' ')
                        {
                            temp.push_back(b);
                            b = m_consume();
                        }
                        m_index -= 1;
                        local_loc.column += temp.size();
                        TOKENS token;
                        if (temp == "chouf")
                            token = TOKENS::KEYWORD_CHOUF;
                        else if (temp == "ila")
                            token = TOKENS::KEYWORD_ILA;
                        else if (temp == "awla")
                            token = TOKENS::KEYWORD_AWLA;
                        else if (temp == "dalaton")
                            token = TOKENS::KEYWORD_DALATON;
                        else if (temp == "kant")
                            token = TOKENS::KEYWORD_KANT;
                        else if (temp == "dir")
                            token = TOKENS::KEYWORLD_DIR;
                        else if (temp == "mota7arik")
                            token = TOKENS::KEYWORD_MOTA7ARIK;
                        else if (temp == "sa7i7" || temp == "khata2")
                            token = TOKENS::TOKEN_BOOL;
                        else if (temp == "rje3")
                            token = TOKENS::TOKEN_RETURN;
                        else if (temp == "jbed")
                            token = TOKENS::TOKEN_IMPORT;
                        else if (temp == "jism")
                            token = TOKENS::TOKEN_STRUCT;
                        else if (temp == "ti3dad")
                            token = TOKENS::TOKEN_ENUM;
                        else if (temp == "lkola")
                            token = TOKENS::TOKEN_FOR;
                        else if (temp == "jadid")
                            token = TOKENS::KEYWORD_JADID;
                        else if (temp == "tabit")
                            token = TOKENS::KEYWORD_TABIT;
                        else
                            token = TOKENS::TOKEN_IDENTIFIER;

                        m_output.push_back(TokenHandle{.token = token, .raw_value = temp, .source_loc = local_loc});
                    }

                    switch (current)
                    {
                    case '=':
                        if (m_input.at(m_index) == '=')
                        {

                            m_advance();
                            local_loc.column += 2;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_EQUALITY, .raw_value = "==", .source_loc = local_loc});
                        }
                        else
                        {
                            local_loc.column += 1;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_EQUAL, .raw_value = "=", .source_loc = local_loc});
                        }
                        break;
                    case '+':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_PLUS, .raw_value = "+", .source_loc = local_loc});
                        break;
                    case '-':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_MINUS, .raw_value = "=", .source_loc = local_loc});
                        break;
                    case '*':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_MULTIPLY, .raw_value = "*", .source_loc = local_loc});
                        break;
                    case '/':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_DIVIDE, .raw_value = "+", .source_loc = local_loc});
                        break;
                    case '(':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_OPEN_PAREN, .raw_value = "(", .source_loc = local_loc});
                        break;
                    case ')':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_CLOSE_PAREN, .raw_value = ")", .source_loc = local_loc});
                        break;
                    case '[':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_OPEN_BRACKET, .raw_value = "[", .source_loc = local_loc});
                        break;
                    case ']':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_CLOSE_BRACKET, .raw_value = "]", .source_loc = local_loc});
                        break;
                    case '{':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_OPEN_BRACE, .raw_value = "{", .source_loc = local_loc});
                        break;
                    case '}':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_CLOSE_BRACE, .raw_value = "}", .source_loc = local_loc});
                        break;
                    case ';':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_SEMICOLON, .raw_value = ";", .source_loc = local_loc});
                        break;
                    case ':':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_COLON, .raw_value = ":", .source_loc = local_loc});
                        break;
                    case ',':
                        local_loc.column += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_COMMA, .raw_value = ",", .source_loc = local_loc});
                        break;
                    case '\'':
                    {

                        local_loc.column += 1;
                        char a = m_consume();
                        std::cout << a << ' ' << m_current() << '\n';
                        if (m_current() != '\'')
                        {
                            throw "expected \"'\" quote after character";
                        }
                        m_index += 1;
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_CHAR, .raw_value = std::format("{}", a), .source_loc = local_loc});
                        break;
                    }
                    case '.':
                        if ((m_index + 1) < m_input.size())
                        {
                            if (m_input.at(m_index) == '.' && m_input.at(m_index + 1) == '.')
                            {
                                std::cout << "so true and real\n";
                                local_loc.column += 3;
                                m_index += 2;
                                m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_RANGE, .raw_value = "...", .source_loc = local_loc});
                            }
                            else
                            {

                                local_loc.column += 1;
                                m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_DOT, .raw_value = ".", .source_loc = local_loc});
                            }
                        }
                        else
                        {

                            local_loc.column += 1;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_DOT, .raw_value = ".", .source_loc = local_loc});
                        }
                        break;
                    case '"':
                    {
                        local_loc.column += 1;
                        std::string str{};
                        auto temp = m_consume();
                        while (temp != '"')
                        {
                            str.push_back(temp);
                            temp = m_consume();
                        }
                        m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_STRING, .raw_value = str, .source_loc = local_loc});
                        break;
                    }
                    case '<':
                        if (m_input[m_index + 1] == '=')
                        {
                            m_advance();
                            local_loc.column += 2;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_LESS_THAN_OR_EQUAL, .raw_value = "<=", .source_loc = local_loc});
                        }
                        else
                        {

                            local_loc.column += 1;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_LESS_THAN, .raw_value = "<", .source_loc = local_loc});
                        }
                        break;
                    case '>':
                        if (m_input[m_index] == '=')
                        {
                            m_advance();
                            local_loc.column += 2;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_GREATER_THAN_OR_EQUAL, .raw_value = ">=", .source_loc = local_loc});
                        }
                        else
                        {
                            local_loc.column += 1;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_GREATER_THAN, .raw_value = ">", .source_loc = local_loc});
                        }
                        break;

                    case '&':
                        if (m_input[m_index] == '&')
                        {
                            m_advance();
                            local_loc.column += 2;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_AND, .raw_value = "&&", .source_loc = local_loc});
                        }
                        else
                        {

                            local_loc.column += 1;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_BIT_AND, .raw_value = "&", .source_loc = local_loc});
                        }
                        break;
                    case '|':
                        if (m_input[m_index] == '|')
                        {
                            m_advance();
                            local_loc.column += 2;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_AND, .raw_value = "||", .source_loc = local_loc});
                        }
                        else if (m_input[m_index] == '>')
                        {
                            m_advance();
                            local_loc.column += 2;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_PIPE, .raw_value = "|>", .source_loc = local_loc});
                        }
                        else
                        {

                            local_loc.column += 1;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_BIT_OR, .raw_value = "|", .source_loc = local_loc});
                        }
                        break;
                    case '?':
                        if (m_input.at(m_index) == '?')
                        {
                            m_advance();
                            local_loc.column += 2;
                            m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_DOUBLE_QST, .raw_value = "??", .source_loc = local_loc});
                        }
                        break;
                    case '\n':
                        local_loc.line += 1;
                        local_loc.column = 0;
                    default:
                        break;
                    }

                    current = m_consume();
                }
                // m_output.push_back(TokenHandle{.token = TOKENS::TOKEN_EOF, .raw_value = "EOF", .source_loc = local_loc});
            }

            std::vector<TokenHandle> get_output()
            {
                return m_output;
            }
        };
    }
}
#endif
