#ifndef PROBLEME_H
#define PROBLEME_H

#include "graphe.h"

#include <stdbool.h>

typedef struct Chemin Chemin;

typedef struct Probleme Probleme;
typedef struct Solution Solution;
typedef struct ListeSol ListeSol;

struct Probleme {
	char *nomFichier;
	int n;					// nombre d'objets
	int nBis;				// nombre d'objets du sous-problème
	int omega1;				// capacités du sac à dos
	int omega2;
	int *profits1;			// coefficients des variables dans la fonction objectif
	int *profits2;
	int *weights1;			// poids des variables dans la fonction objectif
	int *weights2;
	int *indVar;

	int *varFix1;
	int nVarFix1;
	int z1min;
	int z2min;
	int w1min;
	int w2min;

	Solution *solSup1;
	Solution *solSup2;
	int lambda1;
	int lambda2;
	int LB;

	int sumW1;
	int sumW2;
	int sumP1;
	int sumP2;

	int *pCumul1;
	int *pCumul2;
	int *wCumul1;
	int *wCumul2;
};

struct Solution {
	int p1;
	int p2;
	int w1;
	int w2;
	bool *var;				// vrai si l'objet i est dans la solution, faux sinon
};

struct  ListeSol {
	Solution **solutions;
	int nbSol;
	int nbMax;
};

// Ajoute une solution à la liste des solutions efficaces
// Renvoie vrai si une solution n'est pas dominée par les solutions déjà trouvées
// Calcule la borne minimale actuelle
int meilleureBorne(ListeSol *lSolLB, Probleme *p);
bool estEfficace(ListeSol *lSol, Solution *sol);
void ajouterSolution(ListeSol *lSol, Solution *sol);
bool ajouterSolutionDom(ListeSol *lSol, Solution *sol);
bool estComplete(Solution *solution, Probleme *p);
void completerGlouton(Solution *sol, Probleme *p);
ListeSol *completions(Solution *sol, Probleme *p);
void fixerVar0(Probleme *p, int i);
void fixerVar1(Probleme *p, int i);
// Trie les valeurs de indVar dans l'ordre décroissant selon le profit des objets dans la somme pondérée
void trierIndvar(Probleme *p);		// TRIBULLE A PASSER EN TRI EN O(n*log(n))
void calculerProfitsCumules(Probleme *p);
void calculerPoidsCumules(Probleme *p);
Probleme *genererProbleme(char *nomFichier);
Probleme *genererProblemeGautier(char *nomFichier);
Solution *creerSolution(Probleme *p, Chemin *chemin);
void fixer01(Probleme *p, int y1, int y2, ListeSol *lSolHeur);
Solution *copierSolution(Solution *sol, int n);
ListeSol *initListeSol(int nbMax);

#endif