#include "graphe.h"
#include "probleme.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

Noeud ***genererGraphe(Probleme *p, unsigned int **nSol) {
	Noeud ***noeuds;
	Noeud *nouveau, *noeud, *noeudPrec;

	nouveau = malloc(sizeof(Noeud));
	nouveau->val = 0;
	nouveau->poids1 = 0;
	nouveau->poids2 = 0;
	nouveau->precBest = NULL;
	nouveau->precAlt = NULL;
	nouveau->existeAlt = false;
	nouveau->ajoutForce = false;

	unsigned int nbPrec;	// le nombre de noeuds de la dernière colonne construite
	unsigned int nb = 1;	// le nombre de noeuds de la colonne en construction

	noeuds = malloc((p->n+1)*sizeof(Noeud **));
	noeuds[0] = malloc(sizeof(Noeud *));
	noeuds[0][0] = nouveau;

	*nSol = malloc((p->n+1)*sizeof(unsigned int));
	(*nSol)[0] = 1;
	(*nSol)[1] = 2;
	(*nSol)[2] = 3;
	(*nSol)[3] = 4;
	(*nSol)[4] = 5;

	for (int i = 1; i <= p->n; ++i) {
		nbPrec = nb;
		nb = 0;
		noeuds[i] = malloc(2*nbPrec*sizeof(Noeud *));
		for (int j = 0; j < nbPrec; ++j) {
			noeudPrec = noeuds[i-1][j];
			unsigned int k = 0;





			/*unsigned int futurPoids1 = noeudPrec->poids1 + p->poidsCumules1[i-1];
			unsigned int futurPoids2 = noeudPrec->poids2 + p->poidsCumules2[i-1];
			bool potentiel = ((futurPoids1 >= p->capacite1) || (futurPoids2 >= p->capacite2));
			int l;
			if (!potentiel) {
				Noeud *noeudAjoutable = noeudPrec;
				l = i-1;
				while ((l > 0) && ((noeudAjoutable->precBest->poids1 != noeudAjoutable->poids1) || (futurPoids1 + p->poids1[l-1] > p->capacite1) || (futurPoids2 + p->poids2[l-1] > p->capacite2))) {
					--l;
					noeudAjoutable = noeudAjoutable->precBest;
				}
				potentiel = (l == 0);
			}
			if (potentiel) {*/





				if ((noeudPrec->poids1 + p->poids1[i-1] <= p->capacite1) && (noeudPrec->poids2 + p->poids2[i-1] <= p->capacite2)) {
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
						nouveau->existeAlt = noeudPrec->existeAlt;
						nouveau->ajoutForce = noeudPrec->ajoutForce;
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
						noeud->existeAlt = true;
						noeud->ajoutForce = noeudPrec->ajoutForce;
					}
				}
			//}
			k = 0;


			bool potentiel = true;
			if (!noeudPrec->ajoutForce) {
				unsigned int futurPoids1 = noeudPrec->poids1 + p->poidsCumules1[i];
				unsigned int futurPoids2 = noeudPrec->poids2 + p->poidsCumules2[i];
				potentiel = ((futurPoids1 >= p->capacite1) || (futurPoids2 >= p->capacite2));
				if (!potentiel) {
					Noeud *noeudAjoutable = noeudPrec;
					int l = i-1;
					while ((l > 0) && ((noeudAjoutable->precBest->poids1 != noeudAjoutable->poids1) || (futurPoids1 + p->poids1[l-1] > p->capacite1) || (futurPoids2 + p->poids2[l-1] > p->capacite2))) {
						--l;
						noeudAjoutable = noeudAjoutable->precBest;
					}
					potentiel = (l == 0);
				}
			}
			if (potentiel) {





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
					nouveau->existeAlt = noeudPrec->existeAlt;
					nouveau->ajoutForce = noeudPrec->ajoutForce;
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

Solution **initialiserSolutions(Noeud **noeuds, unsigned int n) {
	Solution **sols = malloc(n*sizeof(Solution *));
	for (int i = 0; i < n; ++i) {
		sols[i] = malloc(sizeof(Solution));
		sols[i]->solution = noeuds[i];
		sols[i]->val = noeuds[i]->val;
		sols[i]->deviation = 0;
		sols[i]->existeAlt = noeuds[i]->existeAlt;
		sols[i]->nDeviation = 0;
	}
	return sols;
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

void afficherSolution(Solution *sol, int n) {
	Noeud *noeud;
	printf("[%d]: ", sol->val);

	if (sol->deviation) {
		int nDev = sol->nDeviation;
		unsigned int *deviations = malloc(nDev*sizeof(unsigned int));
		for (int i = 0; i < nDev; ++i) {
			deviations[i] = sol->deviation;
			sol = (Solution*) sol->solution;
		}

		noeud = (Noeud*) sol->solution;
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
		noeud = (Noeud*) sol->solution;
	}

	for ( ; n > 0; --n) {
		printf("(%d,%d,%d)<-", n, noeud->poids1, noeud->poids2);
		noeud = noeud->precBest;
	}
	printf("(%d,%d,%d)", n, noeud->poids1, noeud->poids2);
	
	printf("\n");
}