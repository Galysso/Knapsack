#include "graphe.h"
#include "probleme.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Noeud ***genererGraphe(Probleme *p, unsigned int *n) {
	Noeud ***noeuds;
	Noeud *nouveau, *noeud, *noeudPrec;

	nouveau = malloc(sizeof(Noeud));
	nouveau->val = 0;
	nouveau->poids1 = 0;
	nouveau->poids2 = 0;
	nouveau->precBest = NULL;
	nouveau->precAlt = NULL;

	unsigned int nbPrec;	// le nombre de noeuds de la dernière colonne construite
	unsigned int nb = 1;	// le nombre de noeuds de la colonne en construction

	noeuds = malloc((p->n+1)*sizeof(Noeud **));
	noeuds[0] = malloc(sizeof(Noeud *));
	noeuds[0][0] = nouveau;

	for (int i = 1; i <= p->n; ++i) {
		nbPrec = nb;
		nb = 0;
		noeuds[i] = malloc(2*nbPrec*sizeof(Noeud *));
		for (int j = 0; j < nbPrec; ++j) {
			noeudPrec = noeuds[i-1][j];
			int k = 0;
			while ((k < nb) && ((noeuds[i][k]->poids1 != noeudPrec->poids1) || (noeuds[i][k]->poids2 != noeudPrec->poids2))) {
				++k;
			}
			if (k == nb) {
				nouveau = malloc(sizeof(Noeud));
				nouveau->val = noeudPrec->val;
				nouveau->poids1 = noeudPrec->poids1;
				nouveau->poids2 = noeudPrec->poids2;
				nouveau->precBest = noeudPrec;
				nouveau->precAlt = NULL;
				noeuds[i][nb] = nouveau;
				++nb;
			} else {
				noeud = noeuds[i][k];
				if (noeud->val < noeudPrec->val) {
					noeud->val = noeudPrec->val;
					noeud->precAlt = noeuds[i][k]->precBest;
					noeud->precBest = noeudPrec;
				} else {
					noeud->precAlt = noeudPrec;
				}
			}

			if ((noeudPrec->poids1 + p->poids1[i-1] <= p->capacite1) && (noeudPrec->poids2 +p->poids2[i-1] <= p->capacite2)) {
				k = 0;
				while ((k < nb) && ((noeuds[i][k]->poids1 != noeudPrec->poids1 + p->poids1[i-1]) || (noeuds[i][k]->poids2 != noeudPrec->poids2 + p->poids2[i-1]))) {
					++k;
				}
				if (k == nb) {
					nouveau = malloc(sizeof(Noeud));
					nouveau->val = noeudPrec->val + p->coefficients1[i-1];
					nouveau->poids1 = noeudPrec->poids1 + p->poids1[i-1];
					nouveau->poids2 = noeudPrec->poids2 + p->poids2[i-1];
					nouveau->precBest = noeudPrec;
					nouveau->precAlt = NULL;
					noeuds[i][nb] = nouveau;
					++nb;
				} else {
					noeud = noeuds[i][k];
					if (noeud->val < noeudPrec->val + p->coefficients1[i-1]) {
						noeud->val = noeudPrec->val + p->coefficients1[i-1];
						noeud->precAlt = noeuds[i][k]->precBest;
						noeud->precBest = noeudPrec;
					} else {
						noeud->precAlt = noeudPrec;
					}
				}
			}
		}
		noeuds[i] = realloc(noeuds[i], nb*sizeof(Noeud));
		printf("n = %d\n", nb);
	}

	*n = nb;
	return noeuds;
}

void afficherGraphe(Noeud *node, unsigned int n) {
	printf("(%d,%d,%d) :\n", n, node->poids1, node->poids2);
	printf("val = %d\n", node->val);
	if (node->precBest != NULL) {
		printf("precBest = (%d,%d,%d)\n", n-1, node->precBest->poids1, node->precBest->poids2);
		if (node->precAlt != NULL) {
			printf("precAlt = (%d,%d,%d)\n", n-1, node->precAlt->poids1, node->precAlt->poids2);
		}
	}
}