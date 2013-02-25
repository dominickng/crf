#include "pool.h"

namespace Util {
  namespace hashtable {

    template <typename Value, typename Hash=Hasher::Hash>
    class StringEntry {
      private:
        StringEntry(const uint64_t index, Hash hash, StringEntry *next) :
          index(index), hash(hash), value(), next(next) { }

        ~StringEntry(void) { }

        void *operator new(size_t size, Pool *pool, size_t len) {
          return (void *)pool->alloc(size + len - 1);
        }

        void operator delete(void *, Pool *, size_t) { /* blank */ }

      public:
        uint64_t index;
        const Hash hash;
        Value value;
        StringEntry *next;
        char str[1];

        static StringEntry *create(Pool *pool, const uint64_t index,
            const std::string &str, const Hash hash, StringEntry *next) {
          StringEntry *entry = new (pool, str.size()) StringEntry(index, hash, next);
          strcpy(entry->str, str.c_str());
          return entry;
        }

        bool equal(const Hash hash, const std::string &str) {
          return hash == hash && str == str;
        }

        bool equal(const char c) {
          return str[0] == c && str[1] == '\0';
        }

        StringEntry *find(const Hash hash, const std::string &str) {
          for (StringEntry *l = this; l != NULL; l = l->next)
            if (l->equal(hash, str))
              return l;
          return NULL;
        }

        StringEntry *find(const char c) {
          for (StringEntry *l = this; l != NULL; l = l->next)
            if (l->equal(c))
              return l;
          return NULL;
        }

        size_t nchained(void){ return next ? next->nchained() + 1 : 1; }

        std::ostream &save(std::ostream &out) const {
          return out << str << ' ' << value << '\n';
        }
    };

    template <typename Key, typename Value, typename Hash=Hasher::Hash>
    class KeyValueEntry {
      private:
        KeyValueEntry(const uint64_t index, const Key &key, const Hash &hash,
            KeyValueEntry *next) :
          index(index), hash(hash), next(next), key(key), value() { }

        ~KeyValueEntry(void) {
          if (next)
            delete *next;
        }

        void *operator new(size_t size, Pool *pool) {
          return (void *)pool->alloc(size);
        }

        void operator delete(void *, Pool *) { }

      public:
        uint64_t index;
        Hash hash;
        KeyValueEntry *next;
        Key key;
        Value value;

        KeyValueEntry *find(const Hash &hash) {
          for (KeyValueEntry *l = this; l != NULL; l = l->next)
            if (l->hash == hash)
              return l;
          return NULL;
        }

        KeyValueEntry *find(const Hash &hash, const Key &key) {
          for (KeyValueEntry *l = this; l != NULL; l = l->next)
            if (l->hash == hash && l->key == key)
              return l;
          return NULL;
        }

        static KeyValueEntry *create(Pool *pool, const uint64_t index,
            const Key &key, const Hash &hash, KeyValueEntry *next) {
          return new (pool) KeyValueEntry(index, key, hash, next);
        }

        inline bool equals(const Hash &hash, const Key &key) {
          return hash == hash && key == key;
        }

        size_t nchained(void) { return next ? next->nchained() + 1 : 1; }
      };
  }
}
