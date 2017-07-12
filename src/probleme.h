#ifndef PROBLEME_H
#define PROBLEME_H

#include "graphe.h"

#include <stdbool.h>

typedef struct Chemin Chemin;

typedef struct Probleme Probleme;
typedef struct Solution Solution;

struct Probleme {
	char *nomFichier;
	int n;					// nombre d'objets
	int capacite1;			// capacité du sac à dos
	int capacite2;
	int *coefficients1;		// coefficients des variables dans la fonction objectif
	int *coefficients2;
	int *poids1;			// poids des variables dans la fonction objectif
	int *poids2;
	int *poidsCumules1;
	int *poidsCumules2;
	int *coefCumules1;
	int *coefCumules2;
	int *indVar;
	int lambda1;
	int lambda2;
};

struct Solution {
	int obj1;
	int obj2;
	int poids1;
	int poids2;
	bool *var;						// vrai si l'objet i est dans la solution, faux sinon
};

Probleme *genererProbleme(char *nomFichier);
Probleme *genererProblemeGautier(char *nomFichier);
Solution *creerSolution(Probleme *p, Chemin *chemin);
Probleme *fixer01(Probleme *p, int y1, int y2);

#endif