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
#include "vector.h"
#include "offset_vector.h"
#include "word.h"
#include "tag.h"
#include "raw.h"
#include "input.h"
#include "sentence.h"

#include "pool.h"
#include "shared.h"

inline double
fastlog2(double x) {
  union { double f; uint32_t i; } vx = { x };
  union { uint32_t i; double f; } mx = { (vx.i & 0x007FFFFF) | (0x7e << 23) };
  double y = vx.i;
  y *= 1.0f / (1 << 23);
  return y - 124.22544637f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f);
}

inline double
fastlog(double x) {
  return 0.69314718f * fastlog2(x);
}

inline double
fastpow2(double p) {
  double offset = (p < 0) ? 1.0f : 0.0f;
  double clipp = (p < -126) ? -126.0f : p;
  int w = clipp;
  double z = clipp - w + offset;
  union { uint32_t i; double f; } v = { (uint32_t)((1 << 23) * (clipp + 121.2740838f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z)) };
  return v.f;
}

inline double
fastexp(double p) {
  return fastpow2(1.442695040f * p);
}

inline double
fastpow(double x, double p) {
  return fastpow2(p * fastlog2(x));
}
