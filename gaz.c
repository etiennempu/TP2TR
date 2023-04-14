#include <stdlib.h>
#include <stdio.h>

#include "gaz.h"

#define ALERTE_MAX 4

struct Gaz* newGaz(int index) {
    struct Gaz* gaz = malloc(sizeof(struct Gaz));
    gaz->indice = index;
    gaz->value = malloc(sizeof(int));
    gaz->alerte = malloc(sizeof(int));
    gaz->aug = 0;
    gaz->period = 1;
    return gaz;
}

void freeGaz(struct Gaz* gaz) {
    free(gaz->value);
    free(gaz->alerte);
    free(gaz);
}

void majgaz(struct Gaz* gaz, int fuite) {
    int diff = fuite - *(gaz->value);
    if (diff > gaz->aug) gaz->aug = diff;
    *gaz->value = fuite;
}

int alerte_max(struct Gaz** gaz, int nb_gaz) {
    int max = 0;
    int i = 0;
    while (i < nb_gaz && max != ALERTE_MAX) {
        max = (max < *(gaz[i]->alerte)) ? *(gaz[i]->alerte) : max;
        i++;
    }
    return max;
}

int aug_max(struct Gaz** gaz, int nb_gaz) {
    int max = 0;
    int i = 0;
    while (i < nb_gaz && max != ALERTE_MAX) {
        max = (max < gaz[i]->aug) ? gaz[i]->aug : max;
        i++;
    }
    return max;
}

void aug_adapt(struct Gaz* gaz, int tmp) {
    if (tmp != 0) gaz->aug++;
        // Ou alors la fuite a déjà baissé et on est encore en train d'utiliser une action trop importante
    else gaz->aug = (gaz->aug > 0) ? gaz->aug-- : 0;
}

void up_period(struct Gaz* gaz) {
    gaz->period++;
}

void down_period(struct Gaz* gaz, int last_wait) {
    gaz->period = last_wait;
}