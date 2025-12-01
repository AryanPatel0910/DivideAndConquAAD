#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace std::chrono;

// ==========================================
// CONFIGURATION
// ==========================================
const int NUM_RUNS = 10; 
const vector<int> SIZES = {100, 200, 500, 1000, 2000, 5000, 10000, 20000}; 

// ==========================================
// 1. STANDARD QUICKSORT (Lomuto)
// ==========================================
int partitionStandard(vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return (i + 1);
}

void quickSortStandard(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = partitionStandard(arr, low, high);
        quickSortStandard(arr, low, pi - 1);
        quickSortStandard(arr, pi + 1, high);
    }
}

// ==========================================
// 2. RANDOMIZED QUICKSORT
// ==========================================
int partitionRandomized(vector<int>& arr, int low, int high) {
    int random = low + rand() % (high - low + 1);
    swap(arr[random], arr[high]);
    return partitionStandard(arr, low, high);
}

void quickSortRandomized(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = partitionRandomized(arr, low, high);
        quickSortRandomized(arr, low, pi - 1);
        quickSortRandomized(arr, pi + 1, high);
    }
}

// ==========================================
// 3. DUAL-PIVOT QUICKSORT
// ==========================================
void quickSortDualPivot(vector<int>& arr, int low, int high) {
    if (low < high) {
        // Swap low and high to ensure arr[low] <= arr[high]
        if (arr[low] > arr[high]) {
            swap(arr[low], arr[high]);
        }

        // Pivots
        int p1 = arr[low];
        int p2 = arr[high];

        int l = low + 1; // Left pointer
        int g = high - 1; // Right pointer
        int k = low + 1; // Iterator

        while (k <= g) {
            // If element is smaller than left pivot
            if (arr[k] < p1) {
                swap(arr[k], arr[l]);
                l++;
            }
            // If element is greater than right pivot
            else if (arr[k] > p2) {
                while (arr[g] > p2 && k < g) {
                    g--;
                }
                swap(arr[k], arr[g]);
                g--;
                // After swapping, the new arr[k] might be < p1
                if (arr[k] < p1) {
                    swap(arr[k], arr[l]);
                    l++;
                }
            }
            k++;
        }
        l--; 
        g++;

        // Bring pivots to their correct positions
        swap(arr[low], arr[l]);
        swap(arr[high], arr[g]);

        // Recursively sort the three parts
        quickSortDualPivot(arr, low, l - 1);
        quickSortDualPivot(arr, l + 1, g - 1);
        quickSortDualPivot(arr, g + 1, high);
    }
}

// ==========================================
// DATA GENERATORS & BENCHMARK
// ==========================================
vector<int> generateRandomArray(int n) {
    vector<int> arr(n);
    for (int i = 0; i < n; i++) arr[i] = rand() % 100000;
    return arr;
}

vector<int> generateSortedArray(int n) {
    vector<int> arr(n);
    for (int i = 0; i < n; i++) arr[i] = i;
    return arr;
}

double measureTime(void (*sortFunc)(vector<int>&, int, int), vector<int>& arr) {
    auto start = high_resolution_clock::now();
    sortFunc(arr, 0, arr.size() - 1);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    return duration.count() / 1000.0; 
}

int main() {
    srand(time(0));

    // Updated Header with Dual Pivot
    cout << "Size,InputType,Run_ID,Time_Standard,Time_Randomized,Time_DualPivot" << endl;

    for (int n : SIZES) {
        
        // --- 1. Random Input ---
        for (int k = 1; k <= NUM_RUNS; k++) {
            vector<int> originalData = generateRandomArray(n);
            
            // Three identical copies
            vector<int> d1 = originalData;
            vector<int> d2 = originalData;
            vector<int> d3 = originalData;

            double tStd = measureTime(quickSortStandard, d1);
            double tRnd = measureTime(quickSortRandomized, d2);
            double tDual = measureTime(quickSortDualPivot, d3);

            cout << n << ",Random," << k << "," << tStd << "," << tRnd << "," << tDual << endl;
        }

        // --- 2. Sorted Input ---
        // Note: Dual Pivot (Standard) is also O(n^2) on sorted data unless randomized!
        for (int k = 1; k <= NUM_RUNS; k++) {
            vector<int> d1 = generateSortedArray(n);
            vector<int> d2 = d1;
            vector<int> d3 = d1;

            double tStd = measureTime(quickSortStandard, d1);
            double tRnd = measureTime(quickSortRandomized, d2);
            double tDual = measureTime(quickSortDualPivot, d3);

            cout << n << ",Sorted," << k << "," << tStd << "," << tRnd << "," << tDual << endl;
        }
    }

    return 0;
}