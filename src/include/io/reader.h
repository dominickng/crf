namespace NLP {
  class Reader {
    protected:
      const std::string uri;
      std::istream &input;

    public:
      Reader(const std::string &uri, std::istream &input)
        : uri(uri), input(input) { }

      virtual bool next(Sentence &sent) = 0;

      virtual void reset(void) {
        input.clear();
        input.seekg(0, std::ios::beg);
        if (!input)
          throw IOException("input stream could not be seeked to the beginning", uri);
      };

      void die(const std::string &msg) {
        throw IOException(msg, uri);
      }
  };
}
