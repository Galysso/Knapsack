#ifndef KNAPGLPK_H
#define KNAPGLPK_H

#include "probleme.h"

#include <glpk.h>

Solution **glpkSolutionsSupportees(Probleme *prob, int *nSol, int *nMax);
void glpkDichotomieSupportees(Probleme *prob, glp_prob *glpProb, Solution ***sols, int *nMax, int *nSol, Solution *sol1, Solution *sol2);

#endif