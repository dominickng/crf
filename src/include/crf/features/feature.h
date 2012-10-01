namespace NLP {
  namespace CRF {
    class Feature {
      public:
        TagPair klasses;
        uint64_t freq;
        double lambda;
        double est;

        Feature(TagPair &klasses, const uint64_t freq=1)
          : klasses(klasses), freq(freq), lambda(0.0), est(0.0) { }
    };

    typedef std::vector<Feature> Features;
  }
}
