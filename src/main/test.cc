#include "base.h"
#include "config.h"
#include "hashtable.h"
#include "word.h"
#include "lexicon.h"

#include "io.h"

namespace config = Util::config;
namespace port = Util::port;
namespace hashtable = Util::HashTable;

class TestConfig : public config::Config {
  public:
    config::Op<int> int_option;
    config::Op<std::string> string_option;
    config::OpFlag flag_option;
    config::OpRestricted<std::string> restricted_option;
    config::OpList<std::string> list_option;
    config::OpInput input_option;
    config::OpOutput output_option;

    TestConfig(void) : Config("test", "This is a test program for the config package"),
      int_option(*this, "int", "this is a int option", 1),
      string_option(*this, "str", "this is a string option", "default"),
      flag_option(*this, "flag", "this is a flag option", false),
      restricted_option(*this, "restrict", "this is a restricted option", "a", "a|b|c|d"),
      list_option(*this, "list", "this is a list option", "a,b,c"),
      input_option(*this, "input", "this is an input option"),
      output_option(*this, "output", "this is an output option")
  { }
};

void test_lexicon(NLP::Lexicon &lexicon) {
  lexicon.add("hello", 200);
  lexicon.add("goodbye", 12);
  std::cout << lexicon.size() << ' ' << lexicon["hello"].str() << ' ' << lexicon["hello"].freq() << std::endl;
}

int main(int argc, char *argv[]) {
  NLP::Lexicon lexicon;
  test_lexicon(lexicon);

  TestConfig config;
  Util::Hasher::Hash hash;
  hashtable::HashTable<std::string, int> hash_table;
  std::cout << hash((int)40) << " " << hash("hello") << " " << hash("jello") << std::endl;
  hash_table["hello"] = 100;
  hash_table.add("goodbye", 1);
  std::cout << hash_table["hello"] << " " << hash_table["goodbye"] << std::endl;
  try {
    if(config.process(argc, argv)) {
      std::cout << config.int_option() << " " << config.string_option() << " ";
      std::cout << config.flag_option() << " " << config.restricted_option() << std::endl;
      for (std::vector<std::string>::iterator it = config.list_option.begin();
          it != config.list_option.end(); ++it)
        std::cout << *it << " ";
      std::cout << std::endl;

      NLP::Sentence sent;
      NLP::ReaderFactory reader("conll", "stdin", config.input_option.file(), config.input_option());

      while (reader.next(sent)) {
        for (NLP::Raws::iterator i = sent.entities.begin(); i != sent.entities.end(); ++i)
          std::cout << *i << " ";
        std::cout << std::endl;
        sent.reset();
      }
    }
  }
  catch (config::ConfigException &e) {
    std::cerr << port::RED << e.what() << port::OFF << std::endl;
    config.help(std::cerr);
  }
  return 0;
}
