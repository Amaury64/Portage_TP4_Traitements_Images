#include "vision.h"

#include <gtest/gtest.h>
#include <opencv2/core.hpp>

#include <stdexcept>


TEST(SeuillerOtsu, SepareDeuxNiveauxFranchement) {
    // Image synthetique fabriquee dans le test : aucun fichier externe, et on
    // connait la verite terrain puisqu'on l'a construite.
    // Fond a 50 (sombre), moitie gauche a 200 (claire).
    cv::Mat image(100, 100, CV_8UC1, cv::Scalar(50));
    image(cv::Rect(0, 0, 50, 100)).setTo(cv::Scalar(200));

    const cv::Mat resultat = SeuillerOtsu(image);

    ASSERT_EQ(resultat.rows, image.rows);
    ASSERT_EQ(resultat.cols, image.cols);
    ASSERT_EQ(resultat.type(), CV_8UC1);

    // Otsu doit placer le seuil entre 50 et 200 : la moitie claire passe a 255,
    // la moitie sombre a 0.
    // Attention a l'ordre : at<uchar>(ligne, colonne), donc y puis x.
    EXPECT_EQ(resultat.at<uchar>(50, 10), 255);   // zone claire
    EXPECT_EQ(resultat.at<uchar>(50, 90), 0);     // zone sombre

    // Verification globale, plus robuste que deux pixels isoles.
    EXPECT_EQ(cv::countNonZero(resultat), 50 * 100);
}

TEST(SeuillerOtsu, ImageCouleurRejetee) {
    cv::Mat couleur(10, 10, CV_8UC3, cv::Scalar(0, 0, 0));
    EXPECT_THROW(SeuillerOtsu(couleur), std::invalid_argument);
}

TEST(SeuillerOtsu, ImageVideRejetee ) {
    EXPECT_THROW(SeuillerOtsu(cv::Mat()), std::invalid_argument);
}