/*
 * A looper is a thread that runs on its own, simply repeating a function over and over.
 * Its behavior might be altered by that looping function processing data shared with other threads, but
 * otherwise it can only be started, stopped, paused, and resumed, and slowed or sped up by changing the
 * usleep time in between the times that the looping function is run.
 */

#ifndef PTHREAD_WORKER_H
#define PTHREAD_WORKER_H

#include <pthread.h>
#include <unistd.h>

#define LOOPER_SLEEP_TIME_DEFAULT 500

enum looper_state_t { not_running, running, paused, stopped};

struct looper_t {
    void (*function_to_repeat)(void * d);
    void (*function_to_run_first)(void * d);
    void (*function_to_run_last)(void * d);
    useconds_t usleep_time;
    pthread_mutex_t lock;
    pthread_t thread;
    enum looper_state_t desired_state;
    enum looper_state_t actual_state;
    int nowait;
    void * data;
};

void looper_init(struct looper_t * l);
void looper_nowait(struct looper_t * l);
void looper_start(struct looper_t * l);
void looper_stop(struct looper_t * l);
void looper_pause(struct looper_t * l);
void looper_resume(struct looper_t * l);
void looper_change_usleep_time(struct looper_t * l, useconds_t ms);
void looper_update_data(struct looper_t * l, void * data);

#endif // PTHREAD_WORKER_H

