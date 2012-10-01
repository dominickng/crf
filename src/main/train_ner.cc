#include "base.h"

#include "hashtable.h"
#include "config.h"
#include "io.h"
#include "lbfgs.h"
#include "lexicon.h"
#include "tagset.h"
#include "crf/features.h"
#include "crf.h"
#include "crf/main.h"
#include "main.h"

int run(int argc, char *argv[]) {
  return NLP::run_train<NLP::CRF::NER>(argc, argv);
}
