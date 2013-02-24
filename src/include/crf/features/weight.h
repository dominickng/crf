namespace NLP {
  namespace CRF {
    struct Weight {
      Tag prev;
      Tag curr;
      double lambda;

      Weight(Tag prev, Tag curr, double lambda) :
        prev(prev), curr(curr), lambda(lambda) { }
    };

    typedef std::vector<Weight> Weights;

    struct Attribute {
      Weight *begin;
      Weight *end;

      Attribute(void) : begin(0), end(0) { }
      Attribute(None) : begin(0), end(0) { }
      Attribute(Weight *begin, Weight *end) : begin(begin), end(end) { }
    };

    typedef std::vector<Weight *> Attribs2Weights;
  }
}
