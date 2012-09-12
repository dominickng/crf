#ifndef _HASHTABLE_H
#define _HASHTABLE_H

namespace Util {
  namespace hashtable {
    template <typename Key, typename Value, typename Hash=Hasher::Hash>
    class HashTable : public OrderedHashTable<KeyValueEntry<Key, Value, Hash>,
        Key, Hash> {
      protected:
        typedef KeyValueEntry<Key, Value, Hash> Entry;
        typedef OrderedHashTable<Entry, Key, Hash> Base;
        typedef typename Base::Entries Entries;

        const float _max_load;
        const float _expansion_factor;

        void expand(void) {
          Pool *old = Base::_pool;
          Base::_pool = new Pool(Base::_pool_size);
          Base::_nbuckets *= _expansion_factor;
          Base::_used_buckets = 0;
          Base::_size = 0;

          for (typename Entries::const_iterator it = Base::_entries.begin();
              it != Base::_entries.end(); ++it)
            Base::add(*it);
        }

        using Base::insert;
        virtual Entry *insert(const Key &key, const Value &value,
            const Hash hash, const size_t bucket) {
          Entry *e = insert(key, hash, bucket);
          e->value = value;
          return e;
        }

      public:
        HashTable(const size_t nbuckets=BASE_SIZE,
            const size_t pool_size=SMALL, const float max_load=0.7,
            const float expansion_factor=2.0) : Base(nbuckets, pool_size),
          _max_load(max_load), _expansion_factor(expansion_factor) { }

        virtual ~HashTable(void) { }

        virtual Entry *add(const Key &key, const Value &value) {
          Hash hash(key);
          uint64_t bucket = hash.value() % Base::_nbuckets;
          Entry *e = Base::_buckets[bucket]->find(hash, key);
          if (e)
            return e;
          return insert(key, value, hash, bucket);
        }

        const Entry *operator[](const Key &key) const {
          const Hash hash(key);
          uint64_t bucket = hash.value() % Base::_nbuckets;
          Entry *e = Base::_buckets[bucket]->find(hash, key);
          if (!e) {
            Value v;
            return insert(key, v, hash, bucket);
          }
          return e->value;
        }

        Value &operator[](const Key &key) {
          const Hash hash(key);
          uint64_t bucket = hash.value() % Base::_nbuckets;
          Entry *e = Base::_buckets[bucket]->find(hash, key);
          if (!e) {
            Value v;
            return insert(key, v, hash, bucket)->value;
          }
          return e->value;
        }

    };
  }
}

#endif
