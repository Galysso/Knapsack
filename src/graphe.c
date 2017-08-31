#include "probleme.h"
#include "graphe.h"
#include "2DKPSurrogate/2DKPSurrogate.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

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

void checkGenerationNoeuds(Noeud *noeudPrec, Probleme *p, int i, bool *bAjout, bool *bSansAjout, ListeSol *lSolHeur, int y1, int y2) {
	int indI = p->indVar[i-1];
	*bAjout = (noeudPrec->w1 + p->weights1[indI] <= p->omega1) && (noeudPrec->w2 + p->weights2[indI] <= p->omega2);
	*bSansAjout = (noeudPrec->val + p->lambda1*p->pCumul1[i] + p->lambda2*p->pCumul2[i] >= p->LB);

	if (*bAjout || *bSansAjout) {
		int profit;
		int lambda1 = p->lambda1;
		int lambda2 = p->lambda2;
		int LB = p->LB;
		bool toDelete = false;
		Solution *sol;

		solution *s1, *s2;
		int ret;
		donnees d;
		d.maxZ1 = 0;

		// On teste les cas triviaux pour lesquels dual-surrogate plante
		bool toutEntre0 = ((noeudPrec->w1 + p->wCumul1[i] <= p->omega1) && (noeudPrec->w2 + p->wCumul2[i] <= p->omega2));
		bool toutEntre1 = ((noeudPrec->w1 + p->wCumul1[i-1] + p->weights1[indI] <= p->omega1) && (noeudPrec->w2 + p->wCumul2[i-1] + p->w2min + p->weights2[indI] <= p->omega2));
		bool unObjetEntre0 = toutEntre0;
		bool unObjetEntre1 = toutEntre1;
		int k = i;
		while (((!unObjetEntre0) || (!unObjetEntre1)) && (k < p->nBis)) {
			int indK = p->indVar[k];
			if ((noeudPrec->w1 + p->weights1[indK] <= p->omega1) && (noeudPrec->w2 + p->weights2[indK]<= p->omega2)) {
				unObjetEntre0 = true;
			}
			if ((noeudPrec->w1 + p->weights1[indI] + p->weights1[indK] <= p->omega1) && (noeudPrec->w2 + p->weights2[indI] + p->weights2[indK] <= p->omega2)) {
				unObjetEntre1 = true;
			}
			++k;
		}

		// vrai si on peut éxecuter surrogate en fixant l'objet indI à 0
		bool execSurr0 = unObjetEntre0 && !toutEntre0;
		// vrai si on peut exécuter surrogate en fixant l'objet indI à 1
		bool execSurr1 = unObjetEntre1 && !toutEntre1;

		// -------------------------------------- ON COMPARE AVEC LB (z*lambda)
		if ((*bAjout && execSurr1) || (*bSansAjout && execSurr0)) {
			toDelete = true;
			d.nbItem = p->nBis - i;
			d.p1 = (itype *) malloc ((p->n) * sizeof(itype));
			d.w1 = (itype *) malloc ((p->n) * sizeof(itype));
			d.w2 = (itype *) malloc ((p->n) * sizeof(itype));
			for (int j = i; j < p->nBis; ++j) {
				d.w1[j-i] = p->weights1[p->indVar[j]];
				d.w2[j-i] = p->weights2[p->indVar[j]];
				d.p1[j-i] = lambda1*p->profits1[p->indVar[j]] + lambda2*p->profits2[p->indVar[j]];
			}
		}

		// -------------------------------------- ON FIXE LA VARIABLE INDI A 1
		if (*bAjout) {
			if (execSurr1) {
				//printf("OUI\n");
				d.omega1 = p->omega1 - noeudPrec->w1 - p->weights1[indI];
				d.omega2 = p->omega2 - noeudPrec->w2 - p->weights2[indI];

				ret = initDichoMu(&s1,&s2,&d);
				if (ret == 0) {
					startDichoMu(&s1,&s2,&d);
				}
				if (s1 != NULL) {
					if (s1->z1 < s2->z1) {
						free(s2->tab);
						free(s2);
						s2 = s1;
					} else {
						free(s1->tab);
						free(s1);
					}
				} else {
					sol = creerSolutionSurrGraphe(p, s2, noeudPrec, i, true);
					if (!((sol->p1 > y1) && (sol->p2 > y2) && (ajouterSolutionDomTri(lSolHeur, sol)))) {
						free(sol->var);
						free(sol);
					} else {
						//printf("(%d,%d)\n", sol->p1, sol->p2);
						int newLB = meilleureBorne(lSolHeur, p);
						if (newLB > LB) {
							LB = newLB;
							p->LB = LB;
						}
					}
				}
				profit = s2->z1 + noeudPrec->val + lambda1*p->profits1[indI] + lambda2*p->profits2[indI];
				free(s2->tab);
				free(s2);
			} else {
				if (!unObjetEntre1) {
					profit = noeudPrec->val + lambda1*p->profits1[indI] + lambda2*p->profits2[indI];
				} else if (toutEntre1) {
					profit = noeudPrec->val + lambda1*p->pCumul1[i-1] + lambda2*p->pCumul2[i-1];
				}
			}

			*bAjout = (profit >= LB);
		}

		// -------------------------------------- ON FIXE LA VARIABLE INDI A 0

		if (*bSansAjout) {
			if (execSurr0) {
				d.omega1 = p->omega1 - noeudPrec->w1;
				d.omega2 = p->omega2 - noeudPrec->w2;

				ret = initDichoMu(&s1,&s2,&d);
				if (ret == 0) {
					startDichoMu(&s1,&s2,&d);
				}
				if (s1 != NULL) {
					if (s1->z1 < s2->z1) {
						free(s2->tab);
						free(s2);
						s2 = s1;
					} else {
						free(s1->tab);
						free(s1);
					}
				} else {
					sol = creerSolutionSurrGraphe(p, s2, noeudPrec, i, false);
					if (!((sol->p1 > y1) && (sol->p2 > y2) && (ajouterSolutionDomTri(lSolHeur, sol)))) {
						free(sol->var);
						free(sol);
					} else {
						//printf("(%d,%d)\n", sol->p1, sol->p2);
						int newLB = meilleureBorne(lSolHeur, p);
						if (newLB > LB) {
							LB = newLB;
							p->LB = LB;
						}
					}
				}
				profit = s2->z1 + noeudPrec->val;
				free(s2->tab);
				free(s2);
			} else if (!unObjetEntre0) {
				profit = noeudPrec->val;
			} else if (toutEntre0) {
				profit = noeudPrec->val + lambda1*p->pCumul1[i] + lambda2*p->pCumul2[i];
			}
			*bSansAjout = (profit >= LB);
		}

		if (toDelete) {
			free(d.p1);
			free(d.w1);
			free(d.w2);
		}
	}
}

/*void checkGenerationNoeuds(Noeud *noeudPrec, Probleme *p, int i, bool *bAjout, bool *bSansAjout) {
	int indI = p->indVar[i-1];
	int LB = p->LB;
	int profit;
	*bAjout = (noeudPrec->w1 + p->weights1[indI] <= p->omega1) && (noeudPrec->w2 + p->weights2[indI] <= p->omega2);
	*bSansAjout = (noeudPrec->val + p->lambda1*p->pCumul1[i] + p->lambda2*p->pCumul2[i] >= p->LB);

	donnees *d;
	solution *s1, *s2;
	int ret;

	bool toutEntre0 = ((noeudPrec->w1 + p->wCumul1[i] <= p->omega1) && (noeudPrec->w2 + p->wCumul2[i] <= p->omega2));
	bool toutEntre1 = ((noeudPrec->w1 + p->wCumul1[i-1] + p->weights1[indI] <= p->omega1) && (noeudPrec->w2 + p->wCumul2[i-1] + p->w2min + p->weights2[indI] <= p->omega2));
	bool unObjetEntre0 = toutEntre0;
	bool unObjetEntre1 = toutEntre1;
	int k = i;
	if (*bAjout || *bSansAjout) {
		while (((!unObjetEntre0) || (!unObjetEntre1)) && (k < p->nBis)) {
			int indK = p->indVar[k];
			if ((noeudPrec->w1 + p->weights1[indK] <= p->omega1) && (noeudPrec->w2 + p->weights2[indK]<= p->omega2)) {
				unObjetEntre0 = true;
			}
			if ((noeudPrec->w1 + p->weights1[indI] + p->weights1[indK] <= p->omega1) && (noeudPrec->w2 + p->weights2[indI] + p->weights2[indK]<= p->omega2)) {
				unObjetEntre1 = true;
			}
			++k;
		}
	}
	
	
	if ((*bAjout && unObjetEntre1 && !toutEntre1) || (*bSansAjout && unObjetEntre0 && !toutEntre0)) {
		d = (donnees *) malloc(sizeof(donnees));
		d->nbItem = p->nBis-i;
		d->p1 = (itype *) malloc ((d->nbItem) * sizeof(itype));
		d->w1 = (itype *) malloc ((d->nbItem) * sizeof(itype));
		d->w2 = (itype *) malloc ((d->nbItem) * sizeof(itype));
		d->maxZ1 = 0;

		for (int j = i; j < p->nBis; ++j) {
			int indJ = p->indVar[j];
			d->p1[j-i] = p->lambda1*p->profits1[indJ] + p->lambda2*p->profits2[indJ];
			d->w1[j-i] = p->weights1[indJ];
			d->w2[j-i] = p->weights2[indJ];
		}

		if (*bAjout && unObjetEntre1 && !toutEntre1) {
			// On fixe la variable à 1
			d->omega1 = p->omega1 - noeudPrec->w1 - p->weights1[indI];
			d->omega2 = p->omega2 - noeudPrec->w2 - p->weights2[indI];

			ret = initDichoMu(&s1,&s2,d);
			if (ret == 0) {
				startDichoMu(&s1,&s2,d);
			}
			if (s1 != NULL) {
				if (s1->z1 < s2->z1) {
					free(s2->tab);
					free(s2);
					s2 = s1;
				} else {
					free(s1->tab);
					free(s1);
				}
			}
			profit = s2->z1;
			free(s2->tab);
			free(s2);

			*bAjout = (profit + noeudPrec->val + p->lambda1*(p->profits1[indI]) + p->lambda2*(p->profits2[indI]) >= LB);
		}

		if (*bSansAjout && unObjetEntre0 && !toutEntre0) {
			d->omega1 = p->omega1 - noeudPrec->w1;
			d->omega2 = p->omega2 - noeudPrec->w2;

			ret = initDichoMu(&s1,&s2,d);
			if (ret == 0) {
				startDichoMu(&s1,&s2,d);
			}
			if (s1 != NULL) {
				if (s1->z1 < s2->z1) {
					free(s2->tab);
					free(s2);
					s2 = s1;
				} else {
					free(s1->tab);
					free(s1);
				}
			}
			profit = s2->z1;
			free(s2->tab);
			free(s2);

			*bSansAjout = (profit + noeudPrec->val >= LB);
		}
		free(d->p1);
		free(d->w1);
		free(d->w2);
	}

	if (*bAjout) {
		if (!unObjetEntre1) {
			*bAjout = noeudPrec->val + p->lambda1*p->profits1[indI] + p->lambda2*p->profits2[indI] >= LB;
		} else if (toutEntre1) {
			*bAjout = noeudPrec->val + p->lambda1*p->pCumul1[i-1] + p->lambda2*p->pCumul2[i-1] >= LB;
		}
	}

	if (*bSansAjout) {
		if (!unObjetEntre0) {
			*bSansAjout = noeudPrec->val >= LB;
		} else if (toutEntre0) {
			*bSansAjout = noeudPrec->val + p->lambda1*p->pCumul1[i] + p->lambda2*p->pCumul2[i] >= LB;
		}
	}
}*/

Noeud ***genererGraphe(Probleme *p, int **nSol, Solution *sol1, Solution *sol2, ListeSol *lSolHeur) {
	Noeud ***noeuds;
	Noeud *nouveau, *noeud, *noeudPrec;

	bool bAjout, bSansAjout;

	int lambda1 = p->lambda1;
	int lambda2 = p->lambda2;
	int LB = p->LB;//lambda1*(sol1->p1+1) + lambda2*(sol2->p2+1);

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
			int k;

			checkGenerationNoeuds(noeudPrec, p, i, &bAjout, &bSansAjout, lSolHeur, sol1->p1, sol2->p2);
			LB = p->LB;

			// Si l'objet entre dans le sac alors on cherche à ajouter le noeud correspondant
			if (bAjout) {
				int futurP1 = noeudPrec->w1 + p->weights1[indI];
				int futurP2 = noeudPrec->w2 + p->weights2[indI];

				// On cherche si le noeud à ajouter existe déjà
				k = 0;
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
			if (bSansAjout) {
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
		printf("(%d) nb=%d\n", i, nb);
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
		free(deviations);
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