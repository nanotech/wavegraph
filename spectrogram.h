#include <stdint.h>
#include <vector>

extern "C" {
#include <fftw3.h>
}

struct spectrogram_plan {
public:
  spectrogram_plan(size_t window, size_t overlap);
  ~spectrogram_plan();
  void pad_input(std::vector<int32_t> &);

  fftw_plan plan;
  double *in_data;
  fftw_complex *out_data;
  size_t n;
  size_t window;
};

std::vector<double> spectrogram(const spectrogram_plan &, const std::vector<int32_t> &buf, size_t offset);
