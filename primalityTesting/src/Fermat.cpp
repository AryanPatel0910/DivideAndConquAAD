#include "../include/Fermat.h"
#include <iostream>

// Initialize the random number generator
gmp_randclass rng(gmp_randinit_default);

bool Fermat::test(const mpz_class& n, int k) {
    // 1. Edge Cases
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0) return false; 

    rng.seed(time(NULL));

    // 2. The Main Loop
    for (int i = 0; i < k; i++) {
        // Pick a random base 'a' in range [2, n-2]
        mpz_class a = rng.get_z_range(n - 4) + 2;

        // --- THE FIX IS HERE ---
        // We create a named variable 'exponent' to hold (n - 1)
        mpz_class exponent = n - 1; 
        mpz_class result;
        
        // Now we can safely call .get_mpz_t() on 'exponent'
        mpz_powm(result.get_mpz_t(), a.get_mpz_t(), exponent.get_mpz_t(), n.get_mpz_t());

        // Check Fermat's condition
        if (result != 1) {
            return false;
        }
    }

    return true;
}