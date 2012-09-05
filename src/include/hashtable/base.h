namespace Util {
  namespace HashTable {

    template <typename Key, typename Value, typename Hash=Hasher::Hash,
             size_t PoolSize=MEDIUM>
    class BaseHashTable {
      protected:
        size_t _nbuckets;
        size_t _size;
        size_t _used_buckets;

        typedef Entry<Key, Value, Hash> EntryType;

        Pool *_pool;
        EntryType **_buckets;

        virtual EntryType *insert(const Key &key, const Value &value,
            const Hash hash, const size_t bucket) {
          EntryType *e = EntryType::create(_pool, _size, key, value, hash, _buckets[bucket]);

          ++_size;
          if (_buckets[bucket] == NULL)
            ++_used_buckets;
          _buckets[bucket] = e;
          return e;
        }

      public:
        BaseHashTable(const size_t nbuckets=BASE_SIZE) :
          _nbuckets(nbuckets), _size(0), _used_buckets(0),
          _pool(new Pool(PoolSize)), _buckets(new (_pool) EntryType*[_nbuckets]) { }

        virtual ~BaseHashTable(void) { delete _pool; }

        inline size_t size(void) const { return _size; }

        virtual EntryType *add(const Key &key, const Value &value) {
          Hash hash(key);
          uint64_t bucket = hash.value() % _nbuckets;
          EntryType *e = _buckets[bucket]->find(hash, key);
          if (e)
            return e;
          return insert(key, value, hash, bucket);
        }

        virtual EntryType *add(EntryType *entry) {
          Hash hash(entry->key());
          uint64_t bucket = hash.value() % _nbuckets;
          EntryType *e = _buckets[bucket]->find(hash, entry->key());
          if (e)
            return e;
          return insert(entry->key(), entry->value(), hash, bucket);
        }

        const Value &operator[](const Key &key) const {
          Hash hash(key);
          uint64_t bucket = hash.value() % _nbuckets;
          EntryType *e = _buckets[bucket]->find(hash, key);
          if (!e) {
            Value v;
            return insert(key, v, hash, bucket)->value();
          }
          return e->value();
        }

        Value &operator[](const Key &key) {
          Hash hash(key);
          uint64_t bucket = hash.value() % _nbuckets;
          EntryType *e = _buckets[bucket]->find(hash, key);
          if (!e) {
            Value v;
            return insert(key, v, hash, bucket)->value();
          }
          return e->value();
        }

        virtual void clear(void) {
          _size = 0;
          _used_buckets = 0;
          _pool->clear();
          memset(_buckets, 0, _nbuckets * sizeof(EntryType *));
        }
    };
  }
}
