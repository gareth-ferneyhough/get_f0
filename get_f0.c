/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1990-1996 Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:  David Talkin
 *
 * Brief description:  Estimates F0 using normalized cross correlation and
 *   dynamic programming.
 *
 *
 * Revived 2012 by: Gareth Ferneyhough, University of Nevada, Reno
 *
 */


static char *sccs_id = "@(#)get_f0.c    1.14    10/21/96        ERL";

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <sndfile.h>
#include "f0.h"

#define SYNTAX USAGE("get_f0 [-P param_file][-{pr} range][-s range][-S frame_step_samples]\n     [-i frame_step_seconds][-x debug_level] infile outfile")

char        *ProgName = "get_f0";
static char *Version = "2.0";
static char *Date = "2/22/12";

int debug_level = 0;

extern void fea_skiprec();
static int check_f0_params();
int init_dp_f0();
int dp_f0();
void readFile(const char *filename, double **audio_frames, int* sample_length, double* sample_rate);

int main(int ac, char** av)
{
  int done;
  long buff_size;
  F0_params *parameters;
  char *param_file = NULL;
  float *f0p, *vuvp, *rms_speech, *acpkp;
  int vecsize;
  long sdstep = 0;

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

  readFile(filename, &audio_frames, &sample_length, &sample_rate);
  int frames_remaining = sample_length;
  printf("Frames remaining: %d\n", frames_remaining);
  printf("Samplerate: %f\n", sample_rate);

  if(check_f0_params(parameters, sample_rate)){
    Fprintf(stderr, "%s: invalid/inconsistent parameters -- exiting.\n",
            ProgName);
    exit(1);
  }

  /* Initialize variables in get_f0.c; allocate data structures;
   * determine length and overlap of input frames to read.
   */
  if (init_dp_f0(sample_rate, parameters, &buff_size, &sdstep)
      || buff_size > INT_MAX || sdstep > INT_MAX)
    {
      Fprintf(stderr, "%s: problem in init_dp_f0().\n", ProgName);
      exit(1);
    }

  if (debug_level)
    Fprintf(stderr, "%s: init_dp_f0 returned buff_size %ld, sdstep %ld.\n",
            ProgName, buff_size, sdstep);

  if (buff_size > frames_remaining)
    buff_size = frames_remaining;

  int actsize = buff_size;

  // Main processing loop
  int current_frame = 0;
  while (1) {

    done = (actsize < buff_size) || (frames_remaining == buff_size);

    if (dp_f0(&audio_frames[current_frame], actsize, (int) sdstep, sample_rate, parameters,
              &f0p, &vuvp, &rms_speech, &acpkp, &vecsize, done)) {
      Fprintf(stderr, "%s: problem in dp_f0().\n", ProgName);
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


/*
 * Some consistency checks on parameter values.
 * Return a positive integer if any errors detected, 0 if none.
 */

static int
check_f0_params(F0_params *parameters, double sample_freq)
{
  int     error = 0;
  double  dstep;

  if((parameters->cand_thresh < 0.01) || (parameters->cand_thresh > 0.99)) {
    Fprintf(stderr,
            "%s: ERROR: cand_thresh parameter must be between [0.01, 0.99].\n",
            ProgName);
    error++;
  }
  if((parameters->wind_dur > .1) || (parameters->wind_dur < .0001)) {
    Fprintf(stderr,
            "ERROR: wind_dur parameter must be between [0.0001, 0.1].\n",
            ProgName);
    error++;
  }
  if((parameters->n_cands > 100) || (parameters->n_cands < 3)){
    Fprintf(stderr,
            "%s: ERROR: n_cands parameter must be between [3,100].\n",
            ProgName);
    error++;
  }
  if((parameters->max_f0 <= parameters->min_f0) || (parameters->max_f0 >= (sample_freq/2.0)) ||
     (parameters->min_f0 < (sample_freq/10000.0))){
    Fprintf(stderr,
            "%s: ERROR: min(max)_f0 parameter inconsistent with sampling frequency.\n",
            ProgName);
    error++;
  }
  dstep = ((double)((int)(0.5 + (sample_freq * parameters->frame_step))))/sample_freq;
  if(dstep != parameters->frame_step) {
    if(debug_level)
      Fprintf(stderr,
              "%s: Frame step set to %f to exactly match signal sample rate.\n",
              ProgName, dstep);
    parameters->frame_step = dstep;
  }
  if((parameters->frame_step > 0.1) || (parameters->frame_step < (1.0/sample_freq))){
    Fprintf(stderr,
            "%s: ERROR: frame_step parameter must be between [1/sampling rate, 0.1].\n",
            ProgName);
    error++;
  }

  return(error);
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

  (*audio_frames) = (void*)malloc(*sample_length * sizeof(double));
  int readcount = sf_readf_double(infile, (*audio_frames), *sample_length);

  if (readcount != *sample_length){
    printf("Error reading sound file\n");
    exit(-1);
  }

  else printf("Read %d frames at %f Hz\n", *sample_length, *sample_rate);
}

