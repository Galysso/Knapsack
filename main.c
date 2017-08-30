#include "src/graphe.h"
#include "src/tas.h"
#include "src/reoptimisation.h"
#include "src/knapglpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <assert.h>

void plotAll(ListeSol *lSolSup, ListeSol *lSol);
void plotSup(Solution **solSup, int nbSolSup);

void plotAll(ListeSol *lSolSup, ListeSol *lSol) {
    FILE *plotNonDom = fopen("points non dominés", "w");
    FILE *plotSolSup = fopen("solutions supportées", "w");
    FILE *gnuplotPipe = popen ("gnuplot -persistent", "w");
	fprintf(plotSolSup, "%d %d\n", lSolSup->solutions[0]->p1, lSolSup->solutions[0]->p2);
	for (int i = 1; i < lSolSup->nbSol; ++i) {
		fprintf(plotSolSup, "%d %d\n", lSolSup->solutions[i-1]->p1, lSolSup->solutions[i]->p2);
		fprintf(plotSolSup, "%d %d\n", lSolSup->solutions[i]->p1, lSolSup->solutions[i]->p2);
	}
	for (int i = lSolSup->nbSol-2; i >= 0; --i) {
		fprintf(plotSolSup, "%d %d\n", lSolSup->solutions[i]->p1, lSolSup->solutions[i]->p2);
	}
	for (int i=0; i < lSol->nbSol; i++) {
		fprintf(plotNonDom, "%d %d \n", lSol->solutions[i]->p1, lSol->solutions[i]->p2); //Write the data to a temporary file
	}

	fclose(plotNonDom);
	fclose(plotSolSup);
	fprintf(gnuplotPipe, "set title \"Points non dominés\"\n"); //Send commands to gnuplot one by one.
	fflush(gnuplotPipe);
	fprintf(gnuplotPipe, "plot 'solutions supportées' using 1:2 with lines, 'points non dominés' using 1:2\n");
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
void ajouterSolutionLB(ListeSol *lSolLB, Solution *sol) {
	int min, ind, max;
	int nbSol = lSolLB->nbSol;
	int nbMax = lSolLB->nbMax;

	if (nbSol == nbMax) {
		nbMax = 2*nbMax;
		lSolLB->nbMax = nbMax;
		lSolLB->solutions = (Solution **) realloc(lSolLB->solutions, nbMax*sizeof(Solution *));
	}

	Solution **solutions = lSolLB->solutions;
	Solution *solLB;

	min = 0;
	max = nbSol;
	ind = max/2;

	while (ind != min) {
		solLB = solutions[ind];
		if (sol->p2 < solLB->p2) {
			min = ind;
		} else {
			max = ind;
		}
		ind = (min+max)/2;
	}

	if ((ind != nbSol) && (solutions[ind]->p2 > sol->p2)) {
		++ind;
	}

	for (int i = nbSol; i > ind; --i) {
		solutions[i] = solutions[i-1];
	}

	lSolLB->solutions[ind] = sol;
	lSolLB->nbSol = nbSol+1;
}

// Fuite de mémoire : les chemins ne sont pas désalloués
ListeSol *trouverSolutions(Probleme *p) {
	int lambda1, lambda2;		// poids de la somme pondérée
	int LB, newLB;				// borne inférieure et plus petite borne inférieure actuelle
	int *nNoeuds;				// nombre de noeuds pour chaque colonne du graphe
	ListeSol *lSolSup; 			// solutions supportées
	ListeSol *lSolLB; 			// solutions trouvées dans le triangle pour la borne
	ListeSol *resultat;				// solutions efficaces trouvées

	ListeSol *lSolAdm;

	resultat = initListeSol(p->n);
	lSolLB = initListeSol(p->n);
	lSolSup = glpkSolutionsSupportees(p);
	Solution **solSup = lSolSup->solutions;

	// On ajoute à liste des solutions les points supportés (attention aux extremes)
	ajouterSolution(resultat, solSup[1]);
	ajouterSolution(resultat, solSup[lSolSup->nbSol-2]);
	if (estEfficace(resultat, solSup[0])) {
		ajouterSolution(resultat, solSup[0]);
	}
	if (estEfficace(resultat, solSup[lSolSup->nbSol-1])) {
		ajouterSolution(resultat, solSup[lSolSup->nbSol-1]);
	}
	for (int i = 2; i < lSolSup->nbSol-2; ++i) {
		ajouterSolution(resultat, solSup[i]);
	}

	// Une liste de solutions par triangle
	ListeSol **lSolPR = (ListeSol **) malloc((lSolSup->nbSol-1)*sizeof(ListeSol *));

	for (int i = 1; i < lSolSup->nbSol; ++i) {
		lSolPR[i-1] = initListeSol(lSolSup->nbSol);
		ajouterSolutionDom(lSolPR[i-1], solSup[i-1]);
		ajouterSolutionDom(lSolPR[i-1], solSup[i]);
	}

	// On réalise des paths relinking entre les solutions supportées
	for (int i = 1; i < lSolSup->nbSol; ++i) {
		p->lambda1 = solSup[i-1]->p2 - solSup[i]->p2;
		p->lambda2 = solSup[i]->p1 - solSup[i-1]->p1;
		p->solSup1 = solSup[i-1];
		p->solSup2 = solSup[i];

		trierIndvar(p);

		lSolAdm = pathRelinking(p, solSup[i-1], solSup[i]);
		// On ajoute les solutions du triangle dans les bons triangles
		
		for (int j = 0; j < lSolAdm->nbSol; ++j) {
			Solution *solPR = lSolAdm->solutions[j];
			bool trouve = false;
			int k = 1;
			while ((k < lSolSup->nbSol) && (!trouve)) {
				if (solPR->p2 > solSup[k]->p2) {
					if (solPR->p1 > solSup[k-1]->p1) {
						ajouterSolutionDom(lSolPR[k-1], solPR);
					}
					trouve = true;
				} else {
					++k;
				}
			}
		}
	}

	/*ListeSol *toutesSolPR = initListeSol(p->n);
	for (int i = 1; i < lSolSup->nbSol; ++i) {
		for (int j = 0; j < lSolPR[i-1]->nbSol; ++j) {
			ajouterSolutionDom(toutesSolPR, lSolPR[i-1]->solutions[j]);
		}
	}
	plotAll(lSolSup, toutesSolPR);*/

	// On trie les solutions du path relinking pour en calculer une borne
	for (int i = 0; i < lSolSup->nbSol-1; ++i) {
		bool changement;
		do {
			changement = false;
			for (int j = 1; j < lSolPR[i]->nbSol; ++j) {
				Solution *sol1 = lSolPR[i]->solutions[j-1];
				Solution *sol2 = lSolPR[i]->solutions[j];
				if (sol1->p1 > sol2->p1) {
					lSolPR[i]->solutions[j-1] = sol2;
					lSolPR[i]->solutions[j] = sol1;
					changement = true;
				}
			}
		} while (changement);
	}

	for (int i = 1; i < lSolSup->nbSol; ++i) {
		lSolLB->nbSol = 0;
		Solution *solSup1 = solSup[i-1];
		Solution *solSup2 = solSup[i];

		if ((solSup1->p1 < solSup2->p1 -1) && (solSup1->p2 > solSup2->p2 + 1)) {
			lambda1 = solSup1->p2 - solSup2->p2;
			lambda2 = solSup2->p1 - solSup1->p1;
			p->lambda1 = lambda1;
			p->lambda2 = lambda2;
			p->solSup1 = solSup1;
			p->solSup2 = solSup2;

			ajouterSolutionLB(lSolLB, solSup1);
			ajouterSolutionLB(lSolLB, solSup2);

			LB = meilleureBorne(lSolPR[i-1], p);
			p->LB = LB;

			/*for (int i = 0; i < p->n; ++i) {
				printf("i=%d\n", p->indVar[i]);
			}
			assert(i < 3);*/

			trierIndvar(p);

			/*for (int i = 0; i < p->n; ++i) {
				printf("i=%d\n", p->indVar[i]);
			}
			assert(i < 3);*/

			fixer01(p, solSup1->p1, solSup2->p2, lSolPR[i-1]);

			printf("triangle %d/%d\n", i, lSolSup->nbSol-1);
			if (p->nBis > 0) {
				Tas *tas = TAS_initialiser(p->nBis*p->nBis);
				Noeud ***graphe = genererGraphe(p, &nNoeuds, solSup1, solSup2);
				Chemin **chemins = initialiserChemins(graphe[p->nBis], nNoeuds[p->nBis]);

				for (int j = 0; j < nNoeuds[p->nBis]; ++j) {
					TAS_ajouter(tas, chemins[j]);
				}

				while ((tas->n) && ((TAS_maximum(tas)->val >= LB))) {
					Chemin *chem = TAS_maximum(tas);
					TAS_retirerMax(tas);
					Solution *sol = creerSolution(p, chem);
					if ((sol->p1 > solSup1->p1) && (sol->p2 > solSup2->p2) && estEfficace(resultat, sol)) {
						ajouterSolution(resultat, sol);
						ajouterSolutionLB(lSolLB, sol);
						newLB = meilleureBorne(lSolLB, p);
						if (newLB > LB) {
							LB = newLB;
							p->LB = newLB;
						}
					}
					genererSolutions(chem, tas, p);
				}
				desallouerGraphe(nNoeuds, graphe, p->nBis+1);
			}
		}
	}

	//printf("COCO\n");
	//plotAll(solPathR, nbSolPath, solSup, nbSup);

	for (int i = 0; i < lSolSup->nbSol-1; ++i) {
		for (int j = 0; j < lSolPR[i]->nbSol; ++j) {
			ajouterSolutionDom(resultat, lSolPR[i]->solutions[j]);
		}
	}


	plotAll(lSolSup, resultat);

	return resultat;
}

int main() {
	clock_t debut, fin;

	//Probleme *p = genererProblemeGautier("instance100.DAT");
	Probleme *p = genererProbleme("ZTL250.DAT");

	debut = clock();
	ListeSol *resultat = trouverSolutions(p);
	fin = clock();


	// On trie les solutions efficaces selon l'ordre lexicographique
	bool changement;
	do {
		changement = false;
		for (int i = 1; i < resultat->nbSol; ++i) {
			if (resultat->solutions[i-1]->p2 < resultat->solutions[i]->p2) {
				Solution *sol = resultat->solutions[i];
				resultat->solutions[i] = resultat->solutions[i-1];
				resultat->solutions[i-1] = sol;
				changement = true;
			}
		}
	} while (changement);

	/*printf("%d solutions:\n", resultat->nbSol);
	for (int i = 0; i < resultat->nbSol; ++i) {
		printf("(%d, %d):", resultat->solutions[i]->p1, resultat->solutions[i]->p2);
		for (int j = 0; j < p->n; ++j) {
			printf("%d", (resultat->solutions[i])->var[j]);
		}
		printf("\n");
	}*/
	printf("%d solutions:\n", resultat->nbSol);
	printf("temps: %fs\n", (double) (fin-debut)/CLOCKS_PER_SEC);

	printf("COCO\n");

	//plot(resultat->solutions, nbSol);
}