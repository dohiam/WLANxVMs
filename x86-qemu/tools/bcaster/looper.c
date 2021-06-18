#include "looper.h"
#include <unistd.h>
#include <stdio.h>

void looper_update_state(struct looper_t * l, enum looper_state_t *desired_state, enum looper_state_t actual_state);

void * looper_function(void * looper) {
    struct looper_t * l;
    enum looper_state_t desired_state, actual_state;
    l = (struct looper_t *) looper;
    desired_state = running;
    actual_state = not_running;
    while (desired_state != stopped) {
        if (desired_state != paused) {
            actual_state = running;
            pthread_mutex_lock(&(l->lock));
            l->function_to_repeat(l->data);
            pthread_mutex_unlock(&(l->lock));
        }
        else actual_state = paused;
        looper_update_state(l, &desired_state, actual_state);
        usleep(l->usleep_time);
    }
    actual_state = stopped;
    looper_update_state(l, &desired_state, actual_state);
    return looper;
}

void looper_init(struct looper_t * l) {
    l->function_to_repeat = NULL;
    l->function_to_run_first = NULL;
    l->data = NULL;
    l->usleep_time = LOOPER_SLEEP_TIME_DEFAULT;
    pthread_mutex_init(&(l->lock), NULL);
    l->desired_state = not_running;
    l->actual_state = not_running;
    l->nowait = 0;
}

void looper_nowait(struct looper_t * l) {
    l->nowait = 1;
}


void looper_set_desired_state(struct looper_t * l, enum looper_state_t state) {
     pthread_mutex_lock(&(l->lock));
     l->desired_state = state;
     pthread_mutex_unlock(&(l->lock));
}

enum looper_state_t looper_get_actual_state(struct looper_t * l) {
     enum looper_state_t actual;
     pthread_mutex_lock(&(l->lock));
     actual = l->actual_state;
     pthread_mutex_unlock(&(l->lock));
     return actual;
}

void looper_wait_for_actual_state(struct looper_t * l, enum looper_state_t state) {
    enum looper_state_t actual;
    actual = looper_get_actual_state(l);
    while (actual != state) {
        usleep(l->usleep_time);
        actual = looper_get_actual_state(l);
    }
}

void looper_update_state(struct looper_t * l, enum looper_state_t *desired_state, enum looper_state_t actual_state) {
    pthread_mutex_lock(&(l->lock));
    *desired_state = l->desired_state;
    l->actual_state = actual_state;
    pthread_mutex_unlock(&(l->lock));
}

void looper_change_usleep_time(struct looper_t * l, useconds_t ms) {
     pthread_mutex_lock(&(l->lock));
    l->usleep_time = ms;
     pthread_mutex_unlock(&(l->lock));
}

void looper_start(struct looper_t * l) {
    l->desired_state = running;
    if (l->function_to_run_first) l->function_to_run_first(l->data);
    if (l->function_to_repeat) pthread_create(&(l->thread), NULL, looper_function, (void *) l);
    if (!(l->nowait)) looper_wait_for_actual_state(l, running);
}

void looper_stop(struct looper_t * l) {
     looper_set_desired_state(l, stopped);
     pthread_join(l->thread, NULL);
     if (l->function_to_run_last) l->function_to_run_last(l->data);
}

void looper_pause(struct looper_t * l) {
     looper_set_desired_state(l, paused);
     if (!(l->nowait)) looper_wait_for_actual_state(l, paused);
}

void looper_resume(struct looper_t * l) {
     looper_set_desired_state(l, running);
     if (!(l->nowait)) looper_wait_for_actual_state(l, running);
}

void looper_update_data(struct looper_t * l, void * data) {
     pthread_mutex_lock(&(l->lock));
    l->data = data;
     pthread_mutex_unlock(&(l->lock));
}




