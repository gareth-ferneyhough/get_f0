all:	get_f0 dp_f0 get_cands sigproc pitch_track

get_f0: get_f0.c
	gcc -c -g get_f0.c

dp_f0:	dp_f0.c
	gcc -c -g dp_f0.c

get_cands:	get_cands.c
	gcc -c -g get_cands.c

sigproc:	sigproc.c
	gcc -c -g sigproc.c

pitch_track: get_f0 dp_f0 get_cands sigproc
	gcc -g -o pitch_track *.o -lm -lsndfile

clean:
	rm *.o pitch_track