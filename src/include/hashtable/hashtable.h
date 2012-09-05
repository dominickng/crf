namespace Util {
  namespace HashTable {
    template <typename Key, typename Value, typename Hash=Hasher::Hash,
             size_t PoolSize=MEDIUM>
    class HashTable : public OrderedHashTable<Key, Value, Hash, PoolSize> {
      protected:
        typedef OrderedHashTable<Key, Value, Hash, PoolSize> Base;
        typedef typename Base::EntryType EntryType;
        typedef typename Base::Entries Entries;

        const float _max_load;
        const float _expansion_factor;

        void expand(void) {
          Pool *old = Base::_pool;
          Base::_pool = new Pool(PoolSize);
          Base::_nbuckets *= _expansion_factor;
          Base::_used_buckets = 0;
          Base::_size = 0;

          for (typename Entries::const_iterator it = Base::_entries.begin();
              it != Base::_entries.end(); ++it)
            Base::add(*it);
        }

        virtual EntryType *insert(const Key &key, const Value &value,
            const Hash hash, const size_t bucket) {
          if (Base::_used_buckets / (double) Base::_nbuckets > _max_load)
            expand();
          EntryType *e = EntryType::create(Base::_pool, Base::_size, key,
              value, hash, Base::_buckets[bucket]);
          Base::_buckets[bucket] = e;
          ++Base::_size;
          return e;
        }

      public:
        HashTable(const size_t nbuckets=BASE_SIZE, const float max_load=0.7,
            const float expansion_factor=2.0) : Base(nbuckets),
          _max_load(max_load), _expansion_factor(expansion_factor) { }

        virtual ~HashTable(void) { }
    };
  }
}
