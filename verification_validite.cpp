// Interface exportee par la DLL.
#include "verification_validite.h"
// Copie le texte resultat dans le tampon fourni par l'appelant.
#include <cstring>
// Fonctions pour reconnaitre lettres, chiffres et espaces.
#include <cctype>
// std::stack sert de pile pour retenir les operateurs en attente.
#include <stack>
// Exceptions utilisees pour signaler une expression invalide.
#include <stdexcept>
// std::string sert a stocker l'expression et le texte de chaque jeton.
#include <string>
// std::vector stocke les jetons et les operations symboliques a afficher.
#include <vector>

// Le type permet au programme de distinguer les differentes sortes de jetons.
enum class TypeJeton {
    Nombre,                // Une valeur numerique, par exemple 12 ou 3.5.
    Variable,              // Un nom de variable, par exemple x ou vitesse.
    Operateur,             // Un operateur reconnu, par exemple +, * ou ^.
    ParentheseOuvrante,    // Le symbole ( qui commence un groupe.
    ParentheseFermante     // Le symbole ) qui termine un groupe.
};

// Un jeton conserve a la fois sa categorie et son texte original.
struct Jeton {
    TypeJeton type;       // Sert a choisir le traitement a effectuer.
    std::string texte;    // Sert a afficher ou traiter le symbole lui-meme.
};

// Decoupe l'expression en jetons et ajoute * quand la multiplication est implicite,
// par exemple entre 5 et x dans 5x, ou entre 2 et ( dans 2(x+1).
std::vector<Jeton> analyserJetons(const std::string& expression) {
    // Cette premiere liste contiendra les morceaux reconnus dans l'expression.
    std::vector<Jeton> jetons;

    // position indique le caractere de l'expression qui doit etre examine.
    std::size_t position = 0;

    // Parcourt l'expression de gauche a droite, sans reutiliser un caractere.
    while (position < expression.size()) {
        // Les fonctions de <cctype> recoivent un unsigned char pour eviter
        // les comportements indefinis avec les caracteres dont le code est negatif.
        const unsigned char caractere =
            static_cast<unsigned char>(expression[position]);

        // Les espaces ne produisent pas de jeton : on avance simplement.
        if (std::isspace(caractere)) {
            ++position;
        } else if (std::isdigit(caractere) || expression[position] == '.') {
            // Memorise le debut du nombre afin de pouvoir copier son texte ensuite.
            const std::size_t debut = position;

            // Ces indicateurs empechent plusieurs points et un point sans chiffre.
            bool pointDecimalVu = false;
            bool chiffreVu = false;

            // Lit les chiffres qui composent le nombre et accepte un point decimal.
            while (position < expression.size()) {
                const char courant = expression[position];
                if (std::isdigit(static_cast<unsigned char>(courant))) {
                    // Un chiffre a ete trouve et consomme.
                    chiffreVu = true;
                    ++position;
                } else if (courant == '.' && !pointDecimalVu) {
                    // Le premier point decimal est accepte et consomme.
                    pointDecimalVu = true;
                    ++position;
                } else {
                    // Un autre caractere marque la fin de ce nombre.
                    break;
                }
            }

            // Refuse le texte '.' car il ne contient aucun chiffre.
            if (!chiffreVu) {
                throw std::runtime_error("Un point doit appartenir a un nombre.");
            }
            // Copie la portion analysee et l'enregistre comme jeton Nombre.
            jetons.push_back({TypeJeton::Nombre,
                              expression.substr(debut, position - debut)});
        } else if (std::isalpha(caractere) || expression[position] == '_') {
            // Une variable commence par une lettre ou un underscore.
            const std::size_t debut = position;

            // Son nom peut ensuite contenir des lettres, chiffres et underscores.
            while (position < expression.size()) {
                const unsigned char courant =
                    static_cast<unsigned char>(expression[position]);
                if (!std::isalnum(courant) && expression[position] != '_') {
                    break;
                }
                ++position;
            }
            // Enregistre tout le nom de variable comme un seul jeton.
            jetons.push_back({TypeJeton::Variable,
                              expression.substr(debut, position - debut)});
        } else {
            // Les autres caracteres sont examines comme operateur ou parenthese.
            const char symbole = expression[position++];
            switch (symbole) {
                // Ces cinq signes sont les operateurs pris en charge.
                case '+':
                case '-':
                case '*':
                case '/':
                case '^':
                    jetons.push_back({TypeJeton::Operateur, std::string(1, symbole)});
                    break;
                // Les parentheses gardent leur propre type de jeton.
                case '(':
                    jetons.push_back({TypeJeton::ParentheseOuvrante, "("});
                    break;
                case ')':
                    jetons.push_back({TypeJeton::ParentheseFermante, ")"});
                    break;
                default:
                    // Tout symbole qui n'est pas gere rend l'expression invalide.
                    throw std::runtime_error(
                        std::string("Caractere non reconnu : ") + symbole);
            }
        }
    }

    // Insere une multiplication uniquement dans les cas usuels comme 5x, 2(x+1),
    // ou (x+1)(x-1). Deux nombres juxtaposes restent une erreur de syntaxe.
    // Deuxieme liste : elle reprend les jetons et ajoute les multiplications manquantes.
    std::vector<Jeton> resultat;
    // Examine chaque jeton en le comparant au dernier jeton deja ajoute.
    for (const Jeton& jeton : jetons) {
        if (!resultat.empty()) {
            // Le type precedent permet de decider si deux elements voisins impliquent *.
            const TypeJeton precedent = resultat.back().type;

            // Ex.: 5x signifie 5*x. Deux variables consecutives restent un seul nom
            // lors de la tokenisation (par ex. xy), donc ce cas ne se presente pas ici.
            const bool apresNombreAvantVariable =
                precedent == TypeJeton::Nombre && jeton.type == TypeJeton::Variable;

            // Ex.: 2(x+1), x(x+1) ou (x+1)(x-1) necessitent une multiplication.
            const bool apresFinAvantParenthese =
                (precedent == TypeJeton::Nombre || precedent == TypeJeton::Variable ||
                 precedent == TypeJeton::ParentheseFermante) &&
                jeton.type == TypeJeton::ParentheseOuvrante;

            // Ex.: (x+1)2 ou (x+1)x necessitent aussi une multiplication.
            const bool apresParentheseAvantValeur =
                precedent == TypeJeton::ParentheseFermante &&
                (jeton.type == TypeJeton::Nombre || jeton.type == TypeJeton::Variable);

            // Si l'une des formes precedentes est detectee, insere un jeton * avant
            // le jeton courant. La ligne suivante ajoute ensuite ce jeton courant.
            if (apresNombreAvantVariable || apresFinAvantParenthese ||
                apresParentheseAvantValeur) {
                resultat.push_back({TypeJeton::Operateur, "*"});
            }
        }
        // Recopie le jeton original dans la liste finale.
        resultat.push_back(jeton);
    }

    // Retourne la liste complete, avec les multiplications implicites explicitees.
    return resultat;
}

// Donne la priorite numerique d'un operateur. Les signes unaires sont internes
// a l'algorithme et ne servent qu'a decrire l'ordre des operations.
int priorite(const std::string& operateur) {
    // Plus le nombre retourne est grand, plus l'operateur est prioritaire.
    if (operateur == "^") return 4;
    if (operateur == "u+" || operateur == "u-") return 3;
    if (operateur == "*" || operateur == "/") return 2;
    return 1;
}

// L'exponentiation et les signes unaires s'associent de droite a gauche.
bool associatifADroite(const std::string& operateur) {
    // A^B^C signifie A^(B^C), et les signes unaires portent sur ce qui les suit.
    return operateur == "^" || operateur == "u+" || operateur == "u-";
}

// Applique symboliquement l'operateur au sommet : construit une expression
// texte et enregistre cette operation, sans effectuer de calcul numerique.
void appliquerOperateur(std::stack<std::string>& operateurs,
                        std::stack<std::string>& expressions,
                        std::vector<std::string>& etapes) {
    const std::string operateur = operateurs.top();
    operateurs.pop();

    // Un operateur unaire (u+ ou u-) agit sur une seule expression.
    if (operateur == "u+" || operateur == "u-") {
        if (expressions.empty()) {
            throw std::runtime_error("Operande manquant pour un signe unaire.");
        }

        const std::string operande = expressions.top();
        expressions.pop();

        const std::string signe = operateur == "u-" ? "-" : "+";
        const std::string operation = signe + "(" + operande + ")";
        etapes.push_back(operation);
        expressions.push(operation);
        return;
    }

    // Un operateur binaire a besoin d'une expression a gauche et d'une a droite.
    if (expressions.size() < 2) {
        throw std::runtime_error("Operande manquant pour un operateur.");
    }

    // On retire d'abord la droite, puis la gauche, selon l'ordre de la pile.
    const std::string droite = expressions.top();
    expressions.pop();
    const std::string gauche = expressions.top();
    expressions.pop();

    // Les parentheses conservent explicitement le regroupement de l'expression.
    const std::string operation = gauche + " " + operateur + " " + droite;
    etapes.push_back(operation);
    expressions.push("(" + operation + ")");
}

// Verifie la syntaxe, determine l'ordre de priorite et renvoie les operations
// sous forme symbolique, sans calculer de resultat numerique.
std::vector<std::string> verifierExpression(const std::vector<Jeton>& jetons) {
    // Aucun jeton signifie qu'aucune expression n'a ete saisie.
    if (jetons.empty()) {
        throw std::runtime_error("L'expression est vide.");
    }

    // La pile garde les operateurs en attente et les parentheses ouvrantes.
    std::stack<std::string> operateurs;

    // Cette pile conserve les nombres, variables et expressions deja regroupees.
    std::stack<std::string> expressions;

    // Les operations symboliques seront affichees dans leur ordre de traitement.
    std::vector<std::string> etapes;

    // Vrai au debut et apres un operateur ou '(' : le prochain element
    // doit alors etre une valeur, une parenthese ouvrante ou un signe unaire.
    bool attendOperande = true;

    // Examine les jetons un par un afin de verifier leur ordre grammatical.
    for (const Jeton& jeton : jetons) {
        if (jeton.type == TypeJeton::Nombre || jeton.type == TypeJeton::Variable) {
            // Deux valeurs sans operateur entre elles sont interdites.
            if (!attendOperande) {
                throw std::runtime_error("Il manque un operateur entre deux valeurs.");
            }
            // La valeur devient un fragment que les operations suivantes pourront utiliser.
            expressions.push(jeton.texte);
            // Une valeur vient d'etre lue : un operateur ou ')' peut suivre.
            attendOperande = false;
        } else if (jeton.type == TypeJeton::ParentheseOuvrante) {
            // Une parenthese ouvrante ne peut pas suivre une valeur sans operateur.
            if (!attendOperande) {
                throw std::runtime_error("Il manque un operateur avant la parenthese.");
            }
            // Marque le debut d'un groupe : les operateurs de ce groupe restent
            // dans la pile jusqu'a la parenthese fermante correspondante.
            operateurs.push("(");
            attendOperande = true;
        } else if (jeton.type == TypeJeton::ParentheseFermante) {
            // Refuse () ainsi qu'un groupe dont le contenu se termine par un operateur.
            if (attendOperande) {
                throw std::runtime_error("Parenthese vide ou operande manquant.");
            }

            // Termine les operations du groupe jusqu'a retrouver son '('.
            while (!operateurs.empty() && operateurs.top() != "(") {
                appliquerOperateur(operateurs, expressions, etapes);
            }
            // Si aucune '(' n'a ete trouvee, cette ')' est en trop.
            if (operateurs.empty()) {
                throw std::runtime_error("Parenthese fermante sans ouverture.");
            }
            // Retire la parenthese ouvrante correspondante de la pile.
            operateurs.pop();
            // Le groupe complet compte maintenant comme une valeur terminee.
            attendOperande = false;
        } else if (jeton.type == TypeJeton::Operateur) {
            // Courant est l'operateur dont on determine la place.
            std::string courant = jeton.texte;

            // + et - peuvent etre unaires au debut ou apres un autre operateur.
            if (attendOperande) {
                // Les autres operateurs ont besoin d'une valeur a leur gauche.
                if (courant != "+" && courant != "-") {
                    throw std::runtime_error("Un operateur attend une valeur avant lui.");
                }
                // Le prefixe u distingue le signe unaire de l'addition/soustraction.
                operateurs.push(courant == "+" ? "u+" : "u-");
                // On attend toujours la valeur sur laquelle ce signe va agir.
                continue;
            }

            // Traite d'abord les operateurs precedents qui ont priorite sur le courant.
            while (!operateurs.empty() && operateurs.top() != "(") {
                // precedent est le dernier operateur en attente.
                const std::string& precedent = operateurs.top();
                // Un operateur precedent plus prioritaire doit etre traite d'abord.
                const bool precedentPrioritaire = priorite(precedent) > priorite(courant);
                // A priorite egale, les operateurs associatifs a gauche se traitent
                // de gauche a droite; ceux associatifs a droite restent empiles.
                const bool memePrioriteAGauche =
                    priorite(precedent) == priorite(courant) &&
                    !associatifADroite(courant);
                // Si aucune condition n'oblige a sortir precedent, on peut s'arreter.
                if (!precedentPrioritaire && !memePrioriteAGauche) {
                    break;
                }
                // Traite l'operateur precedent avant le courant et garde le regroupement.
                appliquerOperateur(operateurs, expressions, etapes);
            }
            // Le nouvel operateur attend son operande de droite.
            operateurs.push(courant);
            attendOperande = true;
        }
    }

    // Si l'analyse s'est terminee en attendant une valeur, l'expression est incomplete.
    if (attendOperande) {
        throw std::runtime_error("L'expression se termine par un operateur.");
    }

    // Traite les operations restantes et verifie qu'aucune parenthese n'est oubliee.
    while (!operateurs.empty()) {
        if (operateurs.top() == "(") {
            throw std::runtime_error("Parenthese ouvrante sans fermeture.");
        }
        appliquerOperateur(operateurs, expressions, etapes);
    }

    // Une expression complete doit se reduire a un seul fragment symbolique.
    if (expressions.size() != 1) {
        throw std::runtime_error("L'expression ne forme pas une operation complete.");
    }

    return etapes;
}

int verifier_valider_expression(const char* expression, char* sortie,
                                std::size_t capaciteSortie,
                                std::size_t* tailleRequise) {
    if (tailleRequise == nullptr) {
        return -1;
    }

    std::string message;
    int statut = 1;
    try {
        if (expression == nullptr) {
            throw std::runtime_error("L'expression est nulle.");
        }

        const std::vector<Jeton> jetons = analyserJetons(expression);
        const std::vector<std::string> etapes = verifierExpression(jetons);
        if (etapes.empty()) {
            message = "Aucune operation a effectuer.";
        }
        for (std::size_t index = 0; index < etapes.size(); ++index) {
            if (!message.empty()) {
                message += '\n';
            }
            message += std::to_string(index + 1) + ". " + etapes[index];
        }
    } catch (const std::runtime_error& erreur) {
        statut = 0;
        message = erreur.what();
    }

    *tailleRequise = message.size() + 1;
    if (sortie == nullptr || capaciteSortie < *tailleRequise) {
        return -1;
    }

    std::memcpy(sortie, message.c_str(), *tailleRequise);
    return statut;
}