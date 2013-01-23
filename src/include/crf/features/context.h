namespace NLP {
  namespace CRF {
    class Feature;

    typedef std::vector<Feature *> FeaturePtrs;
    class Context {
      public:
        FeaturePtrs features;
        TagPair klasses;
        uint64_t freq;
        size_t index;

        Context(void) : features(), klasses(), freq(0), index(0) { }
        Context(TagPair klasses, const size_t index=0) : features(), klasses(klasses), freq(0), index(index) { }
        Context(Tag prev, Tag curr, const size_t index=0) : features(), klasses(prev, curr), freq(0), index(index) { }
    };

    class Contexts {
      private:
        std::vector<Context> contexts;

      public:
        mutable PDFs alphas;
        mutable PDFs betas;
        mutable PSIs psis;
        mutable PSIs ind_psis;
        mutable PDFs trans_probs;
        mutable PDF scale;

        Contexts(void) : contexts(), alphas(), betas(), psis() { }
        Contexts(const size_t size, const size_t ntags)
          : contexts(size), alphas(), betas(), psis(), ind_psis(), trans_probs(), scale() {
          for (int i = 0; i < ntags; ++i)
            trans_probs.push_back(PDF(ntags, 0.0));
          for (int i = 0; i < size; ++i) {
            alphas.push_back(PDF(ntags, 0.0));
            betas.push_back(PDF(ntags, 0.0));
            scale.push_back(1.0);
            psis.push_back(PDFs(0));
            ind_psis.push_back(PDFs(0));
            for (int j = 0; j < ntags; ++j) {
              psis[i].push_back(PDF(ntags, 1.0));
              ind_psis[i].push_back(PDF(ntags, 1.0));
            }
          }
        }

        void reset(void) {
          std::fill(scale.begin(), scale.end(), 1.0);
          for (int i = 0; i < trans_probs.size(); ++i)
            std::fill(trans_probs[i].begin(), trans_probs[i].end(), 0.0);

          for (int i = 0; i < alphas.size(); ++i) {
            std::fill(alphas[i].begin(), alphas[i].end(), 0.0);
            std::fill(betas[i].begin(), betas[i].end(), 0.0);
            for (int j = 0; j < psis[i].size(); ++j) {
              std::fill(psis[i][j].begin(), psis[i][j].end(), 1.0);
              std::fill(ind_psis[i][j].begin(), ind_psis[i][j].end(), 1.0);
            }
          }
        }

        size_t size(void) const { return contexts.size(); }

        Context &operator[](const size_t index) {
          return contexts[index];
        }

        typedef std::vector<Context>::iterator iterator;
        typedef std::vector<Context>::const_iterator const_iterator;
        typedef std::vector<Context>::reverse_iterator reverse_iterator;
        typedef std::vector<Context>::const_reverse_iterator const_reverse_iterator;

        iterator begin(void) { return contexts.begin(); }
        const_iterator begin(void) const { return contexts.begin(); }
        iterator end(void) { return contexts.end(); }
        const_iterator end(void) const { return contexts.end(); }

        reverse_iterator rbegin(void) { return contexts.rbegin(); }
        const_reverse_iterator rbegin(void) const { return contexts.rbegin(); }
        reverse_iterator rend(void) { return contexts.rend(); }
        const_reverse_iterator rend(void) const { return contexts.rend(); }
    };

    typedef std::vector<Contexts> Instances;
  }
}
