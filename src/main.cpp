#include "pieces.h"
#include "vision.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

    // Message d'usage sur cerr : c'est un diagnostic, pas le resultat attendu.
    // Le programme peut ainsi etre appele dans un script sans polluer sa sortie.
    void AfficherUsage(const char* nom_programme) {
        std::cerr << "Usage : " << nom_programme
            << " <image> <indice_reference> <valeur_reference_centimes>\n"
            << "  image                       chemin du fichier a analyser\n"
            << "  indice_reference            indice de la region servant a calibrer\n"
            << "  valeur_reference_centimes   200, 100, 50, 20, 10, 5, 2 ou 1\n"
            << "\nExemple : " << nom_programme << " data/pieces1.jpg 0 200\n";
    }

    // Les montants circulent en centimes entiers dans tout le programme : aucune
    // erreur d'arrondi ne s'accumule. La conversion en euros n'a lieu qu'ici,
    // au moment de l'affichage.
    std::string FormaterEuros(int centimes) {
        const int euros = centimes / 100;
        const int reste = centimes % 100;
        return std::to_string(euros) + "," + (reste < 10 ? "0" : "") + std::to_string(reste)
            + " EUR";
    }

}  // namespace

int main(int argc, char** argv) {
    if (argc != 4) {
        AfficherUsage(argv[0]);
        return 1;
    }


    try {
        const std::string chemin = argv[1];
        const std::size_t indice_reference =
            static_cast<std::size_t>(std::stoul(argv[2]));
        const int valeur_reference = std::stoi(argv[3]);

        const cv::Mat gris = ChargerEnNiveauxDeGris(chemin);
        std::cout << "Image : " << chemin << " (" << gris.cols << "x" << gris.rows
            << ")\n";
        // Diagnostic : lister les regions pour choisir la reference.
        const cv::Mat binaire = SeuillerOtsu(gris);
        const std::vector<Region> regions = ExtraireRegions(Imopen(Imfill(binaire)));
        for (std::size_t i = 0; i < regions.size(); ++i) {
            const Region& r = regions[i];
            std::cout << "  [" << i << "] aire=" << r.aire_pixels
                << "  centroide=(" << r.centroide_x << ", " << r.centroide_y << ")"
                << "  ratio=" << r.grand_axe_pixels / r.petit_axe_pixels << "\n";
        }

        const ResultatComptage resultat =
            AnalyserImage(gris, indice_reference, valeur_reference);

        std::cout << "\nPieces detectees : " << resultat.pieces.size() << "\n";
        for (const PieceDetectee& piece : resultat.pieces) {
            std::cout << "  " << std::setw(9) << std::right
                << FormaterEuros(piece.valeur_centimes)
                << "   au centroide (" << std::fixed << std::setprecision(1)
                << piece.centroide_x << ", " << piece.centroide_y << ")\n";
        }

        std::cout << "Regions rejetees : " << resultat.regions_rejetees << "\n";
        std::cout << "\nTotal : " << FormaterEuros(resultat.total_centimes) << "\n";

    }
    catch (const std::exception& e) {
        // Toutes les exceptions de la bibliotheque derivent de std::exception.
        // Sans ce catch, une exception non rattrapee terminerait le programme
        // par abort() : message hostile, code de sortie opaque.
        std::cerr << "Erreur : " << e.what() << "\n";
        return 1;
    }

    return 0;
}