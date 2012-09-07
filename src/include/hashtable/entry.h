#include "pool.h"

namespace Util {
  namespace HashTable {

    template <typename Value, typename Hash=Hasher::Hash>
    class StringEntry {
      private:
        uint64_t _index;
        StringEntry *_next;
        Value _value;
        const Hash _hash;
        char _str[1];

        StringEntry(const uint64_t index, Hash hash, StringEntry *next) :
          _index(index), _hash(hash), _value(), _next(next) { }

        ~StringEntry(void) { }

        void *operator new(size_t size, Pool *pool, size_t len) {
          return (void *)pool->alloc(size + len - 1);
        }

        void operator delete(void *, Pool *, size_t) { /* blank */ }

      public:
        inline const Hash hash(void) const { return _hash; }
        inline const uint64_t index(void) const { return _index; }
        inline uint64_t index(void) { return _index; }
        inline void index(const uint64_t index) { _index = index; }
        inline const StringEntry *next(void) const { return _next; }
        inline const Value &value(void) const { return _value; }
        inline Value &value(void) { return _value; }
        inline void value(const Value &v) { _value = v; }
        inline const char *str(void) const { return _str; }

        static StringEntry *create(Pool *pool, const uint64_t index,
            const std::string &str, const Hash hash, StringEntry *next) {
          StringEntry *entry = new (pool, str.size()) StringEntry(index, hash, next);
          strcpy(entry->_str, str.c_str());
          return entry;
        }

        bool equal(const Hash hash, const std::string &str) {
          return hash == _hash && str == _str;
        }

        bool equal(const char c) {
          return _str[0] == c && _str[1] == '\0';
        }

        StringEntry *find(const Hash hash, const std::string &str) {
          for (StringEntry *l = this; l != 0; l = l->_next)
            if (l->equal(hash, str))
              return l;
          return NULL;
        }

        StringEntry *find(const char c) {
          for (StringEntry *l = this; l != 0; l = l->_next)
            if (l->equal(c))
              return l;
          return NULL;
        }

        size_t size(void){ return _next ? _next->size() + 1 : 1; }

        std::ostream &save(std::ostream &out) const {
          return out << _str << ' ' << _value;
        }
    };

    template <typename Key, typename Value, typename Hash=Hasher::Hash>
    class KeyValueEntry {
      private:
        uint64_t _index;
        Hash _hash;
        KeyValueEntry *_next;
        Key _key;
        Value _value;

        KeyValueEntry(const uint64_t index, const Key &key, const Hash &hash,
            KeyValueEntry *next) :
          _index(index), _hash(hash), _next(next), _key(key), _value() { }

        ~KeyValueEntry(void) {
          if (_next)
            delete *_next;
        }

        void *operator new(size_t size, Pool *pool) {
          return (void *)pool->alloc(size);
        }

        void operator delete(void *, Pool *) { }

      public:
        inline const Hash hash(void) const { return _hash; }
        inline const uint64_t index(void) const { return _index; }
        inline uint64_t index(void) { return _index; }
        inline void index(const uint64_t index) { _index = index; }
        inline const KeyValueEntry *next(void) const { return _next; }
        inline const Key &key(void) const { return _key; }
        inline const Value &value(void) const { return _value; }
        inline Value &value(void) { return _value; }
        inline void value(const Value &v) { _value = v; }

        KeyValueEntry *find(const Hash &hash) {
          for (KeyValueEntry *l = this; l != NULL; l = l->_next)
            if (l->_hash == hash)
              return l;
          return NULL;
        }

        KeyValueEntry *find(const Hash &hash, const Key &key) {
          for (KeyValueEntry *l = this; l != NULL; l = l->_next)
            if (l->_hash == hash && l->_key == key)
              return l;
          return NULL;
        }

        static KeyValueEntry *create(Pool *pool, const uint64_t index,
            const Key &key, const Hash &hash, KeyValueEntry *next) {
          return new (pool) KeyValueEntry(index, key, hash, next);
        }

        inline bool equals(const Hash &hash, const Key &key) {
          return _hash == hash && _key == key;
        }

        size_t size(void) { return _next ? _next->size() + 1 : 1; }
      };
  }
}
