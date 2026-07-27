#include "pieces.h"

#include <cmath>
#include <stdexcept>
constexpr double kPi = 3.14159265358979323846;



double TaillePixelMm(double aire_pixels_reference, double diametre_reel_mm) {
	if (aire_pixels_reference <= 0.0 || diametre_reel_mm <= 0.0) {
		throw std::invalid_argument("TaillePixelMm : aire et diametre doivent etre positifs");
	}
	return (diametre_reel_mm /(2 * std::sqrt(aire_pixels_reference / kPi)));
}
