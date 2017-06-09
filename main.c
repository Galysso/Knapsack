#include "src/graphe.h"
#include "src/tas.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	srand(5);

	probleme *p = genererProblemeTest();

	for (int i = 0; i < p->n; ++i) {
		printf("%d \t %d\n", p->coefficients[i], p->poids[i]);
	}

	unsigned int n;
	noeud ***noeuds = genererGraphe(p, &n);

	afficherGraphe(noeuds[0][0], 0);
	printf("\n");

	afficherGraphe(noeuds[1][0], 1);
	printf("\n");
	afficherGraphe(noeuds[1][1], 1);
	printf("\n");

	afficherGraphe(noeuds[2][0], 2);
	printf("\n");
	afficherGraphe(noeuds[2][1], 2);
	printf("\n");
	afficherGraphe(noeuds[2][2], 2);
	printf("\n");

	afficherGraphe(noeuds[3][0], 3);
	printf("\n");
	afficherGraphe(noeuds[3][1], 3);
	printf("\n");
	afficherGraphe(noeuds[3][2], 3);
	printf("\n");
	afficherGraphe(noeuds[3][3], 3);
	printf("\n");

	afficherGraphe(noeuds[4][0], 4);
	printf("\n");
	afficherGraphe(noeuds[4][1], 4);
	printf("\n");
	afficherGraphe(noeuds[4][2], 4);
	printf("\n");
	afficherGraphe(noeuds[4][3], 4);
	printf("\n");
	afficherGraphe(noeuds[4][4], 4);
	printf("\n");
	afficherGraphe(noeuds[4][5], 4);
	printf("\n");

	printf("COCO\n");
}