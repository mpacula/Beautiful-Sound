#ifndef PNG_TOOLS_H
#define PNG_TOOLS_H

#include <png.h>

typedef struct
{
  int width;
  int height;
  char *data;
  int bpp;
} PngImage;

PngImage pngt_create(int, int);
void pngt_destroy(PngImage*);
void pngt_write(const char*, const PngImage*);
int pngt_get_index(const PngImage*, int, int);
void pngt_set_pixel(const PngImage*, int, int, int*);
void pngt_get_pixel(const PngImage*, int, int, int*);
void pngt_draw_rect(const PngImage*, int, int, int, int, int*);

#endif
