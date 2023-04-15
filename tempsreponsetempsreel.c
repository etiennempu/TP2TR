#include <stdio.h>
#include <time.h>


// fonction pour calculer le temps de réponse
float calculateResponseTime(time_t arrival_time, time_t reaction_time) {
  return difftime(reaction_time, arrival_time);
}

