NVCC=nvcc
NVCCFLAGS=-O3 -arch=sm_61

all: nbody

nbody: nbody.cpp
	$(NVCC) $(NVCCFLAGS) -x cu nbody.cpp -o nbody

clean:
	-rm -f nbody *.out *.err