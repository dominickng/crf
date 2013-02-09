namespace NLP {
  class FormatReader : public Reader {
    private:
      const static size_t BUFFER_SIZE = 1024 * 1024;

      Format format;
      std::string preface;
      uint64_t _nlines;
      size_t _len;
      char _buffer[BUFFER_SIZE];
      using Reader::in;

      bool next_line(void);

    public:
      FormatReader(const std::string &uri, std::istream &input, const std::string &fmt);

      virtual ~FormatReader(void) { }

      virtual bool next(Sentence &sent);

      virtual void reset(void);
  };
}
