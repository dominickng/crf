#include "base.h"

namespace config = Util::config;
namespace port = Util::port;

int run(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  try {
    run(argc, argv);
  }
  catch (config::ConfigException &e) {
    std::cerr << port::RED << e.msg << ": " << e.name;
    if (e.value.size())
      std::cerr << ", " << e.value;
    std::cerr << port::OFF << std::endl;
    exit(1);
  }
}
