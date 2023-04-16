
#include "log.h"
#include "sync.h"

channel_t channel_new(size_t msg_sz) {
    channel_t out;
    out.msg_sz = msg_sz;
    pthread_mutex_init(&out.mutex, NULL);
    out.queue = shitvec_new(msg_sz);
    out.sz = 0;
	return out;
}

void channel_push(channel_t* chan, void* msg) {
    pthread_mutex_lock(&chan->mutex);
	shitvec_push(&chan->queue, msg);
	pthread_mutex_unlock(&chan->mutex);
	__atomic_fetch_add(&chan->sz, 1, __ATOMIC_RELAXED);
}

void* channel_pop(channel_t* chan) {
	void* out;
	pthread_mutex_lock(&chan->mutex);
	out = shitvec_get(&chan->queue, chan->queue.vec_sz-1);
	chan->queue.vec_sz -= 1;
	pthread_mutex_unlock(&chan->mutex);
	__atomic_fetch_sub(&chan->sz, 1, __ATOMIC_RELAXED);
	return out;
}

void* channel_recv(channel_t * chan) {
	while(__atomic_load_n(&chan->sz, __ATOMIC_RELAXED) == 0);	
	return channel_pop(chan);
}
