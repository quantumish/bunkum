#ifndef UTIL_SYNC_H
#define UTIL_SYNC_H

#include <pthread.h>
#include "vec.h"

typedef struct channel {
    pthread_mutex_t mutex;
    vec_t queue;
    size_t msg_sz;
    size_t sz;
} channel_t;

channel_t channel_new(size_t msg_sz);
void channel_push(channel_t* chan, void* msg);
void* channel_pop(channel_t* chan);
void* channel_recv(channel_t* chan);
void* channel_try_recv(channel_t* chan);

#endif
