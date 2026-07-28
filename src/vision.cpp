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

cv::Mat Imopen(const cv::Mat& fill,int rayon) {
	if (rayon < 1) {
		throw std::invalid_argument("Imopen : le rayon doit valoir au moins 1");
	}
	// Matlab strel("disk", r) et cv::MORPH_ELLIPSE ne produisent pas exactement
	// le meme masque (approximations differentes du disque). Ecart assume :
	// l'effet sur des objets bien plus grands que l'element est negligeable.
	// Attention : OpenCV attend un DIAMETRE, Matlab un rayon.
	if (fill.empty()) {
		throw std::invalid_argument("Imopen : image vide");
	}
	if (fill.type() != CV_8UC1) {
		throw std::invalid_argument("Imopen : image attendue en 8 bits, un canal (CV_8UC1)"); 
	}

	const cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(2 * rayon + 1, 2 * rayon + 1));
	cv::Mat sortie;
	cv::morphologyEx(fill, sortie, cv::MORPH_OPEN, element);
	return sortie;
}




std::vector<Region> ExtraireRegions(const cv::Mat& binaire) {
	if (binaire.empty()) {
		throw std::invalid_argument("ExtraireRegions : image vide");
	}
	if (binaire.type() != CV_8UC1) {
		throw std::invalid_argument("ExtraireRegions : image attendue en 8 bits, un canal (CV_8UC1)");
	}
	std::vector<Region> regions;
	cv::Mat labels, stats, centroids;
	const int nb = cv::connectedComponentsWithStats(binaire, labels, stats, centroids, 4, CV_32S);
	regions.reserve(static_cast<std::size_t>(nb - 1));
	for (int k = 1; k < nb; ++k) {   // 0 est le fond
		Region region;
		region.aire_pixels = static_cast<double>(stats.at<int>(k, cv::CC_STAT_AREA));
		region.centroide_x = centroids.at<double>(k, 0);
		region.centroide_y = centroids.at<double>(k, 1);
		regions.push_back(region);
	}
	return regions;
	
}