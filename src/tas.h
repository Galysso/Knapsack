#ifndef TAS_H
#define TAS_H

#include "graphe.h"

typedef struct Tas Tas;

struct Tas {
	Chemin **tab;
	unsigned int n;
	unsigned int taille;
};

Tas *TAS_initialiser(unsigned int taille);
void TAS_ajouter(Tas *tas, Chemin *val);
void TAS_retirerMax(Tas *tas);
Chemin *TAS_maximum(Tas *tas);
void TAS_afficher(Tas *tas);

#endif