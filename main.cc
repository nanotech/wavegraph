#include <cstdio>
#include <cstdint>
#include <cmath>
#include <vector>
#include <deque>

extern "C" {
#include "libpngio/pngio.h"
}

#define BUF_SIZE 1024

typedef struct {
    uint8_t r, g, b, a;
} rgba;

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

template <typename F, typename T, typename L>
T foldl(F f, T acc, L l) { for (auto x : l) { acc = f(x, acc); } return acc; }

template <typename T>
static size_t array_list_length(const std::deque<std::vector<T>> &l) {
  return foldl([](std::vector<T> v, size_t n){ return v.size() + n; }, size_t(0), l);
}

int main(int argc, char const* argv[])
{
  auto dat = read_file_chunked<int32_t>(stdin, BUF_SIZE);
  size_t datSize = array_list_length(dat);

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

  const char *outfile = argc == 2 ? argv[1] : "out.png";

  write_png_file((unsigned char *)img.data(), w, h, sizeof *img.data(), outfile);

  return 0;
}
