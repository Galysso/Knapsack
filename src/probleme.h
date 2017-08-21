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
	int LB;

	int *pCumul1;
	int *pCumul2;

	int *sortedInd;
};

struct Solution {
	int p1;
	int p2;
	int w1;
	int w2;
	bool *var;				// vrai si l'objet i est dans la solution, faux sinon
};

// Ajoute une solution à la liste des solutions efficaces
// Renvoie vrai si une solution n'est pas dominée par les solutions déjà trouvées
// Calcule la borne minimale actuelle
int meilleureBorne(Solution **solutionsLB, int nbSol, Probleme *p);
bool estEfficace(Solution **solutions, int fin, Solution *sol);
void ajouterSolution(Solution ***solutions, Solution *sol, int *nbSol, int *nbSolMax);
void ajouterSolutionDom(Solution ***solutions, Solution *sol, int *nbSol, int *nbSolMax);
bool estComplete(Solution *solution, Probleme *p);
// Tri les valeurs de indVar dans l'ordre décroissant selon le profit des objets dans la somme pondérée
void trierIndvar(Probleme *p);		// TRIBULLE A PASSER EN TRI EN O(n*log(n))
Probleme *genererProbleme(char *nomFichier);
Probleme *genererProblemeGautier(char *nomFichier);
Solution *creerSolution(Probleme *p, Chemin *chemin);
void fixer01(Probleme *p, int y1, int y2);
Solution *copierSolution(Solution *sol, int n);

#endif