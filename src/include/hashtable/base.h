namespace Util {
  namespace hashtable {

    template <typename E, typename K, typename Hash=Hasher::Hash>
    class BaseHashTable {
      protected:
        size_t _nbuckets;
        size_t _size;
        size_t _pool_size;
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
        BaseHashTable(const size_t nbuckets=BASE_SIZE,
            const size_t pool_size=SMALL) :
          _nbuckets(nbuckets), _size(0), _used_buckets(0),
          _pool(new Pool(pool_size)), _buckets(new Entry*[_nbuckets]) {
            memset(_buckets, 0, _nbuckets * sizeof(Entry *));
        }

        virtual ~BaseHashTable(void) {
          delete _pool;
          delete [] _buckets;
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

        virtual Entry *find(const Key &key) const {
          const Hash hash(key);
          return _buckets[hash.value() % _nbuckets]->find(hash, key);
        }

        void print_stats(std::ostream &out) const {
          size_t maxchain = 0;
          size_t nbins = 0;
          size_t nbytes = 0;

          for (size_t i = 0; i < _nbuckets; i++) {
            if (_buckets[i]) {
              uint64_t size = _buckets[i]->nchained();
              if (maxchain < size)
                maxchain = size;
              nbins++;
            }
          }

          out << "number of entries " << _size << '\n';
          out << "number of bins used " << nbins << " (of " << _nbuckets<< ")\n";
          out << "used bins/nbins " << nbins/static_cast<float>(_nbuckets) << '\n';
          out << "maximum chain length " << maxchain << '\n';
          out << "average chain length " << _size/static_cast<float>(nbins) << '\n';

          nbytes = _size * sizeof(Entry);
          out << "      entry objs " << nbytes << " bytes\n";
          nbytes += sizeof(_buckets);
          out << "      bin []     " << sizeof(_buckets) << " bytes\n";
          out << "total            " << nbytes << " bytes\n";
        }
    };
  }
}
