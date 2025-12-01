#include "../include/MillerRabin.h"
#include <iostream>

gmp_randclass rng_mr(gmp_randinit_default);

bool MillerRabin::test(const mpz_class& n, int k) {
    // 1. Handle base cases
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0) return false;

    rng_mr.seed(time(NULL));

    // 2. Find d and r such that n-1 = d * 2^r
    // We basically divide (n-1) by 2 until we can't anymore.
    mpz_class d = n - 1;
    long r = 0;
    while (d % 2 == 0) {
        d /= 2;
        r++;
    }

    // 3. The Witness Loop
    for (int i = 0; i < k; i++) {
        // Pick random 'a' in [2, n-2]
        mpz_class a = rng_mr.get_z_range(n - 4) + 2;

        // Compute x = a^d % n
        mpz_class x;
        mpz_powm(x.get_mpz_t(), a.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());

        // Case 1: If x is 1 or n-1, this iteration passes (continue to next random a)
        if (x == 1 || x == n - 1) continue;

        // Case 2: Square 'x' repeatedly 'r-1' times
        bool composite = true;
        for (int j = 0; j < r - 1; j++) {
            // x = x^2 % n
            mpz_powm_ui(x.get_mpz_t(), x.get_mpz_t(), 2, n.get_mpz_t());

            // If we hit n-1, we are safe for this round
            if (x == n - 1) {
                composite = false;
                break;
            }
        }

        // If we never hit n-1, it's composite
        if (composite) return false;
    }

    // If we survived k rounds, it's very likely prime
    return true;
}