#ifndef _BASE_H_
#define _BASE_H_

#include "std.h"

namespace NLP {

  struct None {
    public:
      const static char *str;
      const static int val = 0;
      const static std::string::size_type len = 8;
      None(void) { }
  };

  struct Sentinel {
    public:
      const static char *str;
      const static int val = 1;
      const static std::string::size_type len = 12;
      Sentinel(void) { }
  };

  extern const None NONE;
  extern const Sentinel SENTINEL;

}

#include "prob.h"
#include "port.h"
#include "exception.h"
#include "version.h"
#include "raw.h"
#include "input.h"
#include "sentence.h"
#include "word.h"
#include "tag.h"

#include "pool.h"
#include "shared.h"

#endif
