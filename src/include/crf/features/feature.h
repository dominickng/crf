namespace NLP {
  namespace CRF {
    class Feature {
      public:
        TagPair klasses;
        uint64_t freq;
        double *lambda;
        double exp;
        bool is_trans;

        Feature(TagPair &klasses, const uint64_t freq=1, const bool is_trans=false)
          : klasses(klasses), freq(freq), lambda(0), exp(0.0), is_trans(is_trans) { }

        double gradient(double inv_sigma_sq) {
          return -(freq - exp - *lambda * inv_sigma_sq);
        }
    };

    typedef std::vector<Feature> Features;
  }
}
