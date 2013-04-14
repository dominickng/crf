#include "base.h"

#include "factor/factor.h"
#include "factor/variable.h"

namespace NLP {
  Factor::Factor(size_t index, size_t nvars)
    : index(index), nvars(nvars), norm(0.0) { }

  void *Factor::operator new(size_t size, Util::Pool *pool, size_t nvars) {
    return pool->alloc(size + sizeof(Variable *) * (nvars - 1));
  }
}
