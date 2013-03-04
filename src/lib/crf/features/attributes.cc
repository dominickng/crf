#include "base.h"

#include "config.h"
#include "hashtable.h"
#include "crf/features/types.h"
#include "crf/features/feature.h"
#include "crf/features/context.h"
#include "crf/features/attributes.h"

namespace NLP {
  namespace CRF {
    namespace Hash = Util::Hasher;

    /**
     * AttribEntry. This object is an entry in the Attributes hash table.
     *
     * Each AttribEntry has a unique index (used for sorting), a frequency,
     * a feature type, a pointer to the next AttribEntry in the chain, text
     * value, and a vector of Feature objects that have been observed
     * with this attribute in the training data.
     *
     * The feature type is stored as a const char * pointer to the canonical
     * string representation of the name of the feature type (in the Types
     * object)
     *
     * The text value of each AttribEntry is generally the unique
     * representation of a feature (e.g. a word, pos tag, underscore-joined
     * conjunction of words, etc.). This is generated from the observed data at
     * training and test time. The value is stored in the str member. When
     * an AttribEntry is allocated, extra memory is allocated to fit the
     * text value exactly, including the terminating null character. The
     * str member is a 1-element char array since some compilers complain
     * about a zero element array.
     */
    class AttribEntry {
      private:
        /**
         * Private constructor so that AttribEntry objects cannot be created
         * directly; they must be created via the static create function so
         * that memory can be appropriately allocated.
         */
        AttribEntry(const char *type, AttribEntry *next) :
          index(0), value(0), next(next), type(type), features() { }

        void *operator new(size_t size, Util::Pool *pool, size_t len) {
          return pool->alloc(size + len);
        }

        void operator delete(void *, Util::Pool *, size_t) { /* nothing */}

      public:
        uint64_t index;
        uint64_t value;
        AttribEntry *next;
        const char *type;
        Features features;
        char str[1];

        ~AttribEntry(void) { }

        /**
         * Static hash function. The hash is computed from the conjoined
         * type string and text value
         */
        static Hash::Hash hash(const char *type, const std::string &str) {
          std::string s = str + ' ' + type;
          Hash::Hash hash(s);
          return hash;
        }

        static AttribEntry *create(Util::Pool *pool, const uint64_t index,
            const std::string &str, const Hash::Hash hash, AttribEntry *next) {
          return NULL;
        }

        /**
         * create.
         * Static function to create AttribEntry objects. Allocates the
         * AttribEntry and copies the text value into the str member.
         */
        static AttribEntry *create(Util::Pool *pool, const char *type,
            const std::string &str, AttribEntry *next) {
          AttribEntry *entry = new (pool, str.size()) AttribEntry(type, next);
          strcpy(entry->str, str.c_str());
          return entry;
        }

        /**
         * insert.
         * Adds a new feature with the tagpair it has been observed with to
         * this attribute.
         */
        void insert(TagPair &tp) {
          features.push_back(Feature(tp, 1));
        }

        /**
         * increment.
         * If a feature has previously been seen with the given tagpair,
         * increment its frequency. Otherwise, add the feature.
         */
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
            if (l->equal(type, str) && l->value > 0)
              return l;
          return NULL;
        }

        /**
         * find.
         * Given a context, a feature type, and the text value extracted by
         * the feature generator for that feature type, add all features on
         * attributes which match the text value to the context
         */
        bool find(const char *type, const std::string &str, Context &c) {
          for (AttribEntry *l = this; l != NULL; l = l->next) {
            if (l->equal(type, str) && l->value > 0) {
              c.features.reserve(c.features.size() + l->features.size());
              for (Features::iterator i = l->features.begin(); i != l->features.end(); ++i)
                c.features.push_back(&(*i));
              return true;
            }
          }
          return false;
        }

        /**
         * cutoff.
         * Eliminates features with a frequency less than cutoff.
         */
        void cutoff(const uint64_t freq) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i) {
            if (i->freq < freq) {
              value -= i->freq;
              i->freq = 0;
            }
          }
        }

        /**
         * save_attribute.
         * Dump the current attribute to the ostream. The output format is:
         * type_string text_value freq
         *
         * This does not need to iterate through the chain of AttribEntry
         * objects as it is called via the sorted list of all AttribEntry
         * objects in the hash table
         */
        void save_attribute(std::ostream &out) const {
          assert(index != 0);
          out << type << ' ' << str << ' ' << value << '\n';
        }

        /**
         * save_features.
         * Dump the features associated with this attribute to the ostream. The
         * output format is:
         * attr_index prev_klass curr_klass freq lambda
         *
         * where attr index is the index of this attribute. This index value
         * is set when the attributes hash table is sorted by decreasing
         * frequency.
         */
        void save_features(std::ostream &out) const {
          assert(index != 0);
          for (Features::const_iterator i = features.begin(); i != features.end(); ++i)
            if (i->freq)
              out << index << ' ' << i->klasses.prev_id() << ' ' << i->klasses.curr_id() << ' ' << i->freq << ' ' << *(i->lambda) << '\n';
        }

        /**
         * nfeatures.
         * Calculates the number of features on this attribute
         */
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

        /**
         * reset_expectations.
         * Resets model expected feature counts to 0 for each L-BFGS iteration
         */
        void reset_expectations(void) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            i->exp = 0.0;
        }

        /**
         * sum_lambda_sq.
         * Returns the sum of the squared lambda values of each feature
         * attached to this attribute
         */
        double sum_lambda_sq(void) {
          double lambda_sq = 0.0;
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            lambda_sq += (*(i->lambda) * *(i->lambda));
          return lambda_sq;
        }

        /**
         * assign_lambdas.
         * Given an array of doubles and a reference to a starting array index,
         * assigns the lambda pointer for each feature attached to this
         * attribute to the indexth position of the array. Increments index
         * for each feature. It is assumed that the array and index are
         * correctly sized as this function is called for each attribute and
         * does not do any bounds checking.
         */
        void assign_lambdas(double *x, size_t &index) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            i->lambda = &x[index++];
        }

        /**
         * zero_lambdas.
         * Sets all lambda pointers to NULL.
         */
        void zero_lambdas(void) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            i->lambda = NULL;
        }

        /**
         * copy_gradients.
         * Computes the gradient of each feature attached to this attribute,
         * and copies that gradient to the supplied array of doubles.
         */
        void copy_gradients(double *x, double inv_sigma_sq, size_t &index) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            x[index++] = i->gradient(inv_sigma_sq);
        }

        /**
         * print.
         * Prints the features attached to this attribute to stdout along with
         * their gradient and lambda values.
         */
        void print(double inv_sigma_sq) {
          for (Features::iterator i = features.begin(); i != features.end(); ++i)
            std::cout << "gradient: " << i->gradient(inv_sigma_sq) << " lambda: " << *(i->lambda) << std::endl;
        }
    };

    typedef HT::OrderedHashTable<AttribEntry, std::string> ImplBase;

    /**
     * Attributes::Impl.
     * Private hashtable implementation of the attributes object.
     */
    class Attributes::Impl : public ImplBase, public Util::Shared {
      private:
        std::string preface;
        Entries::iterator e; //used for finite differences gradient checking
        Features::iterator f; //used for finite differences gradient checking
        double prev_lambda; //used for finite differences gradient checking

      public:
        Impl(const size_t nbuckets, const size_t pool_size)
          : ImplBase(nbuckets, pool_size), Shared(), preface(), trans_features() { }
        Impl(const std::string &filename, const size_t nbuckets,
            const size_t pool_size)
          : ImplBase(nbuckets, pool_size), Shared(), preface(), trans_features() {
          load(filename);
        }

        Impl(const std::string &filename, std::istream &input,
            const size_t nbuckets, const size_t pool_size) :
          ImplBase(nbuckets, pool_size), Shared(), preface(), trans_features() {
            load(filename, input);
        }

        ~Impl(void) {
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->~AttribEntry();
        }

        //cache the transition features to save memory; all possible
        //transition features must be added to each context, so just store
        //one list of them instead of duplicating several times
        FeaturePtrs trans_features;

        using ImplBase::add;
        using ImplBase::insert;
        using ImplBase::find;

        void load_trans_features(const char *type, const std::string &str) {
          AttribEntry *e = Base::_buckets[AttribEntry::hash(type, str).value() % Base::_nbuckets]->find(type, str);
          if (e) {
            for (Features::iterator i = e->features.begin(); i != e->features.end(); ++i)
              trans_features.push_back(&(*i));
          }
        }

        /**
         * add.
         * Adds a feature with a given value, type, an observed tagpair to
         * the hash table. Allow both state and transition features to be added
         *
         * Most of the real work is done in the _add function
         */
        void add(const char *type, const std::string &str, TagPair tp, const bool add_state_feature=true, const bool add_trans_feature=true) {
          if (add_trans_feature)
            _add(type, str, tp);
          if (add_state_feature) {
            tp.prev = None::val;
            _add(type, str, tp);
          }
        }

        /**
         * _add.
         * Adds a feature to the attributes hash table.
         *
         * First, do a lookup to see if an existing AttribEntry matches the
         * given type and text value. If so, then increment the feature
         * matching the observed tagpair on that entry. Otherwise, create a
         * new AttribEntry and add it to the chain at the appropriate
         * location in the hash table (this chain may be NULL if nothing else
         * that collides with the computed hash value has been added yet)
         */
        void _add(const char *type, const std::string &str, TagPair &tp) {
          size_t bucket = AttribEntry::hash(type, str).value() % _nbuckets;
          AttribEntry *entry = _buckets[bucket]->find(type, str);
          if (entry)
            return entry->increment(tp);

          entry = AttribEntry::create(Base::_pool, type, str, _buckets[bucket]);
          _buckets[bucket] = entry;
          _entries.push_back(entry);
          ++_size;
          entry->increment(tp);
        }

        /**
         * add.
         * Adds an observation of a tagpair to the features on the attribute
         * at a specified index of the sorted entries
         */
        void add(uint64_t index, TagPair &tp) {
          _entries[index]->increment(tp);
        }

        /**
         * insert.
         * Similar to the add function, but does not perform an initial lookup
         * and always creates a new AttribEntry. Used for loading the
         * full attributes hashtable from disk.
         */
        void insert(const char *type, const std::string &str, uint64_t freq) {
          size_t bucket = AttribEntry::hash(type, str).value() % Base::_nbuckets;
          AttribEntry *entry = AttribEntry::create(Base::_pool, type, str, Base::_buckets[bucket]);
          entry->value = freq;
          ++_size;
          _buckets[bucket] = entry;
          _entries.push_back(entry);
        }

        /**
         * find.
         * Given a context, feature type, and feature value, add all features
         * that match the feature type and feature value to the context
         */
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

        /**
         * save_attributes.
         * Dumps the attributes file to disk, sorted by decreasing frequency
         */
        void save_attributes(std::ostream &out, const std::string &preface) {
          compact();
          sort_by_rev_value();
          out << preface << '\n';
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            if ((*i)->value)
              (*i)->save_attribute(out);
        }

        /**
         * save_features.
         * Dumps the features to disk, sorted by attributes in decreasing
         * frequency
         */
        void save_features(std::ostream &out, const std::string &preface) {
          out << preface << '\n';
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            if ((*i)->value)
              (*i)->save_features(out);
        }

        /**
         * nfeatures.
         * Returns the total number of features observed in the training data
         */
        uint64_t nfeatures(void) const {
          uint64_t n = 0;
          for (Entries::const_iterator i = _entries.begin(); i != _entries.end(); ++i)
            n += (*i)->nfeatures();
          return n;
        }

        /**
         * apply_attrib_cutoff.
         * Removes all attributes with frequency less than a cutoff value
         */
        void apply_attrib_cutoff(const uint64_t freq) {
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            if ((*i)->value < freq)
              (*i)->value = 0;
        }

        /**
         * apply_cutoff.
         * Removes all features with frequency less than a cutoff value
         */
        void apply_cutoff(const uint64_t freq) {
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->cutoff(freq);
        }

        /**
         * apply_cutoff.
         * Removes all features with a specific type that have a frequency
         * less than a cutoff value
         */
        void apply_cutoff(const char *type, const uint64_t freq) {
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            if ((*i)->type == type)
              (*i)->cutoff(freq);
        }

        /**
         * apply_cutoff.
         * Removes all features with a specific type that have a frequency
         * less than a cutoff value, and all other features with frequency
         * less than a default value
         */
        void apply_cutoff(const char *type, const uint64_t freq, const uint64_t def) {
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            if ((*i)->type == type)
              (*i)->cutoff(freq);
            else
              (*i)->cutoff(def);
        }

        void reset_expectations(void) {
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->reset_expectations();
        }

        double sum_lambda_sq(void) {
          double lambda_sq = 0.0;
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            lambda_sq += (*i)->sum_lambda_sq();
          return lambda_sq;
        }

        void assign_lambdas(double *x) {
          size_t index = 0;
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->assign_lambdas(x, index);
        }

        void zero_lambdas(void) {
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->zero_lambdas();
        }

        void copy_gradients(double *x, double inv_sigma_sq) {
          size_t index = 0;
          for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
            (*i)->copy_gradients(x, inv_sigma_sq, index);
        }

        /**
         * inc_next_lambda.
         * Increments the next feature lambda by val. Restores the previously
         * incremented lambda to its former value. Used for finite
         * difference empirical gradient check.
         */
        bool inc_next_lambda(double val) {
          if (e == _entries.end() && (f+1) == (*e)->features.end()) {
            *(f->lambda) = prev_lambda;
            return false;
          }
          else if (++f != (*e)->features.begin()) {
            *((f-1)->lambda) = prev_lambda;
            if (f == (*e)->features.end()) {
              if (++e == _entries.end())
                return false;
              f = (*e)->features.begin();
            }
          }
          prev_lambda = *(f->lambda);
          *(f->lambda) += val;
          return true;
        }

        /**
         * prep_finite_differences.
         * Sets the entries and feature iterators to the appropriate values
         * before beginning the finite differences empirical gradient check
         */
        void prep_finite_differences(void) {
          e = _entries.begin();
          f = (*e)->features.begin() - 1;
        }

        /**
         * print_current_gradient.
         * Prints the feature that is currently having its empirical gradient
         * checked.
         */
        void print_current_gradient(double val, double inv_sigma_sq) {
          double gradient = f->gradient(inv_sigma_sq);
          if (fabs(gradient - val) >= 1.0e-2) {
            std::cout << "freq: " << f->freq << " exp: " << f->exp;
            std::cout << " lambda: " << prev_lambda << " gradient: " << f->gradient(inv_sigma_sq);
            std::cout << " estimated gradient: " << val << " <" << f->klasses.prev << ' ' << f->klasses.curr << "> " << (*e)->str <<  std::endl;
          }
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

    Attributes::~Attributes(void) {
      release(_impl);
    }

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

    void Attributes::operator()(const char *type, const std::string &str, TagPair &tp, const bool add_state_feature, const bool add_trans_feature) { _impl->add(type, str, tp, add_state_feature, add_trans_feature); }
    void Attributes::operator()(const char *type, const std::string &str, Context &c) { _impl->find(type, str, c); }

    void Attributes::sort_by_freq(void) { _impl->sort_by_rev_value(); }
    void Attributes::reset_expectations(void) { _impl->reset_expectations(); }

    uint64_t Attributes::nfeatures(void) const { return _impl->nfeatures(); }

    void Attributes::apply_attrib_cutoff(const uint64_t freq) { _impl->apply_attrib_cutoff(freq); }
    void Attributes::apply_cutoff(const uint64_t freq) { _impl->apply_cutoff(freq); }
    void Attributes::apply_cutoff(const Type &type, const uint64_t freq) { _impl->apply_cutoff(type.name, freq); }
    void Attributes::apply_cutoff(const Type &type, const uint64_t freq, const uint64_t def) { _impl->apply_cutoff(type.name, freq, def); }

    double Attributes::sum_lambda_sq(void) { return _impl->sum_lambda_sq(); }
    void Attributes::assign_lambdas(double *x) { _impl->assign_lambdas(x);; }
    void Attributes::zero_lambdas(void) { _impl->zero_lambdas();; }
    void Attributes::copy_gradients(double *x, double inv_sigma_sq) { _impl->copy_gradients(x, inv_sigma_sq);; }

    bool Attributes::inc_next_lambda(double val) { return _impl->inc_next_lambda(val); }
    void Attributes::print_current_gradient(double val, double inv_sigma_sq) { _impl->print_current_gradient(val, inv_sigma_sq); }
    void Attributes::print(double inv_sigma_sq) { _impl->print(inv_sigma_sq); }
    void Attributes::prep_finite_differences(void) { _impl->prep_finite_differences(); }

    size_t Attributes::size(void) const { return _impl->size(); }
    void Attributes::load_trans_features(const char *type, const std::string &str) { _impl->load_trans_features(type, str); }
    FeaturePtrs &Attributes::trans_features(void) { return _impl->trans_features; }

} }
