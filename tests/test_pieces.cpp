#include "pieces.h"

#include <gtest/gtest.h>

#include <stdexcept>

#include <optional>

namespace {
    constexpr double kPi = 3.14159265358979323846;
}



namespace {
    // Taille de pixel ronde pour garder les calculs lisibles : 0.1 mm par pixel,
    // donc une piece de 25.75 mm fait 257.5 pixels de diametre.
    constexpr double kTaillePixel = 0.1;
}  // namespace



TEST(TaillePixelMm, PieceDeReference) {
    // Une piece de 23.25 mm mesuree a 100 pixels de diametre
    // => aire = pi * 50^2 = 7853.98 pixels

    const double aire = kPi * 50.0 * 50.0;

    EXPECT_NEAR(TaillePixelMm(aire, 23.25), 23.25 / 100.0, 1e-12);
}

TEST(TaillePixelMm, AireNegativeRejetee) {
    EXPECT_THROW(TaillePixelMm(-1.0, 23.25), std::invalid_argument);
}

TEST(AireTheoriquePixels, DiametreConnu) {
    // 20 mm avec des pixels de 0.2 mm => 100 pixels de diametre,
    // donc une aire de pi * 50^2.
    EXPECT_NEAR(AireTheoriquePixels(20.0, 0.2), kPi * 50.0 * 50.0, 1e-9);
}

TEST(AireTheoriquePixels, DiametreNegativeRejetee) {
    EXPECT_THROW(AireTheoriquePixels(-1.0, 23.25), std::invalid_argument);
}




TEST(IdentifierPiece, DeuxEurosIdentifiee) {
    // On fabrique l'aire d'entree a partir du diametre reel : la verite
    // terrain est connue puisqu'on l'a construite.
    const double aire = AireTheoriquePixels(25.75, kTaillePixel);

    const std::optional<int> valeur = IdentifierPiece(aire, 1.0, kTaillePixel);

    ASSERT_TRUE(valeur.has_value());
    EXPECT_EQ(*valeur, 200);
}

TEST(IdentifierPiece, CinquanteCentimesDistingueeDeUnEuro) {
    // Test central de ce portage : les diametres ne sont PAS monotones avec la
    // valeur. 50 c fait 24.25 mm, 1 EUR seulement 23.25 mm. Un classement par
    // taille croissante = valeur croissante se tromperait ici.
    const double aire_50 = AireTheoriquePixels(24.25, kTaillePixel);
    const double aire_100 = AireTheoriquePixels(23.25, kTaillePixel);

    const std::optional<int> v50 = IdentifierPiece(aire_50, 1.0, kTaillePixel);
    const std::optional<int> v100 = IdentifierPiece(aire_100, 1.0, kTaillePixel);

    ASSERT_TRUE(v50.has_value());
    ASSERT_TRUE(v100.has_value());
    EXPECT_EQ(*v50, 50);
    EXPECT_EQ(*v100, 100);
}

TEST(IdentifierPiece, ObjetTropPetitRejete) {
    // Une pilule : bien plus petite que la plus petite piece (1 c, 16.25 mm).
    const double aire = AireTheoriquePixels(5.0, kTaillePixel);

    EXPECT_FALSE(IdentifierPiece(aire, 1.0, kTaillePixel).has_value());
}

TEST(IdentifierPiece, ObjetTropGrandRejete) {
    // Un jeton de parasol : bien plus grand que la plus grande piece.
    const double aire = AireTheoriquePixels(60.0, kTaillePixel);

    EXPECT_FALSE(IdentifierPiece(aire, 1.0, kTaillePixel).has_value());
}

TEST(IdentifierPiece, ObjetAllongeRejete) {
    // Bonne taille, mais trop allonge pour etre une piece : le test de
    // circularite doit primer sur celui de l'aire.
    const double aire = AireTheoriquePixels(25.75, kTaillePixel);

    EXPECT_FALSE(IdentifierPiece(aire, 1.5, kTaillePixel).has_value());
}

TEST(IdentifierPiece, CirculariteALaLimiteAcceptee) {
    // Test de borne : 1.15 est le maximum admis, il doit passer.
    const double aire = AireTheoriquePixels(25.75, kTaillePixel);

    EXPECT_TRUE(IdentifierPiece(aire, 1.15, kTaillePixel).has_value());
}

TEST(DiametreMmPour, ValeursConnues) {
    EXPECT_DOUBLE_EQ(DiametreMmPour(200), 25.75);
    EXPECT_DOUBLE_EQ(DiametreMmPour(1), 16.25);
    // 50 c est plus GRANDE que 1 EUR : les diametres ne suivent pas la valeur.
    EXPECT_GT(DiametreMmPour(50), DiametreMmPour(100));
}

TEST(DiametreMmPour, ValeurInconnueRejetee) {
    EXPECT_THROW(DiametreMmPour(42), std::invalid_argument);
}