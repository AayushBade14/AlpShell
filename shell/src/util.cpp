#include "util.h"

std::string AlpUtil::alp_flag_util(int argc, char **argv) {
    std::string flags;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            flags += argv[i]; // Append the flag to the string
        }
    }
    return flags; // Return the constructed string
}
