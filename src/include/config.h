#ifndef _CONFIG_H
#define _CONFIG_H

#include "base.h"

#include "config/base.h"
#include "config/group.h"
#include "config/option.h"

namespace Util {
  namespace config {
    class Main : public Config {
      public:
        Main(const std::string &program_name, const std::string &desc)
          : Config(program_name, desc) { }
    };
  }
}

#endif
