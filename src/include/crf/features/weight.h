/**
 * weight.h
 * Defines the utility classes used for storing feature lambdas at tagging time
 */
namespace NLP {
  namespace CRF {
    /**
     * Weight.
     * This object is a feature with a given lambda on tags (prev, curr),
     * where prev may be None::val to indicate a state feature.
     *
     * Weights are stored in a single vector. As the features file is indexed
     * by attribute id, we are guaranteed that all the features associated
     * with an attribute id are read in contiguously, so we can simply store
     * a beginning and end pointer into the weights vector to indicate the
     * start and end of the appropriate features.
     */
    struct Weight {
      Tag prev;
      Tag curr;
      lbfgsfloatval_t lambda;

      Weight(Tag prev, Tag curr, lbfgsfloatval_t lambda) :
        prev(prev), curr(curr), lambda(lambda) { }
    };

    typedef std::vector<Weight> Weights;

    /**
     * Attribute.
     * This object represents an attribute extracted at training time. Each
     * Attribute simply stores pointers to the first and one past the last
     * Weight object associated with the Attribute in the Weights vector. At
     * tagging time, the feature generator and feature dictionary are
     * responsible for mapping between the feature value extracted from the
     * Sentence object to the appropriate Attribute object, which can then
     * be used to access all features seen with that attribute in training.
     */
    struct Attribute {
      Weight *begin;
      Weight *end;

      Attribute(void) : begin(0), end(0) { }
      Attribute(None) : begin(0), end(0) { }
      Attribute(Weight *begin, Weight *end) : begin(begin), end(end) { }
    };

    /**
     * Attribs2Weights. This is a utilty vector that will be used in loading
     * the model for tagging. Each entry in this vector is simply the first
     * Weight for each Attribute.
     */
    typedef std::vector<Weight *> Attribs2Weights;
  }
}
