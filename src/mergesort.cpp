#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <array_size>\n";
        return 1;
    }

    int n = std::atoi(argv[1]);
    std::vector<int> data(n);

    // TODO: generate random data
    // TODO: call merge sort
    // TODO: measure and print time

    return 0;
}
