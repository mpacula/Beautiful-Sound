#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pcm.h"

MultiSignal ms_make_empty(int length)
{
  MultiSignal sig;
  sig.length = length;
  sig.samples = (MultiSample*)malloc(sizeof(MultiSample)*length);
  for(int i = 0; i < length; i++) {
    sig.samples[i].amp = NULL;
  }
  return sig;
}

/**
 * Constructs a multisignal from a source array of samples. If multiple
 * source samples alias to the same multisignal sample, they will
 * be a part of the same multisample.
 */
MultiSignal ms_make(int length, int src_length, double* samples) 
{
  MultiSignal sig = ms_make_empty(length);
  if(length > src_length) {
    fprintf(stderr, "MultiSample stretching not supported.\n");
    return sig;
  }

  const double r = (double)length/src_length;
  const int max_aliased_samples = ceil(1.0/r);
  double* aliased_samples = (double*)malloc(sizeof(double)*max_aliased_samples);
  int c_aliased = 0;
  int i_aliased = 0;
  memset(aliased_samples, 0, sizeof(double)*max_aliased_samples);
  for(int i = 0; i < src_length; i++) {
    int j = (int)(r*i);
    if(j > i_aliased) {
      sig.samples[i_aliased] = ms_make_sample(c_aliased, aliased_samples);
      c_aliased = 0;
    }
    
    aliased_samples[c_aliased] = samples[i];
    i_aliased = j;
    c_aliased++;
  }
  if(c_aliased > 0)
    sig.samples[i_aliased] = ms_make_sample(c_aliased, aliased_samples);

  free(aliased_samples);
  return sig;
}

void ms_destroy(MultiSignal* sig)
{
  for(int i = 0; i < sig->length; i++) {
    ms_destroy_sample(&sig->samples[i]);
  }
  free(sig->samples);
  sig->samples = NULL;
  sig->length = 0;
}

/**
 * Quantizes a multisample into a given array with specified low and high ranges.
 * Each element in the quantized array will be equal to the fraction of samples
 * with magnitude:
 *   - equal to or smaller than the ceiling of the quantum (negative samples)
 *   - equal to or greater than the floor of the quantum (positive samples)
 */
void ms_quantize_sample(MultiSample* sample, const int quants,
			const double lo, const double hi, double* out)
{
  memset(out, 0, quants*sizeof(double));
  const int q_lo = (int)floor(-lo/(hi-lo)*quants);

  for(int i = 0; i < sample->count; i++) {
    double sample_val = sample->amp[i];
    const int q_hi = (int)floor((sample_val-lo)/(hi-lo)*quants);
    if(sample_val >= 0) {
      for(int q = q_lo+1; q < q_hi; q++) {
        out[q] += 1.0/sample->pos_count;        
      }
    }
    else {
      for(int q = q_lo; q >= q_hi; q--) {
        out[q] += 1.0/sample->neg_count;        
      }
    }
  }
}

MultiSample ms_make_sample(int count, double* vals) 
{
  MultiSample ms;
  ms.amp = (double*)malloc(sizeof(double)*count);
  ms.count = count;
  ms.pos_count = ms.neg_count = 0;
  for(int i = 0; i < count; i++) { 
    ms.amp[i] = vals[i];
    if(vals[i] >= 0)
      ms.pos_count++;
    else 
      ms.neg_count++;
  }
  return ms;
}

void ms_destroy_sample(MultiSample* sample) 
{
  if(sample->amp != NULL)
    free(sample->amp);
  sample->amp = NULL;
  sample->count = sample->pos_count = sample->neg_count = 0;
}

void ms_fprint_sample(FILE* f, MultiSample* sample) 
{
  for(int i = 0; i < sample->count; i++) {
    fprintf(f, "%.3f ", sample->amp[i]);
  }
  fprintf(f, "\n");
}

void ms_fprint_quantized_sample(FILE* f, MultiSample* sample, int quants, double lo, double hi)
{
  double* quantized = (double*)malloc(quants*sizeof(double));
  ms_quantize_sample(sample, quants, lo, hi, quantized);

  fprintf(f, "%.3f [", lo);
  for(int i = 0; i < quants; i++) {
    if(i > 0)
      fprintf(f, " ");
    fprintf(f, "%.3f", quantized[i]);
  }
  fprintf(f, "] %.3f\n", hi);
  free(quantized);
}

void ms_fprint(FILE* f, MultiSignal* sig) 
{
  fprintf(f, "MultiSignal length: %d\n", sig->length);
  for(int i = 0; i < sig->length; i++) {
    ms_fprint_sample(f, &sig->samples[i]);
  }
}

void ms_render(int length, double* samples, const PngImage* img, int x, int y, int w, int h, int* rgb)
{ 
  MultiSignal sig = ms_make(w, length, samples);

  double* quants = (double*)malloc(h*sizeof(double));
  y += (h-1);
  for(int i = 0; i < w; i++) {
    int ix = x+i;
    MultiSample* sample = &sig.samples[i];
    ms_quantize_sample(sample, h, -1.0, 1.0, quants);

    for(int q = 0; q < h; q++) { // q=0 corresponds to the smallest value
      double intensity = fabs(quants[q]);

      int r = rgb[0] >= 0 ? (int)(intensity*rgb[0]) : -1; 
      int g = rgb[1] >= 0 ? (int)(intensity*rgb[1]) : -1;
      int b = rgb[2] >= 0 ? (int)(intensity*rgb[2]) : -1;
      int color[3] = { r, g, b };
      pngt_set_pixel(img, ix, y-q, color);
      
//       printf("%d %d %d\n", ix,  y-q, *color);
    }
  }

  ms_destroy(&sig);
  free(quants);
}
