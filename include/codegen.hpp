#ifndef DER_CODEGEN_HPP
#define DER_CODEGEN_HPP
#include <string>
#include <format>
namespace der {
    namespace codegen {
        // now here comes the real shit
        struct Codegen {

            std::string gen_bin_add() {
                return std::format("add {}");
            }
        };
    }
}
#endif