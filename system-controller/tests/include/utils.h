#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>
#include <sstream>
#include "limits.h"

namespace utils {
    static std::string getApplicationDirectory() {
        char result[ PATH_MAX ];
        ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
        std::string appPath = std::string( result, (count > 0) ? count : 0 );

        std::size_t found = appPath.find_last_of("/\\");
        return appPath.substr(0,found);
    }
}
#endif


