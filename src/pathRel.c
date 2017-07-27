#include "pathRel.h"
#include "probleme.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*Solution **completions(Probleme *p, Solution *incomplete, int *nbComp) {
	Solution *resultat = (Solution *) malloc(p->n*sizeof(Solution));
}*/

Solution **pathRelinking(Probleme *p, Solution *initSol, Solution *guidingSol, int *nbSol) {
	/*for (int j = 0; j < p->n; ++j) {
		printf("%d", initSol->var[j]);
	}
	printf("\n");
	for (int j = 0; j < p->n; ++j) {
		printf("%d", guidingSol->var[j]);
	}
	printf("\n");*/




	*nbSol = 0;
	int n = p->n;
	int nbSolMax = 2*n;
	Solution **solAdm = (Solution **) malloc(n*sizeof(Solution *));
	int hd = 0;
	// On calcule la distance de hamming entre les solutions du path relinking
	for (int i = 0; i < p->n; ++i) {
		if (initSol->var[i] != guidingSol->var[i]) {
			++hd;
		}
	}

	//printf("hd=%d\n", hd);

	Solution *X = copierSolution(initSol, n);

	while (hd > 0) {
		int bestI;
		int delta;
		int i = 0;
		Solution **completions;

		// On cherche l'indice du premier objet qui diffère la solution courante avec la solution d'arrivée
		// On cherche tant que l'objet i est dans le même état pour les deux solutions ou qu'il est absent dans la solution courante et qu'il ne peut pas être ajouté
		while ((X->var[i] == guidingSol->var[i]) || ((X->var[i] == false) && ((X->w1 + p->weights1[i] > p->omega1) || (X->w2 + p->weights2[i] > p->omega2)))) {
			++i;
		}
		bestI = i;
		if (X->var[i]) {
			//printf("1 i=%d\n", i);
			delta = -p->lambda1*p->profits1[i] - p->lambda2*p->profits2[i];
			//printf("delta debut = %d\n", p->profits1[i]);
		} else {
			//printf("2 i=%d\n", i);
			delta = p->lambda1*p->profits1[i] + p->lambda2*p->profits2[i];
		}

		for (i = i+1; i < n; ++i) {
			if (X->var[i] != guidingSol->var[i]) {
				if (X->var[i]) {
					if ((X->w1 + p->weights1[i] <= p->omega1) && (X->w2 + p->weights2[i] <= p->omega2) && (p->lambda1*p->profits1[i] + p->lambda2*p->profits2[i] > delta)) {
						bestI = i;
						delta = p->lambda1*p->profits1[i] + p->lambda2*p->profits2[i];
					}
				} else {
					if (-p->lambda1*p->profits1[i] - p->lambda2*p->profits2[i] > delta) {
						bestI = i;
						delta = -p->lambda1*p->profits1[i] - p->lambda2*p->profits2[i];
					}
				}
			}
		}

		//printf("bestDelta=%d\n", delta);
		if (X->var[bestI]) {
			X->var[bestI] = false;
			X->p1 -= p->profits1[bestI];
			X->p2 -= p->profits2[bestI];
			X->w1 -= p->weights1[bestI];
			X->w2 -= p->weights2[bestI];
		} else {
			X->var[bestI] = true;
			X->p1 += p->profits1[bestI];
			X->p2 += p->profits2[bestI];
			X->w1 += p->weights1[bestI];
			X->w2 += p->weights2[bestI];
		}

		// Si X est dans le triangle est n'est pas dominée par des solutions déjà trouvées
		if ((X->p1 > initSol->p1) && (X->p2 > guidingSol->p2) && (estEfficace(solAdm, *nbSol, X))) {
			// On supprime les solutions déjà trouvées et dominées par X
			for (int j = 0; j < *nbSol; ++j) {
				if ((X->p1 > solAdm[j]->p1) && (X->p2 > solAdm[j]->p2)) {
					*nbSol = *nbSol - 1;
					solAdm[j] = solAdm[*nbSol];
					--j;
				}
			}
			// On ajoute X à la liste des solutions pour LB
			solAdm[*nbSol] = copierSolution(X, n);
			*nbSol = *nbSol + 1;
		}

		// On crée les completions si un objet a été retiré
		if (!X->var[bestI]) {
			for (i = 0; i < p->n; ++i) {
				// Si l'objet n'est pas présent dans la solution
				if ((!X->var[i])
				// Et si l'objet entre dans le sac à dos
				&& (X->w1 + p->weights1[i] <= p->omega1) && (X->w2 + p->weights2[i] <= p->omega2)
				// Et si l'objet est dans le triangle
				&& (X->p1 + p->profits1[i] >= initSol->p1) && (X->p2 + p->profits2[i] >= guidingSol->p2)) {
					// ALors on créé la solution
					Solution *Xc = copierSolution(X, n);
					Xc->p1 += p->profits1[i];
					Xc->p2 += p->profits2[i];
					Xc->w1 += p->weights1[i];
					Xc->w2 += p->weights2[i];
					if (estEfficace(solAdm, *nbSol, Xc)) {
						ajouterSolutionDom(&solAdm, )
						solAdm[*nbSol] = Xc;
						*nbSol = *nbSol + 1;
					}
				}
			}
		}

		--hd;

		//printf("\n\n");
	}

	// on trie les solutions
	bool changement;
	do {
		changement = false;
		for (int i = 1; i < *nbSol; ++i) {
			if (solAdm[i-1]->p2 < solAdm[i]->p2) {
				Solution *sol = solAdm[i];
				solAdm[i] = solAdm[i-1];
				solAdm[i-1] = sol;
				changement = true;
			}
		}
	} while (changement);

	return solAdm;
}