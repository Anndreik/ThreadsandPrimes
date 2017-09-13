#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define BUF_SIZE 10
#define MAX_VALUE 1000
#define MIN_VALUE 100

// the buffer works like a stack for
// the sake of simplicity, if needed
// we may implement a queue
typedef struct {
    int buf[BUF_SIZE]; // the buffer
    size_t len; // number of items in the buffer
    pthread_mutex_t mutex; // needed to add/remove data from the buffer
    pthread_cond_t can_produce; // signaled when items are removed
    pthread_cond_t can_consume; // signaled when items are added
} buffer_t;

// produce random numbers

int isPrime(int n){
    int i;
    if(n % 2 == 0){
        return 0;
    }
    for(i = 3; i <= sqrt(n); i = i + 2){
        if(n % i == 0){
            return 0;
        }
    }
    return 1;
}

void* producer(void *arg) {
    buffer_t *buffer = (buffer_t*)arg;

    while(1) {
#ifdef UNDERFLOW
        // used to show that if the producer is somewhat "slow"
        // the consumer will not fail (i.e. it'll just wait
        // for new items to consume)
        usleep(100000);
#endif

        pthread_mutex_lock(&buffer->mutex);

        if(buffer->len == BUF_SIZE) { // full
            // wait until some elements are consumed
            pthread_cond_wait(&buffer->can_produce, &buffer->mutex);
        }

        // in real life it may be some data fetched from
        // sensors, the web, or just some I/O
        int t = (rand() % (MAX_VALUE+1-MIN_VALUE)) + MIN_VALUE;
        printf("Produced: %d\n", t);

        // append data to the buffer
        buffer->buf[buffer->len] = t;
        ++buffer->len;

        // signal the fact that new items may be consumed
        pthread_cond_signal(&buffer->can_consume);
        pthread_mutex_unlock(&buffer->mutex);
    }

    // never reached
    return NULL;
}

// consume random numbers
void* consumer(void *arg) {
    buffer_t *buffer = (buffer_t*)arg;

    while(1) {
#ifdef OVERFLOW
        // show that the buffer won't overflow if the consumer
        // is slow (i.e. the producer will wait)
        //sleep(rand() % 3);
#endif
        pthread_mutex_lock(&buffer->mutex);

        while(buffer->len == 0) { // empty
            // wait for new items to be appended to the buffer
            pthread_cond_wait(&buffer->can_consume, &buffer->mutex);
        }

        // grab data
        --buffer->len;
        if(isPrime(buffer->buf[buffer->len])){
            printf("Thread %ld: number <%d>, prime: YES\n", (long)pthread_self(), buffer->buf[buffer->len]);
        }
        else{
            printf("Thread %ld: number <%d>, prime: NO\n", (long)pthread_self(), buffer->buf[buffer->len]);
        }
        

        // signal the fact that new items may be produced
        pthread_cond_signal(&buffer->can_produce);
        pthread_mutex_unlock(&buffer->mutex);
    }

    // never reached
    return NULL;
}

int main(int argc, char *argv[]){
    buffer_t buffer = {
        .len = 0,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .can_produce = PTHREAD_COND_INITIALIZER,
        .can_consume = PTHREAD_COND_INITIALIZER
    };

    pthread_t prod, cons[3];
    pthread_create(&prod, NULL, producer, (void*)&buffer);
    pthread_create(&cons[0], NULL, consumer, (void*)&buffer);
    pthread_create(&cons[1], NULL, consumer, (void*)&buffer);
    pthread_create(&cons[2], NULL, consumer, (void*)&buffer);

    pthread_join(prod, NULL); // will wait forever
    pthread_join(cons[0], NULL);
    pthread_join(cons[1], NULL);
    pthread_join(cons[2], NULL);

    return 0;
}