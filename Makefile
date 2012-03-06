all:	libgetf0.a f0_test

get_f0.o: get_f0.c
	gcc -c -g get_f0.c

dp_f0.o:	dp_f0.c
	gcc -c -g dp_f0.c

get_cands.o:	get_cands.c
	gcc -c -g get_cands.c

sigproc.o:	sigproc.c
	gcc -c -g sigproc.c

libgetf0.a:	dp_f0.o get_cands.o sigproc.o check_params.o
	ar rcs libgetf0.a dp_f0.o get_cands.o sigproc.o check_params.o

check_params.o:	check_params.c
	gcc -c -g check_params.c

f0_test:	get_f0_test.cpp
	g++ -o f0_test get_f0_test.cpp -L . -lgetf0 -lsndfile -lm

clean:
	rm *.o *.a f0_test