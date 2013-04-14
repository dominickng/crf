#include "base.h"

#include "hashtable.h"

#include "factor/variable.h"
#include "factor/factor.h"
#include "factor/message_map.h"

namespace NLP {
  namespace Hash = Util::Hasher;
  namespace HT = Util::hashtable;

  class MessageEntry {
    private:
      MessageEntry(const void *from, const void *to, const size_t nmessages, MessageEntry *next) :
        from(from), to(to), nmessages(nmessages), next(next) { };

      ~MessageEntry(void) { }

      void *operator new(size_t size, Util::Pool *pool, size_t nmessages) {
        return pool->alloc(size + sizeof(double) * (nmessages - 1)); // -1 for 1 sized array
      }

      void operator delete(void *, Util::Pool, size_t) { /* do nothing */ }

    public:
      const void *from;
      const void *to;
      const size_t nmessages;
      MessageEntry *next;
      double messages[1];

      static Hash::Hash hash(const void *from, const void *to) {
        return Hash::Hash((uint64_t)from * 31 + (uint64_t)to * 37);
      }

      static MessageEntry *create(Util::Pool *pool, const uint64_t index,
          const uint64_t value, const Hash::Hash hash, MessageEntry *next) {
        return NULL;
      }

      static MessageEntry *create(Util::Pool *pool, const void *from,
          const void *to, size_t nmessages, MessageEntry *next) {
        MessageEntry *entry = new (pool, nmessages) MessageEntry(from, to, nmessages, next);
        return entry;
      }

      bool equal(const void *from, const void *to) {
        return this->from == from && this->to == to;
      }

      MessageEntry *find(const Hash::Hash hash, const uint64_t value) {
        return NULL;
      }

      double *find(const void *from, const void *to) {
        for (MessageEntry *l = this; l != NULL; l = l->next)
          if (l->equal(from, to))
            return l->messages;
        return NULL;
      }

      size_t nchained(void) const {
        return next ? next->nchained() + 1 : 1;
      }
  };

  typedef HT::OrderedHashTable<MessageEntry, uint64_t> ImplBase;

  class MessageMap::Impl : public ImplBase, public Util::Shared {
    public:
      Impl(const size_t nbuckets, const size_t pool_size)
        : ImplBase(nbuckets, pool_size), Shared() { }

      virtual ~Impl(void) { /* do nothing */ }

      using ImplBase::find;
      using ImplBase::insert;

      double *find(const void *from, const void *to) {
        return _buckets[MessageEntry::hash(from, to).value() % _nbuckets]->find(from, to);
      }

      double *insert(const void *from, const void *to, size_t nmessages) {
        size_t bucket = MessageEntry::hash(from, to).value() % _nbuckets;
        MessageEntry *entry = MessageEntry::create(ImplBase::_pool, from, to, nmessages, _buckets[bucket]);
        std::fill(entry->messages, entry->messages + nmessages, 1.0);
        _buckets[bucket] = entry;
        _entries.push_back(entry);
        ++_size;
        return entry->messages;
      }

      double *insert(const void *from, const void *to, size_t nmessages, size_t index, double value) {
        double *messages = insert(from, to, nmessages);
        messages[index] = value;
        return messages;
      }

      void add(const void *from, const void *to, size_t nmessages, size_t index, double value) {
        double *messages = find(from, to);
        if (messages)
          messages[index] = value;
        else
          insert(from, to, nmessages, index, value);
      }

      void normalize(const void *from, const void *to, const double norm, const size_t nmessages) {
        double *messages = find(from, to);
        for (size_t i = 0; i < nmessages; ++i)
          messages[i] *= norm;
      }

      void reset(void) {
        for (Entries::iterator i = _entries.begin(); i != _entries.end(); ++i)
          std::fill((*i)->messages, (*i)->messages + (*i)->nmessages, 1.0);
      }

  };

  MessageMap::MessageMap(const size_t nbuckets, const size_t pool_size)
    : _impl(new Impl(nbuckets, pool_size)) { }

  MessageMap::MessageMap(const MessageMap &other) :
    _impl(share(other._impl)) { }

  MessageMap::~MessageMap(void) { release(_impl); }

  void MessageMap::add(const Variable *from, const Factor *to, size_t index, double value) {
    _impl->add(from, to, from->ntags, index, value);
  }

  void MessageMap::add(const Factor *from, const Variable *to, size_t index, double value) {
    _impl->add(from, to, to->ntags, index, value);
  }

  void MessageMap::normalize(const Factor *from, const Variable *to, const double norm) {
    _impl->normalize(from, to, norm, to->ntags);
  }

  double MessageMap::get(const Variable *from, const Factor *to, size_t index) {
    double *messages = get(from, to);
    if (messages)
      return messages[index];
    return 1.0; //not initialized yet
  };

  double MessageMap::get(const Factor *from, const Variable *to, size_t index) {
    double *messages = get(from, to);
    if (messages)
      return messages[index];
    return 1.0; //not initialized yet
  }

  double *MessageMap::get(const Variable *from, const Factor *to) {
    return _impl->find(from, to);
  }

  double *MessageMap::get(const Factor *from, const Variable *to) {
    return _impl->find(from, to);
  }

  void MessageMap::operator()(const Variable *from, const Factor *to, size_t index, double value) { add(from, to, index, value); };
  void MessageMap::operator()(const Factor *from, const Variable *to, size_t index, double value) { add(from, to, index, value); };
  double MessageMap::operator()(const Variable *from, const Factor *to, size_t index) { return get(from, to, index); };
  double MessageMap::operator()(const Factor *from, const Variable *to, size_t index) { return get(from, to, index); };
  double *MessageMap::operator()(const Variable *from, const Factor *to) { return get(from, to); };
  double *MessageMap::operator()(const Factor *from, const Variable *to) { return get(from, to); };

  size_t MessageMap::size(void) const { return _impl->size(); }

  void MessageMap::clear(void) { _impl->clear(); }

  void MessageMap::reset(void) { _impl->reset(); }
}
