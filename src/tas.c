#include "tas.h"
#include "graphe.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


void TAS_initialiser(Tas *tas, unsigned int taille) {
	tas->tab = malloc(taille*sizeof(noeud *));
	tas->taille = taille;
	tas->n = 0;
}

void TAS_ajouter(Tas *tas, noeud *val) {
	if (tas->n == tas->taille) {
		tas->taille = 2*tas->taille;
		tas->tab = realloc(tas->tab, tas->taille*sizeof(noeud *));
	}
	unsigned int indP;	// l'indice du père
	unsigned int indC;	// l'indice de parcours
	noeud *tmp;			// valeur temporaire pour échange
	noeud **tab = tas->tab;
	tab[tas->n] = val;	// on insère la valeur à la fin du tableau
	indC = tas->n;
	indP = (indC-1)/2;	// on récupère l'indice du père (= indC si c'est la racine)

	while ((indC > 0) && (tab[indC]->val > tab[indP]->val)) {	// tant que son père est plus petit
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
	noeud *tmp;				// valeur temporaire pour échange
	bool fini = false;
	tas->n = tas->n - 1;	// il y a un élément de moins dans le tas
	int n = tas->n;
	noeud **tab = tas->tab;
	tab[ind] = tas->tab[n];	// on remplace l'élément à supprimer par le dernier du tas	

	do {//TAS_afficher(tas);
		indFG = 2*ind + 1;
		indFD = 2*ind + 2;
		//printf("ind= %d indFG=%d indFD=%d\n", ind, indFG, indFD);
		if (indFD < n) {
			if (tab[indFD]->val > tab[indFG]->val) {
				if (tab[indFD]->val > tab[ind]->val) {
					tmp = tab[indFD];
					tab[indFD] = tab[ind];
					tab[ind] = tmp;
					ind = indFD;
				} else {
					fini = true;
				}
			} else {
				if (tab[indFG]->val > tab[ind]->val) {
					tmp = tab[indFG];
					tab[indFG] = tab[ind];
					tab[ind] = tmp;
					ind = indFG;
				} else {
					fini = true;
				}
			}
		} else if ((indFG < n) && (tab[indFG]->val > tab[ind]->val)) {
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
	noeud **tab = tas->tab;
	int n = tas->n;
	int sautLigne = 1;
	int curseur = 0;
	printf("n=%d \n|", n);
	for (int i = 0; i < n; ++i) {
		if (curseur == sautLigne) {
			printf("\n|%d|", tab[i]->val);
			sautLigne = 2*sautLigne;
			curseur = 1;
		} else {
			printf("%d|", tab[i]->val);
			++curseur;
		}
	}
	printf("\n");
}