#ifndef PATHREL_HPP
#define PATHREL_HPP

#include "probleme.h"

Solution **pathRelinking(Probleme *p, Solution *initSol, Solution *guidingSol, int *nbSol);
Solution **completions(Probleme *p, Solution *incomplete, int *nbComp);

#endif