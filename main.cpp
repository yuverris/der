#include <iostream>
#include <format>
#include "include/lexer.hpp"
#include "include/parser.hpp"
#include "include/types.hpp"
#include "include/typechecker.hpp"

int main()
{
    std::string src_code = R"(

ti3dad Cmp {
    AAA, BBB, CCC
};

jism No9ta {
    x: ra9m;
    y: ra9m;
};
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
        rje3 6*7;
    } awla {
        rje3 9;
    };
};

dalaton l7ajm(a: ktba): ra9m {
    lkola x: 9...99 {
       rje3 5;
    };
    dir y: [ra9m; 5] = [1,7,8,9,7];
    y[y[7]];
};

dalaton hehe(a: ra9m): ra9m {
    rje3 5;
};


dalaton mini_calc(a: ra9m, op: harf, b: ra9m): ra9m {
    op == '+' ?? rje3 a + b;
    op == '-' ?? rje3 a - b;
    op == '/' ?? rje3 a / b;
    op == '*' ?? rje3 a * b;
};

dalaton main(): ra9m {
    test(5, 7);
    hehe(9);
    "Salam, 3alam!" |> kteb();

    dir x: *ra9m = 78;
    x = 89 + -7 * +9;
    dir z: [ra9m; 3] = [7,8,9];
    dir y: harf = "azerty"[7];
    

    dir f: Cmp = Cmp.AAA;

    dir w: No9ta = jadid No9ta{x: 78, y: 77};

    dir j: ra9m = w.x;

    "uwu";
    9 == 7;
    
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
