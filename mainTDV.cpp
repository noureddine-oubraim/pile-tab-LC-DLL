#include <iostream>      // Permet d'utiliser std::cout et std::cin.
#include <cassert>       // Permet de verifier les resultats des tests.
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <vector>

// Cette classe represente une pile generique.

template <typename T>
class Stack
{
private:
    // Le vector gere le tableau dynamique et sa memoire.
    std::vector<T> elements;

public:
    // Construit une pile avec une capacite reservee de 2 elements.
    Stack()
    {
        elements.reserve(2);
    }

    // Ajoute un element au sommet de la pile.
    void push(const T& value)
    {
        // Le vector agrandit automatiquement sa capacite si necessaire.
        elements.push_back(value);
    }

    // Retire et renvoie l'element situe au sommet de la pile.
    T pop()
    {
        // Une pile vide ne possede aucun sommet a retirer.
        if (isEmpty())
        {
            // On affiche un message au lieu d'acceder a une position inexistante.
            std::cout << "Impossible de retirer un element : la pile est vide.\n";

            // On retourne 0 car aucun element n'a ete retire.
            return T();
        }

        // On copie le sommet avant de le retirer du vector.
        T value = elements.back();
        elements.pop_back();

        // On renvoie la valeur qui vient d'etre retiree.
        return value;
    }

    // Renvoie une copie du sommet sans le retirer.
    T top() const
    {
        // Lire le sommet d'une pile vide est une erreur.
        if (isEmpty())
        {
            // On affiche un message au lieu d'acceder a une position inexistante.
            std::cout << "Impossible de lire le sommet : la pile est vide.\n";

            // On retourne 0 car aucun sommet n'existe.
            return T();
        }

        // back() renvoie le dernier element, donc le sommet de la pile.
        return elements.back();
    }

    // Indique si la pile ne contient aucun element.
    bool isEmpty() const
    {
        return elements.empty();
    }

    // Supprime tous les elements sans liberer la capacite reservee.
    void clear()
    {
        elements.clear();
    }

    // Renvoie le nombre d'elements presents dans la pile.
    std::size_t size() const
    {
        return elements.size();
    }

    // Renvoie la capacite actuellement reservee.
    std::size_t getCapacity() const
    {
        return elements.capacity();
    }
};

// Le programme commence son execution dans la fonction main.
int main()
{
    // Creation d'une pile simple qui contient des entiers.
    Stack<int> numbers;

    // Ajout de trois entiers au sommet de la pile.
    numbers.push(10);
    numbers.push(20);
    numbers.push(30);

    // Verifie les operations de consultation apres les ajouts.
    assert(numbers.size() == 3);
    assert(!numbers.isEmpty());
    assert(numbers.top() == 30);
    assert(numbers.getCapacity() >= 3);

    // Le dernier element ajoute, 30, est maintenant le sommet.
    std::cout << "Pile d'entiers :\n";
    std::cout << "Sommet : " << numbers.top()
              << " | Taille : " << numbers.size() << '\n';

    // pop retire 30, puis le sommet devient 20.
    const int removedValue = numbers.pop();
    std::cout << "Element retire : " << removedValue << '\n';
    assert(removedValue == 30);
    std::cout << "Nouveau sommet : " << numbers.top()
              << " | Taille : " << numbers.size() << '\n';
    assert(numbers.top() == 20);
    assert(numbers.size() == 2);

    // Verifie clear : les elements disparaissent, mais la capacite reste reservee.
    numbers.clear();
    assert(numbers.isEmpty());
    assert(numbers.size() == 0);
    assert(numbers.getCapacity() >= 3);

    // On demontre les messages affiches lorsqu'on consulte une pile vide.
    Stack<int> emptyStack;
    assert(emptyStack.isEmpty());

    // pop et top signalent l'absence d'element et renvoient ici la valeur par defaut (0).
    const int removedFromEmptyStack = emptyStack.pop();
    assert(removedFromEmptyStack == 0);
    const int topOfEmptyStack = emptyStack.top();
    assert(topOfEmptyStack == 0);
    assert(emptyStack.isEmpty());

    constexpr int elementCount = 1000;
    Stack<int> measuredStack;

    const auto operationsStart = std::chrono::steady_clock::now();
    for (int value = 0; value < elementCount; ++value)
    {
        measuredStack.push(value);
    }

    long long poppedSum = 0;
    for (int index = 0; index < elementCount; ++index)
    {
        poppedSum += measuredStack.pop();
    }
    const auto operationsEnd = std::chrono::steady_clock::now();

    const std::chrono::duration<double, std::micro> operationsDuration =
        operationsEnd - operationsStart;
    std::cout << std::fixed << std::setprecision(3)
              << "Tableau dynamique - 1000 empilements + 1000 depilements : "
              << operationsDuration.count() << " us\n";
    assert(poppedSum == 499500);

    // Le code de retour 0 indique que le programme s'est termine correctement.
    return 0;
}
