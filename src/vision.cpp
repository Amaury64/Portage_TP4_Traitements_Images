#include "vision.h"
#include <opencv2/imgproc.hpp>
#include <stdexcept>


cv::Mat SeuillerOtsu(const cv::Mat& gris) {
	if (gris.empty()) {
		throw std::invalid_argument("SeuillerOtsu : image vide");
	}
	if (gris.type() != CV_8UC1) {
		throw std::invalid_argument("SeuillerOtsu : image attendue en 8 bits, un canal (CV_8UC1)");
	}
	cv::Mat sortie;
	cv::threshold(gris, sortie, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	return sortie;
}


cv::Mat Imfill(const cv::Mat& binaire) {
	if (binaire.empty()) {
		throw std::invalid_argument("Imfill : image vide");
	}
	if (binaire.type() != CV_8UC1) {
		throw std::invalid_argument("Imfill : image attendue en 8 bits, un canal (CV_8UC1)"); // Même si en couleur ça marche aussi
	}

	cv::Mat borde;
	cv::copyMakeBorder(binaire, borde, 1, 1, 1, 1, cv::BORDER_CONSTANT, cv::Scalar(0));
	// Un trou est une region sombre qui n'atteint pas le bord de l'image.
	// La bordure noire garantit que le point de depart (0,0) est du fond, meme
	// si un objet touche le coin.
	// LIMITE : un objet coupe par le bord de l'image a son "trou" connecte a
	// l'exterieur ; il ne sera pas bouche. L'information n'est pas dans l'image.
	cv::floodFill(borde, cv::Point(0, 0), cv::Scalar(255));

	// Retirer la bordure : cv::Rect(x, y, largeur, hauteur)
	const cv::Mat fond = borde(cv::Rect(1, 1, binaire.cols, binaire.rows));

	cv::Mat trous;
	cv::bitwise_not(fond, trous);      // ne restent en blanc que les trous

	cv::Mat resultat;
	cv::bitwise_or(binaire, trous, resultat);


	return resultat;
	

}