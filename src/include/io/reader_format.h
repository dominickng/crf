namespace NLP {
  class FormatReader : public Reader {
    private:
      Format format;
      const static size_t BUFFER_SIZE = 1024 * 1024;
      size_t _len;
      char _buffer[BUFFER_SIZE];
      uint64_t _nlines;
      using Reader::in;

      bool next_line(void);

    public:
      FormatReader(const std::string &uri, std::istream &input, const std::string &fmt);

      virtual bool next(Sentence &sent);

      virtual void reset(void);
  };
}
