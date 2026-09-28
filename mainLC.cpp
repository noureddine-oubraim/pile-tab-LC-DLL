// Fournit assert, utilise ici pour verifier les resultats des tests.
#include <cassert>
#include <chrono>
// Fournit le type std::size_t, utilise pour compter les elements.
#include <cstddef>
// Fournit std::cout pour afficher le resultat final.
#include <iostream>
#include <iomanip>
// Fournit std::unique_ptr et std::make_unique pour gerer les maillons.
#include <memory>
// Fournit std::underflow_error, l'exception signalee si la pile est vide.
#include <stdexcept>
// Fournit std::string, utilise pour tester un autre type que int.
#include <string>
// Fournit std::move pour transferer des valeurs et la propriete des pointeurs.
#include <utility>

// Le parametre T rend la pile generique : il sera remplace par le type
// choisi, par exemple int dans Pile<int> ou std::string dans Pile<std::string>.
template <typename T>
class Pile {
private:
    // Un Noeud est un element de la liste chainee : il contient une valeur
    // et un pointeur vers le maillon qui se trouve juste apres lui.
    struct Noeud {
        // La donnee stockee dans ce maillon. Son type est celui de la pile.
        T valeur;

        // Possede le maillon suivant. unique_ptr garantit qu'un maillon
        // n'est possede que par un seul autre objet.
        std::unique_ptr<Noeud> suivant;

        // Recoit la donnee et la propriete du maillon suivant.
        Noeud(T valeur, std::unique_ptr<Noeud> suivant)
            // Initialise les membres en transferant les valeurs recues.
            : valeur(std::move(valeur)), suivant(std::move(suivant)) {}
    };

    // Pointeur vers le premier maillon, qui est aussi le sommet de la pile.
    // Quand la pile est vide, ce pointeur vaut nullptr.
    std::unique_ptr<Noeud> tete_;

    // Nombre de maillons presents. La pile commence avec zero element.
    std::size_t taille_ = 0;

public:
    // Constructeur sans parametre genere par le compilateur.
    Pile() = default;

    // Interdit la copie : les unique_ptr ne peuvent pas partager leurs maillons.
    Pile(const Pile&) = delete;
    Pile& operator=(const Pile&) = delete;

    // Detruit les maillons lorsque la pile cesse d'exister.
    ~Pile() {
        vider();
    }

    // Ajoute une valeur au sommet (dernier entre, premier sorti).
    void empiler(T valeur) {
        // Cree un nouveau maillon et place l'ancien sommet juste apres lui.
        tete_ = std::make_unique<Noeud>(std::move(valeur), std::move(tete_));
        ++taille_;
    }

    // Retire et renvoie la valeur du sommet.
    T depiler() {
        // Une pile vide n'a pas de sommet a retirer.
        if (estVide()) {
            throw std::underflow_error("Impossible de depiler une pile vide");
        }

        // Deplace la valeur du sommet, puis fait du maillon suivant le nouveau sommet.
        T valeur = std::move(tete_->valeur);
        tete_ = std::move(tete_->suivant);
        --taille_;
        return valeur;
    }

    // Renvoie le sommet sans le retirer ni copier sa valeur.
    const T& sommet() const {
        if (estVide()) {
            throw std::underflow_error("Une pile vide n'a pas de sommet");
        }

        return tete_->valeur;
    }

    // Indique si la pile ne contient aucun maillon.
    bool estVide() const {
        return tete_ == nullptr;
    }

    // Renvoie le nombre d'elements presents.
    std::size_t taille() const {
        return taille_;
    }

    // Retire tous les maillons, un par un, et remet le compteur a zero.
    void vider() {
        while (tete_ != nullptr) {
            // Met de cote le maillon suivant avant de detruire le sommet actuel.
            std::unique_ptr<Noeud> noeudSuivant = std::move(tete_->suivant);
            tete_ = std::move(noeudSuivant);
        }
        taille_ = 0;
    }
};

// Point d'entree du programme : l'execution commence ici.
int main() {
    // Cree une pile dont chaque valeur est un entier.
    Pile<int> nombres;

    // Empile trois nombres. Chaque nouvel element devient le sommet.
    nombres.empiler(10);
    nombres.empiler(20);
    nombres.empiler(30);

    // Verifie que les trois ajouts sont comptes.
    assert(nombres.taille() == 3);
    // Le dernier nombre ajoute, 30, doit etre au sommet.
    assert(nombres.sommet() == 30);
    // Les retraits doivent respecter LIFO : 30, puis 20, puis 10.
    assert(nombres.depiler() == 30);
    assert(nombres.depiler() == 20);
    assert(nombres.depiler() == 10);
    // Apres les trois retraits, la pile doit etre vide.
    assert(nombres.estVide());

    // Cree une autre pile, cette fois capable de stocker des chaines.
    Pile<std::string> mots;

    // Ajoute deux chaines ; "tout le monde" se trouve maintenant au sommet.
    mots.empiler("bonjour");
    mots.empiler("tout le monde");
    // Verifie que le sommet est bien retire en premier.
    assert(mots.depiler() == "tout le monde");
    // Verifie que "bonjour" est reste au sommet apres le retrait.
    assert(mots.sommet() == "bonjour");

    // Verifie que vider supprime les elements et remet la taille a zero.
    mots.vider();
    assert(mots.estVide());
    assert(mots.taille() == 0);

    // Cette variable indiquera si l'exception attendue a ete declenchee.
    bool erreurDetectee = false;

    // Tente de retirer un element de la pile nombres, qui est vide.
    try {
        nombres.depiler();
    // depiler() signale ce cas en lancant std::underflow_error.
    } catch (const std::underflow_error&) {
        // Le programme a bien detecte l'erreur attendue.
        erreurDetectee = true;
    }

    // Confirme que l'exception a bien ete lancee et interceptee.
    assert(erreurDetectee);

    // Affiche ce message si toutes les assertions precedentes ont reussi.
    std::cout << "Tous les tests de la pile ont reussi.\n";

    constexpr int nombreElements = 1000;
    Pile<int> pileMesuree;

    const auto debutOperations = std::chrono::steady_clock::now();
    for (int valeur = 0; valeur < nombreElements; ++valeur) {
        pileMesuree.empiler(valeur);
    }

    long long sommeDepilee = 0;
    for (int index = 0; index < nombreElements; ++index) {
        sommeDepilee += pileMesuree.depiler();
    }
    const auto finOperations = std::chrono::steady_clock::now();

    const std::chrono::duration<double, std::micro> dureeOperations =
        finOperations - debutOperations;
    std::cout << std::fixed << std::setprecision(3)
              << "Liste chainee - 1000 empilements + 1000 depilements : "
              << dureeOperations.count() << " us\n";
    assert(sommeDepilee == 499500);

    // Le code 0 indique que le programme s'est termine normalement.
    return 0;
}