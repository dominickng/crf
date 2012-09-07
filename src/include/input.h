namespace NLP {
  extern const std::string PREFACE_HEADER;

  extern void read_preface(const std::string &uri, std::istream &input,
      std::string &preface, uint64_t &nlines, const bool required=true);

  extern void create_preface(int argc, char **argv, std::string &preface);

}
