#include "base.h"
#include "hash.h"
#include "hashtable/entry.h"

namespace NLP {
  typedef Util::hashtable::StringEntry<uint64_t> WordInfo;

  uint64_t Word::_freq(void) const {
    return reinterpret_cast<WordInfo *>(_id)->value();
  }

  uint64_t Word::_index(void) const {
    return reinterpret_cast<WordInfo *>(_id)->index();
  }

  const char *Word::_str(void) const {
    return reinterpret_cast<WordInfo *>(_id)->str();
  }
}
