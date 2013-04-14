#include "base.h"

#include "factor/factor.h"
#include "factor/variable.h"

namespace NLP {
  Variable::Variable(size_t index, size_t ntags, size_t tag_offset, size_t nfactors)
    : index(index), ntags(ntags), tag_offset(tag_offset), nfactors(nfactors) { }

  void *Variable::operator new(size_t size, Util::Pool *pool, size_t nfactors) {
    return pool->alloc(size + sizeof(Factor *) * (nfactors - 1));
  }
}
