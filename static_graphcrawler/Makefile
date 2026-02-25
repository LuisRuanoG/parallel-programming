CXX = g++
CXXFLAGS = -std=c++17 -O2 -I./rapidjson/include
LDFLAGS = -lcurl

all: level_client par_level_client

# -------- SEQUENTIAL --------
level_client: level_client.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

level_client.o: level_client.cpp
	$(CXX) $(CXXFLAGS) -c level_client.cpp

# -------- PARALLEL --------
par_level_client: par_level_client.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

par_level_client.o: par_level_client.cpp
	$(CXX) $(CXXFLAGS) -c par_level_client.cpp

clean:
	rm -f level_client par_level_client *.o