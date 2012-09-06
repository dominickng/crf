namespace Util {
  namespace HashTable {

    template <typename E, typename K, typename Hash=Hasher::Hash, size_t PoolSize=MEDIUM>
    class BaseHashTable {
      protected:
        size_t _nbuckets;
        size_t _size;
        size_t _used_buckets;

        typedef E Entry;
        typedef K Key;

        Pool *_pool;
        Entry **_buckets;

        virtual Entry *insert(const Key &key, const Hash hash,
            const size_t bucket) {
          Entry *e = Entry::create(_pool, _size, key, hash, _buckets[bucket]);

          ++_size;
          if (_buckets[bucket] == NULL)
            ++_used_buckets;
          _buckets[bucket] = e;
          return e;
        }

      public:
        BaseHashTable(const size_t nbuckets=BASE_SIZE) :
          _nbuckets(nbuckets), _size(0), _used_buckets(0),
          _pool(new Pool(PoolSize)), _buckets(new Entry*[_nbuckets]) { }

        virtual ~BaseHashTable(void) {
          delete _pool;
          delete _buckets;
        }

        inline size_t size(void) const { return _size; }

        virtual Entry *add(const Key &key) {
          Hash hash(key);
          uint64_t bucket = hash.value() % _nbuckets;
          Entry *e = _buckets[bucket]->find(hash, key);
          if (e)
            return e;
          return insert(key, hash, bucket);
        }

        virtual void clear(void) {
          _size = 0;
          _used_buckets = 0;
          _pool->clear();
          memset(_buckets, 0, _nbuckets * sizeof(Entry *));
        }
    };
  }
}
