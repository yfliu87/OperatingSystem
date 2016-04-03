#include "priority-readers-writers-practice.h"
#include <pthread.h>
#include <stdio.h>

#define NUM_READERS  4
#define NUM_WRITERS  4
#define NUM_READ 4
#define NUM_WRITE 4

unsigned int gSharedValue = 100;
pthread_mutex_t gSharedMemoryLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readPhase = PTHREAD_COND_INITIALIZER;
pthread_cond_t writePhase = PTHREAD_COND_INITIALIZER;
int waitingReaders = 0;
int activeReaders = 0;

int main(int argc, char** argv){
  int readerNum[NUM_READ];
  int writerNum[NUM_WRITE];
  pthread_t readerThreadIDs[NUM_READERS];
  pthread_t writerThreadIDs[NUM_WRITERS];
  int i = 0;

  srandom((unsigned int)time(NULL));

  for (i = 0; i < NUM_READERS; i++){
    readerNum[i] = i;
    pthread_create(&readerThreadIDs[i], NULL, readerMain, &readerNum[i]);
  }

  for (i = 0; i < NUM_WRITERS; i++){
    writerNum[i] = i;
    pthread_create(&writerThreadIDs[i], NULL, writerMain, &writerNum[i]);
  }

  for (i = 0; i < NUM_READERS; i++){
    pthread_join(readerThreadIDs[i], NULL);
  }

  for (i = 0; i < NUM_WRITERS; i++){
    pthread_join(writerThreadIDs[i], NULL);
  }

  return 0;
}

void* readerMain(void* threadArgument){
  int id = *((int*)threadArgument);
  int numReaders = 0;
  int i = 0;

  for (i = 0;i < NUM_READ; i++){
    usleep(1000 * (random()%NUM_READERS + NUM_WRITERS));

    //enter critical section
    pthread_mutex_lock(&gSharedMemoryLock);
    waitingReaders++;

    while (activeReaders == -1)
      pthread_cond_wait(&readPhase, &gSharedMemoryLock);

    waitingReaders--;
    numReaders = ++activeReaders;
    pthread_mutex_unlock(&gSharedMemoryLock);

    fprintf(stdout, "[r%d] reading %u [active readers:%2d][waiting readers:%2d]\n", id, gSharedValue, numReaders, waitingReaders);

    //exit critical section
    pthread_mutex_lock(&gSharedMemoryLock);
    activeReaders--;
    if (activeReaders == 0)
      pthread_cond_signal(&writePhase);

    pthread_mutex_unlock(&gSharedMemoryLock);
  }
  pthread_exit(0);
}

void* writerMain(void* threadArgument){
  int id = *((int*)threadArgument);
  int numReaders = 0;
  int i = 0;

  for (i = 0; i < NUM_WRITE; i++){
    usleep(1000 * (random()%NUM_READERS + NUM_WRITERS));

    //enter critical section
    pthread_mutex_lock(&gSharedMemoryLock);

    while (activeReaders != 0)
      pthread_cond_wait(&writePhase, &gSharedMemoryLock);

    activeReaders = -1;
    numReaders = activeReaders;
    pthread_mutex_unlock(&gSharedMemoryLock);

    fprintf(stdout, "[w%d] writing %u* [active readers:%2d][waiting readers:%2d]\n", id, ++gSharedValue, numReaders, waitingReaders);

    //exit critical section
    pthread_mutex_lock(&gSharedMemoryLock);
    activeReaders = 0;
    if (waitingReaders > 0)
      pthread_cond_broadcast(&readPhase);
    else
      pthread_cond_signal(&writePhase);

    pthread_mutex_unlock(&gSharedMemoryLock);
  }
  pthread_exit(0);
}
