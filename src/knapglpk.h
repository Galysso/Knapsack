#ifndef KNAPGLPK_H
#define KNAPGLPK_H

#include "probleme.h"

#include <glpk.h>

Solution **glpkSolutionsSupportees(Probleme *prob, unsigned int *nSol);
void glpkDichotomieSupportees(Probleme *prob, glp_prob *glpProb, Solution ***sols, unsigned int *nMax, unsigned int *nSol, Solution *sol1, Solution *sol2);

#endif