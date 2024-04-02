#ifndef _DER_CPRINT
#define _DER_CPRINT
#include <string>
#include <map>
namespace cprint {
    const std::string OPENDELIM = "{{";
    const std::string CLOSEDDELIM = "}}";
    const std::string OPENASCII = "\x1B";
    const std::string CLOSEDASCII = "[0m";
    std::map<std::string, std::string> ascii {
        {"yellow", "33"},
        {"red", "31"},
        {"orange", "38"},
        {"green", "32"},
        {"grey", "37"}
    };
}
#endif
