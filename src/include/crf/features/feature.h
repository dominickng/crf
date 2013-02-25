namespace NLP {
  namespace CRF {
    /**
     * Feature object.
     * This represents some feature on an attribute associated with a pair
     * of tags. Each feature has an empirical frequency (calculated by summing
     * the occurences of the feature in the training data), a pointer to a
     * lambda that exists in a vector used in optimization, and an expected
     * frequency calculated during L-BFGS optimization.
     *
     * klasses.prev is set to None::val if the feature is a state feature; i.e.
     * it doesn't care about the previous tag.
     */
    class Feature {
      public:
        TagPair klasses;
        uint64_t freq;
        double *lambda;
        double exp;

        Feature(TagPair &klasses, const uint64_t freq=1)
          : klasses(klasses), freq(freq), lambda(0), exp(0.0) { }

        double gradient(double inv_sigma_sq) {
          return -(freq - exp - *lambda * inv_sigma_sq);
        }
    };

    typedef std::vector<Feature> Features;
  }
}
