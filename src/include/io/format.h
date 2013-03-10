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

      Format(const std::string &format, const bool chain=false)
        : fields(), separators(), sent_pre(), sent_post() {
          if (chain)
            parse_chain(format);
          else
            parse(format);
      }

      void parse(const std::string &format);
      void parse_chain(const std::string &format);
  };
}
