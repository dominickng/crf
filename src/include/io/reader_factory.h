namespace NLP {
  class ReaderFactory : public Reader {
    private:
      const std::string fmt;
      Reader *reader;
    public:
      ReaderFactory(const std::string &name, const std::string &uri,
          std::istream &in, const std::string &fmt);

      virtual ~ReaderFactory(void);

      virtual bool next(Sentence &sent);

  };
}
