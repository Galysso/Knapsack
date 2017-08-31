#ifndef GRAPHE_H
#define GRAPHE_H

#include "probleme.h"

#include <stdbool.h>

typedef struct ListeSol ListeSol;
typedef struct Probleme Probleme;
typedef struct Solution Solution;

typedef struct Noeud Noeud;
typedef struct Chemin Chemin;

struct Noeud {
	int val;			// valeur de la fonction objectif de la somme pondérée
	int p1;				// valeur de la fonction objectif 1
	int p2;				// valeur de la fonction objectif 2
	int w1;				// 1er poids total des objets ajoutés
	int w2;				// 2eme poids total des objets ajoutés
	bool existeAlt;				// vrai s'il existe au moins deux chemins allant de la racine à ce noeud
	Noeud *precBest;			// noeud précédent donnant le meilleur chemin
	Noeud *precAlt;				// noeud précédent alternatif au meilleur chemin
	bool ajoutForce;
};

struct Chemin {
	void *chemin;				// type Chemin s'il y a une déviation, type Noeud sinon
	int p1;
	int p2;
	int val;			// valeur de la fonction objectif de la somme pondérée
	int deviation;		// indique l'endroit où est la déviation, 0 sinon
	int nDeviation;		// donne le nombre de fois que le chemin a été réoptimisé
	bool existeAlt;		// vrai si la solution admet au moins une réoptimisation
};

Noeud ***genererGraphe(Probleme *p, int **nSol, Solution *sol1, Solution *sol2, ListeSol *lSolHeur);
Chemin **initialiserChemins(Noeud **noeuds, int n);
Noeud *creerNoeudAvecAjout(Probleme *p, int i, Noeud *noeudPrec);
Noeud *creerNoeudSansAjout(Noeud *noeudPrec);
void modifierNoeudAvecAjout(Probleme *p, int i, Noeud *noeudModif, Noeud* noeudPrec);
void modifierNoeudSansAjout(Noeud *noeudModif, Noeud* noeudPrec);
void desallouerGraphe(int *nSol, Noeud ***noeud, int n);
void afficherChemin(Chemin *sol, int n);
void afficherGraphe(Noeud *racine, int n);
void checkGenerationNoeuds(Noeud *noeudPrec, Probleme *p, int i, bool *bAjout, bool *bSansAjout, ListeSol *lSolHeur, int y1, int y2);

#endif