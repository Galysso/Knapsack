#include "graphe.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

probleme *genererProblemeTest() {
	probleme *p = malloc(sizeof(probleme));

	p->n = 4;

	p->capacite = 12;

	p->coefficients = malloc(4*sizeof(unsigned int));

	p->coefficients[0] = 9;
	p->coefficients[1] = 8;
	p->coefficients[2] = 8;
	p->coefficients[3] = 10;

	p->poids = malloc(4*sizeof(unsigned int));

	p->poids[0] = 4;
	p->poids[1] = 4;
	p->poids[2] = 4;
	p->poids[3] = 6;

	return p;
}

noeud ***genererGraphe(probleme *p, unsigned int *n) {
	unsigned int capTot;

	noeud *racine = malloc(sizeof(noeud));
	racine->val = 0;
	racine->poids = 0;
	racine->precBest = NULL;
	racine->precAlt = NULL;

	noeud *nouveau;

	noeud ***noeuds;
	unsigned int nbPrec;	// le nombre de noeuds de la dernière colonne construite
	unsigned int nb = 1;	// le nombre de noeuds de la colonne en construction

	noeuds = malloc((p->n+1)*sizeof(noeud **));
	noeuds[0] = malloc(sizeof(noeud *));
	noeuds[0][0] = racine;

	for (int i = 1; i <= p->n; ++i) {
		nbPrec = nb;
		nb = 0;
		noeuds[i] = malloc(2*nbPrec*sizeof(noeud *));
		for (int j = 0; j < nbPrec; ++j) {
			int k = 0;
			while ((k < nb) && (noeuds[i][k]->poids != noeuds[i-1][j]->poids)) {
				++k;
			}
			if (k == nb) {
				nouveau = malloc(sizeof(noeud));
				nouveau->val = noeuds[i-1][j]->val;
				nouveau->poids = noeuds[i-1][j]->poids;
				nouveau->precBest = noeuds[i-1][j];
				nouveau->precAlt = NULL;
				noeuds[i][nb] = nouveau;
				++nb;
			} else if (noeuds[i][k]->val < noeuds[i-1][j]->val) {
				noeuds[i][k]->val = noeuds[i-1][j]->val;
				noeuds[i][k]->precAlt = noeuds[i][k]->precBest;
				noeuds[i][k]->precBest = noeuds[i-1][j];
			} else {
				noeuds[i][k]->precAlt = noeuds[i-1][j];
			}

			if (noeuds[i-1][j]->poids + p->poids[i-1] <= p->capacite) {
				k = 0;
				while ((k < nb) && (noeuds[i][k]->poids != noeuds[i-1][j] + p->poids[i-1])) {
					++k;
				}
				if (k == nb) {
					nouveau = malloc(sizeof(noeud));
					nouveau->val = noeuds[i-1][j]->val + p->coefficients[i-1];
					nouveau->poids = noeuds[i-1][j]->poids + p->poids[i-1];
					nouveau->precBest = noeuds[i-1][j];
					nouveau->precAlt = NULL;
					noeuds[i][nb] = nouveau;
					++nb;
				} else if (noeuds[i][k]->val < noeuds[i-1][j]->val + p->coefficients[i-1]) {
					noeuds[i][k]->val = noeuds[i-1][j]->val + p->coefficients[i-1];
					noeuds[i][k]->precAlt = noeuds[i][k]->precBest;
					noeuds[i][k]->precBest = noeuds[i-1][j];
				} else {
					noeuds[i][k]->precAlt = noeuds[i-1][j];
				}
			}
		}
		noeuds[i] = realloc(noeuds[i], nb*sizeof(noeud));
	}

	*n = nb;
	return noeuds;
}

void afficherGraphe(noeud *node, int n) {
	printf("(%d,%d) :\n", n, node->poids);
	printf("val = %d\n", node->val);
	if (node->precBest != NULL) {
		printf("precBest = (%d,%d)\n", n-1, node->precBest->poids);
		if (node->precAlt != NULL) {
			printf("precAlt = (%d,%d)\n", n-1, node->precAlt->poids);
		}
	}
}