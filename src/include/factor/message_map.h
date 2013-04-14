/**
 * message_map.h
 * Defines a mapping between variables and factors to messages between them.
 */
namespace NLP {
  namespace HT = Util::hashtable;

  class MessageMap {
    private:
      class Impl;
      Impl *_impl;

    public:
      MessageMap(const size_t nbuckets=HT::SMALL, const size_t pool_size=HT::MEDIUM);
      MessageMap(const std::string &filename, const size_t nbuckets=HT::SMALL,
          const size_t pool_size=HT::MEDIUM);
      MessageMap(const std::string &filename, std::istream &input,
          const size_t nbuckets=HT::SMALL, const size_t pool_size=HT::MEDIUM);
      MessageMap(const MessageMap &other);

      ~MessageMap(void);

      // message from variable to factor for tag
      void add(const Variable *variable, const Factor *factor, size_t index, double msg);

      // message from factor to variable for tag
      void add(const Factor *factor, const Variable *variable, size_t index, double msg);

      void normalize(const Factor *factor, const Variable *variable, const double norm);

      double get(const Variable *from, const Factor *to, size_t index);
      double get(const Factor *from, const Variable *to, size_t index);
      double *get(const Variable *from, const Factor *to);
      double *get(const Factor *from, const Variable *to);

      void operator()(const Variable *from, const Factor *to, size_t index, double value);
      void operator()(const Factor *from, const Variable *to, size_t index, double value);
      double operator()(const Variable *from, const Factor *to, size_t index);
      double operator()(const Factor *from, const Variable *to, size_t index);
      double *operator()(const Variable *from, const Factor *to);
      double *operator()(const Factor *from, const Variable *to);

      size_t size(void) const;

      void clear(void);
      void reset(void);
  };

}
