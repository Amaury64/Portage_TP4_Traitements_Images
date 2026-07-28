#include "vision.h"
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <algorithm>
#include<cmath>
#include <optional> 
#include "pieces.h"


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
		
		// --- Axes de l'ellipse equivalente ---------------------------------
		// Matlab fournit MajorAxisLength / MinorAxisLength ; OpenCV non.
		// On les recalcule a partir des moments centres d'ordre 2 : la matrice
		// de covariance des coordonnees des pixels de la region a pour valeurs
		// propres les carres des demi-axes (a un facteur pres).
		// Meme outil qu'un recalage rigide : covariance + decomposition
		// spectrale, en 2D au lieu de 3D.
		const cv::Mat masque = (labels == k);
		const cv::Moments moments = cv::moments(masque, true);

		// Moments centres normalises par l'aire (m00).
		// Correction +1/12 : un pixel est un carre de cote 1, pas un point.
		// C'est la convention de Matlab ; sans elle les axes sont legerement
		// sous-estimes (negligeable sur de gros objets, visible sur de petits).
		const double a = moments.mu20 / moments.m00 + 1.0 / 12.0;
		const double b = moments.mu11 / moments.m00;
		const double c = moments.mu02 / moments.m00 + 1.0 / 12.0;

		// Valeurs propres d'une matrice symetrique 2x2 : formule fermee.
		const double demi_somme = (a + c) / 2.0;
		const double ecart = std::sqrt((a - c) * (a - c) / 4.0 + b * b);

		// lambda2 peut etre infinitesimalement negatif en arithmetique
		// flottante sur une region parfaitement symetrique : on borne a 0.
		const double lambda1 = demi_somme + ecart;
		const double lambda2 = std::max(demi_somme - ecart, 0.0);

		region.grand_axe_pixels = 4.0 * std::sqrt(lambda1);
		region.petit_axe_pixels = 4.0 * std::sqrt(lambda2);

		regions.push_back(region);
	}
	return regions;
	
}

cv::Mat ChargerEnNiveauxDeGris(const std::string& chemin) {
	cv::Mat image = cv::imread(chemin, cv::IMREAD_GRAYSCALE);
	if (image.empty()) {
		throw std::runtime_error("ChargerEnNiveauxDeGris : lecture impossible de " + chemin);
	}
	return image;
}

ResultatComptage AnalyserImage(const cv::Mat& gris,
	std::size_t indice_reference,
	int valeur_reference_centimes) {
	ResultatComptage resultat;

	const cv::Mat binaire = SeuillerOtsu(gris);
	const cv::Mat bouchee = Imfill(binaire);
	const cv::Mat ouverte = Imopen(bouchee);
	const std::vector<Region> regions = ExtraireRegions(ouverte);

	// Sans region de reference, pas de calibration possible : on ne peut rien
	// conclure. Erreur d'appel, donc exception.
	if (indice_reference >= regions.size()) {
		throw std::invalid_argument(
			"AnalyserImage : indice de reference hors des regions detectees");
	}

	// Calibration : la piece de reference a un diametre reel connu ; son aire
	// en pixels donne l'echelle de toute l'image.
	const double diametre_reference_mm = DiametreMmPour(valeur_reference_centimes);
	const double taille_pixel_mm =
		TaillePixelMm(regions[indice_reference].aire_pixels, diametre_reference_mm);

	for (const Region& region : regions) {
		// Un petit axe nul signifie une region degeneree (ligne d'un pixel
		// d'epaisseur) : le ratio serait une division par zero. On la rejette
		// sans calcul, ce n'est de toute facon pas une piece.
		if (region.petit_axe_pixels <= 0.0) {
			++resultat.regions_rejetees;
			continue;
		}

		const double ratio_axes = region.grand_axe_pixels / region.petit_axe_pixels;

		const std::optional<int> valeur =
			IdentifierPiece(region.aire_pixels, ratio_axes, taille_pixel_mm);

		if (!valeur.has_value()) {
			++resultat.regions_rejetees;
			continue;
		}

		PieceDetectee piece;
		piece.valeur_centimes = *valeur;
		piece.centroide_x = region.centroide_x;
		piece.centroide_y = region.centroide_y;
		resultat.pieces.push_back(piece);

		resultat.total_centimes += *valeur;
	}

	return resultat;
}