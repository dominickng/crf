#ifndef _BASE_H_
#define _BASE_H_

#include "std.h"

namespace NLP {

  struct None {
    public:
      const static std::string str;
      const static uint64_t val = 0;
      const static std::string::size_type len = 8;
      None(void) { }
  };

  struct Sentinel {
    public:
      const static std::string str;
      const static uint64_t val = 1;
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
#include "offset_vector.h"
#include "word.h"
#include "tag.h"
#include "raw.h"
#include "input.h"
#include "sentence.h"

#include "pool.h"
#include "shared.h"

#endif
