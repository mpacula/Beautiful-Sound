#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sndfile.h>
#include <math.h>
#include <complex.h>
#include <png.h>

#include "png_tools.h"
#include "pcm.h"

using namespace std;

typedef struct 
{
  string outputPath;
  int width;
  int height;
  char *songname;
} Options;


void usage(char **argv) 
{
  fprintf(stderr, "Usage: %s -song SONGFILENAME -out OUT.PNG \n", argv[0]);
}

void print_info(const SF_INFO* info) 
{
  fprintf(stderr, "Format: 0x%X\nFrames: %lld\nSample rate: %dHz\nChannels: %d\n",
          info->format, info->frames, info->samplerate, info->channels);
}

void get_channel_data(const int n, const int channels,
                 const int ch, const double* frames, double* chout) 
{
  for(size_t i=0; i<n; i++) {
    chout[i] = frames[channels*i+ch];
  }
}

int main(int argc, char** argv)
{ 
  Options opt;
  opt.outputPath = "out.png";
  opt.width = 1920;
  opt.height = 1080;

  if(argc < 4) {
    usage(argv);
    return 1;
  }

  for(int i = 1; i < argc; i++) {
        if(i < argc - 1) {
    /*      if(strcmp(argv[i], "-w") == 0) {
            opt.width = atoi(argv[i+1]);
            i++;
            continue;
        } else if(strcmp(argv[i], "-h") == 0) {
            opt.height = atoi(argv[i+1]);
            i++;
            continue;
        } */
        if(strcmp(argv[i], "-song") == 0) {
            opt.songname= argv[i+1];
            i++;
        }else if(strcmp(argv[i], "-out") == 0) {
            opt.outputPath = argv[i+1];
            i++;
        }
    }
  }

  SF_INFO sndInfo;
  sndInfo.format = 0;
  SNDFILE* inFile;

  if(!(inFile = sf_open(opt.songname, SFM_READ, &sndInfo))) {
    fprintf(stderr, "Error reading input.\n");
    return 1;
  }
  print_info(&sndInfo);

  if(sndInfo.channels != 2) {
    fprintf(stderr, "Only stereo input is supported, but your file has %d channel(s)\n",
            sndInfo.channels);
    return 1;
  }

  const size_t N = 100000;
  const size_t buf_len = N*sndInfo.channels;
  double* readBuffer = (double*)malloc(buf_len*sizeof(double));
//   double* writeBuffer = (double*)malloc(buf_len*sizeof(double));

  SF_INFO outSndInfo = sndInfo;
  SNDFILE* outFile = sf_open(opt.outputPath.c_str(), SFM_WRITE, &outSndInfo);

  sf_count_t c_frames;
  sf_count_t i_frame = 0;

  double* samples_ch1 = (double*)malloc(sizeof(double) * sndInfo.frames);
  double* samples_ch2 = (double*)malloc(sizeof(double) * sndInfo.frames);

  while((c_frames = sf_readf_double(inFile, readBuffer, buf_len / sndInfo.channels)) > 0) {
    get_channel_data(c_frames, sndInfo.channels, 0, readBuffer, &samples_ch1[i_frame]);
    get_channel_data(c_frames, sndInfo.channels, 1, readBuffer, &samples_ch2[i_frame]);
    i_frame += c_frames;
  }

  free(readBuffer);
//   free(writeBuffer);
  sf_close(inFile);   

  PngImage img = pngt_create(opt.width, opt.height);
  int rgb_ch1[3] = {-1, 255, -1};
  ms_render(sndInfo.frames, samples_ch1, &img, 0, 0, img.width, img.height, rgb_ch1);

  int rgb_ch2[3] = {255, -1, -1};
  ms_render(sndInfo.frames, samples_ch2, &img, 0, 0, img.width, img.height, rgb_ch2);

  pngt_write(opt.outputPath.c_str(), &img);

  pngt_destroy(&img);  
  free(samples_ch1);
  free(samples_ch2);
}
