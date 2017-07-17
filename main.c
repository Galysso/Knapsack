#include "src/graphe.h"
#include "src/tas.h"
#include "src/reoptimisation.h"
#include "src/knapglpk.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Ajoute une solution à la liste des solutions efficaces
void ajouterSolution(Solution ***solutions, Solution *sol, int *nbSol, int *nbSolMax) {
	if (*nbSol == *nbSolMax) {
		*nbSolMax = (*nbSolMax)*(*nbSolMax);
		*solutions = (Solution **) realloc(*solutions, *nbSolMax*sizeof(Solution *));
	}
	(*solutions)[*nbSol] = sol;
	*nbSol = *nbSol + 1;
}

// Calcule la borne minimale actuelle
int meilleureBorne(Solution **solutionsLB, int nbSol, Probleme *p) {
	int lambda1 = p->lambda1;
	int lambda2 = p->lambda2;
	int LB, LBprim;
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

// Ajoute la solution à la liste des points définissant les points de Nadir locaux
// Conserve l'ordre lexicographique des solutions
void ajouterSolutionLB(Solution ***solutionsLB, Solution *sol, int *nbSol, int *nbSolMax) {
	int min, ind, max;
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

// Renvoie vrai si une solution n'est pas dominée par les solutions déjà trouvées
bool estEfficace(Solution **solutions, int fin, Solution *sol) {
	int i = 0;
	while ((i < fin) && ((solutions[i]->obj1 < sol->obj1) || (solutions[i]->obj2 < sol->obj2))) {
		++i;
	}
	return (i == fin);
}

// Fuite de mémoire : les chemins ne sont pas désalloués
Solution **trouverSolutions(Probleme *p, int *nbSol) {
	int lambda1, lambda2;		// poids de la somme pondérée
	int LB, newLB;				// borne inférieure et plus petite borne inférieure actuelle
	int nbSup, nbMaxSup, nbSolMax, nbSolLB, nbSolLBMax;	// remplissages et tailles allouées des tableaux
	int *nNoeuds;				// nombre de noeuds pour chaque colonne du graphe
	Solution **solSup; 			// solutions supportées
	Solution **solutionsLB; 	// solutions trouvées dans le triangle pour la borne
	Solution **resultat;		// solutions efficaces trouvées


	*nbSol = 0;
	nbSolMax = p->n*p->n;
	nbSolLBMax = p->n;

	resultat = (Solution **) malloc(nbSolMax*sizeof(Solution *));
	solutionsLB = (Solution **) malloc(nbSolLBMax*sizeof(Solution *));
	solSup = glpkSolutionsSupportees(p, &nbSup, &nbMaxSup);

	ajouterSolution(&resultat, solSup[1], nbSol, &nbSolMax);
	ajouterSolution(&resultat, solSup[nbSup-2], nbSol, &nbSolMax);
	for (int i = 0; i < nbSup; ++i) {
		if (estEfficace(resultat, *nbSol, solSup[i])) {
			ajouterSolution(&resultat, solSup[i], nbSol, &nbSolMax);
		}
	}

	for (int i = 1; i < nbSup; ++i) {
		Solution *solSup1 = solSup[i-1];
		Solution *solSup2 = solSup[i];
		nbSolLB = 0;

		lambda1 = solSup1->obj2 - solSup2->obj2;
		lambda2 = solSup2->obj1 - solSup1->obj1;

		p->lambda1 = lambda1;
		p->lambda2 = lambda2;

		LB = lambda1*(solSup1->obj1+1) + lambda2*(solSup2->obj2+1);
		fixer01(p, solSup1->obj1, solSup2->obj2);

		/*for (int j = 0; j < p->nBis; ++j) {
			printf("%d\n", p->indVar[j]);
		}*/

		Tas *tas = TAS_initialiser(p->nBis*p->nBis);
		Noeud ***graphe = genererGraphe(p, &nNoeuds, solSup1, solSup2);
		Chemin **chemins = initialiserChemins(graphe[p->nBis], nNoeuds[p->nBis]);

		for (int j = 0; j < nNoeuds[p->nBis]; ++j) {
			TAS_ajouter(tas, chemins[j]);
		}

		ajouterSolutionLB(&solutionsLB, solSup1, &nbSolLB, &nbSolLBMax);
		ajouterSolutionLB(&solutionsLB, solSup2, &nbSolLB, &nbSolLBMax);

		while ((tas->n) && ((TAS_maximum(tas)->val >= LB))) {
			Chemin *chem = TAS_maximum(tas);
			TAS_retirerMax(tas);
			Solution *sol = creerSolution(p, chem);
			if ((sol->obj1 > solSup1->obj1) && (sol->obj2 > solSup2->obj2) && estEfficace(resultat, *nbSol, sol)) {
				ajouterSolution(&resultat, sol, nbSol, &nbSolMax);
				ajouterSolutionLB(&solutionsLB, sol, &nbSolLB, &nbSolLBMax);
				newLB = meilleureBorne(solutionsLB, nbSolLB, p);
				if (newLB > LB) {
					LB = newLB;
				}
			}
			genererSolutions(chem, tas, p);
		}

		desallouerGraphe(nNoeuds, graphe, p->nBis+1);
	}

	return resultat;
}

int main() {
	clock_t debut, fin;

	Probleme *p = genererProblemeGautier("instance100.DAT");
	//Probleme *p = genererProbleme("A4.DAT");
	int nbSol;

	debut = clock();
	Solution **nonDominated = trouverSolutions(p, &nbSol);
	fin = clock();


	// On trie les solutions efficaces selon l'ordre lexicographique
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


	/*typedef struct
	{
		int nbItem; // nombre d'objets dans le sac
		itype *p1; // coefficients premier objectif
		itype *w1; // coefficients première dimension
		itype *w2; // coefficients seconde dimension
		int omega1; // capacité première dimension
		int omega2; // capacité seconde dimension
		int maxZ1; // pour combo
	} donnees;*/

	//fixer01(p, 0, 0);

	printf("COCO\n");
}