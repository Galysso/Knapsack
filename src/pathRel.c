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
			delta = -p->lambda1*p->profits1[i] - p->lambda2*p->profits2[i];
		} else {
			delta = p->lambda1*p->profits1[i] + p->lambda2*p->profits2[i];
		}

		// On cherche le plus grand delta que l'on puisse avoir
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
			ajouterSolutionDom(&solAdm, X, nbSol, &nbSolMax);
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
						ajouterSolutionDom(&solAdm, Xc, nbSol, &nbSolMax);
					}
				}
			}
		// Si l'objet a été ajouté et que la solution est complète
		} else if (estComplete(X, p)) {
			// Alors on effectue une oscillation d'une profondeur de 2
			// Pour cela on trouve d'abord les deux meilleurs objets à ajouter
			Solution *Xc = copierSolution(X, n);

			int bestJ1 = -1;
			int bestJ2 = -1;
			int delta1 = 0;
			int delta2 = 0;
			int delta = 0;
			for (int j = 0; j < n; ++j) {
				delta = p->lambda1*p->profits1[j] + p->lambda2*p->profits2[j];
				if (!Xc->var[j]) {
					if (delta > delta1) {
						delta2 = delta1;
						delta1 = delta;
						bestJ2 = bestJ1;
						bestJ1 = j;
					} else if (delta > delta2) {
						delta2 = delta;
						bestJ2 = j;
					}
				}
			}
			// On ajoute les objets trouvés (en testant s'il y en a pour la généralité)
			if (bestJ1 >= 0) {
				Xc->var[bestJ1] = true;
				Xc->w1 += p->weights1[bestJ1];
				Xc->w2 += p->weights2[bestJ1];
				Xc->p1 += p->profits1[bestJ1];
				Xc->p2 += p->profits2[bestJ1];
			}
			if (bestJ2 >= 0) {
				Xc->var[bestJ2] = true;
				Xc->w1 += p->weights1[bestJ2];
				Xc->w2 += p->weights2[bestJ2];
				Xc->p1 += p->profits1[bestJ2];
				Xc->p2 += p->profits2[bestJ2];
			}

			// On teste toutes les suppressions possibles
			
		}

		--hd;
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