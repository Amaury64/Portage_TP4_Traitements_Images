#pragma once
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
