namespace NLP {
  namespace CRF {
    class Node {
      public:
        mutable const Node *prev;
        Tag tag;
        double score;

        void *operator new(size_t size, NodePool<Node> *pool) { return pool->alloc(size); }

        void operator delete(void *, NodePool<Node> *) { }

        Node(const Node *prev, Tag tag, double score)
          : prev(prev), tag(tag), score(score) { }
    };

    class Lattice {
      private:
        typedef std::vector<Node *> Nodes;

        NodePool<Node> *pool;
        Nodes nodes;
        const Node *max;
        const uint64_t nklasses;
        const uint64_t nklasses2;

      public:
        Lattice(uint64_t nklasses)
          : pool(new NodePool<Node>()), nodes(), max(NULL), nklasses(nklasses),
            nklasses2(nklasses * nklasses) {
          nodes.reserve(nklasses * 100);
        }

        ~Lattice(void) { delete pool; }

        void viterbi(TagSet &tags, PDFs &dist) {
          const Node *prev_max = max;
          if (nodes.size() == 0) {
            for (int curr = 2; curr < nklasses; ++curr) {
              Node *n = new (pool) Node(NULL, curr, dist[Sentinel::val][curr]);
              nodes.push_back(n);
              if (!max || max->score < n->score)
                max = n;
            }
          }
          else {
            size_t size = nodes.size() - 1;
            const Node *new_max = NULL;
            for (int curr = 2; curr < nklasses; ++curr) {
              double best_score = -std::numeric_limits<double>::max();
              Node *best_prev = NULL;
              for (int prev = 2; prev < nklasses; ++prev) {
                double score = dist[prev][curr] + nodes[size - prev + 2]->score;
                if (score > best_score) {
                  best_score = score;
                  best_prev = nodes[size - prev + 2];
                }
              }
              Node *n = new (pool) Node(best_prev, curr, best_score);
              nodes.push_back(n);
              if (!new_max || new_max->score < n->score)
                new_max = n;
            }
            max = new_max;
          }
        }

        void best(TagSet &tags, Raws &raws, int size) {
          int index = size - 1;
          const Node *m = max;
          raws.resize(size);
          while (index >= 0) {
            raws[index--] = tags.str(m->tag);
            m = m->prev;
          }
        }

        void reset(void) {
          pool->clear();
          nodes.clear();
          max = NULL;
        }

        void print(std::ostream &out, TagSet &tags, size_t nwords) {
          for (int i = 0; i < nklasses - 2; ++i) {
            size_t index = nodes.size() - (nklasses - 2) + i;
            out << std::setw(16) << tags.str(i+2);
            _print(out, nodes[index], max, nwords);
            out << '\n';
          }
        }

        void _print(std::ostream &out, const Node *n, const Node *max, size_t nwords) {
          if (max->prev)
            _print(out, (n-nklasses+2), max->prev, nwords);
          if (max->tag == n->tag)
            out << ' ' << Util::port::RED << std::setprecision(4) << std::setw(10) << n->score << Util::port::OFF;
          else
            out << ' ' << std::setprecision(4) << std::setw(10) << n->score;
        }

    };
  }
}
