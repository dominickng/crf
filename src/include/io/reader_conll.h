namespace NLP {
  class CoNLLReader : public Reader {
    private:
      const static size_t BUFFER_SIZE = 1024 * 1024;

      uint64_t _nlines;
      size_t _len;
      char _buffer[BUFFER_SIZE];

      using Reader::in;

      bool next_line(void);

    public:
      CoNLLReader(const std::string &uri, std::istream &input);

      virtual bool next(Sentence &sent);

      virtual void reset(void);
  };
}
