#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "libpngio/pngio.h"
#include "list.h"

#define BUF_SIZE 1024

typedef struct {
    uint8_t r, g, b, a;
} rgba;

typedef struct {
  size_t n_allocd;
  size_t n_filled;
  float *samples;
} samples_array;

struct draw_samples_state {
  rgba *img;
  size_t w, h;
  float maxAmplitude;
  size_t currentX;
};

static void *draw_samples(void *h, void *ctx) {
  samples_array *sa = h;
  struct draw_samples_state *st = ctx;
  //printf("n %zu/%zu; w %zu\n", sa->n_filled, sa->n_allocd, st->w);
  for (int i = 0; i < sa->n_filled; i++) {
    float v = logf(fabsf(sa->samples[i])) / st->maxAmplitude;
    //if (v > 0) printf("v %i\n", v);
    int vh = st->h * v;
    for (int y = st->h-vh; y < vh; y++) {
      st->img[y*st->w + (st->currentX + i)] = (rgba){0,0,0,255};
    }
  }
  st->currentX += sa->n_filled;
  return st;
}

static void free_samples_array(void *vsa) {
  free(((samples_array *)vsa)->samples);
}

int main(int argc, char const* argv[])
{
  size_t window = 64;
  int32_t *buf = malloc(BUF_SIZE * sizeof *buf);
  list *sampleList = NULL;
  size_t totalSamples = 0;
  float maxAmplitude = 0;
  samples_array *sa = NULL;
  size_t averagedSamples = 0;
  for (size_t n; (n = fread(buf, sizeof *buf, BUF_SIZE, stdin));) {
    if (n != BUF_SIZE) printf("buf %zu\n", n);
    for (size_t i = 0; i < n/2; i++, averagedSamples++) {
      if (!sa || sa->n_filled == sa->n_allocd) {
        sa = malloc(sizeof *sa);
        *sa = (samples_array){BUF_SIZE, 0, malloc(BUF_SIZE * sizeof(float))};
        list_mutcons(sa, &sampleList);
      }
      float *currentSample = &sa->samples[sa->n_filled];
      sa->samples[sa->n_filled] += buf[i*2];
      //if (fabsf(buf[i*2]) > fabsf(*currentSample))  {
      //  *currentSample = buf[i*2];
      //}
      if (averagedSamples == window) {
        float amp = logf(fabsf(*currentSample));
        if (amp > maxAmplitude) {
          maxAmplitude = amp;
        }
        totalSamples++;
        sa->n_filled++;
        averagedSamples = 0;
      }
    }
  }
  free(buf);
  printf("max: %f\n", maxAmplitude);

  size_t w = totalSamples, h = 300;
  rgba *img = calloc(w*h, sizeof *img);
  struct draw_samples_state st = {
    img, w, h, maxAmplitude, 0,
  };
  list_foldr(draw_samples, &st, sampleList);
  write_png_file((unsigned char *)img, w, h, sizeof *img, "out.png");
  printf("%zu x %zu\n", w, h);

  free(img);
  list_mapM_(free_samples_array, sampleList);
  list_mapM_(free, sampleList);
  list_freerec(sampleList);
  return 0;
}
