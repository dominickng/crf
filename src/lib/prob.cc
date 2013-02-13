#ifndef _PROB_H
#define _PROB_H

#include <cmath>
#include <vector>
#include "base.h"
#include "prob.h"

namespace NLP {

  void vector_scale(PDF &vec, const double scale, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      vec[i] *= scale;
  }

  double vector_sum_log(PDF &vec, const size_t size) {
    double sum = 0.0;
    for (size_t i = 0; i < size; ++i)
      sum += log(vec[i]);
    return sum;
  }

  double vector_sum(PDF &vec, const size_t size) {
    double sum = 0.0;
    for (size_t i = 0; i < size; ++i)
      sum += vec[i];
    return sum;
  }

  void vector_print(PDF &vec, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      printf("%f ", vec[i]);
    printf("\n");
  }

  void vector_print(const double *vec, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      printf("%f ", vec[i]);
    printf("\n");
  }
}

#endif
