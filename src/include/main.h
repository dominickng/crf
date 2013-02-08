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
  catch (IOException &e) {
    std::cerr << port::RED << e.msg;
    if (e.uri.size())
      std::cerr << ", " << e.uri << " " << e.line;
    std::cerr << port::OFF << std::endl;
    exit(1);
  }
  catch (FormatException &e) {
    std::cerr << port::RED << e.msg;
    if (e.c)
      std::cerr << " " << e.c;
    std::cerr << port::OFF << std::endl;
    exit(1);
  }
  catch (Exception &e) {
    std::cerr << port::RED << e.msg;
    std::cerr << port::OFF << std::endl;
    exit(1);
  }
}
