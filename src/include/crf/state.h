namespace NLP {
  namespace CRF {
    class State {
      public:
        Lattice lattice;
        PDFs dist;

        State(const size_t ntags)
          : lattice(ntags), dist() {
          for (size_t i = 0; i < ntags; ++i)
            dist.push_back(PDF(ntags, 0.0));
        }

        void reset(void) {
          lattice.reset();
          next_word();
        }

        void next_word(void) {
          for (size_t i = 0; i < dist.size(); ++i)
            std::fill(dist[i].begin(), dist[i].end(), 0.0);
        }
    };
  }
}
