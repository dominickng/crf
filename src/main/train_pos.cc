#include "base.h"

#include "crf.h"
#include "crf/main.h"
#include "main.h"

int run(int argc, char *argv[]) {
  return NLP::CRF::run_train<NLP::CRF::POS>(argc, argv, "%w|%p \n", "lbfgs");
}
