#include "graphe.h"
#include "probleme.h"
#include "2DKPSurrogate/2DKPSurrogate.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

// Calcule la borne minimale actuelle
int meilleureBorne(ListeSol *lSolLB, Probleme *p) {
	int lambda1 = p->lambda1;
	int lambda2 = p->lambda2;
	int LB, LBprim;
	Solution *solG, *sol;

	Solution **solutions = lSolLB->solutions;
	int nbSol = lSolLB->nbSol;

	LB = lambda1*(solutions[0]->p1 + 1) + lambda2*(solutions[1]->p2 + 1);

	for (int i = 2; i < nbSol; ++i) {
		solG = solutions[i-1];
		sol = solutions[i];
		LBprim = lambda1*(solG->p1 + 1) + lambda2*(sol->p2 + 1);
		if (LBprim < LB) {
			LB = LBprim;
		}
	}

	return LB;
}

// Renvoie vrai si une solution n'est pas dominée par les solutions déjà trouvées
bool estEfficace(ListeSol *lSol, Solution *sol) {
	int i = 0;
	Solution **solutions = lSol->solutions;
	int nbSol = lSol->nbSol;
	while ((i < nbSol) && ((solutions[i]->p1 < sol->p1) || (solutions[i]->p2 < sol->p2))) {
		++i;
	}
	return (i == nbSol);
}

// Ajoute une solution à la fin de la liste des solutions
void ajouterSolution(ListeSol *lSol, Solution *sol) {
	if (lSol->nbSol == lSol->nbMax) {
		lSol->nbMax = 2*lSol->nbMax;
		lSol->solutions = (Solution **) realloc(lSol->solutions, lSol->nbMax*sizeof(Solution *));
	}
	lSol->solutions[lSol->nbSol] = sol;
	lSol->nbSol += 1;
}

bool ajouterSolutionDom(ListeSol *lSol, Solution *sol) {
	int j = 0;
	bool estDomine = false;
	Solution *solCourante;
	Solution **solutions = lSol->solutions;
	int nbSol = lSol->nbSol;
	int nbMax = lSol->nbMax;

	while ((j < nbSol) && (!estDomine)) {
		solCourante = solutions[j];		
		if ((sol->p1 > solCourante->p1) && (sol->p2 > solCourante->p2)) {
			--nbSol;
			free(solutions[j]);
			solutions[j] = solutions[nbSol];
		} else if ((sol->p1 <= solCourante->p1) && (sol->p2 <= solCourante->p2)) {
			estDomine = true;
		} else {
			++j;
		}
	}

	if (!estDomine) {
		if (nbSol == nbMax) {
			nbMax = nbMax*2;
			lSol->solutions = (Solution **) realloc(lSol->solutions, nbMax*sizeof(Solution *));
		}
		lSol->solutions[nbSol] = sol;
		++nbSol;
	}

	lSol->nbSol = nbSol;
	lSol->nbMax = nbMax;

	return (!estDomine);
}

bool estComplete(Solution *solution, Probleme *p) {
	bool complete = true;
	int n = p->n;
	int i = 0;
	int omega1 = p->omega1;
	int omega2 = p->omega2;
	int w1 = solution->w1;
	int w2 = solution->w2;

	do {
		complete = ((solution->var[i]) || (w1 + p->weights1[i] > omega1) || (w2 + p->weights2[i] > omega2));
		++i;
	} while (complete && (i < n));

	return complete;
}

void completerGlouton(Solution *sol, Probleme *p) {
	int n = p->n;
	int omega1 = p->omega1;
	int omega2 = p->omega2;
	int w1 = sol->w1;
	int w2 = sol->w2;
	int indV;

	for (int i = 0; i < n; ++i) {
		indV = p->indVar[i];
		if ((!sol->var[indV]) && (w1 + p->weights1[indV] <= omega1) && (w2 + p->weights2[indV] <= omega2)) {
			w1 += p->weights1[indV];
			w2 += p->weights2[indV];
			sol->p1 += p->profits1[indV];
			sol->p2 += p->profits2[indV];
			sol->var[indV] = true;
		}
		sol->w1 = w1;
		sol->w2 = w2;
	}
}

ListeSol *completions(Solution *sol, Probleme *p) {
	int n = p->n;
	int profondeur = 0;
	int i = 0;
	int *lastI = (int *) malloc(n*sizeof(int));

	ListeSol *lResultat = initListeSol(n);
	Solution *solC = copierSolution(sol, n);

	while (profondeur >= 0) {
		if (i == n) {
			Solution *solC2 = copierSolution(solC, n);
			ajouterSolutionDom(lResultat, solC2);
			--profondeur;
			if (profondeur >= 0) {
				i = lastI[profondeur];
				assert(solC->var[i]);
				solC->var[i] = false;
				solC->w1 -= p->weights1[i];
				solC->w2 -= p->weights2[i];
				solC->p1 -= p->profits1[i];
				solC->p2 -= p->profits2[i];
				++i;
			}
		} else if (!solC->var[i]) {
			if ((solC->w1 + p->weights1[i] <= p->omega1) && (solC->w2 + p->weights2[i] <= p->omega2)) {
				solC->var[i] = true;
				solC->w1 += p->weights1[i];
				solC->w2 += p->weights2[i];
				solC->p1 += p->profits1[i];
				solC->p2 += p->profits2[i];
				lastI[profondeur] = i;
				++profondeur;
			}
			++i;
		} else {
			++i;
		}
	}

	return lResultat;
}

void trierIndvar(Probleme *p) {
	bool changement;
	int n = p->n;
	int lambda1 = p->lambda1;
	int lambda2 = p->lambda2;
	int indJ1, indJ2;

	for (int i = 0; i < p->n; ++i) {
		p->indVar[i] = i;
	}

	do {
		changement = false;
		for (int i = 1; i < n; ++i) {
			indJ1 = p->indVar[i-1];
			indJ2 = p->indVar[i];
			if (lambda1*p->profits1[indJ1] + lambda2*p->profits2[indJ1] < lambda1*p->profits1[indJ2] + lambda2*p->profits2[indJ2]) {
				p->indVar[i-1] = indJ2;
				p->indVar[i] = indJ1;
				changement = true;
			}
		}
	} while(changement);
}

void calculerProfitsCumules(Probleme *p) {
	// calcul des profits cumules
	int indJ = p->indVar[p->nBis-1];
	p->pCumul1[p->nBis] = 0;
	p->pCumul2[p->nBis] = 0;
	for (int j = p->nBis-1; j >= 0; --j) {
		indJ = p->indVar[j];
		p->pCumul1[j] = p->pCumul1[j+1] + p->profits1[indJ];
		p->pCumul2[j] = p->pCumul2[j+1] + p->profits2[indJ];
	}
}

void calculerPoidsCumules(Probleme *p) {
	// calcul des profits cumules
	int indJ = p->indVar[p->nBis-1];
	p->wCumul1[p->nBis] = 0;
	p->wCumul2[p->nBis] = 0;
	for (int j = p->nBis-1; j >= 0; --j) {
		indJ = p->indVar[j];
		p->wCumul1[j] = p->wCumul1[j+1] + p->weights1[indJ];
		p->wCumul2[j] = p->wCumul2[j+1] + p->weights2[indJ];
	}
}

Probleme *genererProbleme(char *nomFichier) {
	Probleme *p = (Probleme *) malloc(sizeof(Probleme));

	FILE *fichier = fopen(nomFichier, "r");

	if (fichier) {
		p->nomFichier = nomFichier;

		int nbVariable;
		int omega1;
		int omega2;
		int *profits1;
		int *profits2;
		int *weights1;
		int *weights2;
		int *indVar;
		int sumW1 = 0;
		int sumW2 = 0;
		int sumP1 = 0;
		int sumP2 = 0;

		assert(fscanf(fichier, "%d", &nbVariable));
		assert(fscanf(fichier, "%d", &omega1));
		assert(fscanf(fichier, "%d", &omega2));

		profits1 = (int *) malloc(nbVariable*sizeof(int));
		profits2 = (int *) malloc(nbVariable*sizeof(int));
		weights1 = (int *) malloc(nbVariable*sizeof(int));
		weights2 = (int *) malloc(nbVariable*sizeof(int));
		indVar = (int *) malloc((nbVariable)*sizeof(int));

		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &profits1[i]));
			sumP1 += profits1[i];
			assert(fscanf(fichier, "%d", &profits2[i]));
			sumP2 += profits2[i];
			assert(fscanf(fichier, "%d", &weights1[i]));
			sumW1 += weights1[i];
			assert(fscanf(fichier, "%d", &weights2[i]));
			sumW2 += weights2[i];
			indVar[i] = i;
		}
		

		p->n = nbVariable;
		p->omega1 = omega1;
		p->omega2 = omega2;
		p->profits1 = profits1;
		p->profits2 = profits2;
		p->weights1 = weights1;
		p->weights2 = weights2;
		p->indVar = indVar;

		p->pCumul1 = (int *) malloc((1+nbVariable)*sizeof(int));
		p->pCumul2 = (int *) malloc((1+nbVariable)*sizeof(int));

		p->wCumul1 = (int *) malloc((1+nbVariable)*sizeof(int));
		p->wCumul2 = (int *) malloc((1+nbVariable)*sizeof(int));

		p->varFix1 = (int *) malloc(nbVariable*sizeof(int));
		p->nVarFix1 = 0;

		p->w1min = 0;
		p->w2min = 0;

		p->sumW1 = sumW1;
		p->sumW2 = sumW2;
		p->sumP1 = sumP1;
		p->sumP2 = sumP2;
	} else {
		printf("Erreur lors de l'ouverture du fichier %s\n", nomFichier);
	}

	return p;
}

Probleme *genererProblemeGautier(char *nomFichier) {
	Probleme *p = (Probleme *) malloc(sizeof(Probleme));

	FILE *fichier = fopen(nomFichier, "r");

	if (fichier) {
		p->nomFichier = nomFichier;

		int nbVariable;
		int omega1;
		int omega2;
		int *profits1;
		int *profits2;
		int *weights1;
		int *weights2;
		int *indVar;
		int sumW1 = 0;
		int sumW2 = 0;
		int sumP1 = 0;
		int sumP2 = 0;

		assert(fscanf(fichier, "%d", &nbVariable));
		assert(fscanf(fichier, "%d", &omega1));
		assert(fscanf(fichier, "%d", &omega2));

		profits1 = (int *) malloc(nbVariable*sizeof(int));
		profits2 = (int *) malloc(nbVariable*sizeof(int));
		weights1 = (int *) malloc(nbVariable*sizeof(int));
		weights2 = (int *) malloc(nbVariable*sizeof(int));
		indVar = (int *) malloc((nbVariable)*sizeof(int));

		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &profits1[i]));
			sumP1 += profits1[i];
			indVar[i] = i;
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &profits2[i]));
			sumP2 += profits2[i];
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &weights1[i]));
			sumW1 += weights1[i];
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &weights2[i]));
			sumW2 += weights2[i];
		}
		

		p->n = nbVariable;
		p->omega1 = omega1;
		p->omega2 = omega2;
		p->profits1 = profits1;
		p->profits2 = profits2;
		p->weights1 = weights1;
		p->weights2 = weights2;
		p->indVar = indVar;

		p->pCumul1 = (int *) malloc((1+nbVariable)*sizeof(int));
		p->pCumul2 = (int *) malloc((1+nbVariable)*sizeof(int));

		p->wCumul1 = (int *) malloc((1+nbVariable)*sizeof(int));
		p->wCumul2 = (int *) malloc((1+nbVariable)*sizeof(int));

		p->varFix1 = (int *) malloc(nbVariable*sizeof(int));
		p->nVarFix1 = 0;

		p->w1min = 0;
		p->w2min = 0;

		p->sumW1 = sumW1;
		p->sumW2 = sumW2;
		p->sumP1 = sumP1;
		p->sumP2 = sumP2;
	} else {
		printf("Erreur lors de l'ouverture du fichier %s\n", nomFichier);
	}

	return p;
}

// Transforme un chemin en un vecteur de booléens correspondant à la solution (du sous problème => à corriger)
Solution *creerSolution(Probleme *p, Chemin *chemin) {
	Solution *sol = (Solution *) malloc(sizeof(Solution));
	Noeud *noeud, *noeudPrec;
	int nBis = p->nBis;

	bool *var = (bool *) malloc(p->n*sizeof(bool));

	sol->p1 = chemin->p1;
	sol->p2 = chemin->p2;

	// On initialise la solution avec aucun objet
	for (int i = 0; i < p->n; ++i) {
		var[i] = false;
	}

	// On ajoute les objets qui appartiennent forcément à la solution
	for (int i = 0; i < p->nVarFix1; ++i) {
		var[p->varFix1[i]] = true;
	}

	// On note les objets qui sont dans la solution en parcourant le chemin
	// On rappelle que les objets dont les indices ne sont pas dans indVar ont été retirés définitivement pour le triangle
	if (chemin->nDeviation > 0) {
		int nDev = chemin->nDeviation;
		int *deviations = (int *) malloc(nDev*sizeof(int));
		for (int j = 0; j < nDev; ++j) {
			deviations[j] = chemin->deviation;
			chemin = (Chemin*) chemin->chemin;
		}
		noeudPrec = (Noeud*) chemin->chemin;
		sol->w1 = noeudPrec->w1;
		sol->w2 = noeudPrec->w2;

		--nDev;
		for (int i = nBis-1; i >= 0; --i) {
			noeud = noeudPrec;
			if ((nDev >= 0) && (deviations[nDev] == i+1)) {
				noeudPrec = noeud->precAlt;
				--nDev;
			} else {
				noeudPrec = noeud->precBest;
			}
			if (noeud->w1 != noeudPrec->w1) {
				var[p->indVar[i]] = true;
			}
		}
	} else {
		noeudPrec = (Noeud*) chemin->chemin;
		sol->w1 = noeudPrec->w1;
		sol->w2 = noeudPrec->w2;

		for (int i = nBis-1; i >= 0; --i) {
			noeud = noeudPrec;
			noeudPrec = noeud->precBest;
			if (noeud->w1 != noeudPrec->w1) {
				var[p->indVar[i]] = true;
			}
		}
	}

	//printf("(%d,%d)\n", sol->p1, sol->p2);
	sol->var = var;

	return sol;
}

void fixer01(Probleme *p, int y1, int y2, ListeSol *lSolHeur) {
	int nbNull = 0;
	solution *s1, *s2;
	int ret;
	donnees d;
	int LB = p->LB;
	int nb0, nb1;

	int rel0P1, rel0P2, rel1P1, rel1P2;

	d.p1 = (itype *) malloc ((p->n) * sizeof(itype));
	d.w1 = (itype *) malloc ((p->n) * sizeof(itype));
	d.w2 = (itype *) malloc ((p->n) * sizeof(itype));
	d.maxZ1 = 0;

	nb0 = 0;
	nb1 = 0;

	p->nBis = p->n;
	d.nbItem = p->nBis-1;
	int i = 0;

	p->nVarFix1 = 0;
	p->z1min = 0;
	p->z2min = 0;
	p->w1min = 0;
	p->w2min = 0;

	int sumW1 = p->sumW1;
	int sumW2 = p->sumW2;
	int sumP1 = p->sumP1;
	int sumP2 = p->sumP2;

	// Tant que tous les items n'ont pas été testés
	while (i < p->nBis) {printf("COCO 1\n");
		int indI = p->indVar[i];
		bool toutEntre0 = ((sumW1 + p->w1min <= p->omega1) && (sumW2 + p->w2min <= p->omega2));
		bool toutEntre1 = ((sumW1 + p->w1min + p->weights1[indI] <= p->omega1) && (sumW2 + p->w2min + p->weights2[indI] <= p->omega2));
		bool unObjetEntre0 = toutEntre0;
		bool unObjetEntre1 = toutEntre1;
		int k = 0;
		while ((!unObjetEntre0) && (!unObjetEntre1) && (k < p->nBis)) {
			int indK = p->indVar[k];
			if ((k != i) && (p->w1min + p->weights1[indK] <= p->omega1) && (p->w2min + p->weights2[indK]<= p->omega2)) {
				unObjetEntre0 = true;
			}
			if ((k != i) && (p->w1min + p->weights1[indI] + p->weights1[indK] <= p->omega1) && (p->w2min + p->weights2[indI] + p->weights2[indK]<= p->omega2)) {
				unObjetEntre1 = true;
			}
			++k;
		}

		if (true/*unObjetEntre0 || unObjetEntre1*/) {
			d.nbItem = p->nBis-1;
			d.maxZ1 = 0;
			// ------------------------------------------------ 1er cas, on teste selon lambda1*Z1 + lambda2*Z2
			// ------------------------- On fixe la variable à 1
			d.omega1 = p->omega1 - p->w1min - p->weights1[indI];
			d.omega2 = p->omega2 - p->w2min - p->weights2[indI];

			for (int j = 0; j < i; ++j) {
				int indJ = p->indVar[j];
				d.p1[j] = p->lambda1*p->profits1[indJ] + p->lambda2*p->profits2[indJ];
				d.w1[j] = p->weights1[indJ];
				d.w2[j] = p->weights2[indJ];
			}
			for (int j = i+1; j <= d.nbItem; ++j) {
				int indJ = p->indVar[j];
				d.p1[j-1] = p->lambda1*p->profits1[indJ] + p->lambda2*p->profits2[indJ];
				d.w1[j-1] = p->weights1[indJ];
				d.w2[j-1] = p->weights2[indJ];
			}


			printf("%d objets avec la capacité résiduelle : (%d,%d)\n", d.nbItem, d.omega1, d.omega2);
			printf("P1\tW1\tW2\n");
			for (int l = 0; l < d.nbItem; ++l) {
				printf("%d\t%d\t%d\n", d.p1[l], d.w1[l], d.w2[l]);
			}


			//if (unObjetEntre1)
			ret = initDichoMu(&s1,&s2,&d);
			if (ret == 0) {
				startDichoMu(&s1,&s2,&d);
			}

			if ((s1 != NULL) && (s1->z1 < s2->z1)) {
				/*if (s2->tab != NULL) {
					free(s2->tab);
				}*/
				free(s2);
				s2 = s1;
			}
			// Si la borne supérieure de la solution optimale n'atteint pas LB
			if (s2->z1 + p->lambda1*p->z1min + p->lambda2*p->z2min + p->lambda1*p->profits1[indI] + p->lambda2*p->profits2[indI] < LB) {
				// On force la variable à 0, donc on la retire du problème
				printf("FIXATION 0 PAR LB\n");
				p->nBis -= 1;
				for (int j = i; j < p->nBis; ++j) {
					p->indVar[j] = p->indVar[j+1];
				}
				++nb0;
			} else {
				/*if (s2->tab != NULL) {
					free(s2->tab);
				}
				free(s2);*/
				// ------------------------- On fixe la variable à 0
				d.omega1 = p->omega1 - p->w1min;
				d.omega2 = p->omega2 - p->w2min;

				ret = initDichoMu(&s1,&s2,&d);
				if (ret == 0) {
					startDichoMu(&s1,&s2,&d);
				}

				if ((s1 != NULL) && (s1->z1 < s2->z1)) {
					/*if (s2->tab != NULL) {
						free(s2->tab);
					}*/
					free(s2);
					s2 = s1;
				}
				// Si la borne supérieure de la solution optimale n'atteint pas LB
				if (s2->z1 + p->lambda1*p->z1min + p->lambda2*p->z2min < LB) {
					printf("FIXATION 1 PAR LB\n");
					++nb1;
					// On force la variable à 1, donc on la retire du problème
					p->varFix1[p->nVarFix1] = indI;
					p->nVarFix1 += 1;
					p->z1min += p->profits1[indI];
					p->z2min += p->profits2[indI];
					p->w1min += p->weights1[indI];
					p->w2min += p->weights2[indI];
					p->nBis -= 1;
					for (int j = i; j < p->nBis; ++j) {
						p->indVar[j] = p->indVar[j+1];
					}
				} else {
					/*if (s2->tab != NULL) {
						free(s2->tab);
					}
					free(s2);*/
					// ------------------------------------------------ 2ème cas, on teste selon Z2
					// ------------------------- On fixe la variable à 1
					d.omega1 = p->omega1 - p->w1min - p->weights1[indI];
					d.omega2 = p->omega2 - p->w2min - p->weights2[indI];
					for (int j = 0; j < i; ++j) {
						d.p1[j] = p->profits2[p->indVar[j]];
					}
					for (int j = i+1; j < p->nBis; ++j) {
						d.p1[j-1] = p->profits2[p->indVar[j]];
					}

					ret = initDichoMu(&s1,&s2,&d);
					if (ret == 0) {
						startDichoMu(&s1,&s2,&d);
					}

					if (s1 == NULL) {
						++nbNull;
					}
					if ((s1 != NULL) && (s1->z1 < s2->z1)) {
						/*if (s2->tab != NULL) {
							free(s2->tab);
						}*/
						free(s2);
						s2 = s1;
					}
					rel0P2 = s2->z1 + p->z2min;

					if (rel0P2 + p->profits2[indI] <= y2) {
						printf("FIXATION 0 PAR Z2\n");
						p->nBis -= 1;
						for (int j = i; j < p->nBis; ++j) {
							p->indVar[j] = p->indVar[j+1];
						}
						++nb0;
					} else {
						/*if (s2->tab != NULL) {
							free(s2->tab);
						}
						free(s2);*/
						// ------------------------- On fixe la variable à 0
						d.omega1 = p->omega1 - p->w1min;
						d.omega2 = p->omega2 - p->w2min;

						ret = initDichoMu(&s1,&s2,&d);
						if (ret == 0) {
							startDichoMu(&s1,&s2,&d);
						}

						if ((s1 != NULL) && (s1->z1 < s2->z1)) {
							/*if (s2->tab != NULL) {
								free(s2->tab);
							}*/
							free(s2);
							s2 = s1;
						}
						rel1P2 = s2->z1 + p->z2min + p->profits2[indI];

						// Si la borne supérieure de la solution optimale n'atteint pas le triangle
						if (rel1P2 <= y2) {
							printf("FIXATION 1 PAR Z2\n");
							++nb1;
							// On fixe la variable à 1, donc on la retire du problème
							p->varFix1[p->nVarFix1] = indI;
							p->nVarFix1 += 1;
							p->z1min += p->profits1[indI];
							p->z2min += p->profits2[indI];
							p->w1min += p->weights1[indI];
							p->w2min += p->weights2[indI];
							p->nBis -= 1;
							for (int j = i; j < p->nBis; ++j) {
								p->indVar[j] = p->indVar[j+1];
							}
						} else {
							/*if (s2->tab != NULL) {
								free(s2->tab);
							}
							free(s2);*/
							// ------------------------------------------------ 3ème cas, on teste selon Z1
							// ------------------------- On fixe la variable à 0
							for (int j = 0; j < i; ++j) {
								d.p1[j] = p->profits1[p->indVar[j]];
							}
							for (int j = i+1; j < p->nBis; ++j) {
								d.p1[j-1] = p->profits1[p->indVar[j]];
							}

							ret = initDichoMu(&s1,&s2,&d);
							if (ret == 0) {
								startDichoMu(&s1,&s2,&d);
							}

							if (s1 == NULL) {
								++nbNull;
							}
							if ((s1 != NULL) && (s1->z1 < s2->z1)) {
								/*if (s2->tab != NULL) {
									free(s2->tab);
								}*/
								free(s2);
								s2 = s1;
							}
							rel0P1 = s2->z1 + p->z1min;

							if (rel0P1 + p->profits1[indI] <= y1) {
								printf("FIXATION 0 PAR Z1\n");
								p->nBis -= 1;
								for (int j = i; j < p->nBis; ++j) {
									p->indVar[j] = p->indVar[j+1];
								}
								++nb0;
							} else {
								/*if (s2->tab != NULL) {
									free(s2->tab);
								}
								free(s2);*/
								// ------------------------- On fixe la variable à 0
								d.omega1 = p->omega1 - p->w1min;
								d.omega2 = p->omega2 - p->w2min;

								ret = initDichoMu(&s1,&s2,&d);
								if (ret == 0) {
									startDichoMu(&s1,&s2,&d);
								}

								if ((s1 != NULL) && (s1->z1 < s2->z1)) {
									/*if (s2->tab != NULL) {
										free(s2->tab);
									}*/
									free(s2);
									s2 = s1;
								}
								rel1P1 = s2->z1 + p->z1min + p->profits1[indI];

								// Si la borne supérieure de la solution optimale n'atteint pas le triangle
								if (rel1P1 <= y1) {
									printf("FIXATION 1 PAR Z1\n");
									++nb1;
									// On fixe la variable à 1, donc on la retire du problème
									p->varFix1[p->nVarFix1] = indI;
									p->nVarFix1 += 1;
									p->z1min += p->profits1[indI];
									p->z2min += p->profits2[indI];
									p->w1min += p->weights1[indI];
									p->w2min += p->weights2[indI];
									p->nBis -= 1;
									for (int j = i; j < p->nBis; ++j) {
										p->indVar[j] = p->indVar[j+1];
									}
								} else {
									/*if (s2->tab != NULL) {
										free(s2->tab);
									}
									free(s2);*/
									// ------------------------------------------------ 4ème cas, on teste si le point idéal est dominé
									// ------------------------- On fixe la variable à 0
									int k = 0;
									int nbSol = lSolHeur->nbSol;
									Solution **solsHeur = lSolHeur->solutions;
									while ((k < nbSol) && ((rel0P1 > solsHeur[k]->p1) || (rel0P2 > solsHeur[k]->p2))) {
										++k;	
									}
									if (k < nbSol) {
										printf("FIXATION 0 PAR POINT IDEAL\n");
										p->nBis -= 1;
										for (int j = i; j < p->nBis; ++j) {
											p->indVar[j] = p->indVar[j+1];
										}
										++nb0;
									} else {
										k = 0;
										while ((k < nbSol) && ((rel1P1 > solsHeur[k]->p1) || (rel1P2 > solsHeur[k]->p2))) {
											++k;	
										}
										if (k < nbSol) {
											printf("FIXATION 1 PAR POINT IDEAL\n");
											++nb1;
											// On fixe la variable à 1, donc on la retire du problème
											p->varFix1[p->nVarFix1] = indI;
											p->nVarFix1 += 1;
											p->z1min += p->profits1[indI];
											p->z2min += p->profits2[indI];
											p->w1min += p->weights1[indI];
											p->w2min += p->weights2[indI];
											p->nBis -= 1;
											for (int j = i; j < p->nBis; ++j) {
												p->indVar[j] = p->indVar[j+1];
											}
										} else {
											++i;
										}
									}
									// ------------------------- On fixe la variable à 1
								}
							}
						}
					}
				}
			}
		}

		//getchar();

		// Libération mémoire
		/*if (s2->tab != NULL) {
			free(s2->tab);
		}
		free(s2);*/
	}

	/*for (int j = 0; j <= p->nBis; ++j) {
		printf("%d\t%d\n", p->pCumul1[j], p->pCumul2[j]);
	}*/

	free(d.p1);
	free(d.w1);
	free(d.w2);
	printf("nb0=%d\n", nb0);
	printf("nb1=%d\n", nb1);
	//printf("nbNull=%d\n", nbNull);


	calculerProfitsCumules(p);
	calculerPoidsCumules(p);
	// J'adore qu'un plan se déroule sans accroc!
}

Solution *copierSolution(Solution *sol, int n) {
	Solution *copie = (Solution *) malloc(sizeof(Solution));
	copie->var = (bool *) malloc(n*sizeof(bool));
	for (int i = 0; i < n; ++i) {
		copie->var[i] = sol->var[i];
	}
	copie->p1 = sol->p1;
	copie->p2 = sol->p2;
	copie->w1 = sol->w1;
	copie->w2 = sol->w2;

	return copie;
}

ListeSol *initListeSol(int nbMax) {
	ListeSol *lSol = (ListeSol *) malloc(sizeof(ListeSol));
	lSol->solutions = (Solution **) malloc(nbMax*sizeof(Solution *));
	lSol->nbSol = 0;
	lSol->nbMax = nbMax;
}