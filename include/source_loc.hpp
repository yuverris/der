#ifndef DER_SOURCE_LOC
#define DER_SOURCE_LOC

namespace der {
    struct SourceLoc {
        unsigned long column = 0;
        unsigned long line = 0;
        char* filename = (char*)"unkown";
    };
}
#endif
