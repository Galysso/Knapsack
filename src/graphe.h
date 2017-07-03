#ifndef GRAPHE_H
#define GRAPHE_H

#include "probleme.h"

#include <stdbool.h>

typedef struct Probleme Probleme;
typedef struct Solution Solution;

typedef struct Noeud Noeud;
typedef struct Chemin Chemin;

struct Noeud {
	unsigned int val;			// valeur de la fonction objectif de la somme pondérée
	unsigned int obj1;			// valeur de la fonction objectif 1
	unsigned int obj2;			// valeur de la fonction objectif 2
	unsigned int poids1;		// 1er poids total utilisé dans le sac
	unsigned int poids2;		// 2eme poids total utilisé dans le sac
	bool existeAlt;				// vrai s'il existe au moins deux chemins allant de la racine à ce noeud
	Noeud *precBest;			// noeud précédent donnant le meilleur chemin
	Noeud *precAlt;				// noeud précédent alternatif au meilleur chemin
	bool ajoutForce;
};

struct Chemin {
	void *chemin;				// type Chemin s'il y a une déviation, type Noeud sinon
	unsigned int obj1;
	unsigned int obj2;
	unsigned int val;			// valeur de la fonction objectif de la somme pondérée
	unsigned int deviation;		// indique l'endroit où est la déviation, 0 sinon
	unsigned int nDeviation;	// donne le nombre de fois que le chemin a été réoptimisé
	bool existeAlt;				// vrai si la solution admet au moins une réoptimisation
};

Noeud ***genererGraphe(Probleme *p, unsigned int **nSol, Solution *sol1, Solution *sol2);
Chemin **initialiserChemins(Noeud **noeuds, unsigned int n);
void desallouerGraphe(unsigned int *nSol, Noeud ***noeud, unsigned int n);
void afficherChemin(Chemin *sol, int n);
void afficherGraphe(Noeud *racine, unsigned int n);

#endif