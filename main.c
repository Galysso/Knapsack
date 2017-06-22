#include "src/graphe.h"
#include "src/tas.h"
#include "src/reoptimisation.h"
#include "src/knapglpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	//clock_t debutGraphe, finGraphe, debutRanking, finRanking;

	Probleme *p = genererProbleme("A4.DAT");

	unsigned int nSol, nMax;
	Solution **solSup = glpkSolutionsSupportees(p, &nSol, &nMax);

	printf("nSol=%d\n", nSol);

	for (int i = 0; i < nSol; ++i) {
		Solution *sol = solSup[i];
		printf("(%d,%d): ", sol->obj1, sol->obj2);
		for (int j = 0; j < p->n; ++j) {
			printf("%d", sol->var[j]);
		}
		printf("\n");
	}

	unsigned int *nNoeud;
	Noeud ***noeuds = genererGraphe(p, &nNoeud, solSup[0], solSup[1]);
	Tas *tas = TAS_initialiser(p->n*p->n);
	Chemin **chemins = initialiserChemins(noeuds[p->n], nNoeud[p->n]);

	for (int i = 0; i < nNoeud[p->n]; ++i) {
		TAS_ajouter(tas, chemins[i]);
	}

	while (tas->n) {
		//afficherChemin(TAS_maximum(tas), p->n);
		TAS_retirerMax(tas);
	}

	printf("COCO\n");
}