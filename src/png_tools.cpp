#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "png_tools.h"

double inline max(double a, double b) 
{
  if(a < b)
    return b;
  else
    return a;
}

double inline min(double a, double b) 
{
  if(a < b)
    return a;
  else
    return b;
}

long inline clip(long val, long min, long max) 
{
  if(val < min)
    return min;
  else if(val > max)
    return max;
  else
    return val;
}

double inline fclip(double val, double min, double max) 
{
  if(val < min)
    return min;
  else if(val > max)
    return max;
  else
    return val;
}

PngImage pngt_create(int width, int height)
{
  PngImage img;
  img.width = width;
  img.height = height;
  img.bpp = 3; // default to RGB, 8 bits per channel
  img.data = (char*)malloc(img.bpp*img.width*img.height);
  memset(img.data, 0, img.bpp*img.width*img.height);
  return img;
}

void pngt_destroy(PngImage* img)
{
  free(img->data);
  img->data = NULL;
}

void pngt_write(const char* path, const PngImage* img)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_byte bit_depth = 8;
  png_bytep* row_pointers;

  FILE* f = fopen(path, "wb");

  // convert data into PNG rows
  row_pointers = (png_bytep*)malloc(img->height*sizeof(png_bytep));
  for(int y=0; y<img->height; y++) {
    row_pointers[y] = (png_bytep)malloc(img->bpp*img->width);
    memcpy(row_pointers[y], &img->data[img->bpp*img->width*y], img->width*img->bpp);
  }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, f);
  png_set_IHDR(png_ptr, info_ptr, img->width, img->height,
               bit_depth, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, NULL);
  for (int y=0; y<img->height; y++)
    free(row_pointers[y]);
  free(row_pointers);

  fclose(f);
}

void pngt_draw_rect(const PngImage* img, int x, int y, int w, int h, int* rgb)
{
  int x2 = x+w;
  int y2 = y+h;

  int x_norm = min(x, x2);
  int y_norm = min(y, y2);
  int x2_norm = max(x, x2);
  int y2_norm = max(y, y2);

  for(int iy = y_norm; iy <= y2_norm; iy++) {
    for(int ix = x_norm; ix <= x2_norm; ix++) {
      pngt_set_pixel(img, ix, iy, rgb);
    }
  }
}

int pngt_get_index(const PngImage* img, int x, int y)
{
  return img->bpp*(y*img->width + x);
}

void pngt_set_pixel(const PngImage* img, int x, int y, int* rgb)
{
  x = clip(x, 0, img->width-1);
  y = clip(y, 0, img->height-1);
  int i = pngt_get_index(img, x, y);
  if(rgb[0] >= 0)
    img->data[i] = rgb[0];
  if(rgb[1] >= 0)
    img->data[i+1] = rgb[1];
  if(rgb[2] >= 0)
    img->data[i+2] = rgb[2];
}

void pngt_get_pixel(const PngImage* img, int x, int y, int* rgb)
{
  x = clip(x, 0, img->width-1);
  y = clip(y, 0, img->height-1);
  int i = pngt_get_index(img, x, y);
  rgb[0] = img->data[i] & 0xFF;
  rgb[1] = img->data[i+1] & 0xFF;
  rgb[2] = img->data[i+2] & 0xFF;
}


