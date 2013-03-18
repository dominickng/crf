/**
 * taglimits.h
 * Wraps a vector of tagpairs that stores the limits of each type of tag
 * in the factorial CRF
 */

namespace NLP {
  class TagLimits {
    public:
      TagLimits(TagSet tags)
        : _tags(tags), _limits(), _ntags(0), _max_index(0) { }
      ~TagLimits(void) { }

      typedef TagPairs::iterator iterator;
      typedef TagPairs::const_iterator const_iterator;

      iterator begin(void) { return _limits.begin(); }
      const_iterator begin(void) const { return _limits.begin(); }

      iterator end(void) { return _limits.end(); }
      const_iterator end(void) const { return _limits.end(); }

      const TagPair &operator[](const size_t index) const {
        return _limits[index];
      }

      TagPair &operator[](const size_t index) {
        return _limits[index];
      }

      void push_back(const Tag begin, const Tag end) {
        _limits.push_back(TagPair(begin, end));
      }

      void calc(void) {
        uint16_t prev_type = 0;
        uint16_t prev_id = 2; //for NONE and SENTINEL
        size_t i = 0;
        _ntags = _tags.size();
        for (; i < _ntags; ++i) {
          Tag t = _tags[i];
          if (t.type() != prev_type) {
            push_back(Tag(prev_id, prev_type), Tag(i, prev_type));
            prev_id = i;
          }
          prev_type = t.type();
        }
        push_back(Tag(prev_id, prev_type), Tag(i, prev_type));
      }

      void print(std::ostream &out) const {
        for (size_t i = 0; i < _limits.size(); ++i)
          std::cout << i << ' ' << _limits[i].prev << ' ' << _limits[i].curr << std::endl;
      }

      size_t max_index(const size_t nchains) {
        if (_max_index == 0) {
          _max_index = 1;
          for (size_t i = 0; i < nchains; ++i) {
            _max_index *= ntags(i);
          }
        }
        return _max_index;
      }

      inline size_t ntypes(void) const {
        return _limits.size();
      }

      inline size_t ntags(void) const {
        return _ntags;
      }

      inline size_t ntags(size_t type) const {
        if (type == 0)
          return (_limits[type].curr.id() - _limits[type].prev.id()) + 2;
        return _limits[type].curr.id() - _limits[type].prev.id();
      }

      Tag canonize(Tag t) const {
        for (size_t i = 0; i < t.type(); ++i)
          t -= ntags(i);
        return t;
      }

      Tag reindex(Tag t) const {
        for (size_t i = 0; i < t.type(); ++i)
          t += ntags(i);
        return t;
      }

      size_t nskip(size_t type) {
        size_t skip = 1;
        for (size_t i = type; i < ntypes() - 1; ++i)
          skip *= ntags(i+1);
        return skip;
      }

      const char *str(const Tag t) const {
        return _tags.str(t);
      }

    private:
      TagSet _tags;
      TagPairs _limits;
      size_t _ntags;
      size_t _max_index;
  };

  class IndexIterator {
    public:
      IndexIterator(TagLimits &limits, const Tag base1, const Tag base2, const bool skip_sentinel)
        : limits(&limits), base1(base1), base2(base2), indices(), ended(limits.ntypes() == 0), use_base2(true) {
          for (size_t i = 0; i < limits.ntypes(); ++i)
            indices.push_back(0);
          indices[base1.type()] = limits.canonize(base1).id();
          indices[base2.type()] = limits.canonize(base2).id();
          if (skip_sentinel && base1.type() != 0 && base2.type() != 0)
            indices[0] = 2;
      }

      IndexIterator(TagLimits &limits, const Tag base1, const Tag base2)
        : limits(&limits), base1(base1), base2(base2), indices(), ended(limits.ntypes() == 0), use_base2(true) {
          for (size_t i = 0; i < limits.ntypes(); ++i)
            indices.push_back(0);
          indices[base1.type()] = limits.canonize(base1).id();
          indices[base2.type()] = limits.canonize(base2).id();
      }

      IndexIterator(TagLimits &limits, const Tag base, const bool skip_sentinel)
        : limits(&limits), base1(base), indices(), ended(limits.ntypes() == 0), use_base2(false) {
          for (size_t i = 0; i < limits.ntypes(); ++i)
            indices.push_back(0);
          indices[base.type()] = limits.canonize(base).id();
          if (skip_sentinel && base.type() != 0)
            indices[0] = 2; // skip the first two in the first type
      }

      IndexIterator(TagLimits &limits, const Tag base)
        : limits(&limits), base1(base), indices(), ended(limits.ntypes() == 0), use_base2(false) {
          for (size_t i = 0; i < limits.ntypes(); ++i)
            indices.push_back(0);
          indices[base.type()] = limits.canonize(base).id();
      }

      IndexIterator(TagLimits &limits)
        : limits(&limits), base1(0, limits.ntypes() + 1), indices(),
          ended(limits.ntypes() == 0), use_base2(false) {
          for (size_t i = 0; i < limits.ntypes(); ++i)
            indices.push_back(0);
      }

      IndexIterator(TagLimits &limits, const bool skip_sentinel)
        : limits(&limits), base1(0, limits.ntypes() + 1), indices(),
          ended(limits.ntypes() == 0), use_base2(false) {
          for (size_t i = 0; i < limits.ntypes(); ++i)
            indices.push_back(0);
          if (skip_sentinel)
            indices[0] = 2; // skip None and Sentinel
      }

      IndexIterator &operator++(void) {
        for (int i = indices.size() - 1; i >= 0; --i) {
          if (i == base1.type() || (use_base2 && i == base2.type())) {
            if (i == 0)
              ended = true;
          }
          else {
            if (++indices[i] < limits->ntags(i))
              break;
            else if (i != 0)
              indices[i] = 0;
            else {
              ended = true;
              break;
            }
          }
        }
        return *this;
      }

      size_t operator*(void) {
        size_t index = 0;
        for (size_t i = 0; i < indices.size(); ++i) {
          if (i == base1.type())
            index += limits->nskip(i) * limits->canonize(base1).id();
          else if (use_base2 && i == base2.type())
            index += limits->nskip(i) * limits->canonize(base2).id();
          else
            index += limits->nskip(i) * indices[i];
        }
        return index;
      }

      bool end(void) const {
        return ended;
      }

      void print(std::ostream &os) {
        std::string out;
        for (size_t i = 0; i < indices.size(); ++i) {
          out += limits->str(Tag(limits->reindex(Tag(indices[i], i))));
          out += ' ';
        }
        os << out;
      }


    private:
      TagLimits *limits;
      Tag base1;
      Tag base2;
      std::vector<uint16_t> indices;
      bool ended;
      bool use_base2;

  };

  struct IndexPair {
    size_t prev;
    size_t curr;
  };

  class Index2Iterator {
    public:
      Index2Iterator(TagLimits &limits, const Tag base1, const Tag base2)
        : limits(limits), base1(base1), base2(base2), indices(),
          ended(limits.ntypes() == 0) {
          for (size_t i = 0; i < limits.ntypes(); ++i)
            indices.push_back(0);
          indices[base1.type()] = limits.canonize(base1).id();
          indices[base2.type()] = limits.canonize(base2).id();
      }

      size_t operator*(void) {
        size_t index = 2;
        for (size_t i = 0; i < indices.size(); ++i) {
          if (i == base1.type())
            index += limits.nskip(i) * limits.canonize(base1).id();
          else if (i == base2.type())
            index += limits.nskip(i) * limits.canonize(base2).id();
          else
            index += limits.nskip(i) * indices[i];
        }
        return index;
      }

      Index2Iterator &operator++(void) {
        for (int i = indices.size() - 1; i >= 0; --i) {
          if (i == base1.type()) {
            if (i == 0)
              ended = true;
          }
          else if (i == base2.type()) {
            if (i == 0)
              ended = true;
          }
          else {
            if (++indices[i] < limits.ntags(i))
              break;
            else if (i != 0)
              indices[i] = 0;
            else {
              ended = true;
              break;
            }
          }
        }
        return *this;
      }

      bool end(void) const {
        return ended;
      }

    private:
      TagLimits &limits;
      Tag base1;
      Tag base2;
      std::vector<uint16_t> indices;
      bool ended;
  };
}
