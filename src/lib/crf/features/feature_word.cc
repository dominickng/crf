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

    class WordEntry {
      private:
        WordEntry(const char *type, const Word value, WordEntry *next) :
          type(type), value(value), next(next) { }

        ~WordEntry(void) { }

        void *operator new(size_t size, Util::Pool *pool) {
          return pool->alloc(size);
        }

        void operator delete(void *, Util::Pool) { }

      public:
        const char *type;
        const Word value;
        Attribute attrib;
        WordEntry *next;

        static Hash::Hash hash(const char *type, const Word value) {
          return Hash::Hash((value.index() >> 2)*7 + type);
        }

        static WordEntry *create(Util::Pool *pool, const uint64_t index,
            const Word &value, const Hash::Hash hash, WordEntry *next) {
          return NULL;
        }

        static WordEntry *create(Util::Pool *pool, const char *type,
            const Word &value, WordEntry *next) {
          WordEntry *entry = new (pool) WordEntry(type, value, next);
          return entry;
        }

        bool equal(const char *type, const Word &value) {
          return this->type == type && this->value == value;
        }

        WordEntry *find(const Hash::Hash hash, const Word &value) {
          return NULL;
        }

        Attribute find(const char *type, const Word &value) {
          for (WordEntry *l = this; l != NULL; l = l->next) {
            if(l->equal(type, value))
              return l->attrib;
          }
          return NONE;
        }

        size_t nchained(void) const {
          return next ? next->nchained() + 1 : 1;
        }
    };

    typedef HT::BaseHashTable<WordEntry, Word> ImplBase;

    class WordDict::Impl : public ImplBase, public Util::Shared {
      public:
        const Lexicon lexicon;

        Impl(const size_t nbuckets, const size_t pool_size,
            const Lexicon lexicon) : ImplBase(nbuckets, pool_size),
            Shared(), lexicon(lexicon) { }

        virtual ~Impl(void) { }

        Attribute &insert(const char *type, const Word &value) {
          size_t bucket = WordEntry::hash(type, value).value() % _nbuckets;
          WordEntry *entry = WordEntry::create(ImplBase::_pool, type, value, _buckets[bucket]);
          _buckets[bucket] = entry;
          ++_size;
          return entry->attrib;
        }

        Attribute find(const char *type, const Word &value) const {
          return ImplBase::_buckets[WordEntry::hash(type, value).value() % ImplBase::_nbuckets]->find(type, value);
        }
    };

    WordDict::WordDict(const Lexicon lexicon, const size_t nbuckets,
        const size_t pool_size) :
        FeatureDict(), _impl(new Impl(nbuckets, pool_size, lexicon)) { }

    WordDict::WordDict(const WordDict &other) :
      FeatureDict(), _impl(share(other._impl)) { }

    WordDict::~WordDict(void) {
      release(_impl);
    }

    Attribute &WordDict::load(const std::string &type, std::istream &in) {
      Raw value;
      in >> value;

      return _impl->insert(type.c_str(), _impl->lexicon[value]);
    }

    Attribute WordDict::get(const Type &type, Raw &raw) {
      return _impl->find(type.id, _impl->lexicon[raw]);
    }

    Attribute &WordDict::insert(const Type &type, Raw &raw) {
      return _impl->insert(type.id, _impl->lexicon[raw]);
    }

  }
}
