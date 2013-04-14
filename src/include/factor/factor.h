namespace NLP {
  class Variable;

  class Factor {
    public:
      size_t index;
      size_t nvars;
      double norm;
      Variable *variables[1];

      Factor(size_t index, size_t nvars);

      void *operator new(size_t size, Util::Pool *pool, size_t nvars);
      void operator delete(void *, Util::Pool *, size_t) { /* do nothing */}

  };
}
