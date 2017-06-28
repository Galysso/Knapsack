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

	unsigned int deb, nbSup, nbMaxSup;
	solSup = glpkSolutionsSupportees(p, &nbSup, &nbMaxSup);

	ajouterSolution(&resultat, solSup[0], nbSol, &nbSolMax);

	for (int i = 1; i < nbSup; ++i) {
		deb = 0;//*nbSol-1;
		if (estEfficace(resultat, deb, *nbSol, solSup[i])) {
			ajouterSolution(&resultat, solSup[i], nbSol, &nbSolMax);
		}

		unsigned int lambda1 = solSup[i-1]->obj2 - solSup[i]->obj2;
		unsigned int lambda2 = solSup[i]->obj1 - solSup[i-1]->obj1;

		p->lambda1 = lambda1;
		p->lambda2 = lambda2;

		unsigned int LB = lambda1*(solSup[i-1]->obj1+1) + lambda2*(solSup[i]->obj2+1);

		unsigned int *nNoeuds;
		Tas *tas = TAS_initialiser(p->n*p->n);
		Noeud ***graphe = genererGraphe(p, &nNoeuds, solSup[i-1], solSup[i]);
		Chemin **chemins = initialiserChemins(graphe[p->n], nNoeuds[p->n]);

		for (int j = 0; j < nNoeuds[p->n]; ++j) {
			TAS_ajouter(tas, chemins[j]);
		}

		while ((tas->n) && ((TAS_maximum(tas)->val >= LB))) {
			Chemin *chem = TAS_maximum(tas);
			TAS_retirerMax(tas);
			/*printf("deb=%d ", deb);
			printf("nbSol=%d\n", *nbSol);
			printf("test %d,%d: ", creerSolution(p, chem)->obj1, creerSolution(p, chem)->obj2);
			for (int j = 0; j < p->n; ++j) {
				printf("%d", creerSolution(p, chem)->var[j]);
			}
			printf("\n");*/
			Solution *sol = creerSolution(p, chem);
			if (estEfficace(resultat, deb, *nbSol, sol)) {
				ajouterSolution(&resultat, creerSolution(p, chem), nbSol, &nbSolMax);
				if (LB < lambda1*(solSup[i-1]->obj1+1) + (lambda2*(sol->obj2+1))) {
					//LB = lambda1*(solSup[i-1]->obj1+1) + (lambda2*(sol->obj2+1));
					//printf("Mise à jour 1, LB=%d\n", LB);
				}
				if (LB < lambda2*(solSup[i]->obj2+1) + (lambda1*(sol->obj1+1))) {
					//LB = lambda2*(solSup[i]->obj2+1) + (lambda1*(sol->obj1+1));
					//printf("Mise à jour 2, LB=%d\n", LB);
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

	printf("%d solutions:\n", nbSol);
	for (int i = 0; i < nbSol; ++i) {
		printf("(%d,%d) (%d)", nonDominated[i]->obj1, nonDominated[i]->obj2, nonDominated[i]->poids1);
		for (int j = 0; j < p->n; ++j) {
			printf("%d", nonDominated[i]->var[j]);
		}
		printf("\n");
	}

	printf("temps: %fs\n", (double) (fin-debut)/CLOCKS_PER_SEC);
	printf("\n");

	printf("%d solutions:\n", nbSol);
	for (int i = 0; i < nbSol; ++i) {
		printf("(%d,%d)\n", nonDominated[i]->obj1, nonDominated[i]->obj2);
	}

	printf("COCO\n");
}