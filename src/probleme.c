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
		unsigned int *indVar;

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
		indVar = (unsigned int *) malloc((nbVariable)*sizeof(unsigned int));

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
		unsigned int *indVar;

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
		indVar = (unsigned int *) malloc((nbVariable)*sizeof(unsigned int));

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
	unsigned int poids1 = 0;
	unsigned int poids2 = 0;

	if (chemin->nDeviation > 0) {
		int nDev = chemin->nDeviation;
		unsigned int *deviations = (unsigned int *) malloc(nDev*sizeof(unsigned int));
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

Probleme *fixer01(Probleme *p, unsigned int y1, unsigned int y2) {
	solution *s1;
	solution *s2;
	Probleme *sousProb;
	int ret;
	donnees d;
	unsigned int cpt;
	unsigned int LB = p->lambda1*(y1+1) + p->lambda2*(y2+1);
	unsigned int nb0;

	d.p1 = (itype *) malloc ((p->n) * sizeof(itype));
	d.w1 = (itype *) malloc ((p->n) * sizeof(itype));
	d.w2 = (itype *) malloc ((p->n) * sizeof(itype));
	d.maxZ1 = 0;

	nb0 = 0;

	sousProb = genererProbleme(p->nomFichier);

	unsigned int i = 0;
	while (i < d.nbItem) {
		d.omega1 = sousProb->capacite1 - sousProb->poids1[i];
		d.omega2 = sousProb->capacite2 - sousProb->poids2[i];
		d.nbItem = sousProb->n-1;
		d.maxZ1 = 1000000;
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

		printf("fixé à 1 : %d, %d, %d\n", sousProb->coefficients1[i], sousProb->poids1[i], sousProb->poids2[i]);
		printf("capacité1=%d\tcapacité2=%d\n", sousProb->capacite1, sousProb->capacite2);
		printf("omega1=%d\tomega2=%d\n", d.omega1, d.omega2);
		for(int j = 0; j < d.nbItem; ++j) {
			printf("%d\t%d\t%d\t%d\n", j, d.p1[j], d.w1[j], d.w2[j]);
		}
		printf("\n");
		//getchar();

		printf("COCO 1\n");
		ret = initDichoMu(&s1,&s2,&d);
		if (ret == 0) {
			startDichoMu(&s1,&s2,&d);
		}
		printf("COCO 2\n");

		printf("Y1 = %d\n", y1);
		printf("Conclusion : on obtient ");
		if (s1 == NULL)  {
			printf("une solution admissible :\n");
			PrintSolution(s2,d.nbItem-1);
		} else if (s1->z1 < s2->z1) {
			printf("une paire de solutions définissant l'optimalité pour le problème dual-surrogate :\n");
			PrintSolution(s1,d.nbItem-1);
			PrintSolution(s2,d.nbItem-1);
		} else {
			printf("une paire de solutions définissant l'optimalité pour le problème dual-surrogate :\n");
			PrintSolution(s2,d.nbItem-1);
			PrintSolution(s1,d.nbItem-1);
		}
		printf("\n");

		getchar();

		if ((s1 != NULL) && (s1->z1 < s2->z1)) {
			s2 = s1;
		}
		// Si la borne supérieur de la solution optimale n'atteint pas le triangle
		if (s2->z1 + sousProb->coefficients1[i] <= y1) {
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
			// Sinon on 
			++i;
		}

		// Libération mémoire
		
		free(s2->tab);
		free(s2);
		if (s1 != NULL) {
			/*free(s1->tab);
			free(s1);*/
		}
	}

	free(d.p1);
	free(d.w1);
	free(d.w2);
	printf("nb0=%d\n", nb0);
	
	// J'adore qu'un plan se déroule sans accroc!
}