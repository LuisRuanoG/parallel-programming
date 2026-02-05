#include <iostream>
#include <string>
#include <chrono>

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <start_node> <depth>\n";
        return 1;
    }

    string start_node = argv[1];
    int max_depth = stoi(argv[2]);

    auto t0 = high_resolution_clock::now();

    // TODO: BFS goes here

    auto t1 = high_resolution_clock::now();
    duration<double> dt = t1 - t0;

    cout << "Time: " << dt.count() << " seconds\n";
    return 0;
}
