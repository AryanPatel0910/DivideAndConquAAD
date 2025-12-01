#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// ==========================================
// CONFIGURATION
// ==========================================
const int NUM_RUNS = 10;
// Added intermediate steps (50k, 100k) for smoother graphs
const vector<int> SIZES = {
    100, 200, 500, 
    1000, 2000, 5000, 
    10000, 20000, 50000, 
    100000, 200000, 500000, 
    1000000, 2000000, 5000000,
    10000000, 2000000, 500000000,
    100000000
};

// True value of Pi for error calculation
const double REAL_PI = 3.14159265358979323846;

// Random Number Engine (Better than rand())
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0.0, 1.0);

// ==========================================
// 1. BASIC MONTE CARLO
// ==========================================
// Just throw N darts randomly at the square.
double piBasic(int n) {
    int inside = 0;
    for (int i = 0; i < n; i++) {
        double x = dis(gen);
        double y = dis(gen);
        if (x * x + y * y <= 1.0) {
            inside++;
        }
    }
    return 4.0 * inside / n;
}

// ==========================================
// 2. STRATIFIED MONTE CARLO (10x10 Grid)
// ==========================================
// Divide square into 100 smaller squares.
// We force exactly N/100 points into each small square.
double piStratified(int n) {
    int inside = 0;
    int grid_side = 10; // 10x10 grid
    int points_per_cell = n / (grid_side * grid_side);
    
    // Scale factor for grid position (0.1)
    double step = 1.0 / grid_side;

    for (int i = 0; i < grid_side; i++) {
        for (int j = 0; j < grid_side; j++) {
            for (int k = 0; k < points_per_cell; k++) {
                // Generate local random coordinate within the cell
                double local_x = dis(gen);
                double local_y = dis(gen);
                
                // Map to global coordinate
                double x = (i + local_x) * step;
                double y = (j + local_y) * step;

                if (x * x + y * y <= 1.0) {
                    inside++;
                }
            }
        }
    }
    // Note: If N isn't perfectly divisible by 100, we effectively ignore the remainder.
    // For the list provided, all are divisible by 100.
    int total_points = points_per_cell * grid_side * grid_side;
    return 4.0 * inside / total_points;
}

// ==========================================
// 3. ANTITHETIC VARIATES
// ==========================================
// For every point (u, v), we also check (1-u, 1-v).
// This creates negative correlation; if (u,v) is close to (0,0) (inside),
// then (1-u, 1-v) is close to (1,1) (outside).
double piAntithetic(int n) {
    int inside = 0;
    int pairs = n / 2;

    for (int i = 0; i < pairs; i++) {
        double u = dis(gen);
        double v = dis(gen);

        // Point 1
        if (u * u + v * v <= 1.0) inside++;

        // Point 2 (Antithetic)
        double u2 = 1.0 - u;
        double v2 = 1.0 - v;
        if (u2 * u2 + v2 * v2 <= 1.0) inside++;
    }
    
    return 4.0 * inside / (pairs * 2);
}

// ==========================================
// MAIN BENCHMARK
// ==========================================
int main() {
    ofstream csvFile("results.csv");
    
    // Header
    csvFile << "Size,Run_ID,"
            << "Time_Basic,Est_Basic,Err_Basic,"
            << "Time_Strat,Est_Strat,Err_Strat,"
            << "Time_Anti,Est_Anti,Err_Anti" << endl;

    cout << "Starting Simulation... (This might take a moment for N=1,000,000)" << endl;

    for (int n : SIZES) {
        cout << "Running for Size: " << n << endl;
        for (int k = 1; k <= NUM_RUNS; k++) {
            
            // --- Basic ---
            auto t1 = high_resolution_clock::now();
            double valBasic = piBasic(n);
            auto t2 = high_resolution_clock::now();
            double timeBasic = duration_cast<microseconds>(t2 - t1).count() / 1000.0;
            double errBasic = abs(valBasic - REAL_PI);

            // --- Stratified ---
            auto t3 = high_resolution_clock::now();
            double valStrat = piStratified(n);
            auto t4 = high_resolution_clock::now();
            double timeStrat = duration_cast<microseconds>(t4 - t3).count() / 1000.0;
            double errStrat = abs(valStrat - REAL_PI);

            // --- Antithetic ---
            auto t5 = high_resolution_clock::now();
            double valAnti = piAntithetic(n);
            auto t6 = high_resolution_clock::now();
            double timeAnti = duration_cast<microseconds>(t6 - t5).count() / 1000.0;
            double errAnti = abs(valAnti - REAL_PI);

            // Write to CSV
            csvFile << n << "," << k << ","
                    << timeBasic << "," << valBasic << "," << errBasic << ","
                    << timeStrat << "," << valStrat << "," << errStrat << ","
                    << timeAnti << "," << valAnti << "," << errAnti << endl;
        }
    }

    csvFile.close();
    cout << "Done! Data written to results.csv" << endl;
    return 0;
}