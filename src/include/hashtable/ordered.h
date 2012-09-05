namespace Util {
  namespace HashTable {
    template <typename E>
    class KeyCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->key() < e2->key();
        }
    };

    template <typename E>
    class RevKeyCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->key() > e2->key();
        }
    };

    template <typename E>
    class ValueCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->value() < e2->value();
        }
    };

    template <typename E>
    class RevValueCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->value() > e2->value();
        }
    };

    template <typename E>
    class IndexCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->index() > e2->index();
        }
    };

    template <typename Key, typename Value, typename Hash=Hasher::Hash,
             size_t PoolSize=MEDIUM>
    class OrderedHashTable : public BaseHashTable<Key, Value, Hash, PoolSize> {
      protected:
        typedef BaseHashTable<Key, Value, Hash, PoolSize> Base;
        typedef typename Base::EntryType EntryType;
        typedef std::vector<EntryType *> Entries;
        typedef typename std::vector<EntryType *>::iterator iterator;
        typedef typename std::vector<EntryType *>::const_iterator const_iterator;

        Entries _entries;

      public:
        OrderedHashTable(const size_t nbuckets=BASE_SIZE) :
          Base(nbuckets), _entries()  { }

        virtual ~OrderedHashTable(void) { }

        virtual EntryType *insert(const Key &key, const Value &value,
            const Hash hash, const size_t bucket) {
          EntryType * e = Base::insert(key, value, hash, bucket);
          _entries.push_back(e);
          return e;
        }

        virtual void clear(void) {
          Base::clear();
          _entries.resize(0);
        }

        void compact(void) {
          iterator new_end = std::remove(_entries.begin(), _entries.end(), reinterpret_cast<EntryType *>(0));
          _entries.erase(new_end, _entries.end());
        }

        void compress(void) {
          compact();
          renumber();
        }

        void renumber(void) {
          for (size_t i = 0; i != _entries.size(); ++i)
            _entries[i]->index() = i;
        }

        template <typename Comparator>
        void sort(Comparator cmp) {
          std::sort(_entries.begin(), _entries.end(), cmp);
        }

        void sort_by_key(void) {
          sort(KeyCmp<EntryType>());
          renumber();
        }

        void sort_by_rev_key(void) {
          sort(RevKeyCmp<EntryType>());
          renumber();
        }

        void sort_by_value(void) {
          sort(ValueCmp<EntryType>());
          renumber();
        }

        void sort_by_rev_value(void) {
          sort(RevValueCmp<EntryType>());
          renumber();
        }

        void sort_by_index(void) {
          sort(IndexCmp<EntryType>());
          renumber();
        }
    };
  }
}
