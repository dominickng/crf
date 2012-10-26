#ifndef _LEXICON_H
#define _LEXICON_H

#include "base.h"
#include "hashtable.h"
#include "lbfgs.h"
#include "crf/features/feature.h"
#include "crf/features/context.h"
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
          std::string s = str + ' ' + type;
          Hash::Hash hash(s);
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
          ++value;
          for (Features::iterator i = features.begin(); i != features.end(); ++i) {
            if (i->klasses == tp) {
              ++(i->freq);
              return;
            }
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
              id = l->index;
              return l->index != 0;
            }
          return false;
        }

        bool find(const char *type, const std::string &str, Context &c) {
          for (AttribEntry *l = this; l != NULL; l = l->next) {
            if (l->equal(type, str) && l->value > 0) {
              for(Features::iterator i = l->features.begin(); i != l->features.end(); ++i) {
                if(i->klasses == c.klasses)
                  c.features.push_back(&(*i));
                else if(i->klasses.prev == None::val && i->klasses.curr == c.klasses.curr)
                  c.features.push_back(&(*i)); //[> single tag features <]
              }
              return true;
            }
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
              out << index << ' ' << i->klasses.prev_id() << ' ' << i->klasses.curr_id() << ' ' << i->freq << ' ' << i->lambda << '\n';
        }

        void save_weights(std::ostream &out) const {
          assert(index != 0);
          for (Features::const_iterator i = features.begin(); i != features.end(); ++i)
            if (i->freq)
              out << i->lambda << '\n';
        }

        uint64_t nfeatures(void) const {
          assert(index != 0);
          uint64_t total = 0;
          for (Features::const_iterator i = features.begin(); i != features.end(); ++i)
            if (i->freq)
              ++total;
          return total;
        }

        size_t nchained(void) const {
          return next ? next->nchained() + 1 : 1;
        }

        void reset_estimations(void) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            i->est = 0.0;
        }

        double sum_lambda_sq(void) {
          double lambda_sq = 0.0;
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            lambda_sq += (i->lambda * i->lambda);
          return lambda_sq;
        }

        void copy_lambdas(const lbfgsfloatval_t *x, size_t &index) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            i->lambda = x[index++];
        }

        void copy_gradients(lbfgsfloatval_t *x, double inv_sigma_sq, size_t &index) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            x[index++] = -(i->freq - i->est - (i->lambda * inv_sigma_sq));
        }

        void print(double inv_sigma_sq) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            std::cout << "gradient: " << -(i->freq - i->est - (i->lambda * inv_sigma_sq)) << " lambda: " << i->lambda << std::endl;
        }


    };

    typedef HT::OrderedHashTable<AttribEntry, std::string> ImplBase;

    class Attributes::Impl : public ImplBase, public Util::Shared {
      private:
        std::string preface;
        Entries::iterator e;
        Features::iterator f;
        double prev_lambda;

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
          size_t bucket = AttribEntry::hash(type, str).value() % _nbuckets;
          //std::cout << "adding " << type << " " << str << " for " << tp.prev.id() << ' ' << tp.curr.id() << std::endl;
          AttribEntry *entry = _buckets[bucket]->find(type, str);
          if (entry)
            return entry->increment(tp);

          entry = AttribEntry::create(Base::_pool, type, str, _buckets[bucket]);
          _buckets[bucket] = entry;
          _entries.push_back(entry);
          ++_size;
          entry->increment(tp);
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

        bool find(const char *type, const std::string &str, Context &c) {
          return Base::_buckets[AttribEntry::hash(type, str).value() % Base::_nbuckets]->find(type, str, c);
        }

        void load(const std::string &filename) {
          std::ifstream input(filename.c_str());
          if (!input)
            throw IOException("Unable to open lexicon file", filename);
          load(filename, input);
        }

        void load(const std::string &filename, std::istream &input) {
          uint64_t nlines = 0;

          read_preface(filename, input, preface, nlines);

          std::string type;
          std::string str;
          uint64_t freq = 0;
          while (input >> type >> str >> freq) {
            ++nlines;
            if (input.get() != '\n')
              throw IOException("expected newline after frequency in attributes file", filename, nlines);
            insert(type.c_str(), str, freq);
          }

          if (!input.eof())
            throw IOException("could not parse word or frequency information for attributes", filename, nlines);
        }

        void save_attributes(std::ostream &out, const std::string &preface) {
          compact();
          sort_by_rev_value();
          out << preface << '\n';
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->save_attribute(out);
        }

        void save_features(std::ostream &out, const std::string &preface) {
          out << preface << '\n';
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->save_features(out);
        }

        void save_weights(std::ostream &out, const std::string &preface) {
          out << preface << '\n';
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->save_weights(out);
        }

        uint64_t nfeatures(void) const {
          uint64_t n = 0;
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            n += (*i)->nfeatures();
          return n;
        }

        void reset_estimations(void) {
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->reset_estimations();
        }

        double sum_lambda_sq(void) {
          double lambda_sq = 0.0;
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            lambda_sq += (*i)->sum_lambda_sq();
          return lambda_sq;
        }

        void copy_lambdas(const lbfgsfloatval_t *x) {
          size_t index = 0;
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->copy_lambdas(x, index);
        }

        void copy_gradients(lbfgsfloatval_t *x, double inv_sigma_sq) {
          size_t index = 0;
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->copy_gradients(x, inv_sigma_sq, index);
        }

        bool inc_next_gradient(double val) {
          if (e == _entries.end() && (f+1) == (*e)->features.end()) {
            f->lambda = prev_lambda;
            return false;
          }
          else if (++f != (*e)->features.begin()) {
            (f-1)->lambda = prev_lambda;
            if (f == (*e)->features.end()) {
              if (++e == _entries.end())
                return false;
              f = (*(++e))->features.begin();
            }
          }
          prev_lambda = f->lambda;
          f->lambda += val;
          return true;
        }

        bool dec_gradient(double val) {
          f->lambda = prev_lambda - val;
          return true;
        }

        void prep_finite_differences(void) {
          e = _entries.begin();
          f = (*e)->features.begin() - 1;
        }

        void print_current_gradient(double val, double inv_sigma_sq) {
          std::cout << "actual: " << -(f->freq - f->est - (prev_lambda * inv_sigma_sq)) << " estimated: " << val << std::endl;
        }

        size_t size(void) const { return Base::_size; }

        void print(double inv_sigma_sq) {
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->print(inv_sigma_sq);
        }
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

    void Attributes::save_weights(const std::string &filename, const std::string &preface) {
      std::ofstream out(filename.c_str());
      if (!out)
        throw IOException("unable to open file for writing", filename);
      _impl->save_weights(out, preface);
    }

    void Attributes::save_attributes(std::ostream &out, const std::string &preface) { _impl->save_attributes(out, preface); }
    void Attributes::save_features(std::ostream &out, const std::string &preface) { _impl->save_features(out, preface); }
    void Attributes::save_weights(std::ostream &out, const std::string &preface) { _impl->save_weights(out, preface); }

    void Attributes::operator()(const char *type, const std::string &str, TagPair &tp) { _impl->add(type, str, tp); }

    void Attributes::operator()(const char *type, const std::string &str, uint64_t &id) { _impl->find(type, str, id); }

    void Attributes::operator()(const char *type, const std::string &str, Context &c) { _impl->find(type, str, c); }

    void Attributes::sort_by_freq(void) { _impl->sort_by_rev_value(); }
    void Attributes::reset_estimations(void) { _impl->reset_estimations(); }

    uint64_t Attributes::nfeatures(void) const { return _impl->nfeatures(); }

    double Attributes::sum_lambda_sq(void) { return _impl->sum_lambda_sq(); }
    void Attributes::copy_lambdas(const lbfgsfloatval_t *x) { _impl->copy_lambdas(x);; }
    void Attributes::copy_gradients(lbfgsfloatval_t *x, double inv_sigma_sq) { _impl->copy_gradients(x, inv_sigma_sq);; }

    bool Attributes::inc_next_gradient(double val) { return _impl->inc_next_gradient(val); }
    bool Attributes::dec_gradient(double val) { return _impl->dec_gradient(val); }
    void Attributes::print_current_gradient(double val, double inv_sigma_sq) { _impl->print_current_gradient(val, inv_sigma_sq); }
    void Attributes::print(double inv_sigma_sq) { _impl->print(inv_sigma_sq); }
    void Attributes::prep_finite_differences(void) { _impl->prep_finite_differences(); }

    size_t Attributes::size(void) const { return _impl->size(); }
  }
}

#endif
