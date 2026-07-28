#include "pieces.h"

#include <cmath>
#include <limits>
#include <stdexcept>
constexpr double kPi = 3.14159265358979323846;



double TaillePixelMm(double aire_pixels_reference, double diametre_reel_mm) {
	if (aire_pixels_reference <= 0.0 || diametre_reel_mm <= 0.0) {
		throw std::invalid_argument("TaillePixelMm : aire et diametre doivent etre positifs");
	}
	return (diametre_reel_mm /(2 * std::sqrt(aire_pixels_reference / kPi)));
}


double AireTheoriquePixels(double diametre_mm, double taille_pixel_mm) {
	if (diametre_mm <= 0.0 || taille_pixel_mm <= 0.0) {
		throw std::invalid_argument("AireTheoriquePixels : diametre_mm et taille_pixel_mm doivent etre positifs");
	}
	const double rayon_theorique_pixel = diametre_mm / (2*taille_pixel_mm);
	return(kPi * rayon_theorique_pixel * rayon_theorique_pixel);
}


std::optional<int> IdentifierPiece(double aire_pixels,
	double ratio_axes,
	double taille_pixel_mm,
	double seuil_circularite ,
	double marge_aire) {

	if (ratio_axes > seuil_circularite) {
		return std::nullopt;
	}
	double aire_min = 0;
	double aire_max = 0;
	double meilleur_ecart = std::numeric_limits<double>::infinity();
	int meilleure_valeur = 0;
	

	for (const PieceReference& piece : kTableReference) {
		const double aire_theorique = AireTheoriquePixels(piece.diametre_mm, taille_pixel_mm);
		const double ecart = std::abs(aire_theorique - aire_pixels);

		if (ecart < meilleur_ecart) {
			meilleure_valeur = piece.valeur_centimes;
			meilleur_ecart = ecart;
			aire_min = aire_theorique * (1.0 - marge_aire);
			aire_max = aire_theorique * (1.0 + marge_aire);
				
		}
		
	}
	if (aire_pixels<aire_max && aire_pixels>aire_min) {
		return meilleure_valeur;
	}
	return std::nullopt;
	

}