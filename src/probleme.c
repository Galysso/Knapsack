#include "probleme.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

Probleme *genererProbleme(char *nomFichier) {
	Probleme *p = malloc(sizeof(Probleme));

	FILE *fichier = fopen(nomFichier, "r");

	if (fichier) {
		unsigned int nbVariable;
		unsigned int capacite1;
		unsigned int capacite2;
		unsigned int *coef1;
		unsigned int *coef2;
		unsigned int *poids1;
		unsigned int *poids2;

		assert(fscanf(fichier, "%d", &nbVariable));
		assert(fscanf(fichier, "%d", &capacite1));
		assert(fscanf(fichier, "%d", &capacite2));

		coef1 = malloc(nbVariable*sizeof(unsigned int));
		coef2 = malloc(nbVariable*sizeof(unsigned int));
		poids1 = malloc(nbVariable*sizeof(unsigned int));
		poids2 = malloc(nbVariable*sizeof(unsigned int));

		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &coef1[i]));
			assert(fscanf(fichier, "%d", &poids1[i]));
			assert(fscanf(fichier, "%d", &coef2[i]));
			assert(fscanf(fichier, "%d", &poids2[i]));
		}

		p->n = nbVariable;
		p->capacite1 = capacite1;
		p->capacite2 = capacite2;
		p->coefficients1 = coef1;
		p->coefficients2 = coef2;
		p->poids1 = poids1;
		p->poids2 = poids2;
	} else {
		printf("Erreur lors de l'ouverture du fichier %s\n", nomFichier);
	}

	return p;
}