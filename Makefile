all:	pitch_track libgetf0.a

get_f0.o: get_f0.c
	gcc -c -g get_f0.c

dp_f0.o:	dp_f0.c
	gcc -c -g dp_f0.c

get_cands.o:	get_cands.c
	gcc -c -g get_cands.c

sigproc.o:	sigproc.c
	gcc -c -g sigproc.c

pitch_track: get_f0.o dp_f0.o get_cands.o sigproc.o 
	gcc -g -o pitch_track *.o -lm -lsndfile

libgetf0.a:	dp_f0.o get_cands.o sigproc.o
	ar rcs libgetf0.a dp_f0.o get_cands.o sigproc.o

clean:
	rm *.o pitch_track *.a