#include <cstdio>
#include <cstdint>
#include <cmath>
#include <float.h>
#include <vector>
#include <deque>
#include "spectrogram.h"

extern "C" {
#include "libpngio/pngio.h"
#include "libhue/libhue.h"
}

#define BUF_SIZE 1024

typedef hue_RGBA8 rgba;

static std::vector<rgba> draw_samples(
    const std::vector<float> &sa, float maxAmplitude, size_t h)
{
  size_t w = sa.size();
  std::vector<rgba> img(w*h);
  for (size_t i = 0; i < sa.size(); i++) {
    float v = (sa[i] == 0.0f) ? 0.0f : logf(fabsf(sa[i])) / maxAmplitude;
    size_t vh = size_t(v * h);
    //for (size_t y = h - vh; y < vh; y++) {
    //  img[y*w + i] = rgba{0,0,0,255};
    //}
    rgba c = {255,255,255,255};
    for (size_t y = 0; y < h - vh; y++) {
      img[y*w + i] = c;
    }
    for (size_t y = vh; y < h; y++) {
      img[y*w + i] = c;
    }
  }
  return img;
}

static inline double lerp(std::vector<double> &xs, double i) {
  double p = i - floor(i);
  return xs[size_t(floor(i))]*p + xs[size_t(ceil(i))]*(1-p);
}

static std::vector<rgba> draw_spectrogram(
    std::vector<std::vector<double>> &frequencies, double maxAmplitude,
    size_t w, size_t &h)
{
  size_t hs = 4;
  size_t ih = h/hs;
  std::vector<rgba> img(w*ih);
  for (size_t x=0; x<w; ++x) {
    for (size_t y=0; y<ih; ++y) {
      size_t y2 = log1p(y*hs)/log(h)*(h-1);
      if (y2 >= h) {
        printf("y2: %zu\n", y2);
        y2 = y;
      }
      //auto v = frequencies[x][y2] / maxAmplitude;
      auto v = lerp(frequencies[x], y2) / maxAmplitude;
      if (v < 0 || v > 1 || v != v) {
        printf("v: %f\n", v);
      }
      img[y*w + x] = hue_RGB_to_RGBA8(hue_HSL_to_RGB(hue_HSL{y/float(ih),0.7,float(v)}), 1.0);
    }
  }
  h = ih;
  return img;
}

template <typename T>
static std::deque<std::vector<T>> read_file_chunked(FILE *fd, size_t blockSize) {
  std::deque<std::vector<T>> l;
  size_t n;
  do {
    std::vector<T> v(blockSize);
    n = fread(v.data(), sizeof(T), blockSize, fd);
    if (n > 0)  {
      v.resize(n);
      l.push_back(v);
    }
  } while (n > 0);
  return l;
}

template <typename T>
static std::vector<T> concat_chunks(std::deque<std::vector<T>> l) {
  std::vector<T> o;
  for (auto &v : l) {
    for (auto &x : v) {
      o.push_back(x);
    }
  }
  return o;
}
template <typename F, typename T, typename L>
T foldl(F f, T acc, L l) { for (auto x : l) { acc = f(x, acc); } return acc; }

template <typename T>
static size_t array_list_length(const std::deque<std::vector<T>> &l) {
  return foldl([](std::vector<T> v, size_t n){ return v.size() + n; }, size_t(0), l);
}

template <typename T>
static std::vector<T> destride(std::vector<T> v, size_t stride, size_t offset) {
  std::vector<T> o(v.size());
  for (size_t i=offset, n=v.size(); i < n; i += stride) {
    o[i] = v[i];
  }
  return o;
}

int main(int argc, char const* argv[])
{
  auto dat = read_file_chunked<int32_t>(stdin, BUF_SIZE);
  size_t datSize = array_list_length(dat);

  const char *outfile = argc == 2 ? argv[1] : "out.png";

  bool spectromode = true;

  if (spectromode) {
    std::vector<int32_t> samples = destride(concat_chunks(dat), 2, 0);
    std::vector<std::vector<double>> frequencies;
    float maxAmplitude = 0;
    //for (auto &buf : dat) {
    size_t window = 1024*2;
    size_t overlap = 8;
    printf("n: %zu\n", samples.size());
    for (size_t k=0, n=samples.size()/window; k<n; ++k) {
      //printf("%zu-%zu\n", k*window, k*window + window*overlap);
      auto freqs = spectrogram(samples, window*overlap, k*window);
      //if (freqs.size() != BUF_SIZE/2) {
        //printf("skip: %zu != %zu\n", freqs.size(), BUF_SIZE/2);
        //continue;
      //}
      for (auto &amp : freqs) {
        amp = log1p(log1p(amp));
        if (amp > maxAmplitude) maxAmplitude = amp;
      }
      frequencies.push_back(std::move(freqs));
    }
    printf("max: %f\n", maxAmplitude);
    size_t w = frequencies.size(), h = window / 4 * overlap;
    printf("%zu x %zu\n", w, h);
    auto img = draw_spectrogram(frequencies, maxAmplitude, w, h);
    write_png_file((unsigned char *)img.data(), w, h, sizeof *img.data(), outfile);
  } else {
    size_t window = datSize / 700 / 2;
    std::vector<float> samples;
    float maxAmplitude = 0;
    size_t averagedSamples = 0;
    float sampleAccum = 0;

    for (auto &buf : dat) {
      size_t n = buf.size();

      if (n != BUF_SIZE) printf("buf %zu\n", n);
      for (size_t i = 0; i < n/2; i++, averagedSamples++) {
        sampleAccum += buf[i*2];
        //if (fabsf(buf[i*2]) > fabsf(sampleAccum))  {
        //  sampleAccum = buf[i*2];
        //}
        if (averagedSamples == window) {
          float amp = logf(fabsf(sampleAccum));
          if (amp > maxAmplitude) {
            maxAmplitude = amp;
          }
          samples.push_back(sampleAccum);
          sampleAccum = 0;
          averagedSamples = 0;
        }
      }
    }

    printf("max: %f\n", maxAmplitude);

    size_t w = samples.size(), h = 80;
    printf("%zu x %zu\n", w, h);
    auto img = draw_samples(samples, maxAmplitude, h);

    write_png_file((unsigned char *)img.data(), w, h, sizeof *img.data(), outfile);
  }

  return 0;
}
