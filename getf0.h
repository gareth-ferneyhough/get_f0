typedef struct f0_params {
float cand_thresh,	/* only correlation peaks above this are considered */
      lag_weight,	/* degree to which shorter lags are weighted */
      freq_weight,	/* weighting given to F0 trajectory smoothness */
      trans_cost,	/* fixed cost for a voicing-state transition */
      trans_amp,	/* amplitude-change-modulated VUV trans. cost */
      trans_spec,	/* spectral-change-modulated VUV trans. cost */
      voice_bias,	/* fixed bias towards the voiced hypothesis */
      double_cost,	/* cost for octave F0 jumps */
      mean_f0,		/* talker-specific mean F0 (Hz) */
      mean_f0_weight,	/* weight to be given to deviations from mean F0 */
      min_f0,		/* min. F0 to search for (Hz) */
      max_f0,		/* max. F0 to search for (Hz) */
      frame_step,	/* inter-frame-interval (sec) */
      wind_dur;		/* duration of correlation window (sec) */
int   n_cands,		/* max. # of F0 cands. to consider at each frame */
      conditioning;     /* Specify optional signal pre-conditioning. */
} F0_params;

#ifdef __cplusplus
extern "C" {
#endif

  extern int init_dp_f0(double freq, F0_params* par, int* buffsize, int* sdstep);
  extern int dp_f0(float* fdata, int buff_size, int sdstep, double freq, 
		   F0_params* par, float** f0p_pt, float** vuvp_pt, float** rms_speech_pt, 
		   float** acpkp_pt, int* vecsize, int last_time);
  extern int check_f0_params(F0_params* parameters, double sample_freq);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
