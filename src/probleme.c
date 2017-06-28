#include "graphe.h"
#include "probleme.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

Probleme *genererProbleme(char *nomFichier) {
	Probleme *p = (Probleme *) malloc(sizeof(Probleme));

	FILE *fichier = fopen(nomFichier, "r");

	if (fichier) {
		unsigned int nbVariable;
		unsigned int capacite1;
		unsigned int capacite2;
		unsigned int *coef1;
		unsigned int *coef2;
		unsigned int *poids1;
		unsigned int *poids2;
		unsigned int *poidsCumules1;
		unsigned int *poidsCumules2;
		unsigned int *coefCumules1;
		unsigned int *coefCumules2;

		assert(fscanf(fichier, "%d", &nbVariable));
		assert(fscanf(fichier, "%d", &capacite1));
		assert(fscanf(fichier, "%d", &capacite2));

		coef1 = (unsigned int *) malloc(nbVariable*sizeof(unsigned int));
		coef2 = (unsigned int *) malloc(nbVariable*sizeof(unsigned int));
		poids1 = (unsigned int *) malloc(nbVariable*sizeof(unsigned int));
		poids2 = (unsigned int *) malloc(nbVariable*sizeof(unsigned int));
		poidsCumules1 = (unsigned int *) malloc((1+nbVariable)*sizeof(unsigned int));
		poidsCumules2 = (unsigned int *) malloc((1+nbVariable)*sizeof(unsigned int));
		coefCumules1 = (unsigned int *) malloc((1+nbVariable)*sizeof(unsigned int));
		coefCumules2 = (unsigned int *) malloc((1+nbVariable)*sizeof(unsigned int));

		for (int i = 0; i < nbVariable; ++i) {
			assert(fscanf(fichier, "%d", &coef1[i]));
			assert(fscanf(fichier, "%d", &poids1[i]));
			assert(fscanf(fichier, "%d", &coef2[i]));
			assert(fscanf(fichier, "%d", &poids2[i]));
			/*poidsCumules1[i] = poidsCumules1[i-1] + poids1[i];
			poidsCumules2[i] = poidsCumules2[i-1] + poids2[i];*/
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
	} else {
		printf("Erreur lors de l'ouverture du fichier %s\n", nomFichier);
	}

	return p;
}

Probleme *genererProblemeGautier(char *nomFichier) {
	Probleme *p = (Probleme *) malloc(sizeof(Probleme));

	FILE *fichier = fopen(nomFichier, "r");

	if (fichier) {
		unsigned int nbVariable;
		unsigned int capacite1;
		unsigned int capacite2;
		unsigned int *coef1;
		unsigned int *coef2;
		unsigned int *poids1;
		unsigned int *poids2;
		unsigned int *poidsCumules1;
		unsigned int *poidsCumules2;
		unsigned int *coefCumules1;
		unsigned int *coefCumules2;

		assert(fscanf(fichier, "%d", &nbVariable));
		assert(fscanf(fichier, "%d", &capacite1));
		assert(fscanf(fichier, "%d", &capacite2));

		coef1 = (unsigned int *) malloc(nbVariable*sizeof(unsigned int));
		coef2 = (unsigned int *) malloc(nbVariable*sizeof(unsigned int));
		poids1 = (unsigned int *) malloc(nbVariable*sizeof(unsigned int));
		poids2 = (unsigned int *) malloc(nbVariable*sizeof(unsigned int));
		poidsCumules1 = (unsigned int *) malloc((1+nbVariable)*sizeof(unsigned int));
		poidsCumules2 = (unsigned int *) malloc((1+nbVariable)*sizeof(unsigned int));
		coefCumules1 = (unsigned int *) malloc((1+nbVariable)*sizeof(unsigned int));
		coefCumules2 = (unsigned int *) malloc((1+nbVariable)*sizeof(unsigned int));

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
	} else {
		printf("Erreur lors de l'ouverture du fichier %s\n", nomFichier);
	}

	return p;
}

Solution *creerSolution(Probleme *p, Chemin *chemin) {
	Solution *sol = (Solution *) malloc(sizeof(Solution));
	Noeud *noeud, *noeudPrec;
	int n = p->n;

	bool *var = (bool *) malloc(n*sizeof(bool));

	sol->obj1 = 0;
	sol->obj2 = 0;
	sol->poids1 = 0;
	sol->poids2 = 0;

	if (chemin->nDeviation > 0) {
		//printf("COCORICO\n");
		int nDev = chemin->nDeviation;
		unsigned int *deviations = (unsigned int *) malloc(nDev*sizeof(unsigned int));
		for (int j = 0; j < nDev; ++j) {
			deviations[j] = chemin->deviation;
			chemin = (Chemin*) chemin->chemin;
			//printf("dev=%d\n", deviations[j]);
		}
		noeudPrec = (Noeud*) chemin->chemin;

		//printf("nDev=%d\n", nDev);
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
				sol->obj1 = sol->obj1 + p->coefficients1[i];
				sol->obj2 = sol->obj2 + p->coefficients2[i];
				sol->poids1 = sol->poids1 + p->poids1[i];
				sol->poids2 = sol->poids2 + p->poids2[i];
				var[i] = true;
			} else {
				var[i] = false;
			}
		}
	} else {
		//printf("GRAGOGLOBOPS\n");
		noeudPrec = (Noeud*) chemin->chemin;
		for (int i = n-1; i > 0; --i) {
			noeud = noeudPrec;
			noeudPrec = noeud->precBest;
			if (noeud->poids1 != noeudPrec->poids1) {
				sol->obj1 = sol->obj1 + p->coefficients1[i];
				sol->obj2 = sol->obj2 + p->coefficients2[i];
				sol->poids1 = sol->poids1 + p->poids1[i];
				sol->poids2 = sol->poids2 + p->poids2[i];
				var[i] = true;
			} else {
				var[i] = false;
			}
		}
	}

	sol->var = var;

	return sol;
}

/*Solution *creerSolution(Probleme *p, Chemin *chemin) {
	Solution *sol = (Solution *) malloc(sizeof(Solution));
	Noeud *noeud;
	int n = p->n;
	int i = n-1;

	bool *var = (bool *) malloc(n*sizeof(bool));

	sol->obj1 = 0;
	sol->obj2 = 0;

	if (chemin->deviation) {
		int nDev = chemin->nDeviation;
		unsigned int *deviations = (unsigned int *) malloc(nDev*sizeof(unsigned int));
		for (int j = 0; j < nDev; ++j) {
			deviations[j] = chemin->deviation;
			chemin = (Chemin*) chemin->chemin;
		}

		noeud = (Noeud*) chemin->chemin;

		sol->poids1 = noeud->poids1;
		sol->poids2 = noeud->poids2;

		--nDev;
		while (nDev >= 0) {
			while (n > deviations[nDev]) {
				if (noeud->poids1 == noeud->precBest->poids1) {
					var[i] = false;
				} else {
					var[i] = true;
					sol->obj1 = sol->obj1 + p->coefficients1[i];
					sol->obj2 = sol->obj2 + p->coefficients2[i];
				}
				--i;
				noeud = noeud->precBest;
				--n;
			}
			if (noeud->poids1 == noeud->precAlt->poids1) {
				var[i] = false;
			} else {
				var[i] = true;
				sol->obj1 = sol->obj1 + p->coefficients1[i];
				sol->obj2 = sol->obj2 + p->coefficients2[i];
			}
			--i;
			noeud = noeud->precAlt;
			--n;
			--nDev;
		}
	} else {
		noeud = (Noeud*) chemin->chemin;

		sol->poids1 = noeud->poids1;
		sol->poids2 = noeud->poids2;
	}

	for ( ; n > 0; --n) {
		if (noeud->poids1 == noeud->precBest->poids1) {
			var[i] = false;
		} else {
			sol->obj1 = sol->obj1 + p->coefficients1[i];
			sol->obj2 = sol->obj2 + p->coefficients2[i];
			var[i] = true;
		}
		--i;
		noeud = noeud->precBest;
	}

	sol->var = var;

	return sol;
}*/