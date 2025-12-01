#ifndef FERMAT_H
#define FERMAT_H

#include "PrimalityTester.h"

class Fermat : public PrimalityTester {
public:
    bool test(const mpz_class& n, int k) override;
    std::string name() const override { return "Fermat"; }
};

#endif