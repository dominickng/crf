namespace NLP {
  class ReaderFactory : public Reader {
    private:
      const std::string fmt;
      Reader *reader;
    public:
      ReaderFactory(const std::string &name, const std::string &uri,
          std::istream &input, const std::string &fmt);

      virtual bool next(Sentence &sent);

  };
}
