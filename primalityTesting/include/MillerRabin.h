#ifndef MILLER_RABIN_H
#define MILLER_RABIN_H

#include "PrimalityTester.h"

class MillerRabin : public PrimalityTester {
public:
    bool test(const mpz_class& n, int k) override;
    std::string name() const override { return "Miller-Rabin"; }
};

#endif