#include "timer.h"

void start_timer(GameTimer *t, int seconds) {
    t->start_time = time(NULL);
    t->duration = seconds;
}

int get_remaining_time(GameTimer *t) {
    int elapsed = (int)(time(NULL) - t->start_time);
    int remaining = t->duration - elapsed;
    return (remaining > 0) ? remaining : 0;
}

bool is_time_up(GameTimer *t) {
    return get_remaining_time(t) <= 0;
}