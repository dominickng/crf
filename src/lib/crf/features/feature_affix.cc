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

    class AffixEntry {
      private:
        AffixEntry(const char *type, AffixEntry *next) :
          type(type), next(next) { }

        ~AffixEntry(void) { }

        void *operator new(size_t size, Util::Pool *pool, size_t len) {
          return pool->alloc(size + len);
        }

        void operator delete(void *, Util::Pool, size_t) { }

      public:
        const char *type;
        Attribute attrib;
        AffixEntry *next;
        char str[1];

        static Hash::Hash hash(const char *type, const std::string &str) {
          std::string s = type;
          s += str;
          return Hash::Hash(s);
        }

        static AffixEntry *create(Util::Pool *pool, const uint64_t index,
            const std::string &str, const Hash::Hash hash, AffixEntry *next) {
          return NULL;
        }

        static AffixEntry *create(Util::Pool *pool, const char *type,
            const std::string &str, AffixEntry *next) {
          AffixEntry *entry = new (pool, str.size()) AffixEntry(type, next);
          strcpy(entry->str, str.c_str());
          return entry;
        }

        bool equal(const char *type, const std::string &str) {
          return this->type == type && this->str == str;
        }

        AffixEntry *find(const Hash::Hash hash, const std::string &str) {
          return NULL;
        }

        Attribute find(const char *type, const std::string &str) {
          for (AffixEntry *l = this; l != NULL; l = l->next)
            if (l->equal(type, str))
              return l->attrib;
          return NONE;
        }

        size_t nchained(void) const {
          return next ? next->nchained() + 1 : 1;
        }
    };

    typedef HT::BaseHashTable<AffixEntry, std::string> ImplBase;

    class AffixDict::Impl : public ImplBase, public Util::Shared {
      public:
        Impl(const size_t nbuckets, const size_t pool_size)
          : ImplBase(nbuckets, pool_size), Shared() { }

        virtual ~Impl(void) { }

        using ImplBase::find;
        using ImplBase::insert;

        Attribute &insert(const char *type, const std::string &str) {
          size_t bucket = AffixEntry::hash(type, str).value() % _nbuckets;
          AffixEntry *entry = AffixEntry::create(ImplBase::_pool, type, str, _buckets[bucket]);
          _buckets[bucket] = entry;
          ++_size;
          return entry->attrib;
        }

        Attribute find(const char *type, const std::string &str) const {
          return _buckets[AffixEntry::hash(type, str).value() % _nbuckets]->find(type, str);
        }
    };

    AffixDict::AffixDict(const size_t nbuckets, const size_t pool_size)
      : FeatureDict(), _impl(new Impl(nbuckets, pool_size)) { }

    AffixDict::AffixDict(const AffixDict &other)
      : FeatureDict(), _impl(share(other._impl)) { }

    AffixDict::~AffixDict(void) {
      release(_impl);
    }

    Attribute &AffixDict::load(const Type &type, std::istream &in) {
      Raw value;
      in >> value;

      return _impl->insert(type.name, value);
    }

    Attribute AffixDict::get(const Type &type, const Raw &raw) {
      return _impl->find(type.name, raw);
    }

    Attribute &AffixDict::insert(const Type &type, const Raw &raw) {
      return _impl->insert(type.name, raw);
    }

    void AffixDict::print_stats(std::ostream &out) {
      _impl->print_stats(out);
    }

  }
}
