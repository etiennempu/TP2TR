#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


#include "gaz.h"
#include "serveur.h"
#include "client.h"

#include "Led.h"

#include "utils.h"
#include "tempsreponsetempsreel.h"


#define NUM_GAZ 3
#define NUM_THREADS 3+NUM_GAZ

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

// Définition des coûts des actions
#define A1 1
#define A2 2
#define A3 3
#define V1 6
#define V2 9
#define INJ 20





int run = 1; // Indiquer l'arrêt des tâches (mot clé extern : pour que les fonctions de mesure présente dans d'autres fichiers y ai accès)
sem_t verrou_controle[NUM_GAZ]; // Synchroniser les tâches "contrôle" avec l'écoute
sem_t verrou_action; // Pour indiquer au thread action qu'un le statut d'un gaz a été modifié
pthread_mutex_t mutex_alerte[NUM_GAZ] = { PTHREAD_MUTEX_INITIALIZER };
pthread_mutex_t mutex_valeur[NUM_GAZ] = { PTHREAD_MUTEX_INITIALIZER };

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
            
            pthread_mutex_lock(&mutex_valeur[i-1]);
            majgaz(gaz[i-1], fuite);
            pthread_mutex_unlock(&mutex_valeur[i-1]);

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
    int injection = 0;

    char message[5+NUM_GAZ/10];
    const char* c;
    while(run) {
        sem_wait(&verrou_controle[gaz.indice]);
        int taux = *(gaz.value);
        if (taux != tmp) {
            pthread_mutex_lock(&mutex_alerte[gaz.indice]);
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
            pthread_mutex_unlock(&mutex_alerte[gaz.indice]);


            if (*(gaz.alerte) != old_alerte) {

                // Si il faut lancer l'injection on le fait ici car c'est une action qui n'est pas commune à tous les gaz.
                if (*gaz.alerte == 4 && !injection) {
                    injection = 1;
                    sprintf(message, "IG%d", gaz.indice+1);
                    SendMessage(message);
                    /* COUT
                    cout_total += INJ;
                    */
                }
                /* COUT
                else if (*gaz.alerte == 4 && injection) {
                    cout_total += INJ;
                }
                */
                else if (*gaz.alerte != 4 && injection) {
                    injection = 0;
                    sprintf(message, "AIG%d", gaz.indice+1);
                    SendMessage(message);
                }

                sprintf(message, "AG%d", gaz.indice+1);
                strcat(message, c);
                old_alerte = *(gaz.alerte);
                SendMessage(message);
            }
            tmp = taux;
        }
        else {
            pthread_mutex_lock(&mutex_valeur[gaz.indice]);
            // Si pas de changement alors peut-être l'action ne fait que compenser donc augmentaion artificielle de la fuite pour pousser à faire une action plus forte
            if (tmp != 0) gaz.aug++;
            // Ou alors la fuite a déjà baissé et on est encore en train d'utiliser une action trop importante
            else gaz.aug = (gaz.aug > 0) ? gaz.aug-- : 0;
            pthread_mutex_unlock(&mutex_valeur[gaz.indice]);
        }
        sem_post(&verrou_action);
    }
    sem_post(&verrou_action);
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
        for (int i=0; i<NUM_GAZ; i++) sem_wait(&verrou_action);
        //Mise à jour des actions toutes les secondes ou autres

        for (int i=0; i<NUM_GAZ; i++) pthread_mutex_lock(&mutex_alerte[i]);
        int a_max = alerte_max((struct Gaz**) args, NUM_GAZ);
        for (int i=0; i<NUM_GAZ; i++) pthread_mutex_unlock(&mutex_alerte[i]);

        if (a_max < 3) {

            for (int i=0; i<NUM_GAZ; i++) pthread_mutex_lock(&mutex_valeur[i]);
            reaction(aug_max((struct Gaz**) args, NUM_GAZ), niveau, a_max);
            for (int i=0; i<NUM_GAZ; i++) pthread_mutex_unlock(&mutex_valeur[i]);

        }
        else {
            reaction_max(niveau);
        }
		//ici
    }
    pthread_exit(0);
}

int main(int argc, char** argv) {

    /* Initialisation des sémaphores */
    for (int i=0;i++;i<3) {
        sem_init(&verrou_controle[i], 0, 0);
    }
    sem_init(&verrou_action, 0, 0);

    /* Préparation des paramètres pour la création des tâches */
    pthread_t* thread = calloc(NUM_THREADS, sizeof(pthread_t));

    void * functions[NUM_THREADS] = {ecoute, [1 ... NUM_GAZ] = controle, air, leds};
    struct sched_param sched[NUM_THREADS] = {[0 ... NUM_GAZ+1] = SCHED_HAUTE, SCHED_BASSE};
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
	
	printf("arrival %d\n", arrivalIndex);
	printf("reac %d\n", reactionIndex);
	if (arrivalIndex==reactionIndex)
	{
		for(int i=0;i<arrivalIndex;i++)
		{
			calculateResponseTime(arrival_time[arrivalIndex],reaction_time[reactionIndex]);
		}
	}

    exit(0);
}