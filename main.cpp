#include <iostream>
#include <format>
#include "include/lexer.hpp"
#include "include/parser.hpp"
#include "include/types.hpp"
#include "include/typechecker.hpp"

int main()
{
    std::string src_code = R"(


dalaton dkhel(a: ktba): ktba {
    rje3 "aaa";
};


dalaton kteb(a: ktba): ktba {
    rje3 "aaa";
};

dalaton l_ra9m(a: ktba): ra9m {
    rje3 8;
};

dalaton test(a: ra9m, b: ra9m): ra9m {
    ila a < b {
        rje3 6;
    } awla {
        rje3 9;
    };
};

dalaton hehe(a: ra9m, b: ktba): ra9m {
    rje3 a < b.len();
};


dalaton mini_calc(a: ra9m, op: ktba, b: ra9m): ra9m {
    op == "+" ?? rje3 a + b;
    op == "-" ?? rje3 a - b;
    op == "/" ?? rje3 a / b;
    op == "*" ?? rje3 a * b;
};

dalaton main(): ra9m {
    test(5, 6) |> kteb();

    hehe("ahahahah", 79);

    "Salam, 3alam!" |> kteb();
    
    rje3 0;
};
)";
    auto xyz = der::lexer::Lexer(src_code);
    xyz.lex();
    auto abc = der::parser::Parser(xyz.get_output());
    try
    {
        abc.parse();
        for (auto a : abc.get_output())
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
