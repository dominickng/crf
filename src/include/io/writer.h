namespace NLP {
  class Writer {
    protected:
      const std::string uri;
      std::ostream &out;

    public:
      Writer(const std::string &uri, std::ostream &out)
        : uri(uri), out(out) { }

      virtual ~Writer(void) { }

      virtual bool next(Sentence &sent) = 0;

      virtual void write_preface(const std::string &preface) {
        out << preface << '\n';
      }

      void die(const std::string &msg) {
        throw IOException(msg, uri);
      }
  };
}
