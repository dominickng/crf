namespace NLP {
  namespace CRF {
    class Feature;

    typedef std::vector<Feature *> FeaturePtrs;
    class Context {
      public:
        FeaturePtrs features;
        TagPair klasses;
        uint64_t freq;

        Context(void) : features(), klasses(), freq(0) { }
        Context(TagPair klasses) : features(), klasses(klasses), freq(0) { }
        Context(Tag prev, Tag curr) : features(), klasses(prev, curr), freq(0) { }
    };

    typedef std::vector<Context> Contexts;
    typedef std::vector<Contexts> Instances;
  }
}
