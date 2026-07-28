#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp >
#include <string>

cv::Mat SeuillerOtsu(const cv::Mat& gris);
cv::Mat Imfill(const cv::Mat& binaire);
cv::Mat Imopen(const cv::Mat& fill, int rayon = 7);

struct Region {
    double aire_pixels = 0.0;
    double centroide_x = 0.0;
    double centroide_y = 0.0;
    double grand_axe_pixels = 0.0;
    double petit_axe_pixels = 0.0;
};

std::vector<Region> ExtraireRegions(const cv::Mat& binaire);

struct PieceDetectee {
    int valeur_centimes = 0;
    double centroide_x = 0.0;
    double centroide_y = 0.0;
};

struct ResultatComptage {
    int total_centimes = 0;
    std::vector<PieceDetectee> pieces;
    std::size_t regions_rejetees = 0;
};

cv::Mat ChargerEnNiveauxDeGris(const std::string& chemin);

ResultatComptage AnalyserImage(const cv::Mat& gris,
    std::size_t indice_reference,
    int valeur_reference_centimes);
