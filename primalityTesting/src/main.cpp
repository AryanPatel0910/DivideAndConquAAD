#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <chrono> // For high-precision stopwatch
#include <cstring> // For strcmp
#include "../include/MillerRabin.h"
#include "../include/Fermat.h"

// A simple helper to print usage instructions if the user messes up
void printUsage() {
    std::cerr << "Usage: ./primality_test --algo <miller|fermat> --file <path_to_file> --k <iterations>" << std::endl;
}

int main(int argc, char* argv[]) {
    // 1. ARGUMENT PARSING
    // We expect 7 arguments total (program name + 3 flags + 3 values)
    if (argc < 7) {
        printUsage();
        return 1;
    }

    std::string algoType;
    std::string filePath;
    int k = 5;

    // Loop through arguments to find our flags
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--algo") == 0) {
            algoType = argv[i + 1];
        } else if (std::strcmp(argv[i], "--file") == 0) {
            filePath = argv[i + 1];
        } else if (std::strcmp(argv[i], "--k") == 0) {
            k = std::stoi(argv[i + 1]);
        }
    }

    // 2. SETUP THE ALGORITHM
    std::unique_ptr<PrimalityTester> tester;
    if (algoType == "miller") {
        tester = std::make_unique<MillerRabin>();
    } else if (algoType == "fermat") {
        tester = std::make_unique<Fermat>();
    } else {
        std::cerr << "Unknown algorithm: " << algoType << std::endl;
        return 1;
    }

    // 3. OPEN THE FILE
    std::ifstream infile(filePath);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return 1;
    }

    std::string line;
    // 4. PROCESS EACH NUMBER
    // Format of output: Number(truncated), Result(0/1), Time(microseconds)
    std::cout << "Number,Result,TimeUS" << std::endl;

    while (std::getline(infile, line)) {
        if (line.empty()) continue;

        mpz_class n(line); // Convert string to Big Int

        // --- START STOPWATCH ---
        auto start = std::chrono::high_resolution_clock::now();

        bool isPrime = tester->test(n, k);

        auto end = std::chrono::high_resolution_clock::now();
        // --- STOP STOPWATCH ---

        // Calculate duration in microseconds
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Output CSV row
        // We only print the first 20 digits of n to keep logs clean
        std::string n_str = n.get_str();
        std::string n_trunc = (n_str.length() > 20) ? n_str.substr(0, 20) + "..." : n_str;

        std::cout << n_trunc << "," << isPrime << "," << duration << std::endl;
    }

    infile.close();
    return 0;
}