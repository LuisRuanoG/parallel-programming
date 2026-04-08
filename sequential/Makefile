CXX=g++
CXXFLAGS=-O3 -fopenmp

all: nbody

nbody: nbody.cpp
	$(CXX) $(CXXFLAGS) nbody.cpp -o nbody

solar.out: nbody
	date
	./nbody planet 200 5000000 10000 8 > solar.out # maybe a minutes
	date

solar.pdf: solar.out
	python3 plot.py solar.out solar.pdf 1000 

random.out: nbody
	date
	./nbody 1000 1 10000 100 8 > random.out # maybe 5 minutes
	date

clean:
	-rm -f nbody solar.out random.out