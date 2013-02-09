#include "base.h"

#include "hashtable.h"
#include "config.h"
#include "io.h"
#include "lbfgs.h"
#include "lexicon.h"
#include "tagset.h"
#include "crf.h"
#include "crf/main.h"
#include "main.h"

int run(int argc, char *argv[]) {
  return NLP::CRF::run_tag<NLP::CRF::POS>(argc, argv, "%w \n", "%w|%p \n");
}
