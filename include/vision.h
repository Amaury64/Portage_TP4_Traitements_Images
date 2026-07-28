#pragma once
#include <opencv2/core.hpp>

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