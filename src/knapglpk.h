#ifndef KNAPGLPK_H
#define KNAPGLPK_H

#include "probleme.h"

#include <glpk.h>

ListeSol *glpkSolutionsSupportees(Probleme *prob);
void glpkDichotomieSupportees(Probleme *prob, glp_prob *glpProb, ListeSol *lSol, Solution *sol1, Solution *sol2);

#endif