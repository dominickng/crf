namespace NLP {
  namespace CRF {
    typedef std::vector<uint64_t> Attribs;
    class Context {
      public:
        Attribs attribs;
        TagPair klasses;
        uint64_t freq;
    };

    typedef std::vector<Context> Contexts;
    typedef std::vector<Contexts> Instances;
  }
}
