
typedef struct Tas {
	int *tab;
	unsigned int n;
	unsigned int taille;
} Tas;

void TAS_initialiser(Tas *tas, unsigned int taille);
void TAS_ajouter(Tas *tas, int val);
void TAS_retirerMax(Tas *tas);
void TAS_retirerInd(Tas *tas, int ind);
void TAS_minimum(Tas *tas);
void TAS_maximum(Tas *tas);