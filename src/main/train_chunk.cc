#include "base.h"

#include "config.h"
#include "fastmath.h"
#include "hashtable.h"
#include "io.h"
#include "lexicon.h"
#include "prob.h"
#include "tagset.h"
#include "vector.h"
#include "crf.h"
#include "crf/main.h"
#include "main.h"

int run(int argc, char *argv[]) {
  return NLP::CRF::run_train<NLP::CRF::Chunk>(argc, argv, "", "lbfgs");
}
