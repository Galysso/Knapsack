#ifndef PROBLEME_H
#define PROBLEME_H

typedef struct Probleme Probleme;

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
};

Probleme *genererProbleme(char *nomFichier);

#endif