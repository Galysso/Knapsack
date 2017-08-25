#include "probleme.h"
#include "graphe.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

Noeud *creerNoeudAvecAjout(Probleme *p, int i, Noeud *noeudPrec) {
	Noeud *nouveau = (Noeud *) malloc(sizeof(Noeud));
	nouveau->p1 = noeudPrec->p1 + p->profits1[i];
	nouveau->p2 = noeudPrec->p2 + p->profits2[i];
	nouveau->val = noeudPrec->val + p->lambda1*p->profits1[i] + p->lambda2*p->profits2[i];
	nouveau->w1 = noeudPrec->w1 + p->weights1[i];
	nouveau->w2 = noeudPrec->w2 + p->weights2[i];
	nouveau->precBest = noeudPrec;
	nouveau->precAlt = NULL;
	nouveau->existeAlt = noeudPrec->existeAlt;
	nouveau->ajoutForce = noeudPrec->ajoutForce;

	return nouveau;
}

Noeud *creerNoeudSansAjout(Noeud *noeudPrec) {
	Noeud *nouveau = (Noeud *) malloc(sizeof(Noeud));
	nouveau->p1 = noeudPrec->p1;
	nouveau->p2 = noeudPrec->p2;
	nouveau->val = noeudPrec->val;
	nouveau->w1 = noeudPrec->w1;
	nouveau->w2 = noeudPrec->w2;
	nouveau->precBest = noeudPrec;
	nouveau->precAlt = NULL;
	nouveau->existeAlt = noeudPrec->existeAlt;
	nouveau->ajoutForce = noeudPrec->ajoutForce;

	return nouveau;
}

void modifierNoeudAvecAjout(Probleme *p, int i, Noeud *noeudModif, Noeud *noeudPrec) {
	noeudModif->val = noeudPrec->val + p->lambda1*p->profits1[i] + p->lambda2*p->profits2[i];
	noeudModif->p1 = noeudPrec->p1 + p->profits1[i];
	noeudModif->p2 = noeudPrec->p2 + p->profits2[i];
	noeudModif->precAlt = noeudModif->precBest;
	noeudModif->precBest = noeudPrec;
}

void modifierNoeudSansAjout(Noeud *noeudModif, Noeud* noeudPrec) {
	noeudModif->val = noeudPrec->val;
	noeudModif->p1 = noeudPrec->p1;
	noeudModif->p2 = noeudPrec->p2;
	noeudModif->precAlt = noeudModif->precBest;
	noeudModif->precBest = noeudPrec;
}

Noeud ***genererGraphe(Probleme *p, int **nSol, Solution *sol1, Solution *sol2) {
	Noeud ***noeuds;
	Noeud *nouveau, *noeud, *noeudPrec;

	int lambda1 = p->lambda1;
	int lambda2 = p->lambda2;
	int LB = lambda1*(sol1->p1+1) + lambda2*(sol2->p2+1);
//printf("COCO\n");
	nouveau = (Noeud *) malloc(sizeof(Noeud));
	nouveau->val = lambda1*p->z1min + lambda2*p->z2min;
	nouveau->p1 = p->z1min;
	nouveau->p2 = p->z2min;
	nouveau->w1 = p->w1min;
	nouveau->w2 = p->w2min;
	nouveau->precBest = NULL;
	nouveau->precAlt = NULL;
	nouveau->existeAlt = false;
	nouveau->ajoutForce = false;

	int nbPrec;	// le nombre de noeuds de la dernière colonne construite
	int nb = 1;	// le nombre de noeuds de la colonne en construction

	noeuds = (Noeud ***) malloc((p->nBis+1)*sizeof(Noeud **));
	noeuds[0] = (Noeud **) malloc(sizeof(Noeud *));
	noeuds[0][0] = nouveau;

	*nSol = malloc((p->nBis+1)*sizeof(int));
	(*nSol)[0] = 1;

	// Pour chaque objet du sac
	for (int i = 1; i <= p->nBis; ++i) {
		int indI = p->indVar[i-1];
		nbPrec = nb;
		nb = 0;
		// On alloue deux fois le nombre de noeuds obtenus précédemment
		noeuds[i] = (Noeud **) malloc(2*nbPrec*sizeof(Noeud *));
		for (int j = 0; j < nbPrec; ++j) {
			noeudPrec = noeuds[i-1][j];
			int k = 0;
			// Si l'objet entre dans le sac alors on cherche à ajouter le noeud correspondant
			if ((noeudPrec->w1 + p->weights1[indI] <= p->omega1) && (noeudPrec->w2 + p->weights2[indI] <= p->omega2)) {
				int futurP1 = noeudPrec->w1 + p->weights1[indI];
				int futurP2 = noeudPrec->w2 + p->weights2[indI];
				// On cherche si le noeud à ajouter existe déjà
				while ((k < nb) && ((noeuds[i][k]->w1 != futurP1) || (noeuds[i][k]->w2 != futurP2))) {
					++k;
				}
				// S'il n'existe pas on le crée
				if (k == nb) {
					noeuds[i][nb] = creerNoeudAvecAjout(p, indI, noeudPrec);
					++nb;
				} else {
					noeud = noeuds[i][k];
					// S'il existe et que le noeud précédent est meilleur on le modifie
					if (noeud->val < noeudPrec->val + lambda1*p->profits1[indI] + lambda2*p->profits2[indI]) {
						modifierNoeudAvecAjout(p, indI, noeud, noeudPrec);
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
			if (noeudPrec->val + lambda1*p->pCumul1[i] + lambda2*p->pCumul2[i] >= LB) {		// Future condition ?
				k = 0;
				// On regarde si le noeud à ajouter existe déjà
				while ((k < nb) && ((noeuds[i][k]->w1 != noeudPrec->w1) || (noeuds[i][k]->w2 != noeudPrec->w2))) {
					++k;
				}
				// S'il n'existe pas on le crée
				if (k == nb) {
					noeuds[i][nb] = creerNoeudSansAjout(noeudPrec);
					++nb;
				} else {
					noeud = noeuds[i][k];
					// S'il existe et que le noeud précédent est meilleur on le modifie
					if (noeud->val < noeudPrec->val) {
						modifierNoeudSansAjout(noeud, noeudPrec);
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
Chemin **initialiserChemins(Noeud **noeuds, int n) {
	Chemin **chemins = (Chemin **) malloc(n*sizeof(Chemin *));
	for (int i = 0; i < n; ++i) {
		Chemin *chem = (Chemin *) malloc(sizeof(Chemin));
		chemins[i] = chem;
		chem->chemin = noeuds[i];
		chem->val = noeuds[i]->val;
		chem->deviation = 0;
		chem->existeAlt = noeuds[i]->existeAlt;
		chem->nDeviation = 0;
		chem->p1 = noeuds[i]->p1;
		chem->p2 = noeuds[i]->p2;
	}
	return chemins;
}

// Libère la mémoire allouée aux noeuds (après traitement d'un triangle)
void desallouerGraphe(int *nSol, Noeud ***noeuds, int n) {
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < nSol[i]; ++j) {
			free(noeuds[i][j]);
		}
		free(noeuds[i]);
	}
	free(noeuds);
}

void afficherGraphe(Noeud *node, int n) {
	printf("(%d,%d,%d) :\n", n, node->w1, node->w2);
	printf("val = %d\n", node->val);
	if (node->precBest != NULL) {
		printf("precBest = (%d,%d,%d)\n", n-1, node->precBest->w1, node->precBest->w2);
		if (node->precAlt != NULL) {
			printf("precAlt = (%d,%d,%d)\n", n-1, node->precAlt->w1, node->precAlt->w2);
		}
	}
}

void afficherChemin(Chemin *chem, int n) {
	Noeud *noeud;
	printf("[%d]: ", chem->val);

	if (chem->deviation) {
		int nDev = chem->nDeviation;
		int *deviations = (int *) malloc(nDev*sizeof(int));
		for (int i = 0; i < nDev; ++i) {
			deviations[i] = chem->deviation;
			chem = (Chemin*) chem->chemin;
		}

		noeud = (Noeud*) chem->chemin;
		--nDev;
		while (nDev >= 0) {
			while (n > deviations[nDev]) {
				printf("(%d,%d,%d)<-", n, noeud->w1, noeud->w2);
				noeud = noeud->precBest;
				--n;
			}
			printf("(%d,%d,%d)<-", n, noeud->w1, noeud->w2);
			noeud = noeud->precAlt;
			--n;
			--nDev;
		}
	} else {
		noeud = (Noeud*) chem->chemin;
	}

	for ( ; n > 0; --n) {
		printf("(%d,%d,%d)<-", n, noeud->w1, noeud->w2);
		noeud = noeud->precBest;
	}
	printf("(%d,%d,%d)", n, noeud->w1, noeud->w2);
	
	printf("\n");
}