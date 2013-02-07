namespace NLP {
  namespace CRF {
    class State {
      public:
        OffsetWords words;
        OffsetTags pos;
        OffsetTags entities;

        State(void) : words(), pos(), entities() { }
    };
  }
}
