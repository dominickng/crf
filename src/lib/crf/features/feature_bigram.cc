#include "base.h"

#include "config.h"
#include "hashtable.h"
#include "lexicon.h"
#include "tagset.h"
#include "crf/features.h"

namespace NLP {
  namespace CRF {

    namespace Hash = Util::Hasher;
    namespace HT = Util::hashtable;

    /**
     * BigramEntry.
     * Entry object for the BiWordDict hashtable. Each entry stores a type
     * pointer (allowing one BiWordDict to be used for multiple feature types),
     * an Attribute object, two Word objects representing the matching feature
     * values of each part of the bigram, and a pointer to the next chained
     * WordEntry.
     *
     * All feature values stored in this entry are assumed to have been
     * canonized into Word objects, which are thin wrappers around pointers
     * into the lexicon dictionary. Hence, no dynamically sized feature value
     * strings are required.
     */
    class BigramEntry {
      private:
        BigramEntry(const char *type, const Word val1, const Word val2, BigramEntry *next) :
          type(type), val1(val1), val2(val2), next(next) { }

        ~BigramEntry(void) { }

        void *operator new(size_t size, Util::Pool *pool) {
          return pool->alloc(size);
        }

        void operator delete(void *, Util::Pool) { }

      public:
        const char *type;
        const Word val1;
        const Word val2;
        Attribute attrib;
        BigramEntry *next;

        static Hash::Hash hash(const char *type, const Word val1, const Word val2) {
          return Hash::Hash((val2.index() >> 2)*7 + (val1.index() >> 2) + (uint64_t) type);
        }

        static BigramEntry *create(Util::Pool *pool, const uint64_t index,
            const Word &value, const Hash::Hash hash, BigramEntry *next) {
          return NULL;
        }

        static BigramEntry *create(Util::Pool *pool, const char *type,
            const Word &val1, const Word &val2, BigramEntry *next) {
          BigramEntry *entry = new (pool) BigramEntry(type, val1, val2, next);
          return entry;
        }

        bool equal(const char *type, const Word &val1, const Word &val2) {
          return this->type == type && this->val1 == val1 && this->val2 == val2;
        }

        BigramEntry *find(const Hash::Hash hash, const Word &value) {
          return NULL;
        }

        Attribute find(const char *type, const Word &val1, const Word &val2) {
          for (BigramEntry *l = this; l != NULL; l = l->next) {
            if(l->equal(type, val1, val2)) {
              return l->attrib;
            }
          }
          return NONE;
        }

        size_t nchained(void) const {
          return next ? next->nchained() + 1 : 1;
        }
    };

    typedef HT::BaseHashTable<BigramEntry, Word> ImplBase;

    /**
     * BiWordDict::Impl.
     * Private implementation of the BiWordDict as a hashtable.
     */
    class BiWordDict::Impl : public ImplBase, public Util::Shared {
      public:
        const Lexicon lexicon;

        Impl(const size_t nbuckets, const size_t pool_size,
            const Lexicon lexicon) : ImplBase(nbuckets, pool_size),
            Shared(), lexicon(lexicon) { }

        virtual ~Impl(void) { }

        using ImplBase::find;
        using ImplBase::insert;

        Attribute &insert(const char *type, const Word &val1, const Word &val2) {
          size_t bucket = BigramEntry::hash(type, val1, val2).value() % _nbuckets;
          BigramEntry *entry = BigramEntry::create(ImplBase::_pool, type, val1, val2, _buckets[bucket]);
          _buckets[bucket] = entry;
          ++_size;
          return entry->attrib;
        }

        Attribute find(const char *type, const Word &val1, const Word &val2) const {
          return _buckets[BigramEntry::hash(type, val1, val2).value() % _nbuckets]->find(type, val1, val2);
        }
    };

    BiWordDict::BiWordDict(const Lexicon lexicon, const size_t nbuckets,
        const size_t pool_size) :
        FeatureDict(), _impl(new Impl(nbuckets, pool_size, lexicon)) { }

    BiWordDict::BiWordDict(const BiWordDict &other) :
      FeatureDict(), _impl(share(other._impl)) { }

    BiWordDict::~BiWordDict(void) {
      release(_impl);
    }

    Attribute &BiWordDict::load(const Type &type, std::istream &in) {
      Raw val1, val2;
      in >> val1 >> val2;

      return _impl->insert(type.name, _impl->lexicon[val1], _impl->lexicon[val2]);
    }

    Attribute BiWordDict::get(const Type &type, const Raw &raw1, const Raw &raw2) {
      return _impl->find(type.name, _impl->lexicon[raw1], _impl->lexicon[raw2]);
    }

    Attribute &BiWordDict::insert(const Type &type, const Raw &raw1, const Raw &raw2) {
      return _impl->insert(type.name, _impl->lexicon[raw1], _impl->lexicon[raw2]);
    }

  }
}
