/* Code de résolution du problème dual-surrogate  pour le problème de sac à dos bi-dimensionnel 
   (Obtenu par extraction sur un code de calcul de l'OCSUB pour le problème de sac à dos bi-dimensionnel bi-objectif)  */

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "combo.h"
#include "2DKPSurrogate.h"

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SIZEMAX 500


/* ======================================================================
 combo.c,    S.Martello, D.Pisinger, P.Toth     feb 1997
 ====================================================================== */

/* Revised version dec 2002, updated rudidiv() */

/* This is the COMBO algorithm described in 
 *
 *   S.Martello, D.Pisinger, P.Toth
 *   "Dynamic Programming and Strong Bounds for the 0-1 Knapsack Problem",
 *   submitted Management Science (1997)
 *
 * (c) copyright S.Martello, D.Pisinger, P.Toth.
 * The code may only be used for academic or non-commercial purposes.
 *
 * Further details on the project can also be found in
 *
 *   S.Martello, D.Pisinger, P.Toth
 *   "Dynamic programming and tight bounds for the 0-1 knapsack problem",
 *   Report 97/11, DIKU, University of Copenhagen
 *   Universitetsparken 1
 *   DK-2100 Copenhagen
 *
 * The code has been tested on a hp9000/735, and conforms with the
 * ANSI-C standard apart from some of the timing routines (which may
 * be removed).
 *  
 * Errors and questions are refered to:
 *   David Pisinger, associate professor
 *   DIKU, University of Copenhagen,
 *   Universitetsparken 1,
 *   DK-2100 Copenhagen.
 *   e-mail: pisinger@diku.dk
 *   fax: +45 35 32 14 01
 */

/* ======================================================================
 definitions
 ====================================================================== */

// Les chronos pour montrer qu'on explose tous les autres algorithmes de la littérature
struct timeval start_utime, stop_utime;

void crono_start()
{
	struct rusage rusage;
	
	getrusage(RUSAGE_SELF, &rusage);
	start_utime = rusage.ru_utime;
}

void crono_stop()
{
	struct rusage rusage;
	
	getrusage(RUSAGE_SELF, &rusage);
	stop_utime = rusage.ru_utime;
}

double crono_ms()
{
	return (stop_utime.tv_sec - start_utime.tv_sec) * 1000 +
    (stop_utime.tv_usec - start_utime.tv_usec) / 1000 ;
}


/*-------------------------------------------------------------------------
 fonction de chargement des donnees du probleme
 -------------------------------------------------------------------------*/

void loadInstance(char *file,donnees *d)
{
	int i;
	FILE *fin;
	
	int val;
	
	fin = fopen(file,"rt");
	
	/* 3 premières lignes : lecture du nombres d'items, et de la capacité des deux sacs */
	
	fscanf(fin,"%d ",&val);
	d->nbItem = val;
	fscanf(fin,"%d ",&val);
	d->omega1 = val;
	fscanf(fin,"%d ",&val);
	d->omega2 = val;
	
	/* Allocation mémoire pour les items */
	
	d->p1 = (itype *) malloc ((d->nbItem) * sizeof(itype));
	//d->p2 = (itype *) malloc ((d->nbItem) * sizeof(itype));
	d->w1 = (itype *) malloc ((d->nbItem) * sizeof(itype));
	d->w2 = (itype *) malloc ((d->nbItem) * sizeof(itype));
		
	/* Lignes suivantes : profit1, profit2, poids1, poids2*/
	
	d->maxZ1 = 0;
	//d->maxZ2 = 0;
	
	for(i = 0;i < d->nbItem;i++)
	{
		fscanf(fin,"%d ",&val);
		d->p1[i] = val;
		d->maxZ1 += val;
		//fscanf(fin,"%d ",&val);
		//d->p2[i] = val;
		//d->maxZ2 += val;
		fscanf(fin,"%d ",&val);
		d->w1[i] = val;
		fscanf(fin,"%d ",&val);
		d->w2[i] = val;
	}
    
	fclose(fin);
}


/* Fonction de résolution d'un problème de sac à dos mono-objectif dans le contexte de la méthode :
 * On agrège les fonctions objectifs avec lambda1 et lambda2,
 * On agrège les contraintes avec mult1 et mult2 */

void singleOpt(long long mult1, long long mult2, donnees *d, solution *s)
{
	int i;
	long long capa;
	item* prob;
	long long maxZ;
	int index;
	long long sumW = 0;
	
	prob = (item *) malloc ((d->nbItem) * sizeof(item));
	
	for(i = 0;i < d->nbItem;i++)
	{
		prob[i].w = mult1 * d->w1[i] + mult2 * d->w2[i];
		//prob[i].p = lambda1 * d->p1[i] + lambda2 * d->p2[i];
		prob[i].p = d->p1[i];
		prob[i].i = i;
		sumW += prob[i].w;
	}
	
	capa = mult1 * d->omega1 + mult2 * d->omega2;
	//maxZ = lambda1 * d->maxZ1 + lambda2 * d->maxZ2;
	maxZ = d->maxZ1;
	
	/*for (int j = 0; j < d->nbItem; ++j) {
		printf("%lld\t%lld\n", prob[j].p, prob[j].w);
	}
	printf("capa=%lld\n\n", capa);*/

	if (sumW > capa) {
		s->z1 = (int) combo(&prob[0], &prob[d->nbItem - 1], capa, 0, maxZ, 1, 1);
		for (i = 0; i < d->nbItem; i++)
		{
			index = prob[i].i;
			s->tab[index] = prob[i].x;
		}
	} else {
		s->z1 = 0;
		for (i = 0; i < d->nbItem; ++i) {
			s->z1 += prob[i].p;
			s->tab[i] = 1;
		}
	}
	
	free(prob);
}

/* Simple calcul de PGCD */

long long pgcd(long long x,long long y)
{
	long long a,b,tmp;
	
	if (x > y)
    {
		a = x;
		b = y;
    }
	else
    {
		a = y; 
		b = x; 
    }
	
	while (b != 0)
    {
		tmp = a;
		a = b;
		b = tmp % b; 
    }
	return a;
}


/* Détermine l'intervalle de stabilité d'une solution */

void computeSI(donnees *d, solution *s)
{
	int i;
	long long sac1 = 0;
	long long sac2 = 0;
	long long div;
	
	/* Détermination du remplissage des sacs */
	for (i = 0; i < d->nbItem; i++)
	{
		sac1 += s->tab[i] * d->w1[i];
		sac2 += s->tab[i] * d->w2[i];		
	}	
	
	/* Détermination des débordements éventuels des sacs */
	if (sac1 > d->omega1) 
	{
		s->c1 = 0;
		s->c2 = 1;
	}
	else 
	{
		s->c1 = 1;
		if (sac2 <= d->omega2) s->c2 = 1;
		else s->c2 = 0;
	}
	
	/* Détermination éventuelle du multiplicateur critique */
	if (s->c1 == 0)
	{
		s->num = d->omega2 - sac2;
		if (s->num != 0)
		{
			s->den = sac1 - d->omega1 + d->omega2 - sac2;
			div = pgcd(s->num,s->den);
			s->num = s->num / div;
			s->den = s->den / div;
		}
		else s->den = 1;
	}
	else if (s->c2 == 0)
	{
		s->num = sac2 - d->omega2;
		s->den = d->omega1 - sac1 + sac2 - d->omega2;
		div = pgcd(s->num,s->den);
		s->num = s->num / div;
		s->den = s->den / div;
	}
}


/* Affichage complet (avant modification de la structure de données) d'une solution :
 * valeurs des variables,
 * valeur des fonctions objectifs
 * intervalle de stabilité */

void PrintSolution(solution *s, int size)
{
	int i;
	
	/* Affichage solution */
	printf("x = (");
	for (i = 0; i < size; i++) {
		printf("%d",s->tab[i]);
	}
	printf(") avec z(x) = %d\n",s->z1);
	
	/* Affichage intervalle de stabilité */
	printf("Intervalle de stabilité :");
	if (s->c1 == 0) {
		printf("[0,%lld/%lld]\n",s->num,s->den);
	} else if (s->c2 == 0) {
		printf("[%lld/%lld,1]\n",s->num,s->den);
	} else {
		printf("[0,1]\n");
	}
}


/* Fonction déterminant les multiplicateurs (entiers) à appliquer pour chaque contrainte (dans la dichotomie mono-objectif) */

void computeMult(solution *s1,solution *s2,long long *mult1,long long *mult2)
{
	long long num;
	long long den;
	long long div;
	
	num = s1->num * s2->den + s2->num * s1->den;
	den = 2 * s1->den * s2->den;
	div = pgcd(num,den);
	num = num/div;
	den = den/div;
	*mult1 = num;
	*mult2 = den - num;
}

/* Dichotomomie mono-objectif (sur les multiplicateurs) pour résoudre le problème dual-surrogate mono-objectif,
   Fonction appelée lorsque l'initialisation ne résout pas le problème (le plus souvent) */

void startDichoMu(solution **s1, solution **s2, donnees *d)
{
	long long mult1;
	long long mult2;
	solution *s;
	int fin = 0;
	while(fin == 0) // Tant qu'on n'a pas une solution admissible ou un recouvrement des SI de s1 et s2
	{
		computeMult(*s1,*s2,&mult1,&mult2); // On détermine un multiplicateur entre les deux SI
		s = (solution *) malloc (sizeof(solution));
		s->tab = (int *) malloc ((d->nbItem) * sizeof(int)); 
		singleOpt(mult1,mult2,d,s); // résolution
		s->mult1 = mult1;
		s->mult2 = mult2;
		computeSI(d,s); // Détermination Intervalle de stabilité
		
		if (s->c1 == 0) // Nouvelle solution de gauche
		{
			// libération mémoire avant affectation
			free((*s1)->tab);
			free(*s1);
			*s1 = s; // affectation de la nouvelle solution
		}
		else if (s->c2 == 0) // Nouvelle solution de droite
		{
			// libération mémoire avant affectation
			free((*s2)->tab);
			free(*s2);			
			*s2 = s; // affectation de la nouvelle solution
		}
		else // Nouvelle solution admissible
		{
			// libération mémoire
			free((*s1)->tab);
			free(*s1);
			free((*s2)->tab);
			free(*s2);			
			
			*s1 = NULL; // Indique qu'on a une solution admissible
			*s2 = s;
			fin = 1;
		}
		if ((fin == 0) && ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den)) 
		{
			fin = 1;
		}
	}
}

/* Fin d'initialisation de la dichotomie mono-objectif : à utiliser lorsqu'on dispose déjà d'une solution (par défaut placée en s2, avant d'être replacée) */

int initDichoMu1Sol(solution **s1,solution **s2,donnees *d)
{
	long long mult1;
	long long mult2;
	
	if ((*s2)->c1 == 0) // si la première solution est "de gauche"...
	{	
		*s1 = *s2;
		(*s2) = (solution *) malloc (sizeof(solution));	
		(*s2)->tab = (int *) malloc ((d->nbItem) * sizeof(int));
		mult1 = 1;
		mult2 = 0;
		singleOpt(mult1,mult2,d,*s2);
		(*s2)->mult1 = mult1;
		(*s2)->mult2 = mult2;
		computeSI(d,*s2); // Détermination Intervalle de stabilité	
		
		if ((*s2)->c2 == 1) // Par construction, s2->c1 == 1, donc le problème dual-surrogate est alors résolu à l'optimalité
		{
			*s1 = NULL; // Indique qu'on a une solution admissible
			return 1;
		}
		else if ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den) // Si on a un recouvrement des deux SI, le problème dual-surrogate est aussi résolu à l'optimalité
		{
			return 1;
		}
	}
	else if ((*s2)->c2 == 0) // si la première solution est "de droite"
	{	*s1 = (solution *) malloc (sizeof(solution));	
		(*s1)->tab = (int *) malloc ((d->nbItem) * sizeof(int));
		mult1 = 0;
		mult2 = 1;
		singleOpt(mult1,mult2,d,*s1);
		(*s1)->mult1 = mult1;
		(*s1)->mult2 = mult2;
		computeSI(d,*s1); // Détermination Intervalle de stabilité	
		
		if ((*s1)->c1 == 1) // Par construction, s1->c2 == 1, donc le problème dual-surrogate est alors résolu à l'optimalité
		{	
			*s2 = *s1;
			*s1 = NULL; 
			return 1;
		}
		else if ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den) // Si on a un recouvrement des deux SI, le problème dual-surrogate est aussi résolu à l'optimalité
		{
			return 1;
		}
	}
	else // la première solution est admissible => on a déjà fini!
	{
		*s1 = NULL; // Indique qu'on a une solution admissible
		return 1;
	}
	return 0; // indique que l'initialisation ne résout pas le problème
}

/* Initialisation de la dichotomie mono-objectif : à utiliser lorsqu'on n'a pas de solution pour démarrer */

int initDichoMu(solution **s1, solution **s2, donnees *d)
{
	if (d->nbItem > 0) {
		long long mult1 = 1;
		long long mult2 = 1;

		// Détermination de la solution initiale de la dichotomie
		*s2 = (solution *) malloc (sizeof(solution));
		(*s2)->tab = (int *) malloc ((d->nbItem) * sizeof(int)); 
		singleOpt(mult1,mult2,d,*s2);
		(*s2)->mult1 = mult1;
		(*s2)->mult2 = mult2;
		computeSI(d,*s2); // Détermination Intervalle de stabilité

		return initDichoMu1Sol(s1,s2,d);
	} else {
		(*s1) = NULL;
		(*s2) = (solution *) malloc (sizeof(solution));
		(*s2)->tab = NULL;
		(*s2)->z1 = 0;

		return 1;
	}
}



/*int main(int argc,char *argv[])
{
	donnees d;
	double temps = 0;
	solution *s1;
	solution *s2;
	int ret;
	
	// Chargement de l'instance
	
	loadInstance(argv[1],&d);
	
	// Lancement de la méthode de calcul paramétrique de l'OCSUB
	
	crono_start(); // GO!
	
	ret = initDichoMu(&s1,&s2,&d);
	
	if (ret == 0) startDichoMu(&s1,&s2,&d);

	// Affichage des résultats

	 printf("Conclusion : on obtient ");
	 if (s1 == NULL) 
	 {
		 puts("une solution admissible :");
		 PrintSolution(s2,d.nbItem);
	 }	 
	 else if (s1->z1 < s2->z1) 
	 {
		 puts("une paire de solutions définissant l'optimalité pour le problème dual-surrogate :");
		 PrintSolution(s1,d.nbItem);
		 PrintSolution(s2,d.nbItem);
	 }
	 else 
	 {
		 puts("une paire de solutions définissant l'optimalité pour le problème dual-surrogate :");
		 PrintSolution(s2,d.nbItem);
		 PrintSolution(s1,d.nbItem);
	 }
	 puts("");	
	
	 printf("temps : %f\n",temps);	
	
	// Libération mémoire
	
	free(s2->tab);
	free(s2);
	if (s1 != NULL) 
	{
		free(s1->tab);
		free(s1);
	}

	free(d.p1);
	free(d.w1);
	free(d.w2);
	
	// J'adore qu'un plan se déroule sans accroc!
	
	return 0;
}*/
