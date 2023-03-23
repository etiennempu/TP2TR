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
    return gaz;
}

void freeGaz(struct Gaz* gaz) {
    free(gaz->value);
    free(gaz);
}

void* ecoute(void* arg) {
    struct Gaz* gaz = *(struct Gaz**) arg;
    while(1) {
        char* message = ReceiveMessage();
        printf("%s\n", message);
        switch (*(message+2))
        {
        case '1':
            sscanf(message, "%*[A-Z]1%d\n", gaz[0].value);
            break;
        case '2':
            sscanf(message, "%*[A-Z]2%d\n", gaz[1].value);
            break;
        case '3':
            sscanf(message, "%*[A-Z]3%d\n", gaz[2].value);
            break;
        }
        free(message);
    }
}

void * leds(void* arg) {
}

void * controle(void* args) {
    // Effectue le contrôle d'un gaz
    /*struct gaz data = *(struct gaz*) args;
    while(1) {
        sem_wait(&verrou_gaz[data.indice]);
        double value = *(data.value);
        pthread_mutex_lock(&mutex_alerte[data.indice]);
        if (value < ALERTE_B) data.alerte = 0;
        else if (value < ALERTE_M) data.alerte = 1;
        else if (value < ALERTE_H) data.alerte = 2;
        else {
            if (value >=  INJECTION) data.injection = 1; //Lancer injection gaz
            data.alerte = 3;
        }
        pthread_mutex_unlock(&mutex_alerte[data.indice]);
        sem_post(&verrou_gaz[data.indice]);
    }*/
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

    struct Gaz* args_action[3] = {newGaz(1), newGaz(2), newGaz(3)};

    void* restrict args[NUM_THREADS] = {
        args_action,
        args_action,
        args_action[1],
        args_action[2],
        args_action[3],
        args_action
    };

    OuvrirServeur();
    OuvrirClient();

    /* On effectue la création des threads/tâches */

    for (int i=0; i<NUM_THREADS; i++) {
        if (pthread_create(&thread[i], NULL, functions[i], args[i]) != 0) {
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