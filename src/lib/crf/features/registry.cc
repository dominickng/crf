#include "base.h"

#include "shared.h"
#include "config.h"
#include "hashtable.h"
#include "lexicon.h"
#include "tagset.h"
#include "lbfgs.h"
#include "crf/features.h"

namespace NLP {
  namespace CRF {

    namespace Hash = Util::Hasher;
    namespace HT = Util::hashtable;
    namespace config = Util::config;

    /**
     * RegEntry class. This class is an entry in the registry hashtable.
     *
     * Each entry contains a reference to a feature type constant, a pointer
     * to a feature generator object for the feature type, a flag indicating
     * whether the feature is active only for rare words, and a pointer
     * to the next RegEntry (for the chaining in the hash table)
     */
    class RegEntry {
      private:
        RegEntry(const Type &type, FeatureGen *gen, const bool rare, RegEntry *next) :
          type(type), gen(gen), rare(rare), next(next) { }

        void *operator new(size_t size, Util::Pool *pool) {
          return pool->alloc(size);
        }

        void operator delete(void *, Util::Pool) { }

      public:
        const Type &type;
        FeatureGen *gen;
        const bool rare;
        RegEntry *next;

        ~RegEntry(void) {
          delete gen;
        }

        static Hash::Hash hash(const char *type) {
          return Hash::Hash(type);
        }

        static Hash::Hash hash(const std::string &type) {
          return Hash::Hash(type);
        }

        static RegEntry *create(Util::Pool *pool, const uint64_t index,
            const char * type, const Hash::Hash hash, RegEntry *next) {
          return NULL;
        }

        static RegEntry *create(Util::Pool *pool, const Type &type,
            FeatureGen *gen, const bool rare, RegEntry *next) {
          RegEntry *entry = new (pool) RegEntry(type, gen, rare, next);
          return entry;
        }

        bool equal(const std::string &type) {
          return type == this->type.name;
        }

        RegEntry *find(const Hash::Hash hash, const char *type) {
          return NULL;
        }

        RegEntry *find(const std::string &type) {
          for (RegEntry *l = this; l != NULL; l = l->next)
            if (l->equal(type))
              return l;
          return NULL;
        }

        size_t nchained(void) const {
          return next ? next->nchained() + 1 : 1;
        }
    };

    typedef HT::BaseHashTable<RegEntry, const char *>ImplBase;

    /**
     * Registry::Impl. The private registry hashtable implementation.
     */
    class Registry::Impl : public ImplBase, public Util::Shared {
      private:
        const uint64_t rare_cutoff;
        typedef std::vector<Entry *> Entries;
        Entries _actives;
      public:
        Impl(const uint64_t rare_cutoff,
            const size_t nbuckets, const size_t pool_size)
          : ImplBase(nbuckets, pool_size), Shared(), rare_cutoff(rare_cutoff),
            _actives() { }

        virtual ~Impl(void) {
          for (Entries::iterator j = _actives.begin(); j != _actives.end(); ++j)
            (*j)->~RegEntry();
        }

        /**
         * reg.
         * Registers a feature type with its generating object.
         *
         * Each feature type is mapped to a RegEntry containing the
         * pointer to the feature generator
         */
        void reg(const Type &type, FeatureGen *gen, const bool active, const bool rare) {
          if (active) {
            size_t bucket = RegEntry::hash(type.name).value() % _nbuckets;
            RegEntry *entry = RegEntry::create(ImplBase::_pool, type, gen, rare, _buckets[bucket]);
            _actives.push_back(entry);
            _buckets[bucket] = entry;
            ++_size;
          }
        }

        using ImplBase::find;

        RegEntry *find(const std::string &type) const {
          return _buckets[RegEntry::hash(type).value() % _nbuckets]->find(type);
        }

        /**
         * get_tagpair.
         *
         * Utility function to canonize tags in a string format to their
         * internal representation.
         *
         * Uses the Sentinel tag for words at the start or end of the sentence.
         */
        void get_tagpair(TagSet tags, Raws &raws, TagPair &tp, int i) {
          if (i == 0) {
            tp.prev = Tag(Sentinel::val);
            tp.curr = tags.canonize(raws[0]);
          }
          else {
            tp.prev = tags.canonize(raws[i-1]);
            tp.curr = tags.canonize(raws[i]);
          }
        }

        /**
         * generate.
         *
         * Given a reference to the attributes dictionary, the lexicon, and
         * tagset, extracts or generates features for the given sentence and
         * tags.
         *
         * Only features that are active for the given context (e.g. if the
         * word falls under the rare word cutoff and the feature is active for
         * rare words only) are extracted.
         *
         * If extract is true, this function calls each active feature
         * generator in turn to extract features from each sentence and map
         * them to the gold tags sequence. This creates attributes and the
         * feature objects associated with those attributes in the attributes
         * dictionary.
         *
         * If extract is false, this function calls each active feature
         * generator in turn and constructs the contexts object used for
         * training. Each context contains a list of pointers to
         * feature objects in the attributes dictionary that are active for
         * that context.
         */
        void generate(Attributes &attributes, Lexicon lexicon, TagSet tags, Sentence &sent, Raws &rawtags, Contexts &contexts, const bool extract) {
          for (size_t i = 0; i < sent.size(); ++i) {
            for (Entries::iterator j = _actives.begin(); j != _actives.end(); ++j) {
              RegEntry *e = *j;
              if (!(e->rare) || lexicon.freq(sent.words[i]) < rare_cutoff) {
                TagPair tp;
                get_tagpair(tags, rawtags, tp, i);
                if (extract)
                  (*e->gen)(e->type, attributes, sent, tp, i);
                else {
                  contexts[i].klasses = tp;
                  contexts[i].index = i;
                  (*e->gen)(e->type, attributes, sent, contexts[i], i);
                  //std::cout << "added " << e->type.name << " at position " << i << ' ' << " for tag " << tp.curr << " nfeatures = " << contexts[i].features.size() << std::endl;
                }
              }
            }
          }
        }

        /**
         * add_features.
         * Adds the weights of active features for a sentence to a probability
         * distribution. Used in tagging.
         */
        void add_features(Lexicon lexicon, Sentence &sent, PDFs &dist, int i) {
          for (Entries::iterator j = _actives.begin(); j != _actives.end(); ++j) {
            RegEntry *e = *j;
            //std::cout << "Adding " << e->type.name << " features for position " << i << std::endl;
            if (!(e->rare) || lexicon.freq(sent.words[i]) < rare_cutoff)
              (*e->gen)(e->type, sent, dist, i);
          }
        }
    };

    Registry::Registry(const uint64_t rare_cutoff, const size_t nbuckets, const size_t pool_size) :
        _impl(new Impl(rare_cutoff, nbuckets, pool_size)) { }

    Registry::Registry(const Registry &other) :
        _impl(share(other._impl)) { }

    Registry::~Registry(void) {
      release(_impl);
    }

    void Registry::reg(const Type &type, FeatureGen *gen, const bool active, const bool rare) {
      _impl->reg(type, gen, active, rare);
    }

    Attribute &Registry::load(const std::string &type, std::istream &in) {
      RegEntry *entry = _impl->find(type);
      return entry->gen->load(entry->type, in);
    }

    void Registry::generate(Attributes &attributes, Lexicon lexicon, TagSet tags, Sentence &sent, Raws &rawtags, Contexts &contexts, const bool extract) {
      _impl->generate(attributes, lexicon, tags, sent, rawtags, contexts, extract);
    }

    void Registry::add_features(Lexicon lexicon, Sentence &sent, PDFs &dist, int i) {
      _impl->add_features(lexicon, sent, dist, i);
    }

  }
}
