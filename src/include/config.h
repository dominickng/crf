#ifndef _CONFIG_H
#define _CONFIG_H

#include "config/base.h"
#include "config/group.h"
#include "config/option.h"
#include "config/info.h"

namespace Util {
  namespace config {
    class Main : public Config {
      public:
        OpInput input;
        OpOutput output;

        Main(const std::string &program_name, const std::string &desc)
          : Config(program_name, desc),
            input(*this, "input", "input file location", false),
            output(*this, "output", "output file location", false) { }
    };
  }
}

#endif
