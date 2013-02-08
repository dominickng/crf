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

    class RegEntry {
      private:
        RegEntry(const Type &type, FeatureGen *gen, RegEntry *next) :
          type(type), gen(gen), next(next) { }

        ~RegEntry(void) {
          delete gen;
        }

        void *operator new(size_t size, Util::Pool *pool) {
          return pool->alloc(size);
        }

        void operator delete(void *, Util::Pool) { }

      public:
        const Type &type;
        FeatureGen *gen;
        RegEntry *next;

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
            FeatureGen *gen, RegEntry *next) {
          RegEntry *entry = new (pool) RegEntry(type, gen, next);
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

    class Registry::Impl : public ImplBase, public Util::Shared {
      private:
        typedef std::vector<Entry *> Entries;
        Entries _actives;
      public:
        Impl(const size_t nbuckets, const size_t pool_size)
          : ImplBase(nbuckets, pool_size), Shared(), _actives() { }

        virtual ~Impl(void) { }

        void reg(const Type &type, config::Op<bool> &op, FeatureGen *gen) {
          if (op()) {
            size_t bucket = RegEntry::hash(type.name).value() % _nbuckets;
            RegEntry *entry = RegEntry::create(ImplBase::_pool, type, gen, _buckets[bucket]);
            _actives.push_back(entry);
            _buckets[bucket] = entry;
            ++_size;
          }
        }

        using ImplBase::find;

        RegEntry *find(const std::string &type) const {
          return _buckets[RegEntry::hash(type).value() % _nbuckets]->find(type);
        }

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

        void generate(Attributes &attributes, TagSet tags, Sentence &sent, Raws &rawtags, Contexts &contexts, const bool extract) {
          for (size_t i = 0; i < sent.size(); ++i) {
            for (Entries::iterator j = _actives.begin(); j != _actives.end(); ++j) {
              RegEntry *e = *j;
              TagPair tp;
              get_tagpair(tags, rawtags, tp, i);
              if (extract)
                (*e->gen)(e->type, attributes, sent, tp, i);
              else {
                contexts[i].klasses = tp;
                contexts[i].index = i;
                (*e->gen)(e->type, attributes, sent, contexts[i], i);
              }
            }
          }
        }

        void add_features(Sentence &sent, PDFs &dist, int i) {
          for (Entries::iterator j = _actives.begin(); j != _actives.end(); ++j) {
            RegEntry *e = *j;
            (*e->gen)(e->type, sent, dist, i);
          }
        }
    };

    Registry::Registry(const size_t nbuckets, const size_t pool_size) :
        _impl(new Impl(nbuckets, pool_size)) { }

    Registry::Registry(const Registry &other) :
        _impl(share(other._impl)) { }

    Registry::~Registry(void) {
      release(_impl);
    }

    void Registry::reg(const Type &type, config::Op<bool> &op, FeatureGen *gen) {
      _impl->reg(type, op, gen);
    }

    Attribute &Registry::load(const std::string &type, std::istream &in) {
      RegEntry *entry = _impl->find(type);
      return entry->gen->load(entry->type, in);
    }

    void Registry::generate(Attributes &attributes, TagSet tags, Sentence &sent, Raws &rawtags, Contexts &contexts, const bool extract) {
      _impl->generate(attributes, tags, sent, rawtags, contexts, extract);
    }

    void Registry::add_features(Sentence &sent, PDFs &dist, int i) {
      _impl->add_features(sent, dist, i);
    }

  }
}
