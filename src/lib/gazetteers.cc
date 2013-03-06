#include "base.h"

#include "hashtable.h"
#include "gazetteers.h"

namespace NLP {
  typedef HT::StringEntry<uint64_t> Entry;
  typedef HT::BaseHashTable<Entry, std::string> ImplBase;
  class Gazetteers::Impl : public ImplBase, public Util::Shared {
    public:
      std::vector<std::string> names;

      Impl(const size_t nbuckets, const size_t pool_size)
        : ImplBase(nbuckets, pool_size), Shared(), names(sizeof(uint64_t) * 8, "") { }
      Impl(const std::string &dir, const std::string &config,
          const size_t nbuckets, const size_t pool_size)
        : ImplBase(nbuckets, pool_size), Shared(), names(sizeof(uint64_t) * 8, "") {
            load(dir, config);
      }

      using ImplBase::add;
      using ImplBase::insert;

      void add(const std::string &entry, const uint64_t flags) {
        ImplBase::add(entry)->value |= flags;
      }

      void load(const std::string &dir, const std::string &config) {
        std::ifstream input(config.c_str());
        if (!input)
          throw IOException("Unable to open gazetteer config file", config);
        uint64_t index;
        std::string name, filename;
        while (input >> name >> index >> filename) {
          if (filename[0] != '/')
            filename = dir + '/' + filename;
          names[index] = name;
          load(filename, 1 << index);
        }
      }

      void load(const std::string &filename, const uint64_t flag) {
        std::ifstream input(filename.c_str());
        if (!input)
          throw IOException("Unable to open gazetteer file", filename);

        std::string entry;
        while (getline(input, entry))
          add(entry, flag);

        if (!input.eof())
          throw IOException("unexpected content in gazetteer file", filename, _size);
      }

      uint64_t exists(const std::string &str) const {
        Entry *e = find(str.c_str());
        if (e)
          return e->value;
        return 0;
      }

      int gaz_index(const std::string &name) const {
        for (uint64_t i = 0; i < names.size(); ++i)
          if (names[i] == name)
            return i;
        return -1;
      }

      const std::string &gaz_name(const uint64_t flag) const {
        for (uint64_t i = 0; i < names.size(); ++i)
          if (flag & (1 << i))
            return names[i];
        return None::str;
      }

      const std::vector<std::string> &gaz_names(void) const {
        return names;
      }

      size_t size(void) const { return ImplBase::_size; }
  };

  Gazetteers::Gazetteers(const size_t nbuckets, const size_t pool_size) :
    _impl(new Impl(nbuckets, pool_size)) { }

  Gazetteers::Gazetteers(const std::string &dir, const std::string &config,
      const size_t nbuckets, const size_t pool_size)
    : _impl(new Impl(dir, config, nbuckets, pool_size)) { }

  Gazetteers::Gazetteers(const Gazetteers &other) : _impl(share(other._impl)) { }

  Gazetteers::~Gazetteers(void) { release(_impl); }

  void Gazetteers::add(const std::string &entry, const uint64_t flags) { _impl->add(entry, flags); }

  void Gazetteers::load(const std::string &dir, const std::string &config) { _impl->load(dir, config); }
  void Gazetteers::load(const std::string &filename, const uint64_t flag) { _impl->load(filename, flag); }

  uint64_t Gazetteers::exists(const std::string &str) const { return _impl->exists(str); }

  uint64_t Gazetteers::lower(const std::string &str) const {
    std::string buffer;
    buffer.reserve(str.size() + 1);
    for (std::string::const_iterator i = str.begin(); i != str.end(); ++i)
      buffer += tolower(*i);
    return exists(buffer);
  }

  int Gazetteers::gaz_index(const std::string &name) const { return _impl->gaz_index(name); }

  const std::string &Gazetteers::gaz_name(const uint64_t flag) const { return _impl->gaz_name(flag); };
  const std::vector<std::string> &Gazetteers::gaz_names(void) const { return _impl->gaz_names(); };

  size_t Gazetteers::size(void) const { return _impl->size(); }

  void Gazetteers::clear(void) { _impl->clear(); }
}
