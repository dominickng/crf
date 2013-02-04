namespace NLP {
  class FormatWriter : public Writer {
    protected:
      Format format;

    public:
      FormatWriter(const std::string &uri, std::ostream &out, const std::string &format) :
        Writer(uri, out), format(format) { }

      virtual bool next(Sentence &sent);

  };
}
