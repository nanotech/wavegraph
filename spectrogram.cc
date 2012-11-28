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

std::vector<double> spectrogram(std::vector<int32_t> &buf, size_t n, size_t offset) {
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

  auto fft_in = (double *)fftw_malloc(n * sizeof(double));
  auto fft_out = (fftw_complex *)fftw_malloc(out_n * sizeof(fftw_complex));
  //memset(fft_out, 0, out_n * sizeof *fft_out);

  map_with_index_into<double, int32_t>([=](int32_t x, size_t i){
      return (x / (double)INT32_MAX) * hamming(i, n);
  }, n, &(*bufp)[0] + offset, fft_in);
  fftw_plan plan = fftw_plan_dft_r2c_1d(n, fft_in, fft_out, FFTW_ESTIMATE);

  fftw_execute(plan);
  fftw_destroy_plan(plan);
  fftw_free(fft_in); fft_in = NULL;

  for (size_t i = 0; i < out_n; i++) {
    //printf("%f x %fi\n", fft_out[i][0], fft_out[i][1]);
  }

  for (size_t i = 1; i < out_n/2; i++) {
    auto z = fft_out[i+out_n/2];
    frequencies[i-1] = magnitude(z[0], z[1]);
    //printf("%f\n", frequencies[i-1]);
  }

  if (extendedBuf) {
    delete bufp;
  }

  fftw_free(fft_out);

  return frequencies;
}
