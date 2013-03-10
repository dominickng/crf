#include "base.h"

#include "crf.h"
#include "crf/main.h"
#include "main.h"

int run(int argc, char *argv[]) {
  return NLP::CRF::run_tag_factorial<NLP::CRF::NERFactorial>(argc, argv, "", "%w|%p|%c|%e \n", "%p %c %e");
}
