#include "reoptimisation.h"
#include "probleme.h"
#include "graphe.h"
#include "tas.h"

#include <stdlib.h>

void genererSolutions(Solution *sol, Tas *tas, Probleme *prob) {
	int solVal = sol->val;
	int n = prob->n;
	Noeud *noeud;
	Solution *nouvelleSol, *solCourante;
	solCourante = sol;


	if (solCourante->deviation) {
		int nDev = solCourante->nDeviation;
		unsigned int *deviations = (unsigned int *) malloc(nDev*sizeof(unsigned int));
		for (int i = 0; i < nDev; ++i) {
			deviations[i] = solCourante->deviation;
			solCourante = (Solution*) solCourante->solution;
		}

		noeud = (Noeud*) solCourante->solution;
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
		noeud = (Noeud*) solCourante->solution;
	}

	while (noeud->existeAlt) {
		if (noeud->precAlt) {
			nouvelleSol = (Solution *) malloc(sizeof(Solution));
			nouvelleSol->solution = sol;
			if (noeud->val == noeud->precBest->val) {
				nouvelleSol->val = solVal - noeud->precBest->val + noeud->precAlt->val + prob->coefficients1[n-1];
			} else {
				nouvelleSol->val = solVal - noeud->precBest->val + noeud->precAlt->val - prob->coefficients1[n-1];
			}
			nouvelleSol->deviation = n;
			nouvelleSol->nDeviation = sol->nDeviation + 1;
			nouvelleSol->existeAlt = noeud->precAlt->existeAlt;
			TAS_ajouter(tas, nouvelleSol);
		}
		noeud = noeud->precBest;
		--n;
	}
}