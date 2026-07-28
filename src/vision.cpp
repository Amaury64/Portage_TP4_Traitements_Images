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