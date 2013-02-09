namespace NLP {
  class Reader {
    protected:
      const std::string uri;
      std::istream &in;

    public:
      Reader(const std::string &uri, std::istream &in)
        : uri(uri), in(in) { }

      virtual ~Reader(void) { }

      virtual bool next(Sentence &sent) = 0;

      virtual void reset(void) {
        in.clear();
        in.seekg(0, std::ios::beg);
        if (!in)
          throw IOException("input stream could not be seeked to the beginning", uri);
      };

      void die(const std::string &msg) {
        throw IOException(msg, uri);
      }
  };
}
