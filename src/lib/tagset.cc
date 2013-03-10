#include "base.h"

#include "hashtable.h"
#include "tagset.h"

namespace NLP {
  namespace Hash = Util::Hasher;
  /**
    * TagEntry.
    * Entry object for the TagSet hashtable.
    */
  class TagEntry {
    private:
      TagEntry(const uint64_t index, const uint16_t type, TagEntry *next) :
        index(index), value(0), next(next), type(type) { }

      ~TagEntry(void) { }

      void *operator new(size_t size, Util::Pool *pool, size_t len) {
        return (void *)pool->alloc(size + len);
      }

      void operator delete(void *, Util::Pool, size_t) { }

    public:
      uint64_t index;
      uint64_t value;
      TagEntry *next;
      const uint16_t type;
      char str[1];

      static Hash::Hash hash(const std::string &str) {
        return Hash::Hash(str);
      }

      static TagEntry *create(Util::Pool *pool, const uint64_t index,
          const std::string &str, const Hash::Hash hash, TagEntry *next) {
        return NULL;
      }

      static TagEntry *create(Util::Pool *pool, const uint64_t index,
          const uint16_t type, const std::string &str, TagEntry *next) {
        TagEntry *entry = new (pool, str.size()) TagEntry(index, type, next);
        strcpy(entry->str, str.c_str());
        return entry;
      }

      bool equal(const std::string &str, const uint16_t type) {
        return this->type == type && this->str == str;
      }

      TagEntry *find(const Hash::Hash hash, const std::string &str) {
        return NULL;
      }

      TagEntry *find(const std::string &str, const uint16_t type=0) {
        for (TagEntry *l = this; l != NULL; l = l->next)
          if (l->equal(str, type))
            return l;
        return NULL;
      }

      std::ostream &save(std::ostream &out) const {
        return out << str << ' ' << type << ' ' << value << '\n';
      }

      size_t nchained(void) const {
        return next ? next->nchained() + 1 : 1;
      }
  };

  typedef HT::OrderedHashTable<TagEntry, std::string> ImplBase;
  class TagSet::Impl : public ImplBase, public Util::Shared {
    public:
      std::string preface;
      std::string filename;

      Impl(void) : ImplBase(HT::TINY, HT::TINY), Shared(), preface(), filename() { }
      Impl(const std::string &filename)
        : ImplBase(HT::TINY, HT::TINY), Shared(), preface(), filename(filename) { }

      Impl(const std::string &filename, std::istream &input)
        : ImplBase(HT::TINY, HT::TINY), Shared(), preface(), filename(filename) {
        load(filename, input);
      }

      using ImplBase::add;
      using ImplBase::insert;

      void add(const std::string &raw, const uint16_t type, const uint64_t freq) {
        size_t bucket = TagEntry::hash(raw).value() % _nbuckets;
        TagEntry *entry = _buckets[bucket]->find(raw, type);
        if (!entry) {
          entry = TagEntry::create(_pool, _size, type, raw, _buckets[bucket]);
          _buckets[bucket] = entry;
          _entries.push_back(entry);
          ++_size;
        }
        entry->value += freq;
      }

      void insert(const std::string &raw, const uint16_t type, const uint64_t freq) {
        size_t bucket = TagEntry::hash(raw).value() % _nbuckets;
        TagEntry *entry = TagEntry::create(_pool, _size, type, raw, _buckets[bucket]);
        _buckets[bucket] = entry;
        ++_size;
        _entries.push_back(entry);
        entry->value += freq;
      }

      using ImplBase::find;

      TagEntry *find(const std::string &raw, const uint16_t type) const {
        return _buckets[TagEntry::hash(raw).value() % _nbuckets]->find(raw, type);
      }

      void load(void) {
        std::ifstream input(filename.c_str());
        if (!input)
          throw IOException("Unable to open lexicon file", filename);
        load(filename, input);
      }

      void load(const std::string &filename, std::istream &input) {
        clear();
        uint64_t nlines = 0;

        read_preface(filename, input, preface, nlines);

        std::string tag;
        uint16_t type;
        uint64_t freq;
        while (input >> tag >> type >> freq) {
          ++nlines;
          if (input.get() != '\n')
            throw IOException("expected newline after frequency in lexicon file", filename, nlines);
          insert(tag, type, freq);
        }

        if (!input.eof())
          throw IOException("could not parse word or frequency information for lexicon", filename, nlines);
      }

      void save(std::ostream &out, const std::string &preface) {
        out << preface << '\n';
        sort_by_type();
        ImplBase::save(out);
      }

      const Tag canonize(const std::string &raw, const uint16_t type=0) const {
        return canonize(raw.c_str());
      }

      const Tag canonize(const char *raw, const uint16_t type=0) const {
        if (raw[0] == '_' && raw[1] == '_') {
          if (raw == None::str)
            return None::val;
          else if (raw == Sentinel::str)
            return Sentinel::val;
        }

        TagEntry *e = find(raw, type);
        if (!e)
          return SENTINEL;
        return Tag(e->index, e->type);
      }

      void canonize(const Raws &raws, Tags &tags, const uint16_t type=0) const {
        tags.resize(0);
        tags.reserve(raws.size());
        for (Raws::const_iterator i = raws.begin(); i != raws.end(); ++i)
          tags.push_back(canonize(*i, type));
      }

      const char *str(const Tag &tag) const {
        return _entries[tag.id()]->str;
      }

      void str(const Tags &tags, Raws &raws) const {
        raws.resize(0);
        raws.reserve(tags.size());
        for (Tags::const_iterator i = tags.begin(); i != tags.end(); ++i)
          raws.push_back(str(*i));
      }

      size_t size(void) const { return Base::_size; }

      const Tag at(const size_t index) const {
        TagEntry *e = _entries[index];
        if (!e)
          return SENTINEL;
        return Tag(e->index, e->type);
      }
  };

  TagSet::TagSet(const std::string &filename)
    : _impl(new Impl(filename)) {
      insert(None::str, 0, 0);
      insert(Sentinel::str, 0, 0);
  }

  TagSet::TagSet(const std::string &filename, std::istream &input)
    : _impl(new Impl(filename, input)) { }

  TagSet::TagSet(const TagSet &other) : _impl(share(other._impl)) { }

  TagSet &TagSet::operator=(const TagSet &other){
    if (_impl != other._impl){
      release(_impl);
      _impl = share(other._impl);
    }

    return *this;
  }

  TagSet::~TagSet(void) { release(_impl); }

  void TagSet::add(const std::string &raw, const uint16_t type, const uint64_t freq) { _impl->add(raw, type, freq); }
  void TagSet::insert(const std::string &raw, const uint16_t type, const uint64_t freq) { _impl->insert(raw, type, freq); }

  void TagSet::load(void) { _impl->load(); }
  void TagSet::load(const std::string &filename, std::istream &input) { _impl->load(filename, input); }

  void TagSet::save(const std::string &preface) {
    std::ofstream out(_impl->filename.c_str());
    if (!out)
      throw IOException("unable to open file for writing", _impl->filename);
    _impl->save(out, preface);
  }
  void TagSet::save(const std::string &filename, const std::string &preface) {
    std::ofstream out(filename.c_str());
    if (!out)
      throw IOException("unable to open file for writing", filename);
    _impl->save(out, preface);
  }
  void TagSet::save(std::ostream &out, const std::string &preface) { _impl->save(out, preface); }

  const Tag TagSet::canonize(const std::string &raw, const uint16_t type) const { return _impl->canonize(raw.c_str(), type); }
  const Tag TagSet::canonize(const char *raw, const uint16_t type) const { return _impl->canonize(raw, type); }
  void TagSet::canonize(const Raws &raws, Tags &tags, const uint16_t type) const { _impl->canonize(raws, tags, type); }
  const Tag TagSet::operator[](const size_t index) const { return _impl->at(index); }

  const char *TagSet::str(const Tag &word) const { return _impl->str(word); }
  void TagSet::str(const Tags &tags, Raws &raws) const { _impl->str(tags, raws); }

  size_t TagSet::size(void) const { return _impl->size(); }

  size_t TagSet::index(TagPair &tp) const { return tp.index(size()); }
}
