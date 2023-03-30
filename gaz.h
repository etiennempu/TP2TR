#ifndef GAZ
#define GAZ

typedef struct Gaz {
    int indice;
    int* value;
    int* alerte;
    int aug;
} Gaz;

struct Gaz* newGaz(int index);
void freeGaz(struct Gaz* gaz);
void majgaz(struct Gaz* gaz, int fuite);
int alerte_max(struct Gaz** gaz, int num_gaz);
int aug_max(struct Gaz** gaz, int nb_gaz);
int danger(struct Gaz* gaz);

#endif