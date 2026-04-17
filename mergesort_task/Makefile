CFLAGS=-O3 -std=c11 -fPIC -g
CXXFLAGS=-O3 -std=c++17 -fPIC -g -fopenmp
LDFLAGS=-fopenmp
LD=g++


all: mergesort_seq


mergesort_seq: mergesort_seq.o
	$(LD) $(LDFLAGS) mergesort_seq.o $(ARCHIVES) -o mergesort_seq


mergesort_seq.o: mergesort_seq_nocopy.cpp omp_tasking.hpp
	$(LD) $(CXXFLAGS) -c mergesort_seq_nocopy.cpp -o mergesort_seq.o


clean:
	-rm *.o
	-rm mergesort_seq

distclean:
	-rm *.sh.*

.PHONY: all clean distclean mergesort_seq