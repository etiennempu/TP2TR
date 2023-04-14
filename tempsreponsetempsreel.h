#ifndef tempsreponsetempsreel
#define tempsreponsetempsreel

float calculateResponseTime(time_t arrival_time, time_t reaction_time);
float calculateAverageResponseTime(float* response_times, int num_events);
float calculateFailureRate(float* response_times, int num_events);

#endif