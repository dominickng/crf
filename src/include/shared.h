#ifndef _SHARED_H
#define _SHARED_H

namespace Util {
  class Shared {
    private:
      uint64_t _nrefs;

    public:
      Shared(void): _nrefs(1) { };
      ~Shared(void) { }

      void inc_ref(void) { ++_nrefs; }
      bool dec_ref(void) { return --_nrefs == 0; }
  };
}

template <typename T>
T *share(T *t) {
  t->inc_ref();
  return t;
}

template <typename T>
T *release( T *&t) {
  if (t && t->dec_ref()) {
    delete t;
    t = 0;
  }
  return t;
}

#endif
