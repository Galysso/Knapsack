#ifndef TAS_H
#define TAS_H

#include "graphe.h"

typedef struct Tas Tas;

struct Tas {
	Solution **tab;
	unsigned int n;
	unsigned int taille;
};

Tas *TAS_initialiser(unsigned int taille);
void TAS_ajouter(Tas *tas, Solution *val);
void TAS_retirerMax(Tas *tas);
Solution *TAS_maximum(Tas *tas);
void TAS_afficher(Tas *tas);

#endif