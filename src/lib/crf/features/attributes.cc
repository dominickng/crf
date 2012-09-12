#ifndef _LEXICON_H
#define _LEXICON_H

#include "base.h"
#include "hashtable.h"
#include "crf/features/feature.h"
#include "crf/features/attributes.h"

namespace NLP {
  namespace CRF {
    namespace Hash = Util::Hasher;

    class AttribEntry {
      private:
        AttribEntry(const char *type, AttribEntry *next) :
          index(0), value(0), next(next), type(type), features() { }

        ~AttribEntry(void) { }

        void *operator new(size_t size, Util::Pool *pool, size_t len) {
          return pool->alloc(size + len - 1);
        }

        void operator delete(void *, Util::Pool *, size_t) { /* nothing */}

      public:
        uint64_t index;
        uint64_t value;
        AttribEntry *next;
        const char *type;
        Features features;
        char str[1];

        static Hash::Hash hash(const char *type, const std::string &str) {
          Hash::Hash hash(str + ' ' + type);
          return hash;
        }

        static AttribEntry *create(Util::Pool *pool, const uint64_t index,
            const std::string &str, const Hash::Hash hash, AttribEntry *next) {
          return NULL;
        }

        static AttribEntry *create(Util::Pool *pool, const char *type,
            const std::string &str, AttribEntry *next) {
          AttribEntry *entry = new (pool, str.size()) AttribEntry(type, next);
          strcpy(entry->str, str.c_str());
          return entry;
        }

        void insert(TagPair &tp) {
          features.push_back(Feature(tp, 1));
        }

        void increment(TagPair &tp) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            if (i->klasses == tp) {
              ++i->freq;
              ++value;
              return;
            }
          insert(tp);
        }

        bool equal(const char *type, const std::string &str) const {
          return this->type == type && this->str == str;
        }

        AttribEntry *find(const Hash::Hash hash, const std::string &str) {
          return NULL;
        }

        AttribEntry *find(const char *type, const std::string &str) {
          for (AttribEntry *l = this; l != NULL; l = l->next)
            if (l->equal(type, str))
              return l;
          return NULL;
        }

        bool find(const char *type, const std::string &str, uint64_t &id) const {
          for (const AttribEntry *l = this; l != NULL; l = l->next)
            if (l->equal(type, str) && l->value > 0) {
              id = l->index - 1;
              return l->index != 0;
            }
          return false;
        }

        void save_attribute(std::ostream &out) const {
          assert(index != 0);
          out << type << ' ' << str << ' ' << value << '\n';
        }

        void save_features(std::ostream &out) const {
          assert(index != 0);
          for (Features::const_iterator i = features.begin(); i != features.end(); ++i)
            if (i->freq)
              out << index << ' ' << i->klasses.prev_id() << ' ' << i->klasses.curr_id() << ' ' << i->freq << '\n';
        }

        uint64_t nfeatures(void) const {
          assert(index != 0);
          uint64_t total;
          for (Features::const_iterator i = features.begin(); i != features.end(); ++i)
            if (i->freq)
              ++total;
          return total;
        }

        size_t nchained(void) const {
          return next ? next->nchained() + 1 : 1;
        }

    };

    typedef HT::OrderedHashTable<AttribEntry, std::string> ImplBase;

    class Attributes::Impl : public ImplBase, public Util::Shared {
      private:
        std::string preface;

      public:
        Impl(const size_t nbuckets, const size_t pool_size)
          : ImplBase(nbuckets, pool_size), Shared(), preface() { }
        Impl(const std::string &filename, const size_t nbuckets,
            const size_t pool_size)
          : ImplBase(nbuckets, pool_size), Shared(), preface() {
          load(filename);
        }

        Impl(const std::string &filename, std::istream &input,
            const size_t nbuckets, const size_t pool_size) :
          ImplBase(nbuckets, pool_size), Shared(), preface() {
            load(filename, input);
        }

        void add(const char *type, const std::string &str, TagPair &tp) {
          size_t bucket = AttribEntry::hash(type, str).value() % Base::_nbuckets;
          AttribEntry *entry = Base::_buckets[bucket]->find(type, str);
          if (entry)
            return entry->increment(tp);

          entry = AttribEntry::create(Base::_pool, type, str, Base::_buckets[bucket]);
          _buckets[bucket] = entry;
          _entries.push_back(entry);
          ++_size;
          entry->insert(tp);
        }

        void add(uint64_t index, TagPair &tp) {
          _entries[index]->increment(tp);
        }

        void insert(const char *type, const std::string &str, uint64_t freq) {
          size_t bucket = AttribEntry::hash(type, str).value() % Base::_nbuckets;
          AttribEntry *entry = AttribEntry::create(Base::_pool, type, str, Base::_buckets[bucket]);
          entry->value = freq;
          ++_size;
          _buckets[bucket] = entry;
          _entries.push_back(entry);
        }

        bool find(const char *type, const std::string &str, uint64_t &id) {
          return Base::_buckets[AttribEntry::hash(type, str).value() % Base::_nbuckets]->find(type, str, id);
        }

        void load(const std::string &filename) {
          std::ifstream input(filename.c_str());
          if (!input)
            throw IOException("Unable to open lexicon file", filename);
          load(filename, input);
        }

        void load(const std::string &filename, std::istream &input) {
          uint64_t nlines;

          read_preface(filename, input, preface, nlines);

          std::string type;
          std::string str;
          uint64_t freq;
          while (input >> type >> str >> freq) {
            ++nlines;
            if (input.get() != '\n')
              throw IOException("expected newline after frequency in attributes file", filename, nlines);
            insert(type.c_str(), str, freq);
          }

          if (!input.eof())
            throw IOException("could not parse word or frequency information for attributes", filename, nlines);
        }

        void save_features(std::ostream &out, const std::string &preface) {
          out << preface << '\n';
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->save_features(out);
        }

        void save_attributes(std::ostream &out, const std::string &preface) {
          compact();
          sort_by_rev_value();
          out << preface << '\n';
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->save_attribute(out);
        }

        size_t size(void) const { return Base::_size; }
    };

    Attributes::Attributes(const size_t nbuckets, const size_t pool_size)
      : _impl(new Impl(nbuckets, pool_size)) { }

    Attributes::Attributes(const std::string &filename, const size_t nbuckets,
        const size_t pool_size) : _impl(new Impl(filename, nbuckets, pool_size)) { }

    Attributes::Attributes(const std::string &filename, std::istream &input,
        const size_t nbuckets, const size_t pool_size)
      : _impl(new Impl(filename, input, nbuckets, pool_size)) { }

    Attributes::Attributes(const Attributes &other) : _impl(share(other._impl)) { }

    void Attributes::load(const std::string &filename) { _impl->load(filename); }
    void Attributes::load(const std::string &filename, std::istream &input) { _impl->load(filename, input); }

    void Attributes::save_attributes(const std::string &filename, const std::string &preface) {
      std::ofstream out(filename.c_str());
      if (!out)
        throw IOException("unable to open file for writing", filename);
      _impl->save_attributes(out, preface);
    }

    void Attributes::save_features(const std::string &filename, const std::string &preface) {
      std::ofstream out(filename.c_str());
      if (!out)
        throw IOException("unable to open file for writing", filename);
      _impl->save_features(out, preface);
    }

    void Attributes::save_attributes(std::ostream &out, const std::string &preface) { _impl->save_attributes(out, preface); }
    void Attributes::save_features(std::ostream &out, const std::string &preface) { _impl->save_features(out, preface); }

    void Attributes::operator()(const char *type, const std::string &str, TagPair &tp) { _impl->add(type, str, tp); }

    void Attributes::operator()(const char *type, const std::string &str, uint64_t &id) { _impl->find(type, str, id); }

    void Attributes::sort_by_freq(void) { _impl->sort_by_rev_value(); }

    size_t Attributes::size(void) const { return _impl->size(); }
  }
}

#endif
