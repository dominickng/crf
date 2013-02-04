namespace NLP {
  class Format {
    public:
      std::string fields;
      std::string separators;
      std::string sent_pre;
      std::string sent_post;

      char word_sep;

      Format(const std::string &format) : fields(), separators(), sent_pre(), sent_post() {
        parse(format);
      }

      void parse(const std::string &format);
  };
}
