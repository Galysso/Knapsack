#include "knapglpk.h"
#include "probleme.h"

#include <stdlib.h>
#include <glpk.h>
#include <stdbool.h>
#include <assert.h>

ListeSol *glpkSolutionsSupportees(Probleme *prob) {
	ListeSol *lSol = initListeSol(prob->n);
	Solution *sol;

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
	glp_set_row_bnds(glpProb, 1, GLP_UP, 0.0, prob->omega1);
	glp_set_row_bnds(glpProb, 2, GLP_UP, 0.0, prob->omega2);
	glp_add_cols(glpProb, n);

	int cpt = 1;
	for (int i = 0; i < n; ++i) {
		glp_set_col_kind(glpProb, i+1, GLP_BV);
		glp_set_obj_coef(glpProb, i+1, prob->profits1[i]);
		ia[cpt] = 1;
		ja[cpt] = i+1;
		ar[cpt] = prob->weights1[i];
		++cpt; 
	}

	for (int i = 0; i < n; ++i) {
		ia[cpt] = 2;
		ja[cpt] = i+1;
		ar[cpt] = prob->weights2[i];
		++cpt;
	}

	glp_load_matrix(glpProb, 2*n, ia, ja, ar);
	//glp_write_lp(glpProb, NULL, "modelisation");
	glp_simplex(glpProb, NULL);
	glp_intopt(glpProb, NULL);

	sol = (Solution *) malloc(sizeof(Solution));
	sol->var = (bool *) malloc(n*sizeof(bool));
	sol->p1 = (int) glp_mip_obj_val(glpProb);
	sol->p2 = 0;
	sol->w1 = 0;
	sol->w2 = 0;
	for (int i = 0; i < n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			sol->var[i] = true;
			sol->p2 = sol->p2 + prob->profits2[i];
			sol->w1 = sol->w1 + prob->weights1[i];
			sol->w2 = sol->w2 + prob->weights2[i];
		} else {
			sol->var[i] = false;
		}
	}
	ajouterSolution(lSol, sol);

	for (int i = 0; i < n; ++i) {
		glp_set_obj_coef(glpProb, i+1, prob->profits2[i]);
	}

	//glp_write_lp(glpProb, NULL, "modelisation");
	glp_simplex(glpProb, NULL);
	glp_intopt(glpProb, NULL);

	sol = (Solution *) malloc(sizeof(Solution));
	sol->var = (bool *) malloc(n*sizeof(bool));
	sol->p1 = 0;
	sol->p2 = (int) glp_mip_obj_val(glpProb);
	sol->w1 = 0;
	sol->w2 = 0;
	for (int i = 0; i < n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			sol->var[i] = true;
			sol->p1 = sol->p1 + prob->profits1[i];
			sol->w1 = sol->w1 + prob->weights1[i];
			sol->w2 = sol->w2 + prob->weights2[i];
		} else {
			sol->var[i] = false;
		}
	}
	ajouterSolution(lSol, sol);

	Solution **sols = lSol->solutions;
	if ((sols[1]->p1 < sols[0]->p1) && (sols[1]->p2 > sols[0]->p2)) {
		glpkDichotomieSupportees(prob, glpProb, lSol, sols[1], sols[0]);
	}

	// on trie les solutions selon l'ordre lexicographique
	sols = lSol->solutions;
	bool changement;
	do {
		changement = false;
		for (int i = 1; i < lSol->nbSol; ++i) {
			if (sols[i-1]->p2 < sols[i]->p2) {
				sol = sols[i];
				sols[i] = sols[i-1];
				sols[i-1] = sol;
				changement = true;
			}
		}
	} while (changement);

	return lSol;
}


void glpkDichotomieSupportees(Probleme *prob, glp_prob *glpProb, ListeSol *lSol, Solution *sol1, Solution *sol2) {
	int lambda1, lambda2;
	lambda1 = sol1->p2 - sol2->p2;
	lambda2 = sol2->p1 - sol1->p1;

	for (int i = 0; i < prob->n; ++i) {
		glp_set_obj_coef(glpProb, i+1, lambda1*prob->profits1[i] + lambda2*prob->profits2[i]);
	}
	glp_simplex(glpProb, NULL);
	glp_intopt(glpProb, NULL);

	Solution *nouvelleSol = (Solution *) malloc(sizeof(Solution));
	nouvelleSol->var = (bool *) malloc(prob->n*sizeof(bool));
	nouvelleSol->p1 = 0;
	nouvelleSol->p2 = 0;
	nouvelleSol->w1 = 0;
	nouvelleSol->w2 = 0;
	for (int i = 0; i < prob->n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			nouvelleSol->var[i] = true;
			nouvelleSol->p1 = nouvelleSol->p1 + prob->profits1[i];
			nouvelleSol->p2 = nouvelleSol->p2 + prob->profits2[i];
			nouvelleSol->w1 = nouvelleSol->w1 + prob->weights1[i];
			nouvelleSol->w2 = nouvelleSol->w2 + prob->weights2[i];
		} else {
			nouvelleSol->var[i] = false;
		}
	}

	if ((lambda1*nouvelleSol->p1 + lambda2*nouvelleSol->p2) > (lambda1*sol1->p1 + lambda2*sol1->p2)) {
		ajouterSolution(lSol, nouvelleSol);
		glpkDichotomieSupportees(prob, glpProb, lSol, sol1, nouvelleSol);
		glpkDichotomieSupportees(prob, glpProb, lSol, nouvelleSol, sol2);
	}
}