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
		int capacite1;
		int capacite2;
		int *coef1;
		int *coef2;
		int *poids1;
		int *poids2;
		int *poidsCumules1;
		int *poidsCumules2;
		int *coefCumules1;
		int *coefCumules2;
		int *indVar;

		assert(fscanf(fichier, "%d", &nbVariable));
		assert(fscanf(fichier, "%d", &capacite1));
		assert(fscanf(fichier, "%d", &capacite2));

		coef1 = (int *) malloc(nbVariable*sizeof(int));
		coef2 = (int *) malloc(nbVariable*sizeof(int));
		poids1 = (int *) malloc(nbVariable*sizeof(int));
		poids2 = (int *) malloc(nbVariable*sizeof(int));
		poidsCumules1 = (int *) malloc((1+nbVariable)*sizeof(int));
		poidsCumules2 = (int *) malloc((1+nbVariable)*sizeof(int));
		coefCumules1 = (int *) malloc((1+nbVariable)*sizeof(int));
		coefCumules2 = (int *) malloc((1+nbVariable)*sizeof(int));
		indVar = (int *) malloc((nbVariable)*sizeof(int));

		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &coef1[i]));
			assert(fscanf(fichier, "%d", &poids1[i]));
			assert(fscanf(fichier, "%d", &coef2[i]));
			assert(fscanf(fichier, "%d", &poids2[i]));
		}

		poidsCumules1[nbVariable] = 0;
		poidsCumules2[nbVariable] = 0;
		coefCumules1[nbVariable-1] = 0;
		coefCumules2[nbVariable-1] = 0;
		for (int i = nbVariable-1; i >= 0; --i) {
			poidsCumules1[i] = poidsCumules1[i+1] + poids1[i];
			poidsCumules2[i] = poidsCumules2[i+1] + poids2[i];
			coefCumules1[i] = coefCumules1[i+1] + coef1[i];
			coefCumules2[i] = coefCumules2[i+1] + coef2[i];
			indVar[i] = i;
		}
		

		p->n = nbVariable;
		p->capacite1 = capacite1;
		p->capacite2 = capacite2;
		p->coefficients1 = coef1;
		p->coefficients2 = coef2;
		p->poids1 = poids1;
		p->poids2 = poids2;
		p->poidsCumules1 = poidsCumules1;
		p->poidsCumules2 = poidsCumules2;
		p->coefCumules1 = coefCumules1;
		p->coefCumules2 = coefCumules2;
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
		int capacite1;
		int capacite2;
		int *coef1;
		int *coef2;
		int *poids1;
		int *poids2;
		int *poidsCumules1;
		int *poidsCumules2;
		int *coefCumules1;
		int *coefCumules2;
		int *indVar;

		assert(fscanf(fichier, "%d", &nbVariable));
		assert(fscanf(fichier, "%d", &capacite1));
		assert(fscanf(fichier, "%d", &capacite2));

		coef1 = (int *) malloc(nbVariable*sizeof(int));
		coef2 = (int *) malloc(nbVariable*sizeof(int));
		poids1 = (int *) malloc(nbVariable*sizeof(int));
		poids2 = (int *) malloc(nbVariable*sizeof(int));
		poidsCumules1 = (int *) malloc((1+nbVariable)*sizeof(int));
		poidsCumules2 = (int *) malloc((1+nbVariable)*sizeof(int));
		coefCumules1 = (int *) malloc((1+nbVariable)*sizeof(int));
		coefCumules2 = (int *) malloc((1+nbVariable)*sizeof(int));
		indVar = (int *) malloc((nbVariable)*sizeof(int));

		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &coef1[i]));
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &coef2[i]));
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &poids1[i]));
		}
		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &poids2[i]));
		}
		
		poidsCumules1[nbVariable] = 0;
		poidsCumules2[nbVariable] = 0;
		coefCumules1[nbVariable-1] = 0;
		coefCumules2[nbVariable-1] = 0;
		for (int i = nbVariable-1; i >= 0; --i) {
			poidsCumules1[i] = poidsCumules1[i+1] + poids1[i];
			poidsCumules2[i] = poidsCumules2[i+1] + poids2[i];
			coefCumules1[i] = coefCumules1[i+1] + coef1[i];
			coefCumules2[i] = coefCumules2[i+1] + coef2[i];
			indVar[i] = i;
		}
		

		p->n = nbVariable;
		p->capacite1 = capacite1;
		p->capacite2 = capacite2;
		p->coefficients1 = coef1;
		p->coefficients2 = coef2;
		p->poids1 = poids1;
		p->poids2 = poids2;
		p->poidsCumules1 = poidsCumules1;
		p->poidsCumules2 = poidsCumules2;
		p->coefCumules1 = coefCumules1;
		p->coefCumules2 = coefCumules2;
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
	int n = p->n;

	bool *var = (bool *) malloc(n*sizeof(bool));

	sol->obj1 = chemin->obj1;
	sol->obj2 = chemin->obj2;
	int poids1 = 0;
	int poids2 = 0;

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
			if (noeud->poids1 != noeudPrec->poids1) {
				poids1 = poids1 + p->poids1[i];
				poids2 = poids2 + p->poids2[i];
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
			if (noeud->poids1 != noeudPrec->poids1) {
				poids1 = poids1 + p->poids1[i];
				poids2 = poids2 + p->poids2[i];
				var[i] = true;
			} else {
				var[i] = false;
			}
		}
	}

	sol->poids1 = poids1;
	sol->poids2 = poids2;
	sol->var = var;

	return sol;
}

Probleme *fixer01(Probleme *p, int y1, int y2) {
	solution *s1, *s2;
	Probleme *sousProb;
	int ret;
	donnees d;
	int cpt;
	int LB = p->lambda1*(y1+1) + p->lambda2*(y2+1);
	int nb0;

	d.p1 = (itype *) malloc ((p->n) * sizeof(itype));
	d.w1 = (itype *) malloc ((p->n) * sizeof(itype));
	d.w2 = (itype *) malloc ((p->n) * sizeof(itype));
	d.maxZ1 = 0;

	nb0 = 0;

	sousProb = genererProblemeGautier(p->nomFichier);
	sousProb->lambda1 = p->lambda1;
	sousProb->lambda2 = p->lambda2;

	d.nbItem = sousProb->n-1;
	int i = 0;
	while (i < d.nbItem) {
		d.omega1 = sousProb->capacite1 - sousProb->poids1[i];
		d.omega2 = sousProb->capacite2 - sousProb->poids2[i];
		d.nbItem = sousProb->n-1;
		d.maxZ1 = 0;
		for (int j = 0; j < i; ++j) {
			d.p1[j] = sousProb->coefficients1[j];
			d.w1[j] = sousProb->poids1[j];
			d.w2[j] = sousProb->poids2[j];
		}
		for (int j = i+1; j < sousProb->n; ++j) {
			d.p1[j-1] = sousProb->coefficients1[j];
			d.w1[j-1] = sousProb->poids1[j];
			d.w2[j-1] = sousProb->poids2[j];
		}

		ret = initDichoMu(&s1,&s2,&d);
		if (ret == 0) {
			startDichoMu(&s1,&s2,&d);
		}


		if ((s1 != NULL) && (s1->z1 < s2->z1)) {
			free(s2);
			s2 = s1;
		}
		// Si la borne supérieur de la solution optimale n'atteint pas le triangle
		if (s2->z1 + sousProb->coefficients1[i] < y1) {
			// On fixe la variable à 0, donc on la retire du problème
			sousProb->n -= 1;
			for (int j = i; j < sousProb->n; ++j) {
				sousProb->coefficients1[j] = sousProb->coefficients1[j+1];
				sousProb->coefficients2[j] = sousProb->coefficients2[j+1];
				sousProb->poids1[j] = sousProb->poids1[j+1];
				sousProb->poids2[j] = sousProb->poids2[j+1];
				sousProb->indVar[j] = sousProb->indVar[j+1];
			}
			++nb0;
		} else {
			free(s2->tab);
			free(s2);
			for (int j = 0; j < i; ++j) {
				d.p1[j] = sousProb->coefficients2[j];
			}
			for (int j = i+1; j < sousProb->n; ++j) {
				d.p1[j-1] = sousProb->coefficients2[j];
			}

			ret = initDichoMu(&s1,&s2,&d);
			if (ret == 0) {
				startDichoMu(&s1,&s2,&d);
			}

			if ((s1 != NULL) && (s1->z1 < s2->z1)) {
				free(s2);
				s2 = s1;
			}

			if (s2->z1 + sousProb->coefficients2[i] < y2) {
				sousProb->n -= 1;
				for (int j = i; j < sousProb->n; ++j) {
					sousProb->coefficients1[j] = sousProb->coefficients1[j+1];
					sousProb->coefficients2[j] = sousProb->coefficients2[j+1];
					sousProb->poids1[j] = sousProb->poids1[j+1];
					sousProb->poids2[j] = sousProb->poids2[j+1];
					sousProb->indVar[j] = sousProb->indVar[j+1];
				}
				++nb0;
			} else {
				++i;
			}/* else {
				free(s2->tab);
				free(s2);
				for (int j = 0; j < i; ++j) {
					d.p1[j] = p->lambda1*sousProb->coefficients1[j] + p->lambda2*sousProb->coefficients2[j];
				}
				for (int j = i+1; j < sousProb->n; ++j) {
					d.p1[j-1] = p->lambda1*sousProb->coefficients1[j] + p->lambda2*sousProb->coefficients2[j];
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
					sousProb->n -= 1;
					for (int j = i; j < sousProb->n; ++j) {
						sousProb->coefficients1[j] = sousProb->coefficients1[j+1];
						sousProb->coefficients2[j] = sousProb->coefficients2[j+1];
						sousProb->poids1[j] = sousProb->poids1[j+1];
						sousProb->poids2[j] = sousProb->poids2[j+1];
						sousProb->indVar[j] = sousProb->indVar[j+1];
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

	sousProb->poidsCumules1[sousProb->n-1] = 0;
	sousProb->poidsCumules2[sousProb->n-1] = 0;
	sousProb->coefCumules1[sousProb->n-1] = 0;
	sousProb->coefCumules2[sousProb->n-1] = 0;
	for (int i = sousProb->n-1; i >= 0; --i) {
		sousProb->poidsCumules1[i] = sousProb->poidsCumules1[i+1] + sousProb->poids1[i];
		sousProb->poidsCumules2[i] = sousProb->poidsCumules2[i+1] + sousProb->poids2[i];
		sousProb->coefCumules1[i] = sousProb->coefCumules1[i+1] + sousProb->coefficients1[i];
		sousProb->coefCumules2[i] = sousProb->coefCumules2[i+1] + sousProb->coefficients2[i];
	}

	return sousProb;
}