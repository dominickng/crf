namespace NLP {
  class WriterFactory : public Writer {
    private:
      const std::string fmt;
      Writer *writer;
    public:
      WriterFactory(const std::string &name, const std::string &uri,
          std::ostream &out, const std::string &fmt);

      virtual ~WriterFactory(void);

      virtual bool next(Sentence &sent);

  };
}
