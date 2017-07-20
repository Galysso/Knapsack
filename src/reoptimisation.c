#include "reoptimisation.h"
#include "probleme.h"
#include "graphe.h"
#include "tas.h"

#include <stdlib.h>

// Optimisation possible : stockage du noeud le plus loin du chemin
// (plus besoin de parcourir jusqu'à lui et d'enregistrer les déviations)

// Génère les fils d'un chemin pour le ranking
void genererSolutions(Chemin *chem, Tas *tas, Probleme *prob) {
	// On cherche le premier noeud possédant un chemin alternatif
	int solVal = chem->val;
	int solObj1 = chem->p1;
	int solObj2 = chem->p2;
	int n = prob->nBis;
	Noeud *noeud;
	Chemin *nouveauChem, *chemCourant;
	chemCourant = chem;

	// Si le chemin est déjà le dérivé d'un autre
	// Alors on enregistre dans un vecteur les déviations précédentes
	if (chemCourant->nDeviation > 0) {
		int nDev = chemCourant->nDeviation;
		int *deviations = (int *) malloc(nDev*sizeof(int));
		for (int i = 0; i < nDev; ++i) {
			deviations[i] = chemCourant->deviation;
			chemCourant = (Chemin*) chemCourant->chemin;
		}

		noeud = (Noeud*) chemCourant->chemin;
		--nDev;
		// Tant qu'il reste des déviations
		while (nDev >= 0) {
			// Tant que la prochaine déviation n'est pas atteinte
			while (n > deviations[nDev]) {
				// On parcourt le meilleur chemin
				noeud = noeud->precBest;
				--n;
			}
			// Une fois la déviation atteinte, on prend le chemin alternatif
			noeud = noeud->precAlt;
			--n;
			--nDev;
		}
	// Sinon on commence à partir du premier noeud
	} else {
		noeud = (Noeud*) chemCourant->chemin;
	}

	// Tant qu'il existe un chemin alternatif
	while (noeud->existeAlt) {
		int indN = prob->indVar[n-1];
		// Si le noeud possède un chemin alternatif
		if (noeud->precAlt) {
			// Alors on créé un nouveau chemin
			nouveauChem = (Chemin *) malloc(sizeof(Chemin));
			nouveauChem->chemin = chem;
			// Conditionnelle pour savoir si l'objet considéré est ajouté ou non dans le meilleur chemin
			if (noeud->val == noeud->precBest->val) {
				nouveauChem->val = solVal - noeud->val + noeud->precAlt->val + prob->lambda1*prob->profits1[indN] + prob->lambda2*prob->profits2[indN];
				nouveauChem->p1 = solObj1 - noeud->p1 + noeud->precAlt->p1 + prob->profits1[indN];
				nouveauChem->p2 = solObj2 - noeud->p2 + noeud->precAlt->p2 + prob->profits2[indN];
			} else {
				nouveauChem->val = solVal - noeud->val + noeud->precAlt->val;
				nouveauChem->p1 = solObj1 - noeud->p1 + noeud->precAlt->p1;
				nouveauChem->p2 = solObj2 - noeud->p2 + noeud->precAlt->p2;
			}
			nouveauChem->deviation = n;
			nouveauChem->nDeviation = chem->nDeviation + 1;
			nouveauChem->existeAlt = noeud->precAlt->existeAlt;
			// On ajoute le nouveau chemin dans le tas
			TAS_ajouter(tas, nouveauChem);
		}
		// On continue avec le noeud suivant
		noeud = noeud->precBest;
		--n;
	}
}