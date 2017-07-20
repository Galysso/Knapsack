#include "src/graphe.h"
#include "src/tas.h"
#include "src/reoptimisation.h"
#include "src/knapglpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void plotAll(Solution **solutions, int nbSol, Solution **solSup, int nbSolSup) {
    FILE *plotNonDom = fopen("nonDominated.dat", "w");
    FILE *plotSolSup = fopen("solSupportees.dat", "w");
    FILE *gnuplotPipe = popen ("gnuplot -persistent", "w");
    for (int i=0; i < nbSol; i++) {
		fprintf(plotNonDom, "%d %d \n", solutions[i]->p1, solutions[i]->p2); //Write the data to a temporary file
	}
	fprintf(plotSolSup, "%d %d\n", solSup[0]->p1, solSup[0]->p2);
	for (int i = 1; i < nbSolSup; ++i) {
		fprintf(plotSolSup, "%d %d\n", solSup[i-1]->p1, solSup[i]->p2);
		fprintf(plotSolSup, "%d %d\n", solSup[i]->p1, solSup[i]->p2);
	}
	for (int i = nbSolSup-2; i >= 0; --i) {
		fprintf(plotSolSup, "%d %d\n", solSup[i]->p1, solSup[i]->p2);
	}

	fclose(plotNonDom);
	fclose(plotSolSup);
	fprintf(gnuplotPipe, "set title \"Points non dominés\"\n"); //Send commands to gnuplot one by one.
	fflush(gnuplotPipe);
	fprintf(gnuplotPipe, "plot 'nonDominated.dat' using 1:2, 'solSupportees.dat' using 1:2 with lines\n");
	fflush(gnuplotPipe);
}

void plotSup(Solution **solSup, int nbSolSup) {
    FILE *plotSolSup = fopen("solSupportees.dat", "w");
    FILE *gnuplotPipe = popen ("gnuplot -persistent", "w");
	fprintf(plotSolSup, "%d %d\n", solSup[0]->p1, solSup[0]->p2);
	for (int i = 1; i < nbSolSup; ++i) {
		fprintf(plotSolSup, "%d %d\n", solSup[i-1]->p1, solSup[i]->p2);
		fprintf(plotSolSup, "%d %d\n", solSup[i]->p1, solSup[i]->p2);
	}
	for (int i = nbSolSup-2; i >= 0; --i) {
		fprintf(plotSolSup, "%d %d\n", solSup[i]->p1, solSup[i]->p2);
	}

	fclose(plotSolSup);
	fprintf(gnuplotPipe, "set title \"Points non dominés\"\n"); //Send commands to gnuplot one by one.
	fflush(gnuplotPipe);
	fprintf(gnuplotPipe, "plot 'solSupportees.dat' using 1:2 with lines\n");
	fflush(gnuplotPipe);
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
		if (sol->p2 < solLB->p2) {
			min = ind;
		} else {
			max = ind;
		}
		ind = (min+max)/2;
	}

	if ((ind != *nbSol) && ((*solutionsLB)[ind]->p2 > sol->p2)) {
		++ind;
	}

	for (int i = *nbSol; i > ind; --i) {
		(*solutionsLB)[i] = (*solutionsLB)[i-1];
	}
	(*solutionsLB)[ind] = sol;
	*nbSol = *nbSol + 1;
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

	//plotSup(solSup, nbSup);

	ajouterSolution(&resultat, solSup[1], nbSol, &nbSolMax);
	ajouterSolution(&resultat, solSup[nbSup-2], nbSol, &nbSolMax);
	if (estEfficace(resultat, *nbSol, solSup[0])) {
		ajouterSolution(&resultat, solSup[0], nbSol, &nbSolMax);
	}
	if (estEfficace(resultat, *nbSol, solSup[nbSup-1])) {
		ajouterSolution(&resultat, solSup[nbSup-1], nbSol, &nbSolMax);
	}
	for (int i = 2; i < nbSup-2; ++i) {
		ajouterSolution(&resultat, solSup[i], nbSol, &nbSolMax);
	}

	Solution **solAdm;
	int nbSolAdm;
	for (int i = 1; i < nbSup; ++i) {
		Solution *solSup1 = solSup[i-1];
		Solution *solSup2 = solSup[i];
		if ((solSup1->p1 < solSup2->p1 -1) && (solSup1->p2 > solSup2->p2 + 1)) {
			nbSolLB = 0;

			lambda1 = solSup1->p2 - solSup2->p2;
			lambda2 = solSup2->p1 - solSup1->p1;

			p->lambda1 = lambda1;
			p->lambda2 = lambda2;

			LB = lambda1*(solSup1->p1+1) + lambda2*(solSup2->p2+1);
			p->LB = LB;
			solAdm = fixer01(p, solSup1->p1, solSup2->p2, &nbSolAdm);

			if (p->nBis > 0) {
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
					if ((sol->p1 > solSup1->p1) && (sol->p2 > solSup2->p2) && estEfficace(resultat, *nbSol, sol)) {
						ajouterSolution(&resultat, sol, nbSol, &nbSolMax);
						ajouterSolutionLB(&solutionsLB, sol, &nbSolLB, &nbSolLBMax);
						newLB = meilleureBorne(solutionsLB, nbSolLB, p);
						if (newLB > LB) {
							LB = newLB;
							p->LB = newLB;
						}
					}
					genererSolutions(chem, tas, p);
				}

				desallouerGraphe(nNoeuds, graphe, p->nBis+1);
			}
			//printf("COCO 1\n");
			for (int j = 0; j < nbSolAdm; ++j) {
				//printf("COCO\n");
				//printf("(%d,%d)\n", solAdm[j]->p1, solAdm[j]->p2);
				if (estEfficace(resultat, *nbSol, solAdm[j])) {
					ajouterSolution(&resultat, solAdm[j], nbSol, &nbSolMax);
				}
			}
			//printf("COCO 2\n");
		}
	}

	//plotAll(resultat, *nbSol, solSup, nbSup);

	return resultat;
}

int main() {
	clock_t debut, fin;

	Probleme *p = genererProblemeGautier("instance100.DAT");
	//Probleme *p = genererProbleme("A1.DAT");
	int nbSol;

	debut = clock();
	Solution **nonDominated = trouverSolutions(p, &nbSol);
	fin = clock();


	// On trie les solutions efficaces selon l'ordre lexicographique
	bool changement;
	do {
		changement = false;
		for (int i = 1; i < nbSol; ++i) {
			if (nonDominated[i-1]->p2 < nonDominated[i]->p2) {
				Solution *sol = nonDominated[i];
				nonDominated[i] = nonDominated[i-1];
				nonDominated[i-1] = sol;
				changement = true;
			}
		}
	} while (changement);

	printf("%d solutions:\n", nbSol);
	for (int i = 0; i < nbSol; ++i) {
	printf("(%d, %d):", nonDominated[i]->p1, nonDominated[i]->p2);
		/*for (int j = 0; j < p->n; ++j) {
			printf("%d", nonDominated[i]->var[j]);
		}*/
		printf("\n");
	}
	printf("%d solutions:\n", nbSol);
	printf("temps: %fs\n", (double) (fin-debut)/CLOCKS_PER_SEC);

	printf("COCO\n");

	//plot(nonDominated, nbSol);
}