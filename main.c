#include "src/graphe.h"
#include "src/tas.h"
#include "src/reoptimisation.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	clock_t debutGraphe, finGraphe, debutRanking, finRanking;

	Probleme *p = genererProbleme("A1.DAT");
	unsigned int *n;

	debutGraphe = clock();
	Noeud ***noeuds = genererGraphe(p, &n);		// tableau 2D des noeuds
	Solution **sols = initialiserSolutions(noeuds[p->n], n[p->n]);
	finGraphe = clock();

	Tas *tas = TAS_initialiser(3*p->n);
	for (int i = 0; i < n[p->n]; ++i) {
		TAS_ajouter(tas, sols[i]);
	}

	debutRanking = clock();
	int cpt = 1;
	while (tas->n > 0) {
		//printf("%d: ", cpt);
		genererSolutions(TAS_maximum(tas), tas, p);
		//afficherSolution(TAS_maximum(tas), p->n);
		TAS_retirerMax(tas);
		++cpt;
	}
	finRanking = clock();

	printf("nombre de solutions : %d\n", cpt);
	printf("Génération du graphe : %fs\n", ((double) (1000*(finGraphe-debutGraphe)/CLOCKS_PER_SEC))/1000);	
	printf("Calcul du ranking : %fs\n", ((double) (1000*(finRanking-debutRanking)/CLOCKS_PER_SEC))/1000);	

	printf("COCO\n");
}