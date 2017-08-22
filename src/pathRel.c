#include "pathRel.h"
#include "probleme.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>

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
		Solution **solComp;
		int nComp;

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
					if (-p->lambda1*p->profits1[i] - p->lambda2*p->profits2[i] > delta) {
						bestI = i;
						delta = -p->lambda1*p->profits1[i] - p->lambda2*p->profits2[i];
					}
				} else {
					if ((X->w1 + p->weights1[i] <= p->omega1) && (X->w2 + p->weights2[i] <= p->omega2) && (p->lambda1*p->profits1[i] + p->lambda2*p->profits2[i] > delta)) {
						bestI = i;
						delta = p->lambda1*p->profits1[i] + p->lambda2*p->profits2[i];
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

			// Si X est dans le triangle est n'est pas dominée par des solutions déjà trouvées
			if (/*(X->p1 > initSol->p1) && (X->p2 > guidingSol->p2) && */(estEfficace(solAdm, *nbSol, X))) {
				Solution *Xc = copierSolution(X, n);
				ajouterSolutionDom(&solAdm, Xc, nbSol, &nbSolMax);
			}
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
					//completerGlouton(Xc, p);
					//printf("est complet ? %d\n", estComplete(Xc, p));
					solComp = completions(Xc, p, &nComp);
					if ((nComp == 0) && (estEfficace(solAdm, *nbSol, Xc))) {
						ajouterSolutionDom(&solAdm, Xc, nbSol, &nbSolMax);
					} else {
						for (int k = 0; k < nComp; ++k) {
							if (estEfficace(solAdm, *nbSol, solComp[k])) {
								ajouterSolutionDom(&solAdm, solComp[k], nbSol, &nbSolMax);
							}
						}
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
			int indV;
			int j = 0;
			int nDegr = 2;

			/*for (int y = 0; y < p->n; ++y) {
				printf("%d", X->var[y]);
			}
			printf("\n");
			for (int y = 0; y < p->n; ++y) {
				printf("%d", X->var[p->indVar[y]]);
			}
			printf("\n");*/

			while ((nDegr > 0) && (j < n) && (bestJ2 == -1)) {
				indV = p->indVar[j];
				if (!Xc->var[indV]) {
					--nDegr;
					Xc->var[indV] = true;
					Xc->w1 += p->weights1[indV];
					Xc->w2 += p->weights2[indV];
					Xc->p1 += p->profits1[indV];
					Xc->p2 += p->profits2[indV];
					//printf("ajouté : %d\n", p->lambda1*p->profits1[indV] + p->lambda2*p->profits2[indV]);
				}
				++j;
			}

			// On teste toutes les suppressions possibles
			// indice du niveau supérieur de l'arbre de recherche
			int *lastI = malloc(n*sizeof(int));
			// pronfondeur courante
			int profondeur = 0;
			int ind = 0;
			indV = p->indVar[n-ind-1];
			int sum = 0;
			//lastI[0] = ind;


						int sp1 = 0;
						int sp2 = 0;
						int sw1 = 0;
						int sw2 = 0;



			while ((profondeur != -1) && (sum < 500)) {


				// -------------------------------------------------------------
				// TEST --------------------------------------------------------
				//printf("Xc=(%d,%d)[%d,%d]\n",Xc->p1, Xc->p2, Xc->w1, Xc->w2);
				sp1 = 0;
				sp2 = 0;
				sw1 = 0;
				sw2 = 0;
				for (int r = 0; r < p->n; ++r) {
					if (Xc->var[r] == true) {
						//printf("r=%d\n", r);
						sp1 += p->profits1[r];
						sp2 += p->profits2[r];
						sw1 += p->weights1[r];
						sw2 += p->weights2[r];
					}
				}
				//printf("sp1=%d p1=%d\n", sp1, Xc->p1);
				assert(sp1 == Xc->p1);
				assert(sp2 == Xc->p2);
				assert(sw1 == Xc->w1);
				assert(sw2 == Xc->w2);
				// TEST --------------------------------------------------------
				// -------------------------------------------------------------


				indV = p->indVar[n-ind-1];
				if (ind == n) {
					--profondeur;
					if (profondeur >= 0) {
						ind = lastI[profondeur];
						indV = p->indVar[n-ind-1];
						Xc->var[indV] = true;
						Xc->w1 += p->weights1[indV];
						Xc->w2 += p->weights2[indV];
						Xc->p1 += p->profits1[indV];
						Xc->p2 += p->profits2[indV];
						++ind;
					}
				} else if (Xc->var[indV]) {
					// on retire l'objet
					//printf("(%d) retiré : %d\n", profondeur, p->lambda1*p->profits1[indV] + p->lambda2*p->profits2[indV]);
					Xc->var[indV] = false;
					Xc->w1 -= p->weights1[indV];
					Xc->w2 -= p->weights2[indV];
					Xc->p1 -= p->profits1[indV];
					Xc->p2 -= p->profits2[indV];
					// Si la solution est admissible
					if ((Xc->w1 <= p->omega1) && (Xc->w2 <= p->omega2)) {
						// Si la solution est efficace alors on l'ajoute
						++sum;
						Solution *Xc2 = copierSolution(Xc, n);
						//printf("complete sol 1 ? %d\n", estComplete(Xc2, p));
						//completerGlouton(Xc2, p);
						//printf("complete sol 2 ? %d\n", estComplete(Xc2, p));
						//printf("remplissage : %d\n", Xc2->w1);

						solComp = completions(Xc2, p, &nComp);
						if ((nComp == 0) && (estEfficace(solAdm, *nbSol, Xc2))) {
							ajouterSolutionDom(&solAdm, Xc2, nbSol, &nbSolMax);
						} else {
							for (int k = 0; k < nComp; ++k) {
								if (estEfficace(solAdm, *nbSol, solComp[k])) {
									ajouterSolutionDom(&solAdm, solComp[k], nbSol, &nbSolMax);
								}
							}
						}

						// Puis on réajoute l'objet pour continuer sur la même profondeur
						Xc->var[indV] = true;
						Xc->w1 += p->weights1[indV];
						Xc->w2 += p->weights2[indV];
						Xc->p1 += p->profits1[indV];
						Xc->p2 += p->profits2[indV];
						++ind;
					} else {
						lastI[profondeur] = ind;
						++profondeur;
						++ind;
					}
				} else {
					++ind;
				}
			}
			
			free(lastI);
		}

		--hd;
	}

	printf("nbSol = %d\n", *nbSol);

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