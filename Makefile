all:	get_f0 dp_f0 get_cands sigproc pitch_track

get_f0: get_f0.c
	g++ -c -g get_f0.c

dp_f0:	dp_f0.c
	g++ -c -g dp_f0.c

get_cands:	get_cands.c
	g++ -c -g get_cands.c

sigproc:	sigproc.c
	g++ -c -g sigproc.c

pitch_track:
	g++ -g -o pitch_track *.o -lm

clean:
	rm *.o pitch_track