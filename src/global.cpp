#include "feder/global.hpp"

void feder::fatal(const std::string &msg) {
  std::cerr << "Fatal error: " << msg << std::endl;
  exit(1);
}
