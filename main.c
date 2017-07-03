#include "src/graphe.h"
#include "src/tas.h"
#include "src/reoptimisation.h"
#include "src/knapglpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void ajouterSolution(Solution ***solutions, Solution *sol, unsigned int *nbSol, unsigned int *nbSolMax) {
	if (*nbSol == *nbSolMax) {
		*nbSolMax = (*nbSolMax)*(*nbSolMax);
		*solutions = (Solution **) realloc(*solutions, *nbSolMax*sizeof(Solution *));
	}
	(*solutions)[*nbSol] = sol;
	*nbSol = *nbSol + 1;
}

unsigned int meilleureBorne(Solution **solutionsLB, unsigned int nbSol, Probleme *p) {
	//printf("solution borne : %d,%d\n", sol->obj1, sol->obj2);
	// calcul de la nouvelle borne
	unsigned int lambda1 = p->lambda1;
	unsigned int lambda2 = p->lambda2;
	unsigned int LB, LBprim;
	Solution *solG, *sol;

	LB = lambda1*(solutionsLB[0]->obj1 + 1) + lambda2*(solutionsLB[1]->obj2 + 1);

	for (int i = 2; i < nbSol; ++i) {
		solG = solutionsLB[i-1];
		sol = solutionsLB[i];
		LBprim = lambda1*(solG->obj1 + 1) + lambda2*(sol->obj2 + 1);
		if (LBprim < LB) {
			LB = LBprim;
		}
	}

	return LB;
}

void ajouterSolutionLB(Solution ***solutionsLB, Solution *sol, unsigned int *nbSol, unsigned int *nbSolMax) {
	unsigned int min, ind, max;
	Solution *solLB;
	if (*nbSol == *nbSolMax) {
		*nbSolMax = (*nbSolMax)*(*nbSolMax);
		*solutionsLB = (Solution **) realloc(*solutionsLB, *nbSolMax*sizeof(Solution *));
	}

	min = 0;
	max = *nbSol;
	ind = max/2;

	while (ind != min) {
		solLB = (*solutionsLB)[ind];
		if (sol->obj2 < solLB->obj2) {
			min = ind;
		} else {
			max = ind;
		}
		ind = (min+max)/2;
	}

	if ((ind != *nbSol) && ((*solutionsLB)[ind]->obj2 > sol->obj2)) {
		++ind;
	}

	for (int i = *nbSol; i > ind; --i) {
		(*solutionsLB)[i] = (*solutionsLB)[i-1];
	}
	(*solutionsLB)[ind] = sol;
	*nbSol = *nbSol + 1;
}

bool estEfficace(Solution **solutions, unsigned int deb, unsigned int fin, Solution *sol) {
	while ((deb < fin) && ((solutions[deb]->obj1 < sol->obj1) || (solutions[deb]->obj2 < sol->obj2))) {
		++deb;
	}
	return (deb == fin);
}

Solution **trouverSolutions(Probleme *p, unsigned int *nbSol) {
	*nbSol = 0;
	unsigned int nbSolMax = p->n*p->n;
	Solution **solSup;
	Solution **resultat = (Solution **) malloc(nbSolMax*sizeof(Solution *));
	unsigned int nbSolLBMax = p->n;
	Solution **solutionsLB = (Solution **) malloc(nbSolLBMax*sizeof(Solution *));

	unsigned int deb, nbSup, nbMaxSup, nbSolLB;
	solSup = glpkSolutionsSupportees(p, &nbSup, &nbMaxSup);

	ajouterSolution(&resultat, solSup[1], nbSol, &nbSolMax);
	ajouterSolution(&resultat, solSup[nbSup-2], nbSol, &nbSolMax);
	for (int i = 0; i < nbSup; ++i) {
		if (estEfficace(resultat, 0, *nbSol, solSup[i])) {
			ajouterSolution(&resultat, solSup[i], nbSol, &nbSolMax);
		}
	}

	for (int i = 1; i < nbSup; ++i) {
		Solution *solSup1 = solSup[i-1];
		Solution *solSup2 = solSup[i];
		nbSolLB = 0;
		deb = 0;

		unsigned int lambda1 = solSup1->obj2 - solSup2->obj2;
		unsigned int lambda2 = solSup2->obj1 - solSup1->obj1;

		p->lambda1 = lambda1;
		p->lambda2 = lambda2;

		unsigned int LB = lambda1*(solSup1->obj1+1) + lambda2*(solSup2->obj2+1);
		unsigned int newLB;

		unsigned int *nNoeuds;
		Tas *tas = TAS_initialiser(p->n*p->n);
		Noeud ***graphe = genererGraphe(p, &nNoeuds, solSup1, solSup2);
		Chemin **chemins = initialiserChemins(graphe[p->n], nNoeuds[p->n]);

		for (int j = 0; j < nNoeuds[p->n]; ++j) {
			TAS_ajouter(tas, chemins[j]);
		}

		ajouterSolutionLB(&solutionsLB, solSup1, &nbSolLB, &nbSolLBMax);
		ajouterSolutionLB(&solutionsLB, solSup2, &nbSolLB, &nbSolLBMax);
		while ((tas->n) && ((TAS_maximum(tas)->val >= LB))) {
			Chemin *chem = TAS_maximum(tas);
			TAS_retirerMax(tas);
			Solution *sol = creerSolution(p, chem);
			if (/*(sol->obj1 < solSup2->obj1) && (sol->obj2 > solSup1->obj2) && */estEfficace(resultat, deb, *nbSol, sol)) {
				ajouterSolution(&resultat, sol, nbSol, &nbSolMax);
				ajouterSolutionLB(&solutionsLB, sol, &nbSolLB, &nbSolLBMax);
				newLB = meilleureBorne(solutionsLB, nbSolLB, p);
				/*printf("LB=%d\n", LB);
				printf("newLB=%d\n", newLB);*/
				if (newLB > LB) {
					printf("update LB\n");
					LB = newLB;
				}
			}
			genererSolutions(chem, tas, p);
		}
	}

	return resultat;
}

int main() {
	clock_t debut, fin;

	Probleme *p = genererProblemeGautier("instance100.DAT");
	unsigned int nbSol;

	debut = clock();
	Solution **nonDominated = trouverSolutions(p, &nbSol);
	fin = clock();

	/*printf("%d solutions:\n", nbSol);
	for (int i = 0; i < nbSol; ++i) {
		printf("(%d,%d) (%d)", nonDominated[i]->obj1, nonDominated[i]->obj2, nonDominated[i]->poids1);
		for (int j = 0; j < p->n; ++j) {
			printf("%d", nonDominated[i]->var[j]);
		}
		printf("\n");
	}*/








	bool changement;
	do {
		changement = false;
		for (int i = 1; i < nbSol; ++i) {
			if (nonDominated[i-1]->obj2 < nonDominated[i]->obj2) {
				Solution *sol = nonDominated[i];
				nonDominated[i] = nonDominated[i-1];
				nonDominated[i-1] = sol;
				changement = true;
			}
		}
	} while (changement);

	printf("%d solutions:\n", nbSol);
	for (int i = 0; i < nbSol; ++i) {
		printf("(%d, %d)\n", nonDominated[i]->obj1, nonDominated[i]->obj2);
	}
	printf("%d solutions:\n", nbSol);
	printf("temps: %fs\n", (double) (fin-debut)/CLOCKS_PER_SEC);

	printf("COCO\n");
}