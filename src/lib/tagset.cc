#ifndef _LEXICON_H
#define _LEXICON_H

#include "base.h"
#include "hashtable.h"
#include "tagset.h"

namespace NLP {
  typedef HT::StringEntry<uint16_t> Entry;
  typedef HT::OrderedHashTable<Entry, std::string> ImplBase;
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

      void add(const std::string &raw, const uint64_t freq) {
        Base::add(raw)->value += freq;
      }

      void insert(const std::string &raw, const uint64_t freq) {
        Base::add(raw)->value = freq;
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
        uint64_t freq;
        while (input >> tag >> freq) {
          ++nlines;
          if (input.get() != '\n')
            throw IOException("expected newline after frequency in lexicon file", filename, nlines);
          insert(tag, freq);
        }

        if (!input.eof())
          throw IOException("could not parse word or frequency information for lexicon", filename, nlines);
      }

      void save(std::ostream &out, const std::string &preface) {
        out << preface << '\n';
        ImplBase::save(out);
      }

      const Tag canonize(const std::string &raw) const {
        Entry *e = find(raw.c_str());
        if (!e)
          return SENTINEL;
        return Tag(e->index);
      }

      const Tag canonize(const char *raw) const {
        Entry *e = find(raw);
        if (!e)
          return SENTINEL;
        return Tag(e->index);
      }

      void canonize(const Raws &raws, Tags &tags) const {
        tags.resize(0);
        tags.reserve(raws.size());
        for (Raws::const_iterator i = raws.begin(); i != raws.end(); ++i)
          tags.push_back(canonize(*i));
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
  };

  TagSet::TagSet(void) :
    _impl(new Impl()) {
      insert(None::str, 0);
      insert(Sentinel::str, 0);
  }

  TagSet::TagSet(const std::string &filename)
    : _impl(new Impl(filename)) {
      insert(None::str, 0);
      insert(Sentinel::str, 0);
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

  void TagSet::add(const std::string &raw, const uint64_t freq) { _impl->add(raw, freq); }
  void TagSet::insert(const std::string &raw, const uint64_t freq) { _impl->insert(raw, freq); }

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

  const Tag TagSet::canonize(const std::string &raw) const { return _impl->canonize(raw); }
  const Tag TagSet::canonize(const char *raw) const { return _impl->canonize(raw); }
  void TagSet::canonize(const Raws &raws, Tags &tags) const { _impl->canonize(raws, tags); }

  const char *TagSet::str(const Tag &word) const { return _impl->str(word); }
  void TagSet::str(const Tags &tags, Raws &raws) const { _impl->str(tags, raws); }

  size_t TagSet::size(void) const { return _impl->size(); }

  size_t TagSet::index(TagPair &tp) const { return tp.index(size()); }
}

#endif
