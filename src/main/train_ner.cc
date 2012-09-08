#include "crf/main.h"
#include "main.h"

int run(int argc, char *argv[]) {
  return NLP::run_train<NLP::CRF::NER>(argc, argv);
}
