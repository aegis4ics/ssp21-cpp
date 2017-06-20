
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "ssp21/crypto/Crypto.h"

int main(int argc, char*  argv[])
{
    // global setup...
    assert(ssp21::Crypto::initialize());

    int result = Catch::Session().run(argc, argv);

    // global clean-up...

    return result;
}