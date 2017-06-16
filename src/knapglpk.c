#include "knapglpk.h"
#include "probleme.h"

#include <stdlib.h>
#include <glpk.h>

void glpkSolutionsExtremes(Probleme *prob, unsigned int *bestX1, unsigned int *bestY1, unsigned int *bestX2, unsigned int *bestY2) {
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

	*bestX1 = (unsigned int) glp_mip_obj_val(glpProb);
	*bestY1 = 0;
	for (int i = 0; i < n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			*bestY1 = *bestY1 + prob->coefficients2[i];
		}
	}

	for (int i = 0; i < n; ++i) {
		glp_set_obj_coef(glpProb, i+1, prob->coefficients2[i]);
	}

	//glp_write_lp(glpProb, NULL, "modelisation");
	glp_simplex(glpProb, NULL);
	glp_intopt(glpProb, NULL);

	*bestY2 = (unsigned int) glp_mip_obj_val(glpProb);
	*bestX2 = 0;
	for (int i = 0; i < n; ++i) {
		if (glp_mip_col_val(glpProb, i+1) > 0.1) {
			*bestX2 = *bestX2 + prob->coefficients1[i];
		}
	}
}