#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gaz.h"
#include "serveur.h"
#include "client.h"
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

/*
Les combinaisons d'actions (utiles) devraient être
AN ; VN -> Pas de fuite
A1 ; VN -> Micro fuite (aug == 1 ; et value < 5)
A2 ; VN -> Mini fuite (aug == 2-3 et value < 5 ou aug == 1 et value > 5 (pcq signifie que A1 marche pas))
A3 ; VN -> Petite fuite (aug == 4-5 et value < 10 ou aug == 2-3 et value > 10)

ALERTE_B
AN ; V1 -> Fuite (aug == 5-6 et à partir de alerte_M)
A1 ; V1
A2 ; V1
A3 ; V1

ALERTE_M
AN ; V2 -> Grosse fuite (aug == 8-9 et à partir de alerte_H)
A1 ; V2
A2 ; V2

ALERTE_H
A3 ; V3
INJECTION -> Kill la fuite jusqu'à alerte_M
*/

sem_t verrou_controle[NUM_GAZ]; // Synchroniser les tâches "contrôle" avec l'écoute

pthread_mutex_t mutex_alerte[NUM_GAZ] = { PTHREAD_MUTEX_INITIALIZER };

void* ecoute(void* args) {
    struct Gaz** gaz = (struct Gaz**) args;
    char* str = malloc(4*sizeof(char));
    int fuite;
    while(1) {
        char* message = ReceiveMessage();

        const char c[1] = { *(message+2) }; 
        sprintf(str, "%s%s", "LG", c);
        int i = atoi(c) - 1;
        sscanf(message, strcat(str, "%d\n"), &fuite);
        majgaz(gaz[i], fuite);
        sem_post(&verrou_controle[i]);
        
        free(message);
    }
    free(str);
}

void * leds(void* arg) {
    //Allumer les leds de la Sense Hat
}

void * controle(void* arg) {
    // Effectue le contrôle d'un gaz
    struct Gaz gaz = *(struct Gaz*) arg;
    int tmp = 0;
    char* message = malloc(4*sizeof(char));
    while(1) {
        sem_wait(&verrou_controle[gaz.indice]);
        int taux = *(gaz.value);
        if (taux != tmp) {
            if (taux < ALERTE_B) *(gaz.alerte) = 0;
            else if (taux < ALERTE_M) {
                *(gaz.alerte) = 1;
                sprintf(message, "AG%dL", gaz.indice+1);
                SendMessage(message);
            }
            else if (taux < ALERTE_H) {
                *(gaz.alerte) = 2;
                sprintf(message, "AG%dM", gaz.indice+1);
                SendMessage(message);
            }
            else if (taux < INJECTION) {
                *(gaz.alerte) = 3;
                sprintf(message, "AG%dH", gaz.indice+1);
                SendMessage(message);
            }
            else {
                *(gaz.alerte) = 4;
            }
            tmp = taux;
        }
        else {
            // Si pas de changement alors peut-être l'action ne fait que compenser
            // donc augmentaion artificielle de la fuite pour pousser à faire une action plus forte
            if (tmp != 0) gaz.aug++;

            // Ou alors la fuite a baissé et je la matraque avec une action trop importante
            else {
                gaz.aug = (gaz.aug > 0) ? gaz.aug-- : 0;
            }
        }
    }
    free(message);
}


void reaction_sans_alerte(int pire_fuite, int* niveau) {
    // Si la ventilation  n'est pas coupé, je la coupe ici
    if (niveau[1] != 0) {
        SendMessage("VN");
        niveau[1] = 0;
    }
    
    // Les vraies actions
    if (pire_fuite == 0 && niveau[0] != 0) {
        SendMessage("AN");
        niveau[0] = 0;
    }
    else if (pire_fuite == 1 && niveau[0] != 1) {
        SendMessage("AL1");
        niveau[0] = 1;
    }
    else if (pire_fuite <= 3 && niveau[0] != 2) {
        SendMessage("AL2");
        niveau[0] = 2;
    }
    else if (niveau[0] != 3) {
        SendMessage("AL3");
        niveau[0] = 3;
    }
}

void reaction_alerte_basse(int pire_fuite, int* niveau) {
    // Si on atteint l'alerte basse c'est que l'aération n'était pas suffisante.
    // Donc lance la ventilation de niveau 1.
    if (niveau[1] != 1) {
        SendMessage("VL1");
        niveau[1] = 1;
    }

    // Puis on ajuste l'aération selon l'augmentation
    // la suite c'est du code duppliqué à rendre plus propre.
    if (pire_fuite <= 6 && niveau[0] != 0) {
        SendMessage("AN");
        niveau[0] == 0;
    }
    else if (pire_fuite <= 7 && niveau[0] != 1) {
        SendMessage("AL1");
        niveau[0] = 1;
    }
    else if (pire_fuite <= 9 && niveau[0] != 2) {
        SendMessage("AL2");
        niveau[0] = 2;
    }
    else if (pire_fuite <= 11 && niveau[0] != 3) {
        SendMessage("AL3");
        niveau[0] = 3;
    }

}

void reaction_alerte_moy(int pire_fuite, int* niveau) {
    // Si on atteint l'alerte moyenne la ventilation de niveau est insuffisante
    // on passe à la niveau 2
    if (niveau[1] != 2) {
        SendMessage("VL2");
        niveau[1] = 2;
    }

    // Puis on ajuste l'aération selon l'augmentation
    // la suite c'est du code duppliqué à rendre plus propre.
    if (pire_fuite <= 11 && niveau[0] != 0) {
        SendMessage("AN");
        niveau[0] == 0;
    }
    else if (pire_fuite <= 12 && niveau[0] != 1) {
        SendMessage("AL1");
        niveau[0] = 1;
    }
    else if (pire_fuite <= 14 && niveau[0] != 2) {
        SendMessage("AL2");
        niveau[0] = 2;
    }
    else if (pire_fuite <= 16 && niveau[0] != 3) {
        SendMessage("AL3");
        niveau[0] = 3;
    }
}

void reaction_alerte_haute(int pire_fuite, int* niveau) {
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
    int niveau_alerte;
    int niveau[2] = { 0 };
    while (1) {
        sleep(1);
        //Mise à jour des actions toutes les secondes ou autres
        switch (alerte_max((struct Gaz**) args, NUM_GAZ))
        {
        case 0:
            reaction_sans_alerte(aug_max((struct Gaz**) args, NUM_GAZ), niveau);
            break;
        case 1:
            reaction_alerte_basse(aug_max((struct Gaz**) args, NUM_GAZ), niveau);
            break;
        case 2:
            reaction_alerte_moy(aug_max((struct Gaz**) args, NUM_GAZ), niveau);
            break;
        case 3 :
            reaction_alerte_haute(aug_max((struct Gaz**) args, NUM_GAZ), niveau);
        }
    }
}

void * injection(void* args) {
    struct Gaz** gaz = (struct Gaz**) args;
    int bool[NUM_GAZ] = { 0 };
    char str[5];
    while(1) {
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

}