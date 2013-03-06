#include "base.h"

#include "crf.h"
#include "crf/main.h"
#include "main.h"

int run(int argc, char *argv[]) {
  return NLP::CRF::run_tag<NLP::CRF::Chunk>(argc, argv, "", "%w|%p|%c \n");
}
