#include "graphe.h"
#include "probleme.h"
#include "2DKPSurrogate/2DKPSurrogate.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

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
	} else {
		printf("Erreur lors de l'ouverture du fichier %s\n", nomFichier);
	}

	return p;
}

// Transforme un chemin en un vecteur de booléens correspondant à la solution
Solution *creerSolution(Probleme *p, Chemin *chemin) {
	Solution *sol = (Solution *) malloc(sizeof(Solution));
	Noeud *noeud, *noeudPrec;
	int n = p->nBis;

	bool *var = (bool *) malloc(n*sizeof(bool));

	sol->obj1 = chemin->obj1;
	sol->obj2 = chemin->obj2;
	int w1 = 0;
	int w2 = 0;

	if (chemin->nDeviation > 0) {
		int nDev = chemin->nDeviation;
		int *deviations = (int *) malloc(nDev*sizeof(int));
		for (int j = 0; j < nDev; ++j) {
			deviations[j] = chemin->deviation;
			chemin = (Chemin*) chemin->chemin;
		}
		noeudPrec = (Noeud*) chemin->chemin;

		--nDev;
		for (int i = n-1; i >= 0; --i) {
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
				var[i] = true;
			} else {
				var[i] = false;
			}
		}
	} else {
		noeudPrec = (Noeud*) chemin->chemin;
		for (int i = n-1; i >= 0; --i) {
			noeud = noeudPrec;
			noeudPrec = noeud->precBest;
			if (noeud->w1 != noeudPrec->w1) {
				w1 = w1 + p->weights1[i];
				w2 = w2 + p->weights2[i];
				var[i] = true;
			} else {
				var[i] = false;
			}
		}
	}

	sol->w1 = w1;
	sol->w2 = w2;
	sol->var = var;

	return sol;
}

void fixer01(Probleme *p, int y1, int y2) {
	solution *s1, *s2;
	int ret;
	donnees d;
	int LB = p->lambda1*(y1+1) + p->lambda2*(y2+1);
	int nb0;

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
			d.p1[j] = p->profits1[indJ];
			d.w1[j] = p->weights1[indJ];
			d.w2[j] = p->weights2[indJ];
		}
		for (int j = i+1; j < d.nbItem; ++j) {
			int indJ = p->indVar[j];
			d.p1[j-1] = p->profits1[indJ];
			d.w1[j-1] = p->weights1[indJ];
			d.w2[j-1] = p->weights2[indJ];
		}

		ret = initDichoMu(&s1,&s2,&d);
		if (ret == 0) {
			startDichoMu(&s1,&s2,&d);
		}

		if ((s1 != NULL) && (s1->z1 < s2->z1)) {
			free(s2);
			s2 = s1;
		}
		// Si la borne supérieure de la solution optimale n'atteint pas le triangle
		if (s2->z1 + p->profits1[indI] < y1) {
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

			if ((s1 != NULL) && (s1->z1 < s2->z1)) {
				free(s2);
				s2 = s1;
			}

			if (s2->z1 + p->profits2[indI] < y2) {
				p->nBis -= 1;
				for (int j = i; j < p->nBis; ++j) {
					p->indVar[j] = p->indVar[j+1];
				}
				++nb0;
			} else {
				++i;
			}
			/* else {
				free(s2->tab);
				free(s2);
				for (int j = 0; j < i; ++j) {
					d.p1[j] = p->lambda1*p->profits1[j] + p->lambda2*p->profits2[j];
				}
				for (int j = i+1; j < p->nBis; ++j) {
					d.p1[j-1] = p->lambda1*p->profits1[j] + p->lambda2*p->profits2[j];
				}

				ret = initDichoMu(&s1,&s2,&d);
				if (ret == 0) {
					startDichoMu(&s1,&s2,&d);
				}

				if ((s1 != NULL) && (s1->z1 < s2->z1)) {
					free(s2);
					s2 = s1;
				}

				printf("z1=%d\tLB=%d\n", s2->z1, p->lambda1*(y1+1) + p->lambda2*(y2+1));
				if (s2->z1 <= p->lambda1*(y1+1) + p->lambda2*(y2+1)) {
					printf("3\n");
					p->nBis -= 1;
					for (int j = i; j < p->nBis; ++j) {
						p->indVar[j] = p->indVar[j+1];
					}
					++nb0;
				} else {
					++i;
				}
			}*/
		}

		//getchar();

		// Libération mémoire
		free(s2->tab);
		free(s2);
	}

	free(d.p1);
	free(d.w1);
	free(d.w2);
	printf("nb0=%d\n", nb0);
	
	// J'adore qu'un plan se déroule sans accroc!
}