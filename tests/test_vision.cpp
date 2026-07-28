#include "vision.h"
#include "pieces.h"
#include <gtest/gtest.h>
#include <opencv2/core.hpp>

#include <stdexcept>
#include <opencv2/imgproc.hpp>


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


TEST(Imfill, BoucheUnTrouInterieur) {
    // Carre blanc de 40x40 en (30,30), perce d'un trou noir de 10x10 en (45,45).
    cv::Mat image = cv::Mat::zeros(100, 100, CV_8UC1);
    image(cv::Rect(30, 30, 40, 40)).setTo(cv::Scalar(255));
    image(cv::Rect(45, 45, 10, 10)).setTo(cv::Scalar(0));

    const cv::Mat resultat = Imfill(image);

    ASSERT_EQ(resultat.rows, image.rows);
    ASSERT_EQ(resultat.cols, image.cols);
    ASSERT_EQ(resultat.type(), CV_8UC1);

    // at<uchar>(ligne, colonne) : y puis x.
    EXPECT_EQ(resultat.at<uchar>(50, 50), 255);   // au centre du trou : bouche
    EXPECT_EQ(resultat.at<uchar>(10, 10), 0);     // le fond reste noir

    // Le carre plein et rien d'autre.
    EXPECT_EQ(cv::countNonZero(resultat), 40 * 40);
}

TEST(Imfill, ObjetTouchantLeCoinNeContaminePasLeFond) {
    // Sans la bordure noire ajoutee avant le remplissage, le point de depart
    // (0,0) tomberait sur l'objet : le remplissage ne changerait rien, et
    // l'inversion rendrait l'image ENTIEREMENT blanche.
    cv::Mat image = cv::Mat::zeros(100, 100, CV_8UC1);
    image(cv::Rect(0, 0, 40, 40)).setTo(cv::Scalar(255));
    image(cv::Rect(10, 10, 15, 15)).setTo(cv::Scalar(0));

    const cv::Mat resultat = Imfill(image);

    EXPECT_EQ(resultat.at<uchar>(15, 15), 255);   // trou bouche
    EXPECT_EQ(resultat.at<uchar>(90, 90), 0);     // fond intact
    EXPECT_EQ(cv::countNonZero(resultat), 40 * 40);
}

TEST(Imfill, ImageVideRejetee) {
    EXPECT_THROW(Imfill(cv::Mat()), std::invalid_argument);
}

TEST(Imfill, ImageCouleurRejetee) {
    const cv::Mat couleur(10, 10, CV_8UC3, cv::Scalar(0, 0, 0));
    EXPECT_THROW(Imfill(couleur), std::invalid_argument);
}


TEST(Imopen, SupprimeUnPontFinEntreDeuxCarres) {
    // Deux carres de 40x40 relies par un pont de 4 pixels d'epaisseur.
    // Avec un rayon de 7, l'element structurant fait 15x15 : le pont, plus fin,
    // ne survit pas a l'erosion ; les carres, bien plus larges, si.
    cv::Mat image = cv::Mat::zeros(100, 200, CV_8UC1);
    image(cv::Rect(20, 30, 40, 40)).setTo(cv::Scalar(255));    // carre gauche
    image(cv::Rect(140, 30, 40, 40)).setTo(cv::Scalar(255));   // carre droit
    image(cv::Rect(60, 48, 80, 4)).setTo(cv::Scalar(255));     // pont

    // Verification du montage : le pont est bien la au depart.
    ASSERT_EQ(image.at<uchar>(50, 100), 255);

    const cv::Mat resultat = Imopen(image);

    ASSERT_EQ(resultat.rows, image.rows);
    ASSERT_EQ(resultat.cols, image.cols);
    ASSERT_EQ(resultat.type(), CV_8UC1);

    EXPECT_EQ(resultat.at<uchar>(50, 40), 255);    // centre du carre gauche
    EXPECT_EQ(resultat.at<uchar>(50, 160), 255);   // centre du carre droit
    EXPECT_EQ(resultat.at<uchar>(50, 100), 0);     // le pont a disparu
    EXPECT_EQ(resultat.at<uchar>(10, 10), 0);      // le fond reste noir

    // L'ouverture ne peut qu'enlever des pixels, jamais en ajouter.
    EXPECT_LT(cv::countNonZero(resultat), cv::countNonZero(image));
}

TEST(Imopen, RayonInvalideRejete) {
    const cv::Mat image = cv::Mat::zeros(50, 50, CV_8UC1);
    EXPECT_THROW(Imopen(image, 0), std::invalid_argument);
    EXPECT_THROW(Imopen(image, -3), std::invalid_argument);
}

TEST(Imopen, ImageVideRejetee) {
    EXPECT_THROW(Imopen(cv::Mat()), std::invalid_argument);
}

TEST(Imopen, ImageCouleurRejetee) {
    const cv::Mat couleur(10, 10, CV_8UC3, cv::Scalar(0, 0, 0));
    EXPECT_THROW(Imopen(couleur), std::invalid_argument);
}

TEST(ExtraireRegions, DeuxCarresSepares) {
    // Deux carres bien separes, d'aires differentes pour pouvoir les
    // distinguer sans dependre de l'ordre de sortie d'OpenCV.
    cv::Mat image = cv::Mat::zeros(100, 100, CV_8UC1);
    image(cv::Rect(10, 20, 30, 30)).setTo(cv::Scalar(255));   // aire 900
    image(cv::Rect(60, 20, 20, 20)).setTo(cv::Scalar(255));   // aire 400

    std::vector<Region> regions = ExtraireRegions(image);

    // Le fond n'est pas renvoye : deux objets, deux regions.
    ASSERT_EQ(regions.size(), 2u);

    // Rien ne garantit l'ordre : on trie par aire decroissante.
    std::sort(regions.begin(), regions.end(),
        [](const Region& a, const Region& b) {
            return a.aire_pixels > b.aire_pixels;
        });

    EXPECT_DOUBLE_EQ(regions[0].aire_pixels, 900.0);
    EXPECT_DOUBLE_EQ(regions[1].aire_pixels, 400.0);

    // Centroide d'un carre de w pixels commencant en x0 : x0 + (w-1)/2.
    // Grand carre : x de 10 a 39 -> 24.5 ; y de 20 a 49 -> 34.5.
    EXPECT_NEAR(regions[0].centroide_x, 24.5, 1e-9);
    EXPECT_NEAR(regions[0].centroide_y, 34.5, 1e-9);

    // Petit carre : x de 60 a 79 -> 69.5 ; y de 20 a 39 -> 29.5.
    EXPECT_NEAR(regions[1].centroide_x, 69.5, 1e-9);
    EXPECT_NEAR(regions[1].centroide_y, 29.5, 1e-9);
}

TEST(ExtraireRegions, ImageNoireNeDonneAucuneRegion) {
    // Le fond seul ne doit pas etre renvoye comme une region.
    const cv::Mat image = cv::Mat::zeros(50, 50, CV_8UC1);

    EXPECT_TRUE(ExtraireRegions(image).empty());
}

TEST(ExtraireRegions, ImageVideRejetee) {
    EXPECT_THROW(ExtraireRegions(cv::Mat()), std::invalid_argument);
}

TEST(ExtraireRegions, ImageCouleurRejetee) {
    const cv::Mat couleur(10, 10, CV_8UC3, cv::Scalar(0, 0, 0));
    EXPECT_THROW(ExtraireRegions(couleur), std::invalid_argument);
}

TEST(ExtraireRegions, DisqueADesAxesEgaux) {
    // Un disque parfait : l'ellipse equivalente est un cercle, les deux axes
    // valent le diametre et le ratio vaut 1.
    cv::Mat image = cv::Mat::zeros(200, 200, CV_8UC1);
    cv::circle(image, cv::Point(100, 100), 40, cv::Scalar(255), cv::FILLED);

    const std::vector<Region> regions = ExtraireRegions(image);
    ASSERT_EQ(regions.size(), 1u);

    const Region& r = regions[0];

    // Tolerance en pourcentage : la discretisation d'un disque en pixels
    // n'est jamais exacte. Un seuil trop serre rendrait le test fragile.
    EXPECT_NEAR(r.grand_axe_pixels, 80.0, 80.0 * 0.02);
    EXPECT_NEAR(r.petit_axe_pixels, 80.0, 80.0 * 0.02);

    EXPECT_NEAR(r.grand_axe_pixels / r.petit_axe_pixels, 1.0, 0.02);
}

TEST(ExtraireRegions, EllipseRetrouveSesAxes) {
    // Ellipse de demi-axes 60 (horizontal) et 30 (vertical) :
    // les longueurs totales attendues sont 120 et 60.
    cv::Mat image = cv::Mat::zeros(200, 300, CV_8UC1);
    cv::ellipse(image, cv::Point(150, 100), cv::Size(60, 30),
        0.0, 0.0, 360.0, cv::Scalar(255), cv::FILLED);

    const std::vector<Region> regions = ExtraireRegions(image);
    ASSERT_EQ(regions.size(), 1u);

    const Region& r = regions[0];

    EXPECT_NEAR(r.grand_axe_pixels, 120.0, 120.0 * 0.02);
    EXPECT_NEAR(r.petit_axe_pixels, 60.0, 60.0 * 0.02);

    // Le grand axe est bien le plus grand des deux : lambda1 >= lambda2.
    EXPECT_GT(r.grand_axe_pixels, r.petit_axe_pixels);
}

TEST(ExtraireRegions, EllipseTourneeDonneLesMemesAxes) {
    // Meme ellipse, tournee de 30 degres. Les axes sont des grandeurs
    // intrinsequement liees a la forme : ils ne doivent pas dependre de
    // l'orientation. C'est ce que garantit le passage par les valeurs propres
    // plutot que par les variances brutes en x et en y.
    cv::Mat image = cv::Mat::zeros(300, 300, CV_8UC1);
    cv::ellipse(image, cv::Point(150, 150), cv::Size(60, 30),
        30.0, 0.0, 360.0, cv::Scalar(255), cv::FILLED);

    const std::vector<Region> regions = ExtraireRegions(image);
    ASSERT_EQ(regions.size(), 1u);

    EXPECT_NEAR(regions[0].grand_axe_pixels, 120.0, 120.0 * 0.02);
    EXPECT_NEAR(regions[0].petit_axe_pixels, 60.0, 60.0 * 0.02);
}

TEST(ExtraireRegions, RectangleAllongeADesAxesTresDifferents) {
    // Pour un rectangle w x h, l'ellipse equivalente a un ratio d'axes
    // exactement egal a w/h : ici 100/20 = 5, bien au-dela du seuil de
    // circularite de 1.15 utilise pour rejeter les intrus.
    cv::Mat image = cv::Mat::zeros(200, 300, CV_8UC1);
    image(cv::Rect(50, 90, 100, 20)).setTo(cv::Scalar(255));

    const std::vector<Region> regions = ExtraireRegions(image);
    ASSERT_EQ(regions.size(), 1u);

    const Region& r = regions[0];
    EXPECT_NEAR(r.grand_axe_pixels / r.petit_axe_pixels, 5.0, 5.0 * 0.02);
}

TEST(AnalyserImage, CompteDeuxPiecesEtRejetteUnIntrus) {
    cv::Mat image = cv::Mat::zeros(400, 800, CV_8UC1);

    // Piece de reference : 2 EUR (25.75 mm), tracee avec 200 px de diametre.
    // L'echelle deduite sera donc 25.75/200 = 0.128 mm par pixel.
    cv::circle(image, cv::Point(150, 200), 100, cv::Scalar(255), cv::FILLED);

    // Piece de 1 EUR (23.25 mm) : a cette echelle, environ 180 px de diametre.
    cv::circle(image, cv::Point(400, 200), 90, cv::Scalar(255), cv::FILLED);

    // Intrus : ellipse tres allongee, d'aire comparable a une petite piece.
    // Elle passe le test de l'aire mais doit etre rejetee par la circularite.
    cv::ellipse(image, cv::Point(650, 200), cv::Size(100, 35),
        0.0, 0.0, 360.0, cv::Scalar(255), cv::FILLED);

    // Les composantes sont numerotees dans l'ordre de balayage : le disque de
    // 2 EUR, dont le sommet est le plus haut, arrive en premier.
    const ResultatComptage resultat = AnalyserImage(image, 0, 200);

    EXPECT_EQ(resultat.pieces.size(), 2u);
    EXPECT_EQ(resultat.regions_rejetees, 1u);
    EXPECT_EQ(resultat.total_centimes, 300);   // 200 + 100
}

TEST(AnalyserImage, IndiceDeReferenceHorsPlageRejete) {
    cv::Mat image = cv::Mat::zeros(300, 300, CV_8UC1);
    cv::circle(image, cv::Point(150, 150), 80, cv::Scalar(255), cv::FILLED);

    // Une seule region detectee : l'indice 5 n'existe pas.
    EXPECT_THROW(AnalyserImage(image, 5, 200), std::invalid_argument);
}

TEST(AnalyserImage, ValeurDeReferenceInconnueRejetee) {
    cv::Mat image = cv::Mat::zeros(300, 300, CV_8UC1);
    cv::circle(image, cv::Point(150, 150), 80, cv::Scalar(255), cv::FILLED);

    // 42 centimes n'est pas une piece en euros.
    EXPECT_THROW(AnalyserImage(image, 0, 42), std::invalid_argument);
}