#ifndef _POOL_H
#define _POOL_H

#include <cstdlib>
#include <vector>

namespace Util {
  class MemoryZone {
    private:
      const size_t _size;
      char *const _begin;
      char *_current;
      char *const _end;
    public:
      MemoryZone(const size_t size) :
        _size(size), _begin(new char[size]), _current(_begin), _end(_begin + size) { }

      ~MemoryZone(void) { delete [] _begin; }

      char *alloc(size_t size) {
        if (_current + size < _end) {
          char *result = _current;
          _current += size;
          return result;
        }
        return NULL;
      }

      void clear(void) { _current = _begin; }
      size_t size(void) const { return _size; }
      size_t used(void) const { return _current - _begin; }
      size_t unused(void) const { return _end - _current; }
  };

  typedef std::vector<MemoryZone *> MemoryZones;

  class Pool {
    private:
      const size_t _min_size;

      MemoryZones _used;
      MemoryZones _unused;

      MemoryZone *_current;

    public:
      Pool(const size_t min_size) :
        _min_size(min_size), _used(), _unused(), _current(new MemoryZone(min_size)) { }

      ~Pool(void) {
        delete _current;

        for (MemoryZones::iterator it = _used.begin(); it != _used.end(); ++it)
          delete *it;
        for (MemoryZones::iterator it = _unused.begin(); it != _unused.end(); ++it)
          delete *it;
      }

      char *alloc(size_t size) {
        char *buf = _current->alloc(size);
        if (buf)
          return buf;
        if (size < _min_size) {
          _used.push_back(_current);
          if (_unused.size() > 0) {
            _current = _unused.back();
            _unused.pop_back();
          }
          else
            _current = new MemoryZone(_min_size);
          buf = _current->alloc(size);
        }
        else {
          MemoryZone *tmp = new MemoryZone(size);
          buf = tmp->alloc(size);
          _unused.push_back(tmp);
        }
        return buf;
      }

      void clear(void) {
        _current->clear();
        _unused.insert(_unused.end(), _used.begin(), _used.end());
        for (MemoryZones::iterator it = _used.begin(); it != _used.end(); ++it)
          (*it)->clear();

        _used.resize(0);
      }

      size_t min_size(void) const { return _min_size; }
      size_t nzones(void) const { return _used.size(); }
      size_t size(void) const {
        size_t nbytes = _current->size();
        for (MemoryZones::const_iterator it = _used.begin(); it != _used.end(); ++it)
          nbytes += (*it)->size();
        for (MemoryZones::const_iterator it = _unused.begin(); it != _unused.end(); ++it)
          nbytes += (*it)->size();
        return nbytes;
      }
  };
}

void *operator new[](const size_t size, Util::Pool *pool) throw() {
  return (void *) pool->alloc(size);
}

void operator delete[](void *ptr, Util::Pool *pool) throw() { }

#endif
