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
	int nBis;				// nombre d'objets du sous-problème
	int omega1;				// capacité du sac à dos
	int omega2;
	int *profits1;			// coefficients des variables dans la fonction objectif
	int *profits2;
	int *weights1;			// poids des variables dans la fonction objectif
	int *weights2;
	int *indVar;
	int lambda1;
	int lambda2;
};

struct Solution {
	int obj1;
	int obj2;
	int w1;
	int w2;
	bool *var;						// vrai si l'objet i est dans la solution, faux sinon
};

Probleme *genererProbleme(char *nomFichier);
Probleme *genererProblemeGautier(char *nomFichier);
Solution *creerSolution(Probleme *p, Chemin *chemin);
void fixer01(Probleme *p, int y1, int y2);

#endif