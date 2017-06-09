#ifndef TAS_H
#define TAS_H

#include "graphe.h"

typedef struct Tas Tas;

struct Tas {
	noeud **tab;
	unsigned int n;
	unsigned int taille;
};

void TAS_initialiser(Tas *tas, unsigned int taille);
void TAS_ajouter(Tas *tas, noeud *val);
void TAS_retirerMax(Tas *tas);
void TAS_retirerInd(Tas *tas, int ind);
void TAS_minimum(Tas *tas);
void TAS_maximum(Tas *tas);
void TAS_afficher(Tas *tas);

#endif