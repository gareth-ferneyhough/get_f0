#include "f0.h"
#include <stdio.h>
#include <stdlib.h>

int check_f0_params(F0_params *parameters, double sample_freq)
{
  int     error = 0;
  double  dstep;

  if((parameters->cand_thresh < 0.01) || (parameters->cand_thresh > 0.99)) {
    Fprintf(stderr,
            "ERROR: cand_thresh parameter must be between [0.01, 0.99].\n");
    error++;
  }
  if((parameters->wind_dur > .1) || (parameters->wind_dur < .0001)) {
    Fprintf(stderr,
            "ERROR: wind_dur parameter must be between [0.0001, 0.1].\n");
    error++;
  }
  if((parameters->n_cands > 100) || (parameters->n_cands < 3)){
    Fprintf(stderr,
            "ERROR: n_cands parameter must be between [3,100].\n");
    error++;
  }
  if((parameters->max_f0 <= parameters->min_f0) || (parameters->max_f0 >= (sample_freq/2.0)) ||
     (parameters->min_f0 < (sample_freq/10000.0))){
    Fprintf(stderr,
            "ERROR: min(max)_f0 parameter inconsistent with sampling frequency.\n");
    error++;
  }
  dstep = ((double)((int)(0.5 + (sample_freq * parameters->frame_step))))/sample_freq;
  if(dstep != parameters->frame_step) {
      Fprintf(stderr,
              "Frame step set to %f to exactly match signal sample rate.\n", dstep);
    parameters->frame_step = dstep;
  }
  if((parameters->frame_step > 0.1) || (parameters->frame_step < (1.0/sample_freq))){
    Fprintf(stderr,
            "ERROR: frame_step parameter must be between [1/sampling rate, 0.1].\n");
    error++;
  }

  return(error);
}
