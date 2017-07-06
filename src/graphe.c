#include "probleme.h"
#include "graphe.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

Noeud ***genererGraphe(Probleme *p, unsigned int **nSol, Solution *sol1, Solution *sol2) {
	Noeud ***noeuds;
	Noeud *nouveau, *noeud, *noeudPrec;

	unsigned int lambda1 = p->lambda1;
	unsigned int lambda2 = p->lambda2;
	unsigned int LB = lambda1*(sol1->obj1+1) + lambda2*(sol2->obj2+1);

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

	// Pour chaque objet du sac
	for (int i = 1; i <= p->n; ++i) {
		nbPrec = nb;
		nb = 0;
		// On alloue deux fois le nombre de noeuds obtenus précédemment
		noeuds[i] = (Noeud **) malloc(2*nbPrec*sizeof(Noeud *));
		for (int j = 0; j < nbPrec; ++j) {
			noeudPrec = noeuds[i-1][j];
			unsigned int k = 0;
			// Si l'objet entre dans le sac
			if ((noeudPrec->poids1 + p->poids1[i-1] <= p->capacite1) && (noeudPrec->poids2 + p->poids2[i-1] <= p->capacite2)) {
				unsigned int futurP1 = noeudPrec->poids1 + p->poids1[i-1];
				unsigned int futurP2 = noeudPrec->poids2 + p->poids2[i-1];
				// On regarde si le noeud à ajouter existe déjà
				while ((k < nb) && ((noeuds[i][k]->poids1 != futurP1) || (noeuds[i][k]->poids2 != futurP2))) {
					++k;
				}
				// S'il n'existe pas on le crée
				if (k == nb) {
					nouveau = (Noeud *) malloc(sizeof(Noeud));
					nouveau->obj1 = noeudPrec->obj1 + p->coefficients1[i-1];
					nouveau->obj2 = noeudPrec->obj2 + p->coefficients2[i-1];
					nouveau->val = noeudPrec->val + lambda1*p->coefficients1[i-1] + lambda2*p->coefficients2[i-1];
					nouveau->poids1 = futurP1;
					nouveau->poids2 = futurP2;
					nouveau->precBest = noeudPrec;
					nouveau->precAlt = NULL;
					nouveau->existeAlt = noeudPrec->existeAlt;
					nouveau->ajoutForce = noeudPrec->ajoutForce;
					noeuds[i][nb] = nouveau;
					++nb;
				} else {
					noeud = noeuds[i][k];
					// S'il existe et que le noeud précédent est meilleur on le modifie
					if (noeud->val < noeudPrec->val + lambda1*p->coefficients1[i-1] + lambda2*p->coefficients2[i-1]) {
						noeud->val = noeudPrec->val + lambda1*p->coefficients1[i-1] + lambda2*p->coefficients2[i-1];
						noeud->obj1 = noeudPrec->obj1 + p->coefficients1[i-1];
						noeud->obj2 = noeudPrec->obj2 + p->coefficients2[i-1];
						noeud->precAlt = noeuds[i][k]->precBest;
						noeud->precBest = noeudPrec;
					// Sinon on ajoute le noeud précédent en alternatif
					} else {
						noeud->precAlt = noeudPrec;
					}
					noeud->existeAlt = true;
					noeud->ajoutForce = noeudPrec->ajoutForce;
				}
			}
			// Si sans l'ajout de l'objet il est possible d'atteindre la borne
			// Alors on crée le noeud
			if ((noeudPrec->val + lambda1*(p->coefCumules1[i]+1) + lambda2*p->coefCumules2[i]+1) >= LB) {
				k = 0;
				// On regarde si le noeud à ajouter existe déjà
				while ((k < nb) && ((noeuds[i][k]->poids1 != noeudPrec->poids1) || (noeuds[i][k]->poids2 != noeudPrec->poids2))) {
					++k;
				}
				// S'il n'existe pas on le crée
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
					// S'il existe et que le noeud précédent est meilleur on le modifie
					if (noeud->val < noeudPrec->val) {
						noeud->val = noeudPrec->val;
						noeud->obj1 = noeudPrec->obj1;
						noeud->obj2 = noeudPrec->obj2;
						noeud->precAlt = noeuds[i][k]->precBest;
						noeud->precBest = noeudPrec;
					// Sinon on ajoute le noeud précédent en alternatif
					} else {
						noeud->precAlt = noeudPrec;
					}
					noeud->existeAlt = true;
					noeud->ajoutForce = noeudPrec->ajoutForce;
				}
			}
		}
		// On réalloue la colonne en fonction du nombre exact de noeuds créés
		noeuds[i] = realloc(noeuds[i], nb*sizeof(Noeud));
		(*nSol)[i] = nb;
		//printf("nb=%d\n",nb);
	}

	return noeuds;
}

// Créé les chemins initiaux non modifiés
Chemin **initialiserChemins(Noeud **noeuds, unsigned int n) {
	Chemin **chemins = (Chemin **) malloc(n*sizeof(Chemin *));
	for (int i = 0; i < n; ++i) {
		Chemin *chem = (Chemin *) malloc(sizeof(Chemin));
		chemins[i] = chem;
		chem->chemin = noeuds[i];
		chem->val = noeuds[i]->val;
		chem->deviation = 0;
		chem->existeAlt = noeuds[i]->existeAlt;
		chem->nDeviation = 0;
		chem->obj1 = noeuds[i]->obj1;
		chem->obj2 = noeuds[i]->obj2;
	}
	return chemins;
}

// Libère la mémoire allouée aux noeuds (après traitement d'un triangle)
void desallouerGraphe(unsigned int *nSol, Noeud ***noeuds, unsigned int n) {
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < nSol[i]; ++j) {
			free(noeuds[i][j]);
		}
		free(noeuds[i]);
	}
	free(noeuds);
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