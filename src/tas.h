#ifndef TAS_H
#define TAS_H

#include "graphe.h"

typedef struct Tas Tas;

struct Tas {
	Chemin **tab;
	int n;
	int taille;
};

Tas *TAS_initialiser(int taille);
void TAS_ajouter(Tas *tas, Chemin *val);
void TAS_retirerMax(Tas *tas);
Chemin *TAS_maximum(Tas *tas);
void TAS_afficher(Tas *tas);

#endif