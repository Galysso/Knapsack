#ifndef GRAPHE_H
#define GRAPHE_H

#include "probleme.h"

#include <stdbool.h>

typedef struct Noeud Noeud;
typedef struct Solution Solution;

struct Noeud {
	unsigned int val;			// valeur de la fonction objectif de la somme pondérée
	unsigned int coef1;			// valeur de la fonction objectif 1
	unsigned int coef2;			// valeur de la fonction objectif 2
	unsigned int poids1;		// 1er poids total utilisé dans le sac
	unsigned int poids2;		// 2eme poids total utilisé dans le sac
	bool existeAlt;				// vrai s'il existe au moins deux chemins allant de la racine à ce noeud
	Noeud *precBest;			// noeud précédent donnant le meilleur chemin
	Noeud *precAlt;				// noeud précédent alternatif au meilleur chemin
	bool ajoutForce;
};

struct Solution {
	void *solution;				// type Chemin s'il y a une déviation, type Noeud sinon
	unsigned int val;			// valeur de la fonction objectif de la somme pondérée
	unsigned int obj1;			// valeur de la fonction objectif 1
	unsigned int obj2;			// valeur de la fonction objectif 2
	unsigned int deviation;		// indique l'endroit où est la déviation, 0 sinon
	unsigned int nDeviation;	// donne le nombre de fois que le chemin a été réoptimisé
	bool existeAlt;				// vrai si la solution admet au moins une réoptimisation
};

Noeud ***genererGraphe(Probleme *p, unsigned int **nSol);
Solution **initialiserSolutions(Noeud **noeuds, unsigned int n);
void nouvellesSolutions(Solution ***sol, Noeud **noeuds, unsigned int n);
void afficherSolution(Solution *sol, int n);
void afficherGraphe(Noeud *racine, unsigned int n);

#endif