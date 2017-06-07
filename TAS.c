#include "TAS.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void TAS_initialiser(Tas *tas, unsigned int taille) {
	tas->tab = malloc(taille*sizeof(int));
	tas->taille = taille;
	tas->n = 0;
}

void TAS_ajouter(Tas *tas, int val) {
	if (tas->n == tas->taille) {
		tas->taille = 2*tas->taille;
		tas->tab = realloc(tas->tab, tas->taille*sizeof(int));
	}
	unsigned int indP;	// l'indice du père
	unsigned int indC;	// l'indice de parcours
	int tmp;			// valeur temporaire pour échange
	int *tab = tas->tab;
	tab[tas->n] = val;	// on insère la valeur à la fin du tableau
	indC = tas->n;
	indP = (indC-1)/2;		// on récupère l'indice du père (= indC si c'est la racine)

	while ((indC > 0) && (tab[indC] > tab[indP])) {	// tant que son père est plus petit
		tmp = tab[indP];				// on échange le noeud avec son père
		tab[indP] = tab[indC];
		tab[indC] = tmp;
		indC = indP;
		indP = (indC-1)/2;
	}

	tas->n = tas->n + 1;// il y a un élément de plus dans le tas
}

void TAS_retirerMax(Tas *tas) {

}

void TAS_retirerInd(Tas *tas, int ind) {
	unsigned int indFG;		// l'indice du fils gauche
	unsigned int indFD;		// l'indice du fils droit
	int tmp;				// valeur temporaire pour échange
	bool fini = false;
	tas->n = tas->n - 1;	// il y a un élément de moins dans le tas
	int n = tas->n;
	int *tab = tas->tab;
	tab[ind] = tas->tab[n];	// on remplace l'élément à supprimer par le dernier du tas	

	do {//TAS_afficher(tas);
		indFG = 2*ind + 1;
		indFD = 2*ind + 2;
		//printf("ind= %d indFG=%d indFD=%d\n", ind, indFG, indFD);
		if (indFD < n) {
			if (tab[indFD] > tab[indFG]) {
				if (tab[indFD] > tab[ind]) {
					tmp = tab[indFD];
					tab[indFD] = tab[ind];
					tab[ind] = tmp;
					ind = indFD;
				} else {
					fini = true;
				}
			} else {
				if (tab[indFG] > tab[ind]) {
					tmp = tab[indFG];
					tab[indFG] = tab[ind];
					tab[ind] = tmp;
					ind = indFG;
				} else {
					fini = true;
				}
			}
		} else if ((indFG < n) && (tab[indFG] > tab[ind])) {
			tmp = tab[indFG];
			tab[indFG] = tab[ind];
			tab[ind] = tmp;
			ind = indFG;
		} else {
			fini = true;
		}
	} while (!fini);
}

void TAS_minimum(Tas *tas) {

}

void TAS_maximum(Tas *tas) {

}

void TAS_afficher(Tas *tas) {
	int *tab = tas->tab;
	int n = tas->n;
	printf("n=%d \n|", n);
	for (int i = 0; i < n; ++i) {
		printf("%d|", tab[i]);
	}
	printf("\n");
}

int main() {
	srand(5);
	Tas *tas = malloc(sizeof(Tas));

	TAS_initialiser(tas, 10);
	for (int i = 0; i < 20; ++i) {
		int val = rand()%50 - 25;
		//printf("ajout de : %d\n", val);
		TAS_ajouter(tas, val);
	}
	for (int i = 0; i < 10; ++i) {
		int val = rand()%tas->n;
		//printf("retrait de : %d\n", val);
		TAS_retirerInd(tas, val);
	}
	for (int i = 0; i < 20; ++i) {
		int val = rand()%50 - 25;
		//printf("ajout de : %d\n", val);
		TAS_ajouter(tas, val);
	}printf("COCO\n");
	for (int i = 0; i < 10; ++i) {
		int val = rand()%tas->n;
		//printf("ajout de : %d\n", val);
		TAS_retirerInd(tas, val);
	}
	for (int i = 0; i < 20; ++i) {
		int val = rand()%50 - 25;
		//printf("ajout de : %d\n", val);
		TAS_ajouter(tas, val);
	}

	TAS_afficher(tas);
	printf("TEST\n");
}