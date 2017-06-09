#ifndef GRAPHE_H
#define GRAPHE_H

typedef struct probleme probleme;
typedef struct noeud noeud;

struct probleme {
	unsigned int n;				// nombre d'objets
	unsigned int capacite;		// capacité du sac à dos
	unsigned int *coefficients;	// coefficients des variables dans la fonction objectif
	unsigned int *poids;		// poids des variables dans la fonction objectif
};

struct noeud {
	unsigned int val;		// valeur de la fonction objectif
	unsigned int poids;		// poids total utilisé dans le sac
	noeud *precBest;		// noeud précédent donnant le meilleur chemin
	noeud *precAlt;			// noeud précédent alternatif au meilleur chemin
};

probleme *genererProblemeTest();
noeud ***genererGraphe(probleme *p, unsigned int *n);
void afficherGraphe(noeud *racine, int n);

#endif