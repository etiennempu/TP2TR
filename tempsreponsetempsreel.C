#include <stdio.h>
#include <time.h>

#define D 1 // échéance de l'action de calcul de la réaction (en secondes)

/*typedef struct {
  float gas1;
  float gas2;
  float gas3;
} SensorData;

typedef struct {
  int alarm_level;
  char alarm_color;
} Alarm;*/

// fonction pour calculer le temps de réponse
float calculateResponseTime(time_t arrival_time, time_t reaction_time) {
  return difftime(reaction_time, arrival_time);
}

// fonction pour calculer le temps de réponse moyen
float calculateAverageResponseTime(float* response_times, int num_events) {
  float total_response_time = 0;
  for (int i = 0; i < num_events; i++) {
    total_response_time += response_times[i];
  }
  return total_response_time / num_events;
}

// fonction pour calculer le taux d'échec
float calculateFailureRate(float* response_times, int num_events) {
  int num_failures = 0;
  for (int i = 0; i < num_events; i++) {
    if (response_times[i] >= D) {
      num_failures++;
    }
  }
  return (float)num_failures / num_events;
}

// exemple d'utilisation
/*int main() {
  // initialiser les données de capteur et l'alarme
  SensorData sensor_data = {0.05, 0.1, 0.02};
  Alarm alarm = {0, '\0'};

  // supposons que l'arrivée des valeurs des capteurs est maintenant
  time_t arrival_time = time(NULL);

  // effectuer le calcul pour déterminer l'alarme
  // ... code pour déterminer l'alarme en fonction des valeurs de capteur

  // supposons que l'envoi de l'action est maintenant
  time_t reaction_time = time(NULL);

  // calculer le temps de réponse
  float response_time = calculateResponseTime(arrival_time, reaction_time);
  printf("Temps de réponse: %.2f secondes\n", response_time);

  // calculer le temps de réponse moyen
  int num_events = 10; // supposons que nous avons 10 événements
  float response_times[num_events]; // tableau pour stocker les temps de réponse
  for (int i = 0; i < num_events; i++) {
    // ... code pour simuler l'arrivée d'un événement et le calcul de la réaction
    response_times[i] = calculateResponseTime(arrival_time, reaction_time);
  }
  float average_response_time = calculateAverageResponseTime(response_times, num_events);
  printf("Temps de réponse moyen: %.2f secondes\n", average_response_time);

  // calculer le taux d'échec
  float failure_rate = calculateFailureRate(response_times, num_events);
  printf("Taux d'échec: %.2f %%\n", failure_rate * 100);

  return 0;
}*/

//Temps de réponse: 0.00 secondes
//Temps de réponse moyen: 0.00 secondes
//Taux d'échec: 0.00 %