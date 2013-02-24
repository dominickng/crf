/**
 * input.h
 * Defines methods related to reading and creating useful
 * prefaces for input and output files. These prefaces store the
 * program invocation used to generate the provided file.
 */
namespace NLP {
  extern const std::string PREFACE_HEADER;

  extern void read_preface(const std::string &uri, std::istream &input,
      std::string &preface, uint64_t &nlines, const bool required=true);

  extern void create_preface(int argc, char **argv, std::string &preface);

}
