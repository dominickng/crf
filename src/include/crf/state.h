namespace NLP {
  namespace CRF {
    class State {
      public:
        OffsetWords words;
        OffsetTags pos;
        OffsetTags entities;
        Lattice lattice;
        PDFs dist;

        State(const size_t ntags)
          : words(), pos(), entities(), lattice(ntags), dist() {
          for (int i = 0; i < ntags; ++i)
            dist.push_back(PDF(ntags, 0.0));
        }

        void reset(void) {
          lattice.reset();
          for (int i = 0; i < dist.size(); ++i)
            std::fill(dist[i].begin(), dist[i].end(), 0.0);
        }
    };
  }
}
