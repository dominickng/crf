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

        double gradient(double inv_sigma_sq) {
          return -(freq - est - lambda * inv_sigma_sq);
        }
    };

    typedef std::vector<Feature> Features;
  }
}
