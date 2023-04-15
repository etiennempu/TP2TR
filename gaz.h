#ifndef GAZ
#define GAZ

typedef struct Gaz {
    int indice;
    int* value;
    int* alerte;
    int aug;
    int period;
} Gaz;

struct Gaz* newGaz(int index);
void freeGaz(struct Gaz* gaz);
void majgaz(struct Gaz* gaz, int fuite);
int alerte_max(struct Gaz** gaz, int num_gaz);
int aug_max(struct Gaz** gaz, int nb_gaz);
void aug_adapt(struct Gaz* gaz, int tmp);
void up_period(struct Gaz* gaz);
void down_period(struct Gaz* gaz, int last_wait);

#endif