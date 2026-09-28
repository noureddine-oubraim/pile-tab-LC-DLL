#ifndef VERIFICATION_VALIDITE_H
#define VERIFICATION_VALIDITE_H

#include <stddef.h>

#ifdef _WIN32
#ifdef VERIFICATION_DLL_BUILD
#define VERIFICATION_API __declspec(dllexport)
#else
#define VERIFICATION_API __declspec(dllimport)
#endif
#else
#define VERIFICATION_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Retourne 1 si l'expression est valide, 0 si elle est invalide et -1 si
// le tampon est trop petit ou si taille_requise est nul. La taille indiquee
// inclut le caractere nul final; un appel avec sortie == NULL permet de la lire.
VERIFICATION_API int verifier_valider_expression(
    const char* expression,
    char* sortie,
    size_t capacite_sortie,
    size_t* taille_requise);

#ifdef __cplusplus
}
#endif

#endif