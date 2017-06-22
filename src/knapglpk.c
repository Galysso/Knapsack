#include "knapglpk.h"
#include "probleme.h"

#include <stdlib.h>
#include <glpk.h>
#include <stdbool.h>

Solution **glpkSolutionsSupportees(Probleme *prob, unsigned int *nSol) {
	unsigned int nMax = prob->n;
	Solution **sols = (Solution **) malloc(nMax*sizeof(Solution *));
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

	unsigned int cpt = 1;
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


	if (*nSol == nMax) {
		nMax = 2*nMax;
		sols = (Solution **) realloc(sols, nMax*sizeof(Solution *));
	}
	*nSol = *nSol + 1;
	sols[0] = (Solution *) malloc(sizeof(Solution));
	sols[0]->var = (bool *) malloc(n*sizeof(bool));
	sols[0]->obj1 = (unsigned int) glp_mip_obj_val(glpProb);
	sols[0]->obj2 = 0;
	sols[0]->poids1 = 0;
	sols[0]->poids2 = 0;
	for (int i = 0; i < n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			sols[0]->var[i] = true;
			sols[0]->obj2 = sols[0]->obj2 + prob->coefficients2[i];
			sols[0]->poids1 = sols[0]->poids1 + prob->poids1[i];
			sols[0]->poids2 = sols[0]->poids2 + prob->poids2[i];
		} else {
			sols[0]->var[i] = false;
		}
	}

	for (int i = 0; i < n; ++i) {
		glp_set_obj_coef(glpProb, i+1, prob->coefficients2[i]);
	}

	//glp_write_lp(glpProb, NULL, "modelisation");
	glp_simplex(glpProb, NULL);
	glp_intopt(glpProb, NULL);

	if (*nSol == nMax) {
		nMax = 2*nMax;
		sols = (Solution **) realloc(sols, nMax*sizeof(Solution *));
	}
	*nSol = *nSol + 1;
	sols[1] = (Solution *) malloc(sizeof(Solution));
	sols[1]->var = (bool *) malloc(n*sizeof(bool));
	sols[1]->obj1 = (unsigned int) glp_mip_obj_val(glpProb);
	sols[1]->obj2 = 0;
	sols[1]->poids1 = 0;
	sols[1]->poids2 = 0;
	for (int i = 0; i < n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			sols[1]->var[i] = true;
			sols[1]->obj2= sols[1]->obj2 + prob->coefficients2[i];
			sols[1]->poids1 = sols[1]->poids1 + prob->poids1[i];
			sols[1]->poids2 = sols[1]->poids2 + prob->poids2[i];
		} else {
			sols[1]->var[i] = false;
		}
	}

	if ((sols[1]->obj1 < sols[0]->obj1) && (sols[1]->obj2 > sols[0]->obj2)) {
		glpkDichotomieSupportees(prob, glpProb, &sols, &nMax, nSol, sols[1], sols[0]);
	}

	return sols;
}

void glpkDichotomieSupportees(Probleme *prob, glp_prob *glpProb, Solution ***sols, unsigned int *nMax, unsigned int *nSol, Solution *sol1, Solution *sol2) {
	printf("(%d < %d ET %d < %d)\n", sol1->obj1, sol2->obj1, sol2->obj2, sol1->obj2);

	//printf("OUI\n");
	unsigned int lambda1, lambda2;
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
	(*sols)[*nSol] = (Solution *) malloc(sizeof(Solution));
	(*sols)[*nSol]->var = (bool *) malloc(prob->n*sizeof(bool));
	(*sols)[*nSol]->obj1 = 0;
	(*sols)[*nSol]->obj2 = 0;
	(*sols)[*nSol]->poids1 = 0;
	(*sols)[*nSol]->poids2 = 0;
	for (int i = 0; i < prob->n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			(*sols)[*nSol]->var[i] = true;
			(*sols)[*nSol]->obj1 = (*sols)[*nSol]->obj1 + prob->coefficients1[i];
			(*sols)[*nSol]->obj2 = (*sols)[*nSol]->obj2 + prob->coefficients2[i];
			(*sols)[*nSol]->poids1 = (*sols)[*nSol]->poids1 + prob->poids1[i];
			(*sols)[*nSol]->poids2 = (*sols)[*nSol]->poids2 + prob->poids2[i];
		} else {
			(*sols)[*nSol]->var[i] = false;
		}
	}
	printf("l1=%d l2=%d val=%d\n\n", lambda1, lambda2, (unsigned int) glp_mip_obj_val(glpProb));
	*nSol = *nSol + 1;

	if ((lambda1*(*sols)[*nSol-1]->obj1 + lambda2*(*sols)[*nSol-1]->obj2) < (lambda1*sol1->obj1 + lambda2*sol1->obj2)) {
		glpkDichotomieSupportees(prob, glpProb, sols, nMax, nSol, sol1, (*sols)[*nSol-1]);
		glpkDichotomieSupportees(prob, glpProb, sols, nMax, nSol, (*sols)[*nSol-1], sol2);
	}
}