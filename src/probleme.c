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

// Ajoute une solution à la liste des solutions efficaces
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
			assert(fscanf(fichier, "%d", &profits2[i]));
			assert(fscanf(fichier, "%d", &weights1[i]));
			assert(fscanf(fichier, "%d", &weights2[i]));
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

		p->pCumul1[nbVariable] = 0;
		p->pCumul2[nbVariable] = 0;
		for (int j = nbVariable-1; j >= 0; --j) {
			int indJ = indVar[j];
			p->pCumul1[j] = p->pCumul1[j+1] + profits1[indJ];
			p->pCumul2[j] = p->pCumul2[j+1] + profits2[indJ];
		}
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
			indVar[i] = i;
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &profits2[i]));
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &weights1[i]));
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &weights2[i]));
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
	int w1 = 0;
	int w2 = 0;

	// On initialise la solution avec aucun objet
	for (int i = 0; i < p->n; ++i) {
		var[i] = false;
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
				w1 = w1 + p->weights1[i];
				w2 = w2 + p->weights2[i];
				var[p->indVar[i]] = true;
			}
		}
	} else {
		noeudPrec = (Noeud*) chemin->chemin;
		for (int i = nBis-1; i >= 0; --i) {
			noeud = noeudPrec;
			noeudPrec = noeud->precBest;
			if (noeud->w1 != noeudPrec->w1) {
				w1 = w1 + p->weights1[i];
				w2 = w2 + p->weights2[i];
				var[p->indVar[i]] = true;
			}
		}
	}

	sol->w1 = w1;
	sol->w2 = w2;
	sol->var = var;

	return sol;
}

void fixer01(Probleme *p, int y1, int y2, ListeSol *lSolHeur) {
	int nbNull = 0;
	solution *s1, *s2;
	int ret;
	donnees d;
	int LB = p->LB;
	int nb0;

	int relP1, relP2;

	d.p1 = (itype *) malloc ((p->n) * sizeof(itype));
	d.w1 = (itype *) malloc ((p->n) * sizeof(itype));
	d.w2 = (itype *) malloc ((p->n) * sizeof(itype));
	d.maxZ1 = 0;

	nb0 = 0;

	for (int i = 0; i < p->n; ++i) {
		p->indVar[i] = i;
	}

	p->nBis = p->n;
	d.nbItem = p->nBis-1;
	int i = 0;
	while (i < d.nbItem) {
		int indI = p->indVar[i];
		d.omega1 = p->omega1 - p->weights1[indI];
		d.omega2 = p->omega2 - p->weights2[indI];
		d.nbItem = p->nBis-1;
		d.maxZ1 = 0;
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

		ret = initDichoMu(&s1,&s2,&d);
		if (ret == 0) {
			startDichoMu(&s1,&s2,&d);
		}

		if (s1 == NULL) {
			++nbNull;
			Solution *sol = malloc(sizeof(Solution *));
			sol->p1 = p->profits1[indI];
			sol->p2 = p->profits2[indI];
			sol->w1 = p->weights1[indI];
			sol->w2 = p->weights2[indI];
			for (int j = 0; j < i; ++j) {
				if (s2->tab[j]) {
					int indJ = p->indVar[j];
					sol->p1 += p->profits1[indJ];
					sol->p2 += p->profits2[indJ];
					sol->w1 += p->weights1[indJ];
					sol->w2 += p->weights2[indJ];
				}
			}
			for (int j = i+1; j <= d.nbItem; ++j) {
				if (s2->tab[j-1]) {
					int indJ = p->indVar[j];
					sol->p1 += p->profits1[indJ];
					sol->p2 += p->profits2[indJ];
					sol->w1 += p->weights1[indJ];
					sol->w2 += p->weights2[indJ];
				}
			}
		}
		if ((s1 != NULL) && (s1->z1 < s2->z1)) {
			free(s2);
			s2 = s1;
		}
		// Si la borne supérieure de la solution optimale n'atteint pas le triangle
		if (s2->z1 + p->lambda1*p->profits1[indI] + p->lambda2*p->profits2[indI] < LB) {
			// On fixe la variable à 0, donc on la retire du problème
			p->nBis -= 1;
			for (int j = i; j < p->nBis; ++j) {
				p->indVar[j] = p->indVar[j+1];
			}
			++nb0;
		} else {
			free(s2->tab);
			free(s2);
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
				free(s2);
				s2 = s1;
			}
			relP2 = s2->z1;

			if (s2->z1 + p->profits2[indI] < y2) {
				p->nBis -= 1;
				for (int j = i; j < p->nBis; ++j) {
					p->indVar[j] = p->indVar[j+1];
				}
				++nb0;
			} else {
				free(s2->tab);
				free(s2);
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
					free(s2);
					s2 = s1;
				}
				relP1 = s2->z1;

				if (s2->z1 + p->profits1[indI] < y1) {
					p->nBis -= 1;
					for (int j = i; j < p->nBis; ++j) {
						p->indVar[j] = p->indVar[j+1];
					}
					++nb0;
				} else {
					// On regarde si le point idéal est dominé par un point du path relinking
					int k = 0;
					int nbSol = lSolHeur->nbSol;
					Solution **solsHeur = lSolHeur->solutions;
					while ((k < nbSol) && ((relP1 > solsHeur[k]->p1) || (relP2 > solsHeur[k]->p2))) {
						++k;	
					}
					if (k < nbSol) {
						printf("suppression par point idéal\n");
						p->nBis -= 1;
						for (int j = i; j < p->nBis; ++j) {
							p->indVar[j] = p->indVar[j+1];
						}
						++nb0;
					} else {
						++i;
					}
				}
			}
		}

		//getchar();

		// Libération mémoire
		free(s2->tab);
		free(s2);
	}

	// calcul des profits cumules
	int indJ = p->indVar[p->nBis-1];
	p->pCumul1[p->nBis] = 0;
	p->pCumul2[p->nBis] = 0;
	for (int j = p->nBis-1; j >= 0; --j) {
		indJ = p->indVar[j];
		p->pCumul1[j] = p->pCumul1[j+1] + p->profits1[indJ];
		p->pCumul2[j] = p->pCumul2[j+1] + p->profits2[indJ];
	}

	/*for (int j = 0; j <= p->nBis; ++j) {
		printf("%d\t%d\n", p->pCumul1[j], p->pCumul2[j]);
	}*/

	free(d.p1);
	free(d.w1);
	free(d.w2);
	//printf("nb0=%d\n", nb0);
	//printf("nbNull=%d\n", nbNull);

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