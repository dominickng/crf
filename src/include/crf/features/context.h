namespace NLP {
  namespace CRF {
    class Feature;

    typedef std::vector<Feature *> FeaturePtrs;
    class Context {
      public:
        FeaturePtrs features;
        TagPair klasses;
        size_t index;

        Context(void) : features(), klasses(), index(0) { }
        Context(TagPair klasses, const size_t index=0) : features(), klasses(klasses), index(index) { }
        Context(Tag prev, Tag curr, const size_t index=0) : features(), klasses(prev, curr), index(index) { }
    };

    class Contexts {
      private:
        std::vector<Context> contexts;

      public:
        Contexts(void) : contexts() { }
        Contexts(const size_t size)
          : contexts(size) { }

        size_t size(void) const { return contexts.size(); }

        Context &operator[](const size_t index) {
          return contexts[index];
        }

        typedef std::vector<Context>::iterator iterator;
        typedef std::vector<Context>::const_iterator const_iterator;
        typedef std::vector<Context>::reverse_iterator reverse_iterator;
        typedef std::vector<Context>::const_reverse_iterator const_reverse_iterator;

        iterator begin(void) { return contexts.begin(); }
        const_iterator begin(void) const { return contexts.begin(); }
        iterator end(void) { return contexts.end(); }
        const_iterator end(void) const { return contexts.end(); }

        reverse_iterator rbegin(void) { return contexts.rbegin(); }
        const_reverse_iterator rbegin(void) const { return contexts.rbegin(); }
        reverse_iterator rend(void) { return contexts.rend(); }
        const_reverse_iterator rend(void) const { return contexts.rend(); }
    };

    typedef std::vector<Contexts> Instances;
  }
}
