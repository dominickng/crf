#include "base.h"

#include "config.h"
#include "fastmath.h"
#include "hashtable.h"
#include "io.h"
#include "lbfgs.h"
#include "lexicon.h"
#include "prob.h"
#include "tagset.h"
#include "vector.h"
#include "crf.h"
#include "crf/main.h"
#include "main.h"

int run(int argc, char *argv[]) {
  return NLP::CRF::run_tag<NLP::CRF::NER>(argc, argv, "", "%w|%p|%c|%e \n");
}
