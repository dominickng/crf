#ifndef _PROB_H
#define _PROB_H

#include <cmath>
#include <vector>
#include "base.h"
#include "prob.h"

namespace NLP {

  void vector_scale(PDF &vec, const double scale) {
    for (size_t i = 0; i < vec.size(); ++i)
      vec[i] *= scale;
  }

  double vector_sum_log(PDF &vec) {
    double sum = 0.0;
    for (size_t i = 0; i < vec.size(); ++i)
      sum += log(vec[i]);
    return sum;
  }
}

#endif
