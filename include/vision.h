#pragma once
#include <opencv2/core.hpp>

cv::Mat SeuillerOtsu(const cv::Mat& gris);
cv::Mat Imfill(const cv::Mat& binaire);
cv::Mat Imopen(const cv::Mat& fill, int rayon = 7);