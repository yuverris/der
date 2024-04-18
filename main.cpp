#include <iostream>
#include <format>
#include <fstream>
#include "include/lexer.hpp"
#include "include/parser.hpp"
#include "include/types.hpp"
#include "include/typechecker.hpp"

int main(int argc, char** argv)
{
    if(argc < 2) {
        std::cout << "no input file specified.";
        return 1;
    }
    std::ifstream file{argv[1]};
    std::string filename = argv[1];
    std::string input = {};
    std::string tmp;
    while (std::getline(file, tmp)) (input += tmp) += '\n';
    auto xyz = der::lexer::Lexer(input);
    xyz.lex();
    auto abc = der::parser::Parser(xyz.get_output());
    try
    {
        abc.parse();
        for (const auto& a : abc.get_output())
        {
            std::cout << a.expr->debug() << '\n';
        }
        auto ijk = der::typechecker::TypeChecker(abc.get_output());
        try
        {
            ijk.do_the_thing();
            // for(auto& [key, _]: ijk.local_scope) 
            //     der_debug(key);
            std::cout << ijk.get_output();
        }
        catch (const der::types::CompilationErr &exc)
        {
            std::cout << std::format("\u001b[1m\u001b[31m[khata2 t9ni]:\u001b[m {} (line: {} , col: {})\n", exc.msg, exc.loc.line + 1, exc.loc.column + 1);
        }
    }
    catch (const der::parser::SyntaxErr &exc)
    {
        std::cout << std::format("\u001b[1m\u001b[31m[khata2 imla2i]:\u001b[m {} (line: {}, col: {})\n", exc.msg, exc.loc.line + 1, exc.loc.column + 1);
    }
}
