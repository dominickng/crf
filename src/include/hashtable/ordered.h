namespace Util {
  namespace hashtable {
    template <typename E>
    class KeyCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->key < e2->key;
        }
    };

    template <typename E>
    class RevKeyCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->key > e2->key;
        }
    };

    template <typename E>
    class ValueCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->value < e2->value;
        }
    };

    template <typename E>
    class RevValueCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->value > e2->value;
        }
    };

    template <typename E>
    class IndexCmp {
      public:
        bool operator()(const E *const e1, const E *const e2) {
          return e1->index > e2->index;
        }
    };

    template <typename E, typename K, typename Hash=Hasher::Hash>
    class OrderedHashTable : public BaseHashTable<E, K, Hash> {
      protected:
        typedef E Entry;
        typedef K Key;
        typedef BaseHashTable<Entry, Key, Hash> Base;
        typedef std::vector<Entry *> Entries;
        typedef typename std::vector<Entry *>::iterator iterator;
        typedef typename std::vector<Entry *>::const_iterator const_iterator;

        Entries _entries;

      public:
        OrderedHashTable(const size_t nbuckets=BASE_SIZE,
            const size_t pool_size=SMALL) :
          Base(nbuckets, pool_size), _entries()  { }

        virtual ~OrderedHashTable(void) { }

        virtual Entry *insert(const Key &key, const Hash hash, const size_t bucket) {
          Entry *e = Base::insert(key, hash, bucket);
          _entries.push_back(e);
          return e;
        }

        virtual void clear(void) {
          Base::clear();
          _entries.resize(0);
        }

        void compact(void) {
          iterator new_end = std::remove(_entries.begin(), _entries.end(), reinterpret_cast<Entry *>(0));
          _entries.erase(new_end, _entries.end());
        }

        void compress(void) {
          compact();
          renumber();
        }

        void renumber(void) {
          for (size_t i = 0; i != _entries.size(); ++i)
            _entries[i]->index = i+1;
        }

        template <typename Comparator>
        void sort(Comparator cmp) {
          std::sort(_entries.begin(), _entries.end(), cmp);
        }

        void sort_by_key(void) {
          sort(KeyCmp<Entry>());
          renumber();
        }

        void sort_by_rev_key(void) {
          sort(RevKeyCmp<Entry>());
          renumber();
        }

        void sort_by_value(void) {
          sort(ValueCmp<Entry>());
          renumber();
        }

        void sort_by_rev_value(void) {
          sort(RevValueCmp<Entry>());
          renumber();
        }

        void sort_by_index(void) {
          sort(IndexCmp<Entry>());
          renumber();
        }

        void save(std::ostream &out) const {
          for (typename Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->save(out);
        }
    };
  }
}
