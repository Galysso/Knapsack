#include "src/graphe.h"
#include "src/tas.h"
#include "src/reoptimisation.h"
#include "src/knapglpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	clock_t debutGraphe, finGraphe, debutRanking, finRanking;

	Probleme *p = genererProbleme("jouet.DAT");
	unsigned int *n;

	debutGraphe = clock();
	Noeud ***noeuds = genererGraphe(p, &n, 1, 0);		// tableau 2D des noeuds
	Chemin **sols = initialiserSolutions(noeuds[p->n], n[p->n]);
	finGraphe = clock();

	Tas *tas = TAS_initialiser(3*p->n);
	for (int i = 0; i < n[p->n]; ++i) {
		TAS_ajouter(tas, sols[i]);
	}

	debutRanking = clock();
	int cpt = 0;
	while (tas->n > 0) {
		++cpt;
		printf("%d: ", cpt);
		genererSolutions(TAS_maximum(tas), tas, p);
		afficherSolution(TAS_maximum(tas), p->n);
		/*Solution *sol = creerSolution(p, TAS_maximum(tas));
		printf("obj1=%d obj2=%d poids1=%d poids2=%d\n", sol->obj1, sol->obj2, sol->poids1, sol->poids2);
		for (int i = 0; i < p->n; ++i) {
			printf("%d", sol->var[i]);
		}
		printf("\n");*/
		TAS_retirerMax(tas);
	}
	finRanking = clock();

	printf("nombre de solutions : %d\n", cpt);
	printf("Génération du graphe : %fs\n", ((double) (1000*(finGraphe-debutGraphe)/CLOCKS_PER_SEC))/1000);
	printf("Calcul du ranking : %fs\n", ((double) (1000*(finRanking-debutRanking)/CLOCKS_PER_SEC))/1000);


	unsigned int nSol;
	Solution **solSup = glpkSolutionsSupportees(p, &nSol);

	printf("nSol=%d\n", nSol);

	for (int i = 0; i < nSol; ++i) {
		Solution *sol = solSup[i];
		printf("obj1=%d\nobj2=%d\npoids1=%d\npoids2=%d\n", sol->obj1, sol->obj2, sol->poids1, sol->poids2);
		for (int j = 0; j < p->n; ++j) {
			printf("%d", sol->var[j]);
		}
		printf("\n");
	}

	printf("COCO\n");
}