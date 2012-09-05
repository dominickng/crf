#include "pool.h"

namespace Util {
  namespace HashTable {

    template <typename Key, typename Value, typename Hash>
      class Entry {
        private:
          uint64_t _index;
          Hash _hash;
          Entry *_next;
          Key _key;
          Value _value;

          Entry(const uint64_t index, const Key &key, const Value &value,
              const Hash &hash, Entry *next) :
            _index(index), _hash(hash), _next(next), _key(key), _value(value) { }

          ~Entry(void) {
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
          inline const Entry *next(void) const { return _next; }
          inline const Key &key(void) const { return _key; }
          inline const Value &value(void) const { return _value; }
          inline Value &value(void) { return _value; }

          Entry *find(const Hash &hash) {
            for (Entry *l = this; l != NULL; l = l->_next)
              if (l->_hash == hash)
                return l;
            return NULL;
          }

          Entry *find(const Hash &hash, const Key &key) {
            for (Entry *l = this; l != NULL; l = l->_next)
              if (l->_hash == hash && l->_key == key)
                return l;
            return NULL;
          }

          static Entry *create(Pool *pool, const uint64_t index, const Key &key,
              const Value &value, const Hash &hash, Entry *next) {
            return new (pool) Entry(index, key, value, hash, next);
          }

          inline bool equals(const Hash &hash, const Key &key) {
            return _hash == hash && _key == key;
          }

          size_t size(void) { return _next ? _next->size() + 1 : 1; }
      };
  }
}
