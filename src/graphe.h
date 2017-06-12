#ifndef GRAPHE_H
#define GRAPHE_H

#include "probleme.h"

typedef struct Noeud Noeud;

struct Noeud {
	unsigned int val;			// valeur de la fonction objectif de la somme pondérée
	unsigned int coef1;			// valeur de la fonction objectif 1
	unsigned int coef2;			// valeur de la fonction objectif 2
	unsigned int poids1;		// 1er poids total utilisé dans le sac
	unsigned int poids2;		// 2eme poids total utilisé dans le sac 
	Noeud *precBest;			// noeud précédent donnant le meilleur chemin
	Noeud *precAlt;				// noeud précédent alternatif au meilleur chemin
};

Noeud ***genererGraphe(Probleme *p, unsigned int *n);
void afficherGraphe(Noeud *racine, unsigned int n);

#endif