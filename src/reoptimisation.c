#include "reoptimisation.h"
#include "probleme.h"
#include "graphe.h"
#include "tas.h"

#include <stdlib.h>

void genererSolutions(Chemin *sol, Tas *tas, Probleme *prob) {
	/*if ((sol->obj1 == 2431) && (sol->obj2 == 3321)) {
		printf("LE VERT EST REOPTIMISE\n");
	}*/

	//printf("val=%d\n", sol->val);
	unsigned int solVal = sol->val;
	unsigned int solObj1 = sol->obj1;
	unsigned int solObj2 = sol->obj2;
	int n = prob->n;
	Noeud *noeud;
	Chemin *nouvelleSol, *solCourante;
	solCourante = sol;


	if (solCourante->nDeviation > 0) {
		int nDev = solCourante->nDeviation;
		unsigned int *deviations = (unsigned int *) malloc(nDev*sizeof(unsigned int));
		for (int i = 0; i < nDev; ++i) {
			deviations[i] = solCourante->deviation;
			solCourante = (Chemin*) solCourante->chemin;
		}

		noeud = (Noeud*) solCourante->chemin;
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
		noeud = (Noeud*) solCourante->chemin;
	}

	while (noeud->existeAlt) {
		if (noeud->precAlt) {
			nouvelleSol = (Chemin *) malloc(sizeof(Chemin));
			nouvelleSol->chemin = sol;
			if (noeud->val == noeud->precBest->val) {
				nouvelleSol->val = solVal - noeud->val + noeud->precAlt->val + prob->lambda1*prob->coefficients1[n-1] + prob->lambda2*prob->coefficients2[n-1];
				nouvelleSol->obj1 = solObj1 - noeud->obj1 + noeud->precAlt->obj1 + prob->coefficients1[n-1];
				nouvelleSol->obj2 = solObj2 - noeud->obj2 + noeud->precAlt->obj2 + prob->coefficients2[n-1];
			} else {
				nouvelleSol->val = solVal - noeud->val + noeud->precAlt->val;
				nouvelleSol->obj1 = solObj1 - noeud->obj1 + noeud->precAlt->obj1;
				nouvelleSol->obj2 = solObj2 - noeud->obj2 + noeud->precAlt->obj2;
			}
			nouvelleSol->deviation = n;
			//printf("n=%d\n", n);
			nouvelleSol->nDeviation = sol->nDeviation + 1;
			nouvelleSol->existeAlt = noeud->precAlt->existeAlt;
			/*if ((nouvelleSol->obj1 == 2431) && (nouvelleSol->obj2 == 3321)) {
				printf("LE VERT EST TROUVE PAR REOPTIMISATION ET AJOUTE\n");
			}
			if ((nouvelleSol->obj1 == 2428) && (nouvelleSol->obj2 ==3319)) {
				printf("LE ROUGE EST TROUVE PAR REOPTIMISATION ET AJOUTE\n");
			}*/
			TAS_ajouter(tas, nouvelleSol);
			//printf("Sol (%d,%d)  = %d\n", creerSolution(prob, nouvelleSol)->obj1, creerSolution(prob, nouvelleSol)->obj2, prob->lambda1*creerSolution(prob, nouvelleSol)->obj1 + prob->lambda2*creerSolution(prob, nouvelleSol)->obj2);
			//printf("Chem (%d,%d) = %d\n", nouvelleSol->obj1, nouvelleSol->obj2, nouvelleSol->val);
		}
		noeud = noeud->precBest;
		--n;
	}
}