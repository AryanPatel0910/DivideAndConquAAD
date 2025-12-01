#ifndef PRIMALITY_TESTER_H
#define PRIMALITY_TESTER_H

#include <gmpxx.h>
#include <string>

// Abstract Base Class
class PrimalityTester {
public:
    virtual ~PrimalityTester() {}

    // Pure virtual function: subclasses MUST implement this
    // n: the number to test
    // k: the number of iterations (accuracy parameter)
    virtual bool test(const mpz_class& n, int k) = 0;

    // Helper to get the name of the algorithm (useful for CSV output)
    virtual std::string name() const = 0;
};

#endif