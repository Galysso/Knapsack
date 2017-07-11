#include "knapglpk.h"
#include "probleme.h"

#include <stdlib.h>
#include <glpk.h>
#include <stdbool.h>
#include <assert.h>

Solution **glpkSolutionsSupportees(Probleme *prob, int *nSol, int *nMax) {
	*nMax = prob->n*prob->n;
	Solution **sols = (Solution **) malloc(*nMax*sizeof(Solution *));
	Solution *sol;
	*nSol = 0;

	glp_term_out(0);

	int n = prob->n;
	int *ia, *ja;
	double *ar;
	ia = (int *) malloc((1+2*n)*sizeof(int));
	ja = (int *) malloc((1+2*n)*sizeof(int));
	ar = (double *) malloc((1+2*n)*sizeof(double));

	glp_prob *glpProb = glp_create_prob();
	glp_set_obj_dir(glpProb, GLP_MAX);
	glp_add_rows(glpProb, 2);
	glp_set_row_bnds(glpProb, 1, GLP_UP, 0.0, prob->capacite1);
	glp_set_row_bnds(glpProb, 2, GLP_UP, 0.0, prob->capacite2);
	glp_add_cols(glpProb, n);

	int cpt = 1;
	for (int i = 0; i < n; ++i) {
		glp_set_col_kind(glpProb, i+1, GLP_BV);
		glp_set_obj_coef(glpProb, i+1, prob->coefficients1[i]);
		ia[cpt] = 1;
		ja[cpt] = i+1;
		ar[cpt] = prob->poids1[i];
		++cpt; 
	}

	for (int i = 0; i < n; ++i) {
		ia[cpt] = 2;
		ja[cpt] = i+1;
		ar[cpt] = prob->poids2[i];
		++cpt;
	}

	glp_load_matrix(glpProb, 2*n, ia, ja, ar);
	//glp_write_lp(glpProb, NULL, "modelisation");
	glp_simplex(glpProb, NULL);
	glp_intopt(glpProb, NULL);


	if (*nSol == *nMax) {
		*nMax = 2*(*nMax);
		sols = (Solution **) realloc(sols, *nMax*sizeof(Solution *));
	}
	*nSol = *nSol + 1;
	sol = (Solution *) malloc(sizeof(Solution));
	sol->var = (bool *) malloc(n*sizeof(bool));
	sol->obj1 = (int) glp_mip_obj_val(glpProb);
	sol->obj2 = 0;
	sol->poids1 = 0;
	sol->poids2 = 0;
	for (int i = 0; i < n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			sol->var[i] = true;
			sol->obj2 = sol->obj2 + prob->coefficients2[i];
			sol->poids1 = sol->poids1 + prob->poids1[i];
			sol->poids2 = sol->poids2 + prob->poids2[i];
		} else {
			sol->var[i] = false;
		}
	}
	sols[0] = sol;

	for (int i = 0; i < n; ++i) {
		glp_set_obj_coef(glpProb, i+1, prob->coefficients2[i]);
	}

	//glp_write_lp(glpProb, NULL, "modelisation");
	glp_simplex(glpProb, NULL);
	glp_intopt(glpProb, NULL);

	if (*nSol == *nMax) {
		*nMax = 2*(*nMax);
		sols = (Solution **) realloc(sols, *nMax*sizeof(Solution *));
	}
	*nSol = *nSol + 1;
	sol = (Solution *) malloc(sizeof(Solution));
	sol->var = (bool *) malloc(n*sizeof(bool));
	sol->obj1 = 0;
	sol->obj2 = (int) glp_mip_obj_val(glpProb);
	sol->poids1 = 0;
	sol->poids2 = 0;
	for (int i = 0; i < n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			sol->var[i] = true;
			sol->obj1 = sol->obj1 + prob->coefficients1[i];
			sol->poids1 = sol->poids1 + prob->poids1[i];
			sol->poids2 = sol->poids2 + prob->poids2[i];
		} else {
			sol->var[i] = false;
		}
	}
	sols[1] = sol;

	if ((sols[1]->obj1 < sols[0]->obj1) && (sols[1]->obj2 > sols[0]->obj2)) {
		glpkDichotomieSupportees(prob, glpProb, &sols, nMax, nSol, sols[1], sols[0]);
	}

	// on trie les solutions selon l'ordre lexicographique
	bool changement;
	do {
		changement = false;
		for (int i = 1; i < *nSol; ++i) {
			if (sols[i-1]->obj2 < sols[i]->obj2) {
				sol = sols[i];
				sols[i] = sols[i-1];
				sols[i-1] = sol;
				changement = true;
			}
		}
	} while (changement);

	return sols;
}


void glpkDichotomieSupportees(Probleme *prob, glp_prob *glpProb, Solution ***sols, int *nMax, int *nSol, Solution *sol1, Solution *sol2) {
	int lambda1, lambda2;
	lambda1 = sol1->obj2 - sol2->obj2;
	lambda2 = sol2->obj1 - sol1->obj1;

	for (int i = 0; i < prob->n; ++i) {
		glp_set_obj_coef(glpProb, i+1, lambda1*prob->coefficients1[i] + lambda2*prob->coefficients2[i]);
	}
	glp_simplex(glpProb, NULL);
	glp_intopt(glpProb, NULL);

	if (*nSol == *nMax) {
		*nMax = 2*(*nMax);
		*sols = (Solution **) realloc(*sols, *nMax*sizeof(Solution *));
	}
	Solution *nouvelleSol = (Solution *) malloc(sizeof(Solution));
	nouvelleSol->var = (bool *) malloc(prob->n*sizeof(bool));
	nouvelleSol->obj1 = 0;
	nouvelleSol->obj2 = 0;
	nouvelleSol->poids1 = 0;
	nouvelleSol->poids2 = 0;
	for (int i = 0; i < prob->n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			nouvelleSol->var[i] = true;
			nouvelleSol->obj1 = nouvelleSol->obj1 + prob->coefficients1[i];
			nouvelleSol->obj2 = nouvelleSol->obj2 + prob->coefficients2[i];
			nouvelleSol->poids1 = nouvelleSol->poids1 + prob->poids1[i];
			nouvelleSol->poids2 = nouvelleSol->poids2 + prob->poids2[i];
		} else {
			nouvelleSol->var[i] = false;
		}
	}

	if ((lambda1*nouvelleSol->obj1 + lambda2*nouvelleSol->obj2) > (lambda1*sol1->obj1 + lambda2*sol1->obj2)) {
		(*sols)[*nSol] = nouvelleSol;
		*nSol = *nSol + 1;
		glpkDichotomieSupportees(prob, glpProb, sols, nMax, nSol, sol1, nouvelleSol);
		glpkDichotomieSupportees(prob, glpProb, sols, nMax, nSol, nouvelleSol, sol2);
	}
}