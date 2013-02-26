namespace NLP {
  class Format {
    protected:
      void _parse_escape(const char **str, std::string &dest);
    public:
      std::string fields;
      std::string separators;
      std::string sent_pre;
      std::string sent_post;

      char word_sep;

      Format(const std::string &format)
        : fields(), separators(), sent_pre(), sent_post() {
        parse(format);
      }

      void parse(const std::string &format);
  };
}
