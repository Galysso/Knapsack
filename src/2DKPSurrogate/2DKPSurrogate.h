#ifndef DKPSURROGATE_H
#define DKPSURROGATE_H

#include "combo.h"

typedef struct
{
	int nbItem; // nombre d'objets dans le sac
	itype *p1; // coefficients premier objectif
	//itype *p2; // coefficients deuxième objectif
	itype *w1; // coefficients première dimension
	itype *w2; // coefficients seconde dimension
	int omega1; // capacité première dimension
	int omega2; // capacité seconde dimension
	int maxZ1; // pour combo
	//int maxZ2; // pour combo
} donnees;

typedef struct
{
	int *tab; // vecteur de valeurs (booléennes) des solutions (à remanier plus tard si nécessaire)
	int z1; // valeur obj1
	//int z2; // valeur obj2
	//long long val; // valeur objectif pondéré // Inutile donc supprimé // Vérifier partout dans le code
	short int c1; // booléen indiquant si la contrainte 1 est satisfaite (1) ou non (0)
	short int c2; // booléen indiquant si la contrainte 2 est satisfaite (1) ou non (0)
	long long mult1; 
	long long mult2; // multiplicateur utilisé pour trouver la solution
	long long num; // numérateur du multiplicateur critique (si une seule contrainte est satisfaite)
	long long den; // dénominateur du multiplicateur critique (si une seule contrainte est satisfaite)
	//long long lambda1;
	//long long lambda2; // (lambda1,lambda2) critique (i.e. minimum) pour l'obtention de cette solution à mu constant
} solution;

int initDichoMu(solution **s1, solution **s2, donnees *d);
void startDichoMu(solution **s1, solution **s2, donnees *d);

#endif