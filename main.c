#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gaz.h"
#include "serveur.h"
#include "client.h"
#include "utils.h"
//#include "leds.h"

#define NUM_GAZ 3
#define NUM_THREADS 4+NUM_GAZ

#define SCHED_HAUTE 3
#define SCHED_MOYEN 2
#define SCHED_BASSE 1

// Définition des seuils d'alerte

#define ALERTE_B 20
#define ALERTE_M 40
#define ALERTE_H 60
#define INJECTION 80

// Définition des seuils des fuites

#define FUITE {0, 1, 3, 6, 7, 9, 11, 12, 14}

int run = 1; // Indiquer l'arrêt des tâches
sem_t verrou_controle[NUM_GAZ]; // Synchroniser les tâches "contrôle" avec l'écoute
pthread_mutex_t mutex_alerte[NUM_GAZ] = { PTHREAD_MUTEX_INITIALIZER };

void* ecoute(void* args) {
    struct Gaz** gaz = (struct Gaz**) args;

    char* buffer;
    /* Le système ne reçoit des messages que si le logiciel de simulation communique */
    while(strcmp((buffer = ReceiveMessage()), "") != 0) {
        char** cmds = parser(buffer, "\n");
        char** shards;

        while( cmds != NULL ) {
            char c[1] = { *(*(cmds)+2) };
            int i = atoi(c);
            shards = parser(*(cmds), c);
            int fuite = atoi(*(shards+1));
            majgaz(gaz[i-1], fuite);
            sem_post(&verrou_controle[i-1]);
            cmds = loop_parser(cmds, "\n");
            free(shards);
        }

        free(cmds);
        free(buffer);
    }
    free(buffer);
    run = 0;
    for (int i=0; i<3; i++) {
        //Pour libérer les tâches de contrôle. Elles effectuent leur dernier tout de boucle puis elles sortent de la boucle.
        sem_post(&verrou_controle[i]);
    }
    pthread_exit(0);
}

void * leds(void* arg) {
    //Allumer les leds de la Sense Hat
}

void * controle(void* arg) {
    // Effectue le contrôle d'un gaz
    struct Gaz gaz = *(struct Gaz*) arg;

    // Des variables locales pour ajuster les actions et éviter d'envoyer des messages inutiles.
    int tmp = 0;
    int old_alerte = 0;

    char message[5+NUM_GAZ/10];
    const char* c;
    while(run) {
        sem_wait(&verrou_controle[gaz.indice]);
        int taux = *(gaz.value);
        if (taux != tmp) {
            if (taux < ALERTE_B) {
                *(gaz.alerte) = 0;
                c = "";
            }
            else if (taux < ALERTE_M) {
                *(gaz.alerte) = 1;
                c = "L";
            }
            else if (taux < ALERTE_H) {
                *(gaz.alerte) = 2;
                c = "M";
            }
            else if (taux < INJECTION) {
                *(gaz.alerte) = 3;
                c = "H";
            }
            else *(gaz.alerte) = 4;
            
            if (*(gaz.alerte) != old_alerte) {
                sprintf(message, "AG%d", gaz.indice+1);
                strcat(message, c);
                old_alerte = *(gaz.alerte);
                SendMessage(message);
            }
            tmp = taux;
        }
        else {
            // Si pas de changement alors peut-être l'action ne fait que compenser donc augmentaion artificielle de la fuite pour pousser à faire une action plus forte
            if (tmp != 0) gaz.aug++;
            // Ou alors la fuite a déjà baissé et on est encore en train d'utiliser une action trop importante
            else gaz.aug = (gaz.aug > 0) ? gaz.aug-- : 0;
        }
    }
    pthread_exit(0);
}

void reaction(int pire_fuite, int* niveau, int alerte_max) {
    int seuil_fuite[] = FUITE;
    
    // Ajustement ventilation
    if (niveau[1] != alerte_max) {
        switch (alerte_max)
        {
        case 0:
            SendMessage("VN");
            break;
        case 1:
            SendMessage("VL1");
            break;
        default:
            SendMessage("VL2");
        }
        niveau[1] = (alerte_max <= 2) ? alerte_max : 2;
    }

    // Ajustement aération
    int s = 3 * alerte_max;
    if (pire_fuite <= seuil_fuite[s] && niveau[0] != 0) {
        SendMessage("AN");
        niveau[0] = 0;
    }
    else if (pire_fuite <= seuil_fuite[s+1] && niveau[0] != 1) {
        SendMessage("AL1");
        niveau[0] = 1;
    }
    else if (pire_fuite <= seuil_fuite[s+2] && niveau[0] != 2) {
        SendMessage("AL2");
        niveau[0] = 2;
    }
    else if (niveau[0] != 3) {
        SendMessage("AL3");
        niveau[0] = 3;
    }
}

void reaction_max(int* niveau) {
    // La fuite est très grande toute les actions au maximum
    if (niveau[0] != 3) {
        SendMessage("AL3");
        niveau[0] = 3;
    }
    if (niveau[1] != 2) {
        SendMessage("VL2");
        niveau[1] = 2;
    }
}

void * air(void* args){
    // Contrôle de l'aération et de la ventilation
    int niveau[2] = { 0 };
    while (run) {
        sleep(1);
        //Mise à jour des actions toutes les secondes ou autres
        if (alerte_max((struct Gaz**) args, NUM_GAZ) < 3) {
            reaction(aug_max((struct Gaz**) args, NUM_GAZ), niveau, alerte_max((struct Gaz**) args, NUM_GAZ));
        }
        else {
            reaction_max(niveau);
        }
    }
}

void * injection(void* args) {
    // En vrai le logiciel de simulation n'a pas de fuite suffisamment importante pour que les injections soient nécessaires.
    struct Gaz** gaz = (struct Gaz**) args;
    int bool[NUM_GAZ] = { 0 };
    char str[5];
    while(run) {
        sleep(1);
        for (int i=0; i<NUM_GAZ; i++) {
            if (!bool[i] && danger(*(gaz+i))) {
                sprintf(str, "%s%d", "IG", i+1);
                bool[i] = 1;
                SendMessage(str);
            }
            else if (bool[i] && !danger(*(gaz+i))) {
                sprintf(str, "%s%d", "AIG", i+1);
                bool[i] = 0;
                SendMessage(str);
            }
        }
    }
}

int main(int argc, char** argv) {

    /* Initialisation des sémaphores */
    for (int i=0;i++;i<3) {
        sem_init(&verrou_controle[i], 0, 1);
        sem_wait(&verrou_controle[i]);
    }

    /* Préparation des paramètres pour la création des tâches */
    pthread_t* thread = calloc(NUM_THREADS, sizeof(pthread_t));

    void * functions[NUM_THREADS] = {ecoute, [1 ... NUM_GAZ] = controle, air, injection, leds};
    struct sched_param sched[NUM_THREADS] = {[0 ... NUM_GAZ] = SCHED_HAUTE, SCHED_MOYEN, SCHED_MOYEN, SCHED_BASSE};
    int sched_policy[NUM_THREADS] = {SCHED_FIFO, [1 ... NUM_GAZ] = SCHED_RR, [NUM_GAZ+1 ... NUM_THREADS-1] = SCHED_FIFO};

    struct Gaz* args_action[NUM_GAZ];
    for (int i=0; i<NUM_GAZ; i++) {
        args_action[i] = newGaz(i);
    }

    void* restrict args[NUM_THREADS] = { [NUM_GAZ+1 ... NUM_THREADS-1] = args_action };
    args[0] = args_action;
    for (int i=0; i<NUM_GAZ; i++) {
        args[i+1] = args_action[i];
    }

    /* Création des sockets pour la communication */
    OuvrirServeur();
    OuvrirClient();

    /* Attendre le lancement de la simulation */
    AttenteOuvertureServeur();
    AttenteOuvertureClient();

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

    /* Fermeture des sockets */
    FermerServeur();
    FermerClient();

    for (int i=0; i<NUM_GAZ; i++) {
        freeGaz(args_action[i]);
    }
    free(thread);

}