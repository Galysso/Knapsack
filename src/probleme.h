#ifndef PROBLEME_H
#define PROBLEME_H

#include "graphe.h"

#include <stdbool.h>

typedef struct Chemin Chemin;

typedef struct Probleme Probleme;
typedef struct Solution Solution;

struct Probleme {
	unsigned int n;					// nombre d'objets
	unsigned int capacite1;			// capacité du sac à dos
	unsigned int capacite2;
	unsigned int *coefficients1;	// coefficients des variables dans la fonction objectif
	unsigned int *coefficients2;
	unsigned int *poids1;			// poids des variables dans la fonction objectif
	unsigned int *poids2;
	unsigned int *poidsCumules1;
	unsigned int *poidsCumules2;
	unsigned int *coefCumules1;
	unsigned int *coefCumules2;
	unsigned int lambda1;
	unsigned int lambda2;

	bool *estFixe0;
	bool *estFixe1;
};

struct Solution {
	unsigned int obj1;
	unsigned int obj2;
	unsigned int poids1;
	unsigned int poids2;
	bool *var;						// vrai si l'objet i est dans la solution, faux sinon
};

Probleme *genererProbleme(char *nomFichier);
Probleme *genererProblemeGautier(char *nomFichier);
Solution *creerSolution(Probleme *p, Chemin *chemin);
void fixer01(Probleme *p, unsigned int y1, unsigned int y2);

#endif