#include "pieces.h"

#include <gtest/gtest.h>

#include <stdexcept>
namespace {
    constexpr double kPi = 3.14159265358979323846;
}



TEST(TaillePixelMm, PieceDeReference) {
    // Une piece de 23.25 mm mesuree a 100 pixels de diametre
    // => aire = pi * 50^2 = 7853.98 pixels

    const double aire = kPi * 50.0 * 50.0;

    EXPECT_NEAR(TaillePixelMm(aire, 23.25), 23.25 / 100.0, 1e-12);
}

TEST(TaillePixelMm, AireNegativeRejetee) {
    EXPECT_THROW(TaillePixelMm(-1.0, 23.25), std::invalid_argument);
}