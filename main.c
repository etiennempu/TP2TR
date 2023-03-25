#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "serveur.h"
#include "client.h"
#include "Led.h"

#define NUM_THREADS 6

#define ALERTE_B 5
#define ALERTE_M 15
#define ALERTE_H 30
#define INJECTION 60

sem_t verrou_gaz[3]; // Pour synchroniser la réception d'une valeur pour un gaz avec son contrôle
pthread_mutex_t mutex_alerte[3] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

typedef struct gaz {
    int indice;
    int* value;
    int alerte;
    int injection;
    int period;
} gaz;

void* ecoute(void* arg) {
    while(1) {
        char* message = ReceiveMessage();
        printf("%s", message);
		int length = strlen(message);
		char tmp[3]; 
        strncpy(tmp, message, 2);
        tmp[2] = '\0';
		printf("length:%d tmp:%s \n",length,tmp);
    // Vérifiez si la chaîne a au moins 3 caractères
		if ((length == 4 || length == 5)&& tmp=="LG") {
        char Cvaleur[length - 2]; 
        strncpy(Cvaleur, message + 3, length - 3);
        Cvaleur[length - 3] = '\0';
		int valeur = atoi(Cvaleur);
		printf("valeur:%d\n",valeur);
		if(valeur < 30){
			
			LedUpdate(atoi(&message[2]));
		
		}
		else if( 30 <= valeur < 60){
			
			LedUpdate(atoi(&message[2])+1);
			
		}
		else{
			
			LedUpdate(atoi(&message[2])+2);
			
		}
		}
			
		/*switch (atoi(&message[2]))
        {
        case 1:
            // On met dans le gaz 1 
            break;
        case 2:
            // On met dans le gaz 2 
        case 3:
            // On met dans le gaz 3 
            break;
        }*/
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

struct gaz* newGaz(int index) {
    struct gaz* gaz = malloc(sizeof(struct gaz));
    gaz->indice = index;
    gaz->value = malloc(sizeof(double));
    gaz->alerte = 0;
    gaz->injection = 0;
    return gaz;
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
	
	
	LedUpdate(0);
	/*sleep(2);
	LedUpdate(1);
	sleep(2);
	LedUpdate(2);
	sleep(2);
	LedUpdate(3);
	sleep(2);
	LedUpdate(4);
	sleep(2);
	LedUpdate(5);
	sleep(2);
	LedUpdate(6);
	sleep(2);
	LedUpdate(7);
	sleep(2);
	LedUpdate(8);
	sleep(2);
	LedUpdate(9);
	sleep(2);
	LedUpdate(10);*/
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