#include "reoptimisation.h"
#include "probleme.h"
#include "graphe.h"
#include "tas.h"

#include <stdlib.h>

void genererReoptimisations(Chemin *chem, Tas *tas, Probleme *prob) {
	unsigned int lambda1 = prob->lambda1;
	unsigned int lambda2 = prob->lambda2;
	int solVal = chem->val;
	int n = prob->n;
	Noeud *noeud;
	Chemin *nouveauChemin, *cheminCourant;
	cheminCourant = chem;

	if (cheminCourant->deviation) {
		int nDev = cheminCourant->nDeviation;
		unsigned int *deviations = (unsigned int *) malloc(nDev*sizeof(unsigned int));
		for (int i = 0; i < nDev; ++i) {
			deviations[i] = cheminCourant->deviation;
			cheminCourant = (Chemin*) cheminCourant->chemin;
		}

		noeud = (Noeud*) cheminCourant->chemin;
		--nDev;
		while (nDev >= 0) {
			while (n > deviations[nDev]) {
				noeud = noeud->precBest;
				--n;
			}
			noeud = noeud->precAlt;
			--n;
			--nDev;
		}
	} else {
		noeud = (Noeud*) cheminCourant->chemin;
	}

	while (noeud->existeAlt) {
		if (noeud->precAlt) {
			nouveauChemin = (Chemin *) malloc(sizeof(Chemin));
			nouveauChemin->chemin = chem;
			if (noeud->val == noeud->precBest->val) {
				nouveauChemin->val = solVal - noeud->precBest->val + noeud->precAlt->val + lambda1*prob->coefficients1[n-1] +lambda2*prob->coefficients2[n-1];
			} else {
				nouveauChemin->val = solVal - noeud->/*precBest->*/val + noeud->precAlt->val;// - prob->coefficients1[n-1];
			}
			nouveauChemin->deviation = n;
			nouveauChemin->nDeviation = chem->nDeviation + 1;
			nouveauChemin->existeAlt = noeud->precAlt->existeAlt;
			TAS_ajouter(tas, nouveauChemin);
		}
		noeud = noeud->precBest;
		--n;
	}
}