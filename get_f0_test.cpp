/*
 * get_f0_test.cpp
 * Gareth Ferneyhough, 2012
 * University of Nevada, Reno
 *
 * This is a test for my revised version of
 * David Talkin's get_f0 code from Entropic Research Laboratory, Inc.
 *
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include "getf0.h"

void readFile(const char *filename, double **audio_frames, int* sample_length, double* sample_rate);

int main(int argc, char** argv)
{
  int done;
  int buff_size;
  F0_params *parameters;
  char *param_file = NULL;
  float *f0p, *vuvp, *rms_speech, *acpkp;
  int vecsize;
  int sdstep = 0;

  // Set parameters
  parameters = (F0_params *) malloc(sizeof(F0_params));
  parameters->cand_thresh = 0.3;
  parameters->lag_weight = 0.3;
  parameters->freq_weight = 0.02;
  parameters->trans_cost = 0.005;
  parameters->trans_amp = 0.5;
  parameters->trans_spec = 0.5;
  parameters->voice_bias = 0.5;//0.0;
  parameters->double_cost = 0.35;
  parameters->min_f0 = 50;
  parameters->max_f0 = 550;
  parameters->frame_step = 0.01;//0.01;
  parameters->wind_dur = 0.0075;
  parameters->n_cands = 20;
  parameters->mean_f0 = 200;         /* unused */
  parameters->mean_f0_weight = 0.0;  /* unused */
  parameters->conditioning = 0;      /* unused */

  double *audio_frames;
  double sample_rate;
  int sample_length;
  const char *filename = "in.wav";

  // Read file using libsndfile
  readFile(filename, &audio_frames, &sample_length, &sample_rate);
  int frames_remaining = sample_length;
  printf("Frames remaining: %d\n", frames_remaining);
  printf("Samplerate: %f\n", sample_rate);

  if(check_f0_params(parameters, sample_rate)){
    fprintf(stderr, "invalid/inconsistent parameters -- exiting.\n");
    exit(1);
  }

  /* Initialize variables in get_f0.c; allocate data structures;
   * determine length and overlap of input frames to read.
   */
  if (init_dp_f0(sample_rate, parameters, &buff_size, &sdstep)
      || buff_size > INT_MAX || sdstep > INT_MAX)
    {
      fprintf(stderr, "problem in init_dp_f0().\n");
      exit(1);
    }

  fprintf(stderr, "init_dp_f0 returned buff_size %ld, sdstep %ld.\n",
          buff_size, sdstep);

  if (buff_size > frames_remaining)
    buff_size = frames_remaining;

  int actsize = buff_size;

  // Main processing loop
  int current_frame = 0;
  while (1) {
    done = (actsize < buff_size) || (frames_remaining == buff_size);

    if (dp_f0((float*)&audio_frames[current_frame], actsize, (int) sdstep, sample_rate, parameters,
              &f0p, &vuvp, &rms_speech, &acpkp, &vecsize, done)) {
      fprintf(stderr, "problem in dp_f0().\n");
      exit(1);
    }

    // Save stats for region
    int i;
    for (i = vecsize - 1; i >= 0; i--) {
      printf("%f",    f0p[i]);
      printf(",%f",   vuvp[i]);
      printf(",%f",   rms_speech[i]);
      printf(",%f\n", acpkp[i]);
    }

    if (done)
      break;

    current_frame += sdstep;
    frames_remaining -= sdstep;

    if (actsize > frames_remaining)
      actsize = frames_remaining;
  }
  exit(0);
}

// Read input file with libsndfile
void readFile(const char *filename, double **audio_frames, int *sample_length, double *sample_rate)
{
  SNDFILE *infile;
  SF_INFO sfinfo;

  // check file
  if(!(infile = sf_open(filename, SFM_READ, &sfinfo)))
    {
      printf("Unable to open %s\n", filename);
      puts(sf_strerror(NULL));
      exit(-1);
    }

  if(sfinfo.channels > 1)
    {
      printf("Not able to process more than 1 channel.\n");
      exit(-1);
    }

  //read sound file
  *sample_length = sfinfo.frames;
  *sample_rate = (double)sfinfo.samplerate;

  (*audio_frames) = (double*)malloc(*sample_length * sizeof(double));
  int readcount = sf_readf_double(infile, (*audio_frames), *sample_length);

  if (readcount != *sample_length){
    printf("Error reading sound file\n");
    exit(-1);
  }

  else printf("Read %d frames at %f Hz\n", *sample_length, *sample_rate);
}

