#include <iostream>
#include <vector>
#include <random>       // For random numbers
#include <cmath>        // For std::acos
#include <iomanip>      // For std::setprecision
#include <thread>       // For std::thread::hardware_concurrency
#include <future>       // For std::async and std::future
#include <chrono>       // <-- 1. INCLUDE FOR TIMING

// Use the actual value of pi for error calculation
const double PI_ACTUAL = std::acos(-1.0);

/**
 * @brief Worker function to calculate 'inside' count for a subset of points.
 * (This function is unchanged)
 */
long long estimateInside(long long num_points, unsigned int seed) 
{
    // Create a THREAD-LOCAL generator
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    long long inside = 0;
    for (long long i = 0; i < num_points; i++) {
        double x = dis(gen);
        double y = dis(gen);
        if ((x * x + y * y) <= 1.0) {
            inside++;
        }
    }
    return inside;
}


int main() {
    // --- Setup ---
    
    // 1. Define the "n" values (points per experiment)
    std::vector<long long> N_values = {100,1000,10000,100000,1000000,10000000};
    
    // 2. Define the "m" values (trials per experiment)
    std::vector<int> M_values = {1,10,100,1000};

    // 3. Get thread count
    const unsigned int num_threads = std::max(4u, std::thread::hardware_concurrency());
    // std::cout << "Using " << num_threads << " threads for estimation." << std::endl;
    
    // 4. Create one random_device to get seeds from
    std::random_device rd; 

    // 5. Print CSV header (now includes runtime_sec)
    std::cout << "n,m_trials,mean_pi,variance,std_dev,mean_error,runtime_sec" << std::endl; // <-- 2. UPDATED
    std::cout << std::fixed << std::setprecision(8);

    // --- Experiment Loops ---

    // Outer loop: iterate over the different values of m
    for (int m : M_values) {
    
        // Inner loop: iterate over the different values of n
        for (long long n : N_values) {
            
            // <-- 3. START TIMER
            auto start_time = std::chrono::steady_clock::now();
            
            std::vector<double> pi_estimates;
            pi_estimates.reserve(m); // Reserve space for efficiency
            
            // Innermost loop: run 'm' trials for the current (n, m) pair
            for (int j = 0; j < m; j++) {
                
                // --- Parallel Estimation ---
                std::vector<std::future<long long>> futures;
                long long points_per_thread = n / num_threads;
                long long remainder = n % num_threads;

                for (unsigned int i = 0; i < num_threads; i++) {
                    long long points_for_this_thread = points_per_thread;
                    if (i == 0) {
                        points_for_this_thread += remainder;
                    }
                    
                    futures.push_back(std::async(std::launch::async, 
                                                 estimateInside, 
                                                 points_for_this_thread, 
                                                 rd()));
                }

                // Collect results
                long long total_inside = 0;
                for (auto& fut : futures) {
                    total_inside += fut.get(); 
                }
                
                double pi_estimate = (static_cast<double>(total_inside) * 4.0) / n;
                pi_estimates.push_back(pi_estimate);
            }
            
            // --- Analyze the 'm' trials for this (n, m) pair ---
            double sum = 0.0;
            for (double est : pi_estimates) { sum += est; }
            double mean_pi = sum / m; // Use 'm'

            double var_sum = 0.0;
            for (double est : pi_estimates) { var_sum += (est - mean_pi) * (est - mean_pi); }
            
            // Use 'm' for sample variance
            double variance = (m > 1) ? var_sum / (m - 1) : 0.0; 
            double std_dev = std::sqrt(variance);
            double mean_error = std::abs(mean_pi - PI_ACTUAL);
            
            // <-- 4. CAPTURE END TIME AND CALCULATE DURATION
            auto end_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> duration_sec = end_time - start_time;
            double runtime_seconds = duration_sec.count();

            // Print the CSV data row (now includes runtime)
            std::cout << n << "," << m << "," 
                      << mean_pi << "," << variance << ","
                      << std_dev << "," << mean_error << ","
                      << runtime_seconds << std::endl; // <-- 5. UPDATED
        }
    }

    return 0;
}