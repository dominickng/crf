namespace NLP {
  class Factor;

  class Variable {
    public:
      size_t index;
      size_t ntags;
      size_t tag_offset;
      size_t nfactors;
      Factor *factors[1];

      Variable(size_t index, size_t ntags, size_t tag_offset, size_t nfactors);

      void *operator new(size_t size, Util::Pool *pool, size_t nfactors);

      void operator delete(void *, Util::Pool *, size_t) { /* do nothing */}

  };
}
