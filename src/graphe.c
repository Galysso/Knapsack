#include "probleme.h"
#include "graphe.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

Noeud ***genererGraphe(Probleme *p, unsigned int **nSol, Solution *sol1, Solution *sol2) {
	Noeud ***noeuds;
	Noeud *nouveau, *noeud, *noeudPrec;

	unsigned int lambda1 = sol1->obj2 - sol2->obj2;
	unsigned int lambda2 = sol2->obj1 - sol1->obj1;

	nouveau = (Noeud *) malloc(sizeof(Noeud));
	nouveau->val = 0;
	nouveau->obj1 = 0;
	nouveau->obj2 = 0;
	nouveau->poids1 = 0;
	nouveau->poids2 = 0;
	nouveau->precBest = NULL;
	nouveau->precAlt = NULL;
	nouveau->existeAlt = false;
	nouveau->ajoutForce = false;

	unsigned int nbPrec;	// le nombre de noeuds de la dernière colonne construite
	unsigned int nb = 1;	// le nombre de noeuds de la colonne en construction

	noeuds = (Noeud ***) malloc((p->n+1)*sizeof(Noeud **));
	noeuds[0] = (Noeud **) malloc(sizeof(Noeud *));
	noeuds[0][0] = nouveau;

	*nSol = malloc((p->n+1)*sizeof(unsigned int));
	(*nSol)[0] = 1;

	for (int i = 1; i <= p->n; ++i) {
		nbPrec = nb;
		nb = 0;
		noeuds[i] = (Noeud **) malloc(2*nbPrec*sizeof(Noeud *));
		for (int j = 0; j < nbPrec; ++j) {
			noeudPrec = noeuds[i-1][j];
			unsigned int k = 0;
			if ((noeudPrec->poids1 + p->poids1[i-1] <= p->capacite1) && (noeudPrec->poids2 + p->poids2[i-1] <= p->capacite2)/* && (noeudPrec->obj1 + p->coefCumules1[i-1] > sol1->obj1) && (noeudPrec->obj2 + p->coefCumules2[i-1] > sol2->obj2)*/) {
				while ((k < nb) && ((noeuds[i][k]->poids1 != noeudPrec->poids1 + p->poids1[i-1]) || (noeuds[i][k]->poids2 != noeudPrec->poids2 + p->poids2[i-1]))) {
					++k;
				}
				if (k == nb) {
					nouveau = (Noeud *) malloc(sizeof(Noeud));
					nouveau->obj1 = noeudPrec->obj1 + p->coefficients1[i-1];
					nouveau->obj2 = noeudPrec->obj2 + p->coefficients2[i-1];
					nouveau->val = noeudPrec->val + lambda1*p->coefficients1[i-1] + lambda2*p->coefficients2[i-1];
					nouveau->poids1 = noeudPrec->poids1 + p->poids1[i-1];
					nouveau->poids2 = noeudPrec->poids2 + p->poids2[i-1];
					nouveau->precBest = noeudPrec;
					nouveau->precAlt = NULL;
					nouveau->existeAlt = noeudPrec->existeAlt;
					nouveau->ajoutForce = noeudPrec->ajoutForce;
					noeuds[i][nb] = nouveau;
					++nb;
				} else {
					noeud = noeuds[i][k];
					if (noeud->val < noeudPrec->val + lambda1*p->coefficients1[i-1] + lambda2*p->coefficients2[i-1]) {
						noeud->val = noeudPrec->val + lambda1*p->coefficients1[i-1] + lambda2*p->coefficients2[i-1];
						nouveau->obj1 = noeudPrec->obj1 + p->coefficients1[i-1];
						nouveau->obj2 = noeudPrec->obj2 + p->coefficients2[i-1];
						noeud->precAlt = noeuds[i][k]->precBest;
						noeud->precBest = noeudPrec;
					} else {
						noeud->precAlt = noeudPrec;
					}
					noeud->existeAlt = true;
					noeud->ajoutForce = noeudPrec->ajoutForce;
				}
			}
			if ((noeudPrec->obj1 + p->coefCumules1[i] > sol1->obj1) && (noeudPrec->obj2 + p->coefCumules2[i] > sol2->obj2)) {
				k = 0;
				while ((k < nb) && ((noeuds[i][k]->poids1 != noeudPrec->poids1) || (noeuds[i][k]->poids2 != noeudPrec->poids2))) {
					++k;
				}
				if (k == nb) {
					nouveau = (Noeud *) malloc(sizeof(Noeud));
					nouveau->val = noeudPrec->val;
					nouveau->poids1 = noeudPrec->poids1;
					nouveau->poids2 = noeudPrec->poids2;
					nouveau->obj1 = noeudPrec->obj1;
					nouveau->obj2 = noeudPrec->obj2;
					nouveau->precBest = noeudPrec;
					nouveau->precAlt = NULL;
					nouveau->existeAlt = noeudPrec->existeAlt;
					nouveau->ajoutForce = noeudPrec->ajoutForce;
					noeuds[i][nb] = nouveau;
					++nb;
				} else {
					noeud = noeuds[i][k];
					if (noeud->val < noeudPrec->val) {
						noeud->val = noeudPrec->val;
						nouveau->obj1 = noeudPrec->obj1;
						nouveau->obj2 = noeudPrec->obj2;
						noeud->precAlt = noeuds[i][k]->precBest;
						noeud->precBest = noeudPrec;
					} else {
						noeud->precAlt = noeudPrec;
					}
					noeud->existeAlt = true;
					noeud->ajoutForce = noeudPrec->ajoutForce;
				}
			}
		}
		noeuds[i] = realloc(noeuds[i], nb*sizeof(Noeud));
		printf("%d : %d\n", i, nb);
		(*nSol)[i] = nb;
	}

	return noeuds;
}

Chemin **initialiserChemins(Noeud **noeuds, unsigned int n) {
	Chemin **chemins = (Chemin **) malloc(n*sizeof(Chemin *));
	for (int i = 0; i < n; ++i) {
		chemins[i] = (Chemin *) malloc(sizeof(Chemin));
		chemins[i]->chemin = noeuds[i];
		chemins[i]->val = noeuds[i]->val;
		chemins[i]->deviation = 0;
		chemins[i]->existeAlt = noeuds[i]->existeAlt;
		chemins[i]->nDeviation = 0;
	}
	return chemins;
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

void afficherChemin(Chemin *chem, int n) {
	Noeud *noeud;
	printf("[%d]: ", chem->val);

	if (chem->deviation) {
		int nDev = chem->nDeviation;
		unsigned int *deviations = (unsigned int *) malloc(nDev*sizeof(unsigned int));
		for (int i = 0; i < nDev; ++i) {
			deviations[i] = chem->deviation;
			chem = (Chemin*) chem->chemin;
		}

		noeud = (Noeud*) chem->chemin;
		--nDev;
		while (nDev >= 0) {
			while (n > deviations[nDev]) {
				printf("(%d,%d,%d)<-", n, noeud->poids1, noeud->poids2);
				noeud = noeud->precBest;
				--n;
			}
			printf("(%d,%d,%d)<-", n, noeud->poids1, noeud->poids2);
			noeud = noeud->precAlt;
			--n;
			--nDev;
		}
	} else {
		noeud = (Noeud*) chem->chemin;
	}

	for ( ; n > 0; --n) {
		printf("(%d,%d,%d)<-", n, noeud->poids1, noeud->poids2);
		noeud = noeud->precBest;
	}
	printf("(%d,%d,%d)", n, noeud->poids1, noeud->poids2);
	
	printf("\n");
}