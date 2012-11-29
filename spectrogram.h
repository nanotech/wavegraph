#include <stdint.h>
#include <vector>

extern "C" {
#include <fftw3.h>
}

struct spectrogram_plan {
public:
  spectrogram_plan(size_t n);
  ~spectrogram_plan();

  fftw_plan plan;
  double *in_data;
  fftw_complex *out_data;
  size_t n;
};

std::vector<double> spectrogram(const spectrogram_plan &, std::vector<int32_t> &buf, size_t offset);
