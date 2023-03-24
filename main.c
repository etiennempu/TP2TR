#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "serveur.h"
#include "client.h"
//#include "leds.h"

#define NUM_THREADS 6

#define ALERTE_B 5
#define ALERTE_M 15
#define ALERTE_H 30
#define INJECTION 60

sem_t verrou_gaz[3]; // Pour synchroniser la réception d'une valeur pour un gaz avec son contrôle
pthread_mutex_t mutex_alerte[3] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

typedef struct Gaz {
    int indice;
    int* value;
    int alerte;
    int injection;
    int period;
} Gaz;

struct Gaz* newGaz(int index) {
    struct Gaz* gaz = malloc(sizeof(struct Gaz));
    gaz->indice = index;
    gaz->value = malloc(sizeof(int));
    gaz->period = 1;
    return gaz;
}

void freeGaz(struct Gaz* gaz) {
    free(gaz->value);
    free(gaz);
}

void* ecoute(void* arg) {
    struct Gaz* gaz_1 = *((struct Gaz**) arg);
    struct Gaz* gaz_2 = *((struct Gaz**) arg + 1);
    struct Gaz* gaz_3 = *((struct Gaz**) arg + 2);
    while(1) {
        char* message = ReceiveMessage();
        switch (*(message+2))
        {
        case '1':
            sscanf(message, "LG1%d\n", gaz_1->value);
            break;
        case '2':
            sscanf(message, "LG2%d\n", gaz_2->value);
            break;
        case '3':
            sscanf(message, "LG3%d\n", gaz_3->value);
            break;
        }
        free(message);
    }
}

void * leds(void* arg) {
}

void * controle(void* arg) {
    // Effectue le contrôle d'un gaz
    /*struct Gaz gaz = *(struct Gaz*) arg;
    while(1) {
        int taux = *(gaz.value);
        if (taux < ALERTE_B) gaz.alerte = 0;
        else if (taux < ALERTE_M) gaz.alerte = 1;
        else if (taux < ALERTE_H) gaz.alerte = 2;
        else {
            if (taux >=  INJECTION) gaz.injection = 1; //Lancer injection gaz
            gaz.alerte = 3;
        }
    }
    */
}

// Je pense qu'il faut fusionner aeration et ventilation, parce qu'ils sont interdépendants

void * action(void* args){
    //args est une liste de struct gaz
    //while (1) {
        //Mise à jour des actions toutes les secondes ou autres*
    //    sleep(1);
    //}
}

int main(int argc, char** argv) {

    for (int i=0;i++;i<3) sem_init(&verrou_gaz[i], 0, 1);

    pthread_t* thread = calloc(NUM_THREADS, sizeof(pthread_t));
    void * functions[NUM_THREADS] = {
        ecoute,
        leds,
        controle,
        controle,
        controle,
        action,
    };
    struct sched_param sched[NUM_THREADS];
    int sched_policy[NUM_THREADS] = {SCHED_FIFO, SCHED_FIFO, SCHED_RR, SCHED_RR, SCHED_RR, SCHED_FIFO};

    struct Gaz* args_action[3] = {newGaz(0), newGaz(1), newGaz(2)};

    void* restrict args[NUM_THREADS] = {
        args_action,
        args_action,
        args_action[0],
        args_action[1],
        args_action[2],
        args_action
    };

    OuvrirServeur();
    OuvrirClient();

    /* On effectue la création des threads/tâches */

    for (int i=0; i<NUM_THREADS; i++) {
        if (pthread_create(&thread[i], NULL, functions[i], (void *)args[i]) != 0) {
            perror("pthreac_create() :");
            exit(1);
        }
        pthread_setschedparam(thread[i], sched_policy[i], &sched[i]);
        printf("Creation Thread %d\n", i);
    }

    /* On attend la fin des threads/tâches */

    for (int j=0; j<NUM_THREADS; j++) {
        pthread_join(thread[j], NULL);
        printf("Sortie Thread %d\n", j);
    }

    FermerServeur();
    FermerClient();

}