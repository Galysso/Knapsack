#include "src/graphe.h"
#include "src/tas.h"
#include "src/reoptimisation.h"
#include "src/knapglpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

bool estDomine(Solution **solutions, unsigned int obj1, unsigned int obj2, unsigned int debut, unsigned int fin) {
	while ((debut < fin) && ((solutions[debut]->obj1 < obj1) || (solutions[debut]->obj2 < obj2))) {
		++debut;
	}
	return (debut != fin);
}

void ranking(Probleme *p) {
	unsigned int nbSolSup, debutNb, nbSol, nbSolAlloc;
	unsigned int *nNoeud;
	unsigned int LB;
	Noeud ***graphe;
	Solution **solSup, **solutions;
	Tas *tas;

	solSup = glpkSolutionsSupportees(p, &nbSolSup);

	printf("SOLUTIONS SUPPORTEES\n");
	for (unsigned int i = 0; i < nbSolSup; ++i) {
		Solution *sol = solSup[i];
		printf("(%d,%d): ", sol->obj1, sol->obj2);
		for (int j = 0; j < p->n; ++j) {
			printf("%d", sol->var[j]);
		}
		printf("\n");
	}

	nbSolAlloc = 4*nbSolSup;
	nbSol = nbSolSup;
	solutions = (Solution **) malloc(nbSolAlloc*sizeof(Solution*));
	for (int i = 0; i < nbSolSup; ++i) {
		solutions[i] = solSup[i];
	}

	tas = TAS_initialiser(p->n*p->n);

	for (int j = 1; j < nbSolSup; ++j) {
		debutNb = nbSol;
		unsigned int lambda1 = solSup[j-1]->obj2 - solSup[j]->obj2;
		unsigned int lambda2 = solSup[j]->obj1 - solSup[j-1]->obj1;
		p->lambda1 = lambda1;
		p->lambda2 = lambda2;
		LB = p->lambda1*(solSup[j-1]->obj1+1) + p->lambda2*(solSup[j]->obj2+1);

		graphe = genererGraphe(p, &nNoeud, solSup[j-1], solSup[j]);

		Chemin **chemins = initialiserChemins(graphe[p->n], nNoeud[p->n]);
		//printf("\nCHEMINS\n");
		for (int i = 0; i < nNoeud[p->n]; ++i) {
			/*afficherGraphe(graphe[p->n][i], p->n);
			afficherChemin(chemins[i], p->n);
			printf("\n\n");*/
			TAS_ajouter(tas, chemins[i]);
		}
		unsigned int toto = 0;
		while ((tas->n) && (TAS_maximum(tas)->val > LB)) {
			Solution *sol = creerSolution(p, TAS_maximum(tas));
			//printf("lambda1=%d lambda2=%d\n", lambda1, lambda2);
			//printf("LB=%d\n",LB);
			//Solution *sol = solutions[nbSol-1];
			/*printf("(%d,%d)[%d]: ", sol->obj1, sol->obj2, lambda1*sol->obj1 + lambda2*sol->obj2);
			for (int j = 0; j < p->n; ++j) {
				printf("%d", sol->var[j]);
			}
			printf("\n");*/
			if (!estDomine(solutions, sol->obj1, sol->obj2, debutNb, nbSol)) {			// si non dominée
				//printf("NON DOMINEE\n");
				if (nbSol == nbSolAlloc) {
					nbSolAlloc = 2*nbSolAlloc;
					solutions = (Solution **) realloc(solutions, nbSolAlloc*sizeof(Solution*));
				}
				solutions[nbSol] = sol;
				++nbSol;
				if (LB < lambda1*(solSup[j-1]->obj1+1) + (lambda2*(sol->obj2+1))) {
					//printf("Mise à jour 1\n");
					LB = lambda1*(solSup[j-1]->obj1+1) + (lambda2*(sol->obj2+1));
				}
				if (LB < lambda2*(solSup[j]->obj2+1) + (lambda1*(sol->obj1+1))) {
					//printf("Mise à jour 2\n");
					LB = lambda2*(solSup[j]->obj2+1) + (lambda1*(sol->obj1+1));
				}
			} else {
				//printf("DOMINEE\n");
				//free(sol);
			}
			//printf("nbSol: %d\n",nbSol);
			genererReoptimisations(TAS_maximum(tas), tas, p);
			TAS_retirerMax(tas);
			//TAS_afficher(tas);
			++toto;
			printf("nb=%d\n",toto);
			//getchar();
		}
		tas->n = 0;
	}

	printf("\nTOUTES LES SOLUTIONSs\n");
	for (unsigned int i = 0; i < nbSol; ++i) {
		Solution *sol = solutions[i];
		printf("%d,%d", sol->obj1, sol->obj2);
		/*for (int j = 0; j < p->n; ++j) {
			printf("%d", sol->var[j]);
		}*/
		printf("\n");
	}
	printf("nombre de solution : %d\n", nbSol);
}

int main() {
	//clock_t debutGraphe, finGraphe, debutRanking, finRanking;

	Probleme *p = genererProblemeGautier("instance100.DAT");
	ranking(p);

	/*unsigned int *nNoeud;
	Noeud ***noeuds = genererGraphe(p, &nNoeud, solSup[0], solSup[1]);
	Tas *tas = TAS_initialiser(p->n*p->n);
	Chemin **chemins = initialiserChemins(noeuds[p->n], nNoeud[p->n]);

	for (int i = 0; i < nNoeud[p->n]; ++i) {
		TAS_ajouter(tas, chemins[i]);
	}*/

	printf("COCO\n");
}