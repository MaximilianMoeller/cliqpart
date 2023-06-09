//
// Created by max on 26.05.23.
//

#ifndef CLIQPART_TYPES_H
#define CLIQPART_TYPES_H

#include <string>

enum class Verbosity {
    Error, Warn, Info, Debug, Trace
};

inline std::ostream &operator<<(std::ostream &os, const Verbosity &v) {
    std::string t;
    switch (v) {
        case Verbosity::Error:
            t = "ERROR";
            break;
        case Verbosity::Warn:
            t = "WARN";
            break;
        case Verbosity::Info:
            t = "INFO";
            break;
        case Verbosity::Debug:
            t = "DEBUG";
            break;
        case Verbosity::Trace:
            t = "TRACE";
            break;
    }
    os << t;
    return os;
}

#endif //CLIQPART_TYPES_H
