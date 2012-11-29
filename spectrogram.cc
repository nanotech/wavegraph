#include "spectrogram.h"
#include <math.h>
#include <functional>

extern "C" {
#include <fftw3.h>
}

template <typename U, typename T>
static void map_with_index_into(std::function<U(T, size_t)> f, size_t n, T *v, U *o) {
  for (size_t i=0; i<n; ++i) {
    o[i] = f(v[i], i);
  }
}

static inline double magnitude(double a, double b)
{ return sqrt(a*a + b*b); }

static inline double hamming(double i, double n) {
  return 0.54 - (0.46 * cos(2 * M_PI * (i / ((n - 1) * 1.0))));
}

spectrogram_plan::spectrogram_plan(size_t n_) {
  n = n_;
  size_t out_n = n/2 + 1;

  in_data = (double *)fftw_malloc(n * sizeof(double));
  out_data = (fftw_complex *)fftw_malloc(out_n * sizeof(fftw_complex));
  plan = fftw_plan_dft_r2c_1d(n, in_data, out_data, FFTW_MEASURE);
}

spectrogram_plan::~spectrogram_plan() {
  fftw_destroy_plan(plan);
  fftw_free(out_data);
  fftw_free(in_data);
}

std::vector<double> spectrogram(
    const struct spectrogram_plan &plan,
    std::vector<int32_t> &buf, size_t offset)
{
  size_t n = plan.n;
  size_t out_n = n/2 + 1;
  std::vector<double> frequencies(n/4);
  auto *bufp = &buf;
  bool extendedBuf = false;

  if (offset + n > bufp->size()) {
    printf("extend %zu %zu %zu %zu\n", offset, n, offset+n, bufp->size());
    bufp = new std::vector<int32_t>(buf.begin(), buf.end());
    bufp->resize(n);
    extendedBuf = true;
    return frequencies;
  }

  map_with_index_into<double, int32_t>([=](int32_t x, size_t i){
      return (x / (double)INT32_MAX) * hamming(i, n);
  }, n, &(*bufp)[0] + offset, plan.in_data);

  fftw_execute(plan.plan);

  for (size_t i = 1; i < out_n/2; i++) {
    auto z = plan.out_data[i+out_n/2];
    frequencies[i-1] = magnitude(z[0], z[1]);
  }

  if (extendedBuf) {
    delete bufp;
  }

  return frequencies;
}
