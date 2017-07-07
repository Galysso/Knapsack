/* Méthode paramétrique pour le calcul de l'OCSUB
 * Pour le problème de sac à dos bi-dimensionnel bi-objectif */

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "combo.h"

#include "bool.h"
#include "geometry.h"

/*	convex-hull.c
 
 Compute convex hulls of points in the plane using the
 Gries/Graham scan algorithm.
 
 begun: September 13, 2002
 by: Steven Skiena
 */

/*
 Copyright 2003 by Steven S. Skiena; all rights reserved. 
 
 Permission is granted for use in non-commerical applications
 provided this copyright notice remains intact and unchanged.
 
 This program appears in my book:
 
 "Programming Challenges: The Programming Contest Training Manual"
 by Steven Skiena and Miguel Revilla, Springer-Verlag, New York 2003.
 
 See our website www.programming-challenges.com for additional information.
 
 This book can be ordered from Amazon.com at
 
 http://www.amazon.com/exec/obidos/ASIN/0387001638/thealgorithmrepo/
 
 */


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

typedef struct
{
	int nbItem; // nombre d'objets dans le sac
	itype *p1; // coefficients premier objectif
	itype *p2; // coefficients deuxième objectif
	itype *w1; // coefficients première dimension
	itype *w2; // coefficients seconde dimension
	int omega1; // capacité première dimension
	int omega2; // capacité seconde dimension
	int maxZ1; // pour combo
	int maxZ2; // pour combo
} donnees;

typedef struct
{
	int *tab; // vecteur de valeurs (booléennes) des solutions (à remanier plus tard)
	int z1; // valeur obj1
	int z2; // valeur obj2
	//long long val; // valeur objectif pondéré // Inutile donc supprimé // Vérifier partout dans le code
	short int c1; // booléen indiquant si la contrainte 1 est satisfaite (1) ou non (0)
	short int c2; // booléen indiquant si la contrainte 2 est satisfaite (1) ou non (0)
	long long mult1; 
	long long mult2; // multiplicateur utilisé pour trouver la solution
	long long num; // numérateur du multiplicateur critique (si une seule contrainte est satisfaite)
	long long den; // dénominateur du multiplicateur critique (si une seule contrainte est satisfaite)
	long long lambda1;
	long long lambda2; // (lambda1,lambda2) critique (i.e. minimum) pour l'obtention de cette solution à mu constant
} solution;

typedef struct
{
	solution liste[SIZEMAX]; // Tableau de solutions optimales pour un dual surrogate mono-objectif donné par une somme pondérée
	long long lambda1[SIZEMAX]; // valeur minimale du ratio (lambda1,lambda2) pour lequel la solution est optimale
	long long lambda2[SIZEMAX]; // ... // Devient inutile aussi! Attention! // réintroduit par simplicité (car on recopie des solutions)
	int lgListe; // taille du tableau
} listeSol;

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
	d->p2 = (itype *) malloc ((d->nbItem) * sizeof(itype));
	d->w1 = (itype *) malloc ((d->nbItem) * sizeof(itype));
	d->w2 = (itype *) malloc ((d->nbItem) * sizeof(itype));
		
	/* Lignes suivantes : profit1, profit2, poids1, poids2*/
	
	d->maxZ1 = 0;
	d->maxZ2 = 0;
	
	for(i = 0;i < d->nbItem;i++)
	{
		fscanf(fin,"%d ",&val);
		d->p1[i] = val;
		d->maxZ1 += val;
		fscanf(fin,"%d ",&val);
		d->p2[i] = val;
		d->maxZ2 += val;
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

void singleOpt(long long lambda1, long long lambda2, long long mult1, long long mult2, donnees *d, solution *s)
{
	int i;
	long long capa;
	item* prob;
	long long maxZ;
	int index;
	
	prob = (item *) malloc ((d->nbItem) * sizeof(item));
	
	for(i = 0;i < d->nbItem;i++)
	{
		prob[i].w = mult1 * d->w1[i] + mult2 * d->w2[i];
		prob[i].p = lambda1 * d->p1[i] + lambda2 * d->p2[i];
		prob[i].i = i;
	}
	
	capa = mult1 * d->omega1 + mult2 * d->omega2;
	maxZ = lambda1 * d->maxZ1 + lambda2 * d->maxZ2;
		
	combo(&prob[0], &prob[d->nbItem - 1], capa, 0, maxZ, 1, 1);
	
	
	for (i = 0; i < d->nbItem; i++)
	{
		index = prob[i].i;
		s->tab[index] = prob[i].x;
	}
	
	free(prob);
}

/* Détermine le point correspondant à une solution */

void computePoint(donnees *d, solution *s)
{
	int i;
	
	s->z1 = 0;
	s->z2 = 0;
	for (i = 0; i < d->nbItem; i++) 
	{
		s->z1 += s->tab[i] * d->p1[i];
		s->z2 += s->tab[i] * d->p2[i];
	}
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

/* Fonction pour le premier ajout d'une solution dans le tableau (pour éviter d'accéder à la case -1) */

void addSolInit(solution *s, int size, listeSol *L)
{
	int i;
	
	L->lambda1[0] = 1;
	L->lambda2[0] = 0;
	(L->liste[0]).z1 = s->z1;
	(L->liste[0]).z2 = s->z2;
	(L->liste[0]).tab = (int *) malloc (size * sizeof(int));
	for(i = 0;i < size;i++) (L->liste[0]).tab[i] = s->tab[i];
	(L->lgListe)++;
}

/* Fonction d'ajout d'une solution dans le tableau (tient compte des modifications de structures de données) */

void addSolution(solution *s, int size, listeSol *L)
{
	int i;
	
	if ( (s->z1 == (L->liste[L->lgListe-1]).z1) && (s->z2 == (L->liste[L->lgListe-1]).z2)) // simple MAJ de (lambda1,lambda2)
	{
		//printf("simple MAJ standard pour (%d,%d), nouveau (l1,l2) = (%lld,%lld)\n",s->z1,s->z2,s->lambda1,s->lambda2);
		L->lambda1[L->lgListe - 1] = s->lambda1;
		L->lambda2[L->lgListe - 1] = s->lambda2;
	}
	else
	{
		if (L->lgListe < SIZEMAX)
		{
			//printf("Ajout standard de (%d,%d) avec (l1,l2) = (%lld,%lld)\n",s->z1,s->z2,s->lambda1,s->lambda2);
			(L->liste[L->lgListe]).z1 = s->z1;
			(L->liste[L->lgListe]).z2 = s->z2;
			(L->liste[L->lgListe]).tab = (int *) malloc (size * sizeof(int));
			for(i = 0;i < size;i++) (L->liste[L->lgListe]).tab[i] = s->tab[i];
			L->lambda1[L->lgListe] = s->lambda1;
			L->lambda2[L->lgListe] = s->lambda2;
			(L->lgListe)++;
		}
		else 
		{
			puts("SIZEMAX trop petite!");
			exit(0);
		}
	}
}

/* Fonction d'ajout d'une solution dans le tableau (cas particulier pour lequel la solution ne donne pas le (lambda1,lambda2)) */

void addSolutionBreak(solution *s, long long lambda1, long long lambda2, int size, listeSol *L)
{
	int i;
	
	if ( (s->z1 == (L->liste[L->lgListe-1]).z1) && (s->z2 == (L->liste[L->lgListe-1]).z2)) // simple MAJ de (lambda1,lambda2)
	{
		//printf("simple MAJ break pour (%d,%d), nouveau (l1,l2) = (%lld,%lld)\n",s->z1,s->z2,lambda1,lambda2);
		L->lambda1[L->lgListe - 1] = lambda1;
		L->lambda2[L->lgListe - 1] = lambda2;
	}
	else
	{
		if (L->lgListe < SIZEMAX)
		{
			//printf("Ajout break de (%d,%d) avec (l1,l2) = (%lld,%lld)\n",s->z1,s->z2,lambda1,lambda2);	
			(L->liste[L->lgListe]).z1 = s->z1;
			(L->liste[L->lgListe]).z2 = s->z2;
			(L->liste[L->lgListe]).tab = (int *) malloc (size * sizeof(int));
			for(i = 0;i < size;i++) (L->liste[L->lgListe]).tab[i] = s->tab[i];
			L->lambda1[L->lgListe] = lambda1;
			L->lambda2[L->lgListe] = lambda2;
			(L->lgListe)++;
		}
		else 
		{
			puts("SIZEMAX trop petite!");
			exit(0);
		}
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
	for (i = 0; i < size; i++) printf("%d",s->tab[i]);
	printf(") avec z(x) = (%d,%d)\n",s->z1,s->z2);
	
	/* Affichage intervalle de stabilité */
	printf("Intervalle de stabilité :");
	if (s->c1 == 0) printf("[0,%lld/%lld]\n",s->num,s->den);
	else if (s->c2 == 0) printf("[%lld/%lld,1]\n",s->num,s->den);
	else printf("[0,1]\n");
}

/* Affichage des résultats du simplexe paramétrique */

void PrintResult(listeSol *L)
{
	int i;
	long long numprec;
	long long denprec;
	long long num;
	long long den;
	
	numprec = L->lambda1[0];
	denprec = L->lambda1[0] + L->lambda2[0];
	
	puts("");
	puts("----------------------------------------");
	puts("Résultats de l'algorithme paramétrique :");
	puts("----------------------------------------\n");

	printf("(%d,%d) pour lambda dans [%lld/%lld,1]\n",(L->liste[0]).z1,(L->liste[0]).z2,numprec,denprec);	
	for(i = 1;i < L->lgListe - 1;i++)
	{
		num = L->lambda1[i];
		den = L->lambda1[i] + L->lambda2[i];
		printf("(%d,%d) pour lambda dans [%lld/%lld,%lld/%lld]\n",(L->liste[i]).z1,(L->liste[i]).z2,num,den,numprec,denprec);
		numprec = num; 
		denprec = den;
	}
	printf("(%d,%d) pour lambda dans [0,%lld/%lld]\n",(L->liste[L->lgListe - 1]).z1,(L->liste[L->lgListe - 1]).z2,numprec,denprec);	
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

/* Dichotomomie mono-objectif (sur les multiplicateurs) pour résoudre le problème dual-surrogate mono-objectif (lambda1,lambda2) étant fixé,
   Fonction appelée lorsque l'initialisation ne résout pas le problème (le plus souvent) */

void startDichoMu(long long lambda1, long long lambda2, solution **s1, solution **s2, donnees *d)
{
	//puts("lancement de la boucle principale de la dichotomie");
	long long mult1;
	long long mult2;
	solution *s;
	int fin = 0;
	while(fin == 0) // Tant qu'on n'a pas une solution admissible ou un recouvrement des SI de s1 et s2
	{
		computeMult(*s1,*s2,&mult1,&mult2); // On détermine un multiplicateur entre les deux SI
		s = (solution *) malloc (sizeof(solution));
		s->tab = (int *) malloc ((d->nbItem) * sizeof(int)); 
		singleOpt(lambda1,lambda2,mult1,mult2,d,s); // résolution
		s->mult1 = mult1;
		s->mult2 = mult2;
		computePoint(d,s); // Détermination point	
		computeSI(d,s); // Détermination Intervalle de stabilité
		s->lambda1 = -1; // Indique le (lambda1,lambda2) min à mu constant n'est pas calculé
		
		/*
		puts("nouvelle solution");
		PrintSolution(s,d->nbItem);
		puts("");*/
		
		if (s->c1 == 0) // Nouvelle solution de gauche
		{
			//puts("Nouvelle solution de type 0M");
			// libération mémoire avant affectation
			free((*s1)->tab);
			free(*s1);
			*s1 = s; // affectation de la nouvelle solution
		}
		else if (s->c2 == 0) // Nouvelle solution de droite
		{
			//puts("Nouvelle solution de type M1");
			// libération mémoire avant affectation
			free((*s2)->tab);
			free(*s2);			
			*s2 = s; // affectation de la nouvelle solution
		}
		else // Nouvelle solution admissible
		{
			//puts("Nouvelle solution admissible");
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
			//puts("Recouvrement des deux SI => FIN\n");
			fin = 1;
		}
	}
	
	// Il reste à retourner la solution optimale associé à la valeur optimale de la borne
	
	//if (*s1 == NULL) return *s2;
	//if ((*s1)->val < (*s2)->val) return *s1; 
	//return *s2; // Inutile car on est obligé de retester quand (lambda1,lambda2) évolue
}





/* Fin d'initialisation de la dichotomie mono-objectif : à utiliser lorsqu'on dispose déjà d'une solution (par défaut placée en s2, avant d'être replacée) */

int initDichoMu1Sol(long long lambda1,long long lambda2,solution **s1,solution **s2,donnees *d)
{
	long long mult1;
	long long mult2;
	
	/*
	puts("");
	puts("solution initiale");
	PrintSolution(*s2,d->nbItem);
	puts("");*/
	
	if ((*s2)->c1 == 0) // si la première solution est "de gauche"...
	{	//puts("Comme la première solution est de type 0M, on cherche une seconde solution (éventuellement M1)");
		*s1 = *s2;
		(*s2) = (solution *) malloc (sizeof(solution));	
		(*s2)->tab = (int *) malloc ((d->nbItem) * sizeof(int));
		mult1 = 1;
		mult2 = 0;
		singleOpt(lambda1,lambda2,mult1,mult2,d,*s2);
		(*s2)->mult1 = mult1;
		(*s2)->mult2 = mult2;
		computePoint(d,*s2); // Détermination point	
		computeSI(d,*s2); // Détermination Intervalle de stabilité	
		(*s2)->lambda1 = -1; // Indique que le (lambda1,lambda2) min à mu constant n'est pas calculé 
		/*PrintSolution(*s2,d->nbItem);
		puts("");*/
		
		if ((*s2)->c2 == 1) // Par construction, s2->c1 == 1, donc le problème dual-surrogate est alors résolu à l'optimalité
		{
			//puts("On obtient une solution admissible");
			// libération mémoire de s avant de retourner s2
			//free(s->tab);
			//free(s); // devenu inutile avec la fusion initial de s et de s2
			*s1 = NULL; // Indique qu'on a une solution admissible
			return 1;
		}
		else if ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den) // Si on a un recouvrement des deux SI, le problème dual-surrogate est aussi résolu à l'optimalité
		{
			//puts("On a un recouvrement entre les deux SI => FIN");
			// Ici, pas de libération mémoire, car les deux solutions peuvent être utiles pour la suite...
			//if ((*s1)->val < (*s2)->val) return *s1; 
			//return *s2; // Inutile car on va restester
			return 1;
		}
	}
	else if ((*s2)->c2 == 0) // si la première solution est "de droite"
	{	//puts("Comme la première solution est de type 1M, on cherche une seconde solution (éventuellement 0M)");
		//*s2 = s;
		*s1 = (solution *) malloc (sizeof(solution));	
		(*s1)->tab = (int *) malloc ((d->nbItem) * sizeof(int));
		mult1 = 0;
		mult2 = 1;
		singleOpt(lambda1,lambda2,mult1,mult2,d,*s1);
		(*s1)->mult1 = mult1;
		(*s1)->mult2 = mult2;
		computePoint(d,*s1); // Détermination point	
		computeSI(d,*s1); // Détermination Intervalle de stabilité	
		(*s1)->lambda1 = -1; // Indique que le (lambda1,lambda2) min à mu constant n'est pas calculé
		/*PrintSolution(*s1,d->nbItem);
		puts("");*/
		
		if ((*s1)->c1 == 1) // Par construction, s1->c2 == 1, donc le problème dual-surrogate est alors résolu à l'optimalité
		{	//puts("On obtient une solution admissible");
			// libération mémoire de s avant de retourner s1
			//free(s->tab);
			//free(s);// devenu inutile avec la fusion initial de s et de s2
			*s2 = *s1;
			*s1 = NULL; // Indique qu'on a une solution admissible
			//return *s2;
			return 1;
		}
		else if ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den) // Si on a un recouvrement des deux SI, le problème dual-surrogate est aussi résolu à l'optimalité
		{
			//puts("On a un recouvrement entre les deux SI => FIN");
			// Ici, pas de libération mémoire, car les deux solutions peuvent être utiles pour la suite...
			//if ((*s1)->val < (*s2)->val) return *s1; // Note : il faudra s'assurer de l'évolution correcte de "val" (en particulier de sa MAJ)
			//return *s2; // Inutile car on va retester
			return 1;
		}
	}
	else // la première solution est admissible => on a déjà fini!
	{
		//puts("On obtient directement une solution admissible");
		*s1 = NULL; // Indique qu'on a une solution admissible
		//*s2 = s;
		//return *s2;
		return 1;
	}
	return 0; // indique que l'initialisation ne résout pas le problème
}

/* Fin d'initialisation de la dichotomie mono-objectif : à utiliser lorsqu'on dispose de deux solutions initiales (qui seront éventuellement replacées) */

int initDichoMu2Sol(long long lambda1,long long lambda2,solution **s1,solution **s2,donnees *d)
{
	/*
	puts("solution initiale 1");
	PrintSolution(*s1,d->nbItem);
	puts("");
	
	puts("solution initiale 2");
	PrintSolution(*s2,d->nbItem);
	puts("");*/
	
	solution *s; // variable "tampon" utilisée pour un swap
	
	if (((*s1)->c1 == 1) && ((*s1)->c2 == 1)) // solution admissible
	{
		//printf("On obtient directement une solution admissible");
		*s2 = *s1;
		*s1 = NULL;
		return 1;
	}
	
	if (((*s2)->c1 == 1) && ((*s2)->c2 == 1)) // solution admissible
	{
		//printf("On obtient directement une solution admissible");
		*s1 = NULL;
		return 1;
	}
	
	if ((*s1)->c1 == 0)
		if ((*s2)->c1 == 0) // on a deux solutions "de gauche", on garde dans s1 (transféré en s2 pour l'appel à initDichoMu1Sol) celle au plus grand multiplicateur critique
		{
			if ((*s2)->num * (*s1)->den > (*s1)->num * (*s2)->den) // si s2 a un plus grand multiplicateur critique, s2 sera le nouveau s1
			{
				free((*s1)->tab);
				free(*s1);
				return initDichoMu1Sol(lambda1,lambda2,s1,s2,d);
			}
			else // sinon s1 sera le nouveau s1 
			{
				free((*s2)->tab);
				free(*s2);
				*s2 = *s1;
				return initDichoMu1Sol(lambda1,lambda2,s1,s2,d);
			}
		}
		else // s1 est "de gauche" et s2 est "de droite" => test d'optimalité
		{
			if ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den) 
			{
				//puts("On a un recouvrement entre les deux SI => FIN");
				return 1;
			}
		}
	else // s1 est "de droite"
	{
		if ((*s2)->c1 == 0) // s2 est "de gauche"
		{
			s = *s1;
			*s1 = *s2;
			*s1 = s; // swap
			if ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den) 
			{
				//puts("On a un recouvrement entre les deux SI => FIN");
				return 1;
			}
		}
		else // On a deux solutions "de droite", on garde dans s2 celle au plus petit multiplicateur critique
		{
			if ((*s2)->num * (*s1)->den < (*s1)->num * (*s2)->den) // si s2 a un plus petit multiplicateur critique, s2 sera le nouveau s2
			{
				free((*s1)->tab);
				free(*s1);
				return initDichoMu1Sol(lambda1,lambda2,s1,s2,d);
			}
			else // sinon s1 sera le nouveau s2 
			{
				free((*s2)->tab);
				free(*s2);
				*s2 = *s1;
				return initDichoMu1Sol(lambda1,lambda2,s1,s2,d);
			}
		}
	}
	return 0; // l'initialisation ne résout pas le problème
}

/* Initialisation de la dichotomie mono-objectif : à utiliser lorsqu'on n'a pas de solution pour démarrer */

int initDichoMu(long long lambda1, long long lambda2, solution **s1, solution **s2, donnees *d)
{
	long long mult1 = 1;
	long long mult2 = 1;
	//solution *s; // Devient inutile car on manipule directement s2
	
	//puts("initialisation de la dichotomie\n");
	
	// Détermination de la solution initiale de la dichotomie
	*s2 = (solution *) malloc (sizeof(solution));
	(*s2)->tab = (int *) malloc ((d->nbItem) * sizeof(int)); 
	singleOpt(lambda1,lambda2,mult1,mult2,d,*s2);
	(*s2)->mult1 = mult1;
	(*s2)->mult2 = mult2;
	computePoint(d,*s2); // Détermination point	
	computeSI(d,*s2); // Détermination Intervalle de stabilité
	(*s2)->lambda1 = -1; // Indique que le (lambda1,lambda2) min à mu constant n'est pas calculé 
	
	//puts("solution initiale");
	//PrintSolution(*s2,d->nbItem);
	//puts("");

	return initDichoMu1Sol(lambda1,lambda2,s1,s2,d);
	
	// what follows is replaced by InitDichoMu1Sol to factorize code
	/* 
	if (s->c1 == 0) // si la première solution est "de gauche"...
	{	puts("Comme la première solution est de type 0M, on cherche une seconde solution (éventuellement M1)");
		*s1 = s;
		(*s2) = (solution *) malloc (sizeof(solution));	
		(*s2)->tab = (int *) malloc ((d->nbItem) * sizeof(int));
		mult1 = 1;
		mult2 = 0;
		singleOpt(lambda1,lambda2,mult1,mult2,d,*s2);
		(*s2)->mult1 = mult1;
		(*s2)->mult2 = mult2;
		computePoint(d,*s2); // Détermination point	
		computeSI(d,*s2); // Détermination Intervalle de stabilité	
		(*s2)->lambda1 = -1; // Indique que le (lambda1,lambda2) min à mu constant n'est pas calculé 
		PrintSolution(*s2,d->nbItem);
		puts("");
		
		if ((*s2)->c2 == 1) // Par construction, s2->c1 == 1, donc le problème dual-surrogate est alors résolu à l'optimalité
		{
			puts("On obtient une solution admissible");
			// libération mémoire de s avant de retourner s2
			free(s->tab);
			free(s);
			*s1 = NULL; // Indique qu'on a une solution admissible
			return 1;
		}
		else if ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den) // Si on a un recouvrement des deux SI, le problème dual-surrogate est aussi résolu à l'optimalité
		{
			puts("On a un recouvrement entre les deux SI => FIN");
			// Ici, pas de libération mémoire, car les deux solutions peuvent être utiles pour la suite...
			//if ((*s1)->val < (*s2)->val) return *s1; 
			//return *s2; // Inutile car on va restester
			return 1;
		}
	}
	else if (s->c2 == 0) // si la première solution est "de droite"
	{	puts("Comme la première solution est de type 1M, on cherche une seconde solution (éventuellement 0M)");
		*s2 = s;
		*s1 = (solution *) malloc (sizeof(solution));	
		(*s1)->tab = (int *) malloc ((d->nbItem) * sizeof(int));
		mult1 = 0;
		mult2 = 1;
		singleOpt(lambda1,lambda2,mult1,mult2,d,*s1);
		(*s1)->mult1 = mult1;
		(*s1)->mult2 = mult2;
		computePoint(d,*s1); // Détermination point	
		computeSI(d,*s1); // Détermination Intervalle de stabilité	
		(*s1)->lambda1 = -1; // Indique que le (lambda1,lambda2) min à mu constant n'est pas calculé
		PrintSolution(*s1,d->nbItem);
		puts("");

		if ((*s1)->c1 == 1) // Par construction, s1->c2 == 1, donc le problème dual-surrogate est alors résolu à l'optimalité
		{	puts("On obtient une solution admissible");
			// libération mémoire de s avant de retourner s1
			free(s->tab);
			free(s);
			*s2 = *s1;
			*s1 = NULL; // Indique qu'on a une solution admissible
			//return *s2;
			return 1;
		}
		else if ((*s2)->num * (*s1)->den <= (*s1)->num * (*s2)->den) // Si on a un recouvrement des deux SI, le problème dual-surrogate est aussi résolu à l'optimalité
		{
			puts("On a un recouvrement entre les deux SI => FIN");
			// Ici, pas de libération mémoire, car les deux solutions peuvent être utiles pour la suite...
			//if ((*s1)->val < (*s2)->val) return *s1; // Note : il faudra s'assurer de l'évolution correcte de "val" (en particulier de sa MAJ)
			//return *s2; // Inutile car on va retester
			return 1;
		}
	}
	else // la première solution est admissible => on a déjà fini!
	{
		puts("On obtient directement une solution admissible");
		*s1 = NULL; // Indique qu'on a une solution admissible
		*s2 = s;
		//return *s2;
		return 1;
	}
	return 0; // indique que l'initialisation ne résout pas le problème
	*/
}

/* Fonction déterminant jusqu'à quel (lambda1,lambda2) (valuer minimale) une solution reste admissible avec (mult1,mult2) fixé */

solution *dichoPart(solution *s,donnees *d)
{
	//puts("dichopart");
	long long val;
	long long valn;
	long long div;
	int fin;
	long long lambda1;
	long long lambda2;
	solution *n;
	solution *nprec;
	solution *ntemp; // On a besoin de deux solutions et des swap seront nécessaires
		
	// Valeur initiale de (lambda1,lambda2) (lexopt appliqué)
	lambda1 = 1;
	lambda2 = d->maxZ1;

	// Initialisation de la solution
	n = (solution *) malloc (sizeof(solution));
	n->tab = (int *) malloc ((d->nbItem) * sizeof(int)); 
	n->mult1 = s->mult1;
	n->mult2 = s->mult2;

	// résolution du premier problème pondéré
	singleOpt(lambda1,lambda2,s->mult1,s->mult2,d,n);
	computePoint(d,n);

	/*printf("Pour (lambda1,lambda2) = (%lld,%lld), on obtient :\n",lambda1,lambda2);
	PrintSolution(n,d->nbItem);*/
	
	if (n->z2 == s->z2) // cas final, aucune dichotomie partielle n'est nécessaire
	{
		free(n->tab);
		free(n);
		s->lambda1 = 0;
		s->lambda2 = 1;
		return s;
	}
	else
	{
		// Initialisation de la solution (en faisant attention car on retourne l'avant dernière, sauf si une seule solution est énumérée)
		nprec = (solution *) malloc (sizeof(solution));
		nprec->tab = (int *) malloc ((d->nbItem) * sizeof(int)); 
		nprec->mult1 = s->mult1;
		nprec->mult2 = s->mult2;

		// Initialisation du lambda pour la dichotomie
		lambda1 = n->z2 - s->z2;
		lambda2 = s->z1 - n->z1;
		div = pgcd(lambda1,lambda2);
		lambda1 = lambda1 / div;
		lambda2 = lambda2 / div;
		
		fin = 0;
		while(fin == 0)
		{
			//printf("Pour (lambda1,lambda2) = (%lld,%lld), on obtient :\n",lambda1,lambda2);
		
			// Permutation de nprec et n (pour conserver correctement la mémoire allouée
			ntemp = nprec;
			nprec = n;
			n = ntemp;
		
			// résolution du problème pondéré
			singleOpt(lambda1,lambda2,s->mult1,s->mult2,d,n);
			computePoint(d,n);
		
			//PrintSolution(n,d->nbItem);
		
			// comparaison avec la solution *s
			val = lambda1 * s->z1 + lambda2 * s->z2; 
			valn = lambda1 * n->z1 + lambda2 * n->z2;
			if (val == valn) // lambda min avec solution associée obtenue, ainsi que la solution de remplacement
			{	
				free(n->tab);
				free(n);
				s->lambda1 = lambda1;
				s->lambda2 = lambda2;
				fin = 1;
				//computeSI(d,n); // Utile ici?
				nprec->lambda1 = -1; // indique que le (lambda1,lambda2) min à mu constant n'est pas calculé
			}
			else 
			{
				lambda1 = n->z2 - s->z2;
				lambda2 = s->z1 - n->z1;
				div = pgcd(lambda1,lambda2);
				lambda1 = lambda1 / div;
				lambda2 = lambda2 / div;
			}
		}
		return nprec;
	}
}

/* Fonction de calcul d'un lambda de changement de solution optimale pour le dual-surrogate pour une paire de solution donnée */

void computeBreak(solution *s1,solution *s2,long long *lambda1break, long long *lambda2break)
{
	long long div;

	if (s1->z1 > s2->z1)
	{
		*lambda1break = s2->z2 - s1->z2;
		*lambda2break = s1->z1 - s2->z1;
	}
	else 
	{
		*lambda1break = s1->z2 - s2->z2;
		*lambda2break = s2->z1 - s1->z1;	
	}
	
	div = pgcd(*lambda1break,*lambda2break);
	*lambda1break = *lambda1break / div;
	*lambda2break = *lambda2break / div;
}

/* Fonction de lancement (hésitations en cours) pour le calcul de l'OCSUB */

void startSurrogate(listeSol *L,donnees *d)
{
	//solution *s; // toujours utile?
	solution *s1;
	solution *s2;
	solution *n1;
	solution *n2;
	long long num1; // numérateur pour s1 à dénominateur commun avec s2
	long long num2; // numérateur pour s2 à dénominateur commun avec s1
	//long long den; // dénominateur commun // Ne semble plus utile
	long long lambda1break;
	long long lambda2break; // (lambda1,lambda2) de changement pour une même paire de solutions
	long long vals1prec;
	long long vals2prec; // valeur pour la fonction objectif pour le (lambda1,lambda2) initial
	long long vals1;
	long long vals2; // valeur pour la fonction objectif pour la (lambda1,lambda2) critique
	int ret; // valeur de retour dans la détermination d'un solution optimale pour le problème dual-surrogate
	
	//puts("Lancement de la résolution");
	
	ret = initDichoMu(d->maxZ2,1,&s1,&s2,d);
	if (ret == 0) startDichoMu(d->maxZ2,1,&s1,&s2,d);
	
	/*puts("");puts("Conclusion : la solution optimale du dual-surrogate est donnée par");
	if (s1 == NULL) PrintSolution(s2,d->nbItem);
	else if (s1->z1 < s2->z1) PrintSolution(s1,d->nbItem);
	else PrintSolution(s2,d->nbItem);
	puts("");*/	
	
	if (s1 == NULL) addSolInit(s2,d->nbItem,L);
	else if (s1->z1 < s2->z1) addSolInit(s1,d->nbItem,L);
	else addSolInit(s2,d->nbItem,L);
	
	//exit(0);
	
	long long lambda1prec = 1;
	long long lambda2prec = 0; // couple (lambda1,lambda2) de l'itération précédente
	
	while(lambda1prec != 0) // Boucle principale : on s'arrêtera lorsque lambda1 sera égal à 0
	{	//getchar();
		if (s1 == NULL) // Si on a qu'une solution en sortie de résolution du dual-surrogate mono-objectif
		{
			//puts("boucle principale : cas d'une solution admissible\n");
			// Obtention de la solution remplaçante au lambda critique (à mu constant)
			if (s2->lambda1 == -1) n2 = dichoPart(s2,d); 
			//puts("après dichopart");
			
			// Ajout de la solution s2 dans la liste avec son lambda critique
			addSolution(s2,d->nbItem,L);
			
			// MAJ de (lambda1,lambda2)
			lambda1prec = s2->lambda1;
			lambda2prec = s2->lambda2;

			// libération mémoire de s2
			free(s2->tab);
			free(s2);
			
			if (lambda1prec != 0) // Si on n'est pas encore à la fin, on réoptimise le dual-surrogate pour la prochaine itération
			{
				// Affectation du nouveau s2
				computeSI(d,n2); 
				s2 = n2;
			
				// Il faut ensuite compléter la résolution du dual-surrogate mono-objectif avant de relancer la boucle principale
				ret = initDichoMu1Sol(lambda1prec,lambda2prec,&s1,&s2,d);
				if (ret == 0) startDichoMu(lambda1prec,lambda2prec,&s1,&s2,d);
			}
			/*
			else // FIN, on libère également la mémoire de n2
			{
				free(n2->tab);
				free(n2);
			} */ // Plus utile, car libération mémoire effectuée dans dichopart
			//puts("boucle principale : cas d'une solution admissible (OK)\n");
		}
		else // Si on a deux solutions en sortie de résolution du dual-surrogate mono-objectif
		{
			//puts("boucle principale : cas classique (paire de solutions)\n");
			/*puts("paire considérée :");
			PrintSolution(s1,d->nbItem);
			PrintSolution(s2,d->nbItem);*/
			
			// Obtention de la solution remplaçante au lambda critique (à mu constant) pour les deux solutions			
			if (s1->lambda1 == -1) n1 = dichoPart(s1,d); 
			if (s2->lambda1 == -1) n2 = dichoPart(s2,d); // Il faudra vérifier qu'on se souvient de n1, n2!
			
			/*
			puts("paire de remplacement :");
			PrintSolution(n1,d->nbItem);
			PrintSolution(n2,d->nbItem);
			*/
			
			// mise au dénominateur commun
			num1 = s1->lambda1 * (s2->lambda1 + s2->lambda2);
			num2 = s2->lambda1 * (s1->lambda1 + s1->lambda2);
			
			// On vérifie s'il y a un changement de solution optimale pendant la diminution de lambda jusqu'à la valeur critique
			vals1prec = lambda1prec * s1->z1 + lambda2prec * s1->z2;
			vals2prec = lambda1prec * s2->z1 + lambda2prec * s2->z2;
			/*
			vals1 = s1->lambda1 * s1->z1 + s1->lambda2 * s1->z2;
			vals2 = s1->lambda1 * s2->z1 + s1->lambda2 * s2->z2; // mauvais placement
			*/
			if (num1 == num2) // Si les deux solutions disparaissent en même temps avec en chemin un changement éventuel de solution optimale
			{
				vals1 = s1->lambda1 * s1->z1 + s1->lambda2 * s1->z2;
				vals2 = s1->lambda1 * s2->z1 + s1->lambda2 * s2->z2;
				if (vals1prec <= vals2prec)
				{
					if (vals1 <= vals2) addSolution(s1,d->nbItem,L); // Pas de changement ajout "standard" de la solution s1
					else // On a s1 puis s2
					{
						// On doit calculer le (lambda1,lambda2) de break
						computeBreak(s1,s2,&lambda1break,&lambda2break);
					
						// ajout de s1 (jusqu'au break) puis de s2 (standard)
						addSolutionBreak(s1,lambda1break,lambda2break,d->nbItem,L);
						addSolution(s2,d->nbItem,L);
					}
				}
				else // Ici, on a vals2prec < vals1prec
				{
					if (vals2 <= vals1) addSolution(s2,d->nbItem,L); // Pas de changement ajout "standard" de la solution s2
					else // On a s2 puis s1
					{
						// On doit calculer le (lambda1,lambda2) de break
						computeBreak(s1,s2,&lambda1break,&lambda2break);

						// ajout de s2 (jusqu'au break) puis de s1 (standard)
						addSolutionBreak(s2,lambda1break,lambda2break,d->nbItem,L);
						addSolution(s1,d->nbItem,L);
					}
				}
				// MAJ de (lambda1,lambda2)
				lambda1prec = s1->lambda1;
				lambda2prec = s1->lambda2;
				
				// libération mémoire de s1 et s2
				free(s1->tab);
				free(s1);
				free(s2->tab);
				free(s2);
			
				if (lambda1prec != 0) // Si on n'est pas encore à la fin, on réoptimise le dual-surrogate pour la prochaine itération
				{
					// Affectation des nouveaux s1 et s2
					computeSI(d,n1); 
					s1 = n1;
					
					computeSI(d,n2); 
					s2 = n2;
					
					// Il faut ensuite compléter la résolution du dual-surrogate mono-objectif avant de relancer la boucle principale												
					ret = initDichoMu2Sol(lambda1prec,lambda2prec,&s1,&s2,d);
					if (ret == 0) startDichoMu(lambda1prec,lambda2prec,&s1,&s2,d);
				}
				/*
				else // FIN, on libère également la mémoire de n1 et n2
				{
					free(n1->tab);
					free(n1);
					free(n2->tab);
					free(n2);
				} */ // Plus utile, car libération mémoire effectuée dans dichopart
			}
			else // Une solution disparaîtra avant l'autre
			{	
				if (num1 > num2) // s1 disparaîtra le premier 
				{
					vals1 = s1->lambda1 * s1->z1 + s1->lambda2 * s1->z2;
					vals2 = s1->lambda1 * s2->z1 + s1->lambda2 * s2->z2;
					if (vals1prec <= vals2prec)
					{
						if (vals1 <= vals2) addSolution(s1,d->nbItem,L); // Ajout "standard" de la solution s1 dans la liste
						else // On a s1 puis s2 
						{
							//puts("cas 1");
							// On doit calculer le (lambda1,lambda2) de break
							computeBreak(s1,s2,&lambda1break,&lambda2break);
							
							// Ajout de s1 (jusqu'au break) puis de s2 (limité à la disparition de s1)
							addSolutionBreak(s1,lambda1break,lambda2break,d->nbItem,L);
							addSolutionBreak(s2,s1->lambda1,s1->lambda2,d->nbItem,L);
						}
					}
					else // Ici, on a vals2prec < vals1prec
					{
						if (vals2 <= vals1) addSolutionBreak(s2,s1->lambda1,s1->lambda2,d->nbItem,L); // Ajout de la solution s2 dans la liste (limité à la disparition de s1)
						else // On a s2 puis s1
						{
							// On doit calculer le (lambda1,lambda2) de break
							computeBreak(s1,s2,&lambda1break,&lambda2break);
							
							// Ajout de s2 (jusqu'au break) puis de s1 (standard)
							addSolutionBreak(s2,lambda1break,lambda2break,d->nbItem,L);
							addSolution(s1,d->nbItem,L);
						}
					}
					// MAJ de (lambda1,lambda2)
					lambda1prec = s1->lambda1;
					lambda2prec = s1->lambda2;
					
					// libération mémoire du précédent s1 remplacé par n1 (Remarque : ici, on a nécessairement lambda1prec > 0)
					free(s1->tab);
					free(s1);
					computeSI(d,n1); 
					s1 = n1;
					
					// Il faut ensuite compléter la résolution du dual-surrogate mono-objectif avant de relancer la boucle principale
					ret = initDichoMu2Sol(lambda1prec,lambda2prec,&s1,&s2,d);
					if (ret == 0) startDichoMu(lambda1prec,lambda2prec,&s1,&s2,d);
				}
				else // s2 disparaîtra le premier
				{	
					vals1 = s2->lambda1 * s1->z1 + s2->lambda2 * s1->z2;
					vals2 = s2->lambda1 * s2->z1 + s2->lambda2 * s2->z2;
					if (vals1prec <= vals2prec)
					{
						if (vals1 <= vals2) addSolutionBreak(s1,s2->lambda1,s2->lambda2,d->nbItem,L); // Ajout de la solution s1 dans la liste (limité à la disparition de s2)
						else // On a s1 puis s2 
						{
							// On doit calculer le (lambda1,lambda2) de break
							computeBreak(s1,s2,&lambda1break,&lambda2break);
						
							// Ajout de s1 (jusqu'au break) puis de s2 (standard)
							addSolutionBreak(s1,lambda1break,lambda2break,d->nbItem,L);
							addSolution(s2,d->nbItem,L);
						}
					}
					else // Ici, on a vals2prec < vals1prec
					{
						//puts("cas 2");
						if (vals2 <= vals1) addSolution(s2,d->nbItem,L); // Ajout "standard" de la solution s2
						else // On a s2 puis s1
						{
							// On doit calculer le (lambda1,lambda2) de break
							computeBreak(s1,s2,&lambda1break,&lambda2break);
						
							// Ajout de s2 (jusqu'au break) puis de s1 (limité à la disparition de s2)
							addSolutionBreak(s2,lambda1break,lambda2break,d->nbItem,L);
							addSolutionBreak(s1,s2->lambda1,s2->lambda2,d->nbItem,L);
						}
					}
					// MAJ de (lambda1,lambda2)
					lambda1prec = s2->lambda1;
					lambda2prec = s2->lambda2;
					
					// libération mémoire de s2 avant son remplacement (Remarque : ici, on a nécessairement lambda1prec > 0)
					free(s2->tab);
					free(s2);
					computeSI(d,n2); 
					s2 = n2;
					
					// Il faut ensuite compléter la résolution du dual-surrogate mono-objectif avant de relancer la boucle principale
					ret = initDichoMu2Sol(lambda1prec,lambda2prec,&s1,&s2,d);
					if (ret == 0) startDichoMu(lambda1prec,lambda2prec,&s1,&s2,d);
				}
			}
			//puts("boucle principale : cas classique (paire de solutions) OK\n");
		}
	}
}

point first_point;		/* first hull point */

convex_hull(point in[], int n, polygon *hull)
{
	int i;			/* input counter */
	int top;		/* current hull size */
	bool smaller_angle();
	
	if (n <= 3) { 		/* all points on hull! */
		for (i=0; i<n; i++)
			copy_point(in[i],hull->p[i]);
		hull->n = n;
		return;
	}
	
	sort_and_remove_duplicates(in,&n);
	copy_point(in[0],&first_point);
	
	qsort(&in[1], n-1, sizeof(point), smaller_angle);
	
	copy_point(first_point,hull->p[0]);
	copy_point(in[1],hull->p[1]);
	
	copy_point(first_point,in[n]);	/* sentinel to avoid special case */
	top = 1;
	i = 2;
	
	while (i <= n) {
		if (!ccw(hull->p[top-1], hull->p[top], in[i])) 
			top = top-1;	/* top not on hull */
		else {
			top = top+1;
			copy_point(in[i],hull->p[top]);
			i = i+1;
		}
	}
	
	hull->n = top;
}


sort_and_remove_duplicates(point in[], int *n)
{
	int i;                  /* counter */
	int oldn;               /* number of points before deletion */
	int hole;               /* index marked for potential deletion */
	bool leftlower();
	
	qsort(in, *n, sizeof(point), leftlower);
	
	oldn = *n;
	hole = 1;
	for (i=1; i<oldn; i++) {
		if ((in[hole-1][X] == in[i][X]) && (in[hole-1][Y] == in[i][Y])) 
			(*n)--;
		else {
			copy_point(in[i],in[hole]);
			hole = hole + 1;
		}
	}
	copy_point(in[oldn-1],in[hole]);
}

bool leftlower(point *p1, point *p2)
{
	if ((*p1)[X] < (*p2)[X]) return (-1);
	if ((*p1)[X] > (*p2)[X]) return (1);
	
	if ((*p1)[Y] < (*p2)[Y]) return (-1);
	if ((*p1)[Y] > (*p2)[Y]) return (1);
	
	return(0);
}

bool smaller_angle(point *p1, point *p2)
{
	if (collinear(first_point,*p1,*p2)) {
		if (distance(first_point,*p1) <= distance(first_point,*p2))
			return(-1);
		else
			return(1);
	}
	
	if (ccw(first_point,*p1,*p2))
		return(-1);
	else
		return(1);
}

/* Fonction qui filtre les cas de dégénérescence 

void filtre(listeSol *L, listeSol *Lfiltre) 
{
	int i;
	
	
	
	
}
 */
 

int main(int argc,char *argv[])
{
	donnees d;
	listeSol L;
	L.lgListe = 0;
	double temps = 0;
	long long number; // utilisé pour la détermination des points duaux pour le calcul de l'enveloppe inférieure
	
	// Chargement de l'instance
	
	loadInstance(argv[1],&d);
	
	// Lancement de la méthode de calcul paramétrique de l'OCSUB
	
	crono_start(); // GO!
	
	startSurrogate(&L,&d);

	// Calcul de l'enveloppe inférieure des droites obtenues
	
	point in[MAXPOLY];		// input points 
	polygon hull;			// convex hull 
	int n;				// number of points 
	int i;				// counter 
	int j; // un autre compteur

	/* Il faudra faire attention aux points duaux identiques pour obtenir les bons points d'intersection, le plus simple est de traiter la liste initiale */
	
	/*
	listeSol Lfiltre;
	Lfiltre.lgListe = 0;
	
	filtre(&L,&Lfiltre);
	*/
	
	n = (L.lgListe) + 2;
	in[0][X] = -100000000; in[0][Y] = -100000000; /* point fictif pour obtenir un polytope */
	in[1][X] = 100000000 /((double) (L.liste[0]).z1); in[1][Y] = 0; /* premier point (lambda = 1) */
	for(i = 0;i < L.lgListe - 1;i++) 
	{
		number = L.lambda1[i] * (L.liste[i]).z1 + L.lambda2[i] * (L.liste[i]).z2;
		in[i+2][X] = (L.lambda1[i] * 100000000) /((double) number);
		in[i+2][Y] = (L.lambda2[i] * 100000000) /((double) number);
	}
	in[n-1][X] = 0; in[n-1][Y] = 100000000 /((double) (L.liste[L.lgListe - 1]).z2);
	
	for(i = 0;i < n;i++) printf("(%lf,%lf)\n",in[i][X],in[i][Y]);
	puts("\n");
	 
	convex_hull(in,n,&hull);
	
	crono_stop();
	temps = crono_ms()/1000,0;
	
	// Calcul des intersection pour obtenir un format comparable avec Audrey
	/*	
	i = 1;
	while(in[i+1][Y] == 0) i++;	// Filtrage des faiblement non-dominés pour un calcul correct des intersections
	int deb = i;
	int fin; // pour le test d'appartenance
	
	int indout[MAXPOLY];
	indout[0] = deb;
	int taille = 1;
	int debj = 2; // car on connaît le contenu des deux premières cases
	for(i = deb + 1;i < n + 1;i++) 
	{
		j = 2; 
		while(fin == 0)
		{
			if ( (in[i][X] == hull.p[j][X]) && (in[i][X] == hull.p[j][Y]) ) 
			{
				indout[taille++] = i;
				fin = 1;
			}
			j++;
		}
	}
	
	for(i = 0;i < taille;i++) printf("%d  ",indout[i]);
	puts("");
	*/
	
	// Affichage des résultats
	
	PrintResult(&L);
	
	puts("");
	
	printf("Points duaux définissant l'enveloppe convexe :\n");
	print_polygon(&hull);

	puts("");
	
	printf("temps : %f\n",temps);	
	
	// J'adore qu'un plan se déroule sans accroc!
	
	return 0;
}
