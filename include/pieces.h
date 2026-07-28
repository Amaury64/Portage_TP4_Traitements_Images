#pragma once

#include <optional>
#include <vector>



struct PieceReference {
    int valeur_centimes;
    double diametre_mm;
};

inline const std::vector<PieceReference> kTableReference = {
    {200, 25.75}, {100, 23.25}, {50, 24.25}, {20, 22.25},
    {10, 19.75},  {5, 21.25},   {2, 18.75},  {1, 16.25}
};

// Combien de millimètres represente un pixel ?
// On le deduit d'une piece dont on connait la valeur.
double TaillePixelMm(double aire_pixels_reference, double diametre_reel_mm);
double AireTheoriquePixels(double diametre_mm, double taille_pixel_mm);

std::optional<int> IdentifierPiece(double aire_pixels,
    double ratio_axes,
    double taille_pixel_mm,
    double seuil_circularite = 1.15,
    double marge_aire = 0.20);

double DiametreMmPour(int valeur_centimes);
