CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

TARGET = mergesort
SRC = src/mergesort.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
