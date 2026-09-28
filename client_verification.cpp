#include "verification_validite.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string expression;
    std::cout << "Entrez une expression mathematique : ";
    if (!std::getline(std::cin, expression)) {
        return 1;
    }

    std::vector<char> sortie(256);
    std::size_t tailleRequise = 0;

    const auto debut = std::chrono::steady_clock::now();
    int statut = verifier_valider_expression(
        expression.c_str(), sortie.data(), sortie.size(), &tailleRequise);
    if (statut == -1 && tailleRequise > sortie.size()) {
        sortie.resize(tailleRequise);
        statut = verifier_valider_expression(
            expression.c_str(), sortie.data(), sortie.size(), &tailleRequise);
    }
    const auto fin = std::chrono::steady_clock::now();

    const std::chrono::duration<double, std::micro> duree = fin - debut;
    if (statut == -1) {
        std::cerr << "Impossible de recuperer le resultat de la DLL.\n";
        return 1;
    }

    if (statut == 1) {
        std::cout << "Expression valide. Ordre des operations (sans resultat) :\n";
    } else {
        std::cout << "Expression invalide : ";
    }
    std::cout << sortie.data() << '\n';
    std::cout << std::fixed << std::setprecision(3)
              << "Temps de validation : " << duree.count() << " us\n";

    return statut == 1 ? 0 : 1;
}