#ifndef tempsreponsetempsreel
#define tempsreponsetempsreel

time_t arrival_time[];
int arrivalIndex=0;
time_t reaction_time[];
int reactionIndex=0;

float calculateResponseTime(time_t arrival_time, time_t reaction_time);
float calculateAverageResponseTime(float* response_times, int num_events);
float calculateFailureRate(float* response_times, int num_events);

#endif