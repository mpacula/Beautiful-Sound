#ifndef PCM_H
#define PCM_H

#include "png_tools.h"

typedef struct {
  double* amp;
  int count;
  int pos_count;
  int neg_count;
} MultiSample;

typedef struct {
  MultiSample* samples;
  int length;
} MultiSignal;

MultiSample ms_make_sample(int, double*);
void ms_destroy_sample(MultiSample*);
void ms_render(int, double*, const PngImage*, int, int, int, int, int*);
void ms_fprint(FILE*, MultiSignal*);

#endif
 
