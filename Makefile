all:	get_f0 dp_f0 get_cands sigproc pitch_track

get_f0: get_f0.c
	gcc -c get_f0.c

dp_f0:	dp_f0.c
	gcc -c dp_f0.c

get_cands:	get_cands.c
	gcc -c get_cands.c

sigproc:	sigproc.c
	gcc -c sigproc.c

pitch_track:
	gcc -o pitch_track *.o -lm