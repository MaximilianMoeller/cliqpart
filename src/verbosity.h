//
// Created by max on 26.05.23.
//

#ifndef CLIQPART_SRC_VERBOSITY_H_
#define CLIQPART_SRC_VERBOSITY_H_

#include <string>

enum class Verbosity { ERROR, WARN, INFO, DEBUG, TRACE };

inline std::ostream &operator<<(std::ostream &os, const Verbosity &v) {
  std::string t;
  switch (v) {
	case Verbosity::ERROR:t = "ERROR";
	  break;
	case Verbosity::WARN:t = "WARN";
	  break;
	case Verbosity::INFO:t = "INFO";
	  break;
	case Verbosity::DEBUG:t = "DEBUG";
	  break;
	case Verbosity::TRACE:t = "TRACE";
	  break;
  }
  os << t;
  return os;
}

#endif // CLIQPART_SRC_VERBOSITY_H_
