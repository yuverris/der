#ifndef DER_DEBUG_HH
#define DER_DEBUG_HH
#include <string>
#include <iostream>
#include <source_location>
namespace der
{
    void dd_debug(const std::string &e, size_t line, const char *fname)
    {
        std::cout << "\u001b[34m[" << fname << ':' << line << "]\u001b[m \u001b[35m" << e << "\u001b[m" << '\n';
    }
    template <class T>
    void dd_debug_(const std::string &e, size_t line, const char *fname, const T &value, const std::string &extra = {})
    {
        std::cout << "\u001b[34m[" << fname << ':' << line << "]\u001b[m \u001b[35m" << extra << "\u001b[m \"" << e << " => " << value << '"' << '\n';
    }
#ifdef DER_ALLOW_DEBUG
#define der_debug_e(expr) der::dd_debug_(#expr, __LINE__, __func__, expr)
#define der_debug_m(msg, expr) der::dd_debug_(#expr, __LINE__, __func__, expr, msg)
#define der_debug(msg) der::dd_debug(msg, __LINE__, __func__)
#endif
#ifndef DER_ALLOW_DEBUG
#define der_debug_e(expr)
#define der_debug_m(msg, expr)
#define der_debug(msg)
#endif
}
#endif