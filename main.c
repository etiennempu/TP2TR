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

typedef struct gaz {
    int indice;
    double value;
    int alerte;
    int injection;
    int period;
} gaz;

void* ecoute(void* arg) {
    while(1) {
        char* message = ReceiveMessage();
        printf("%s", message);
        switch (atoi(&message[2]))
        {
        case 1:
            /* On met dans le gaz 1 */
            break;
        case 2:
            /* On met dans le gaz 2 */
        case 3:
            /* On met dans le gaz 3 */
            break;
        }
        free(message);
    }
}

void * leds(void* arg) {
    // TODO: Implement this function
}

void * controle(void* args) {
    struct gaz* data = (struct gaz*) args;
    while(1) {
        sem_wait(&verrou_gaz[data->indice]);
        double value = data->value;
        pthread_mutex_lock(&mutex_alerte[data->indice]);
        if (value < ALERTE_B) data->alerte = 0;
        else if (value < ALERTE_M) data->alerte = 1;
        else if (value < ALERTE_H) data->alerte = 2;
        else {
            if (value >=  INJECTION) data->injection = 1; //Lancer injection gaz
            data->alerte = 3;
        }
        pthread_mutex_unlock(&mutex_alerte[data->indice]);
        sem_post(&verrou_gaz[data->indice]);
        usleep(data->period);
    }
}

void * action(void* args){
    struct gaz** data = (struct gaz**) args;
    while (1) {
        int alerte_total = 0;
        for (int i = 0; i < 3; i++) {
            pthread_mutex_lock(&mutex_alerte[i]);
            alerte_total += data[i]->alerte;
            pthread_mutex_unlock(&mutex_alerte[i]);
        }
        if (alerte_total >= 5) {
            SendMessage("ALERTE ROUGE\n");
        } else if (alerte_total >= 3) {
            SendMessage("ALERTE ORANGE\n");
        } else if (alerte_total >= 1) {
            SendMessage("ALERTE JAUNE\n");
        } else {
            SendMessage("TOUT VA BIEN\n");
        }
        sleep(5);
    }
}

struct gaz* newGaz(int index) {
    struct gaz* gaz = malloc(sizeof(struct gaz));
    gaz->indice = index;
    gaz->value = malloc(sizeof(double));
    gaz->alerte = 0;
    gaz->injection = 0;
    return gaz;
}

// Fonction qui calcule le coût en fonction des actions prises et de leurs niveaux de graduation
int calcul_cout(int* actions, int taille_actions) {
    int cout = 0;
    int i;
    for (i = 0; i < taille_actions; i++) {
        switch (actions[i]) {
            case 1: // Aération niveau 1
                cout += 1;
                break;
            case 2: // Aération niveau 2
                cout += 2;
                break;
            case 3: // Aération niveau 3
                cout += 3;
                break;
            case 4: // Ventilation niveau 1
                cout += 6;
                break;
            case 5: // Ventilation niveau 2
                cout += 9;
                break;
            case 6: // Injection de gaz
                cout += 20;
                break;
            case 7: // Commande d'annulation
                cout += 0;
                break;
            default:
                printf("Action invalide");
                return -1;
        }
    }
    return cout;
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


    struct gaz** args_action = malloc(3*sizeof(struct gaz*));
    for (int i=0; i++; i<3) {
        args_action[i] = newGaz(i);
    }

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