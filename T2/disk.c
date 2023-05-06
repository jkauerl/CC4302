#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "disk.h"

// ... defina aca sus variables globales ...
int thread_amount[MAXTRACK];
int last_track=-1;
int busy=0;

pthread_mutex_t m=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c=PTHREAD_COND_INITIALIZER;

// ... defina aca otras funciones si las necesita ...

int searchThread(int track) {
  int i=track;
  while(thread_amount[i] != 1) {
    i = (i + 1) % MAXTRACK;
  }
  return i;
}

void requestDisk(int track) {
  pthread_mutex_lock(&m);
  thread_amount[track]++;

  int next = searchThread(track);
  while((next != track || busy == 1) && last_track!=-1) {
    pthread_cond_wait(&c, &m);
    next = searchThread(last_track);
  }
  busy=1;
  last_track = track;
  pthread_mutex_unlock(&m);
}

void releaseDisk(void) {
  pthread_mutex_lock(&m);
  busy=0;
  thread_amount[last_track]--;

  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&m);
}

void diskInit(void) {
  pthread_mutex_init(&m, NULL);
  pthread_cond_init(&c, NULL);
  for(int i=0; i<MAXTRACK; i++) {
    thread_amount[i] = 0;
  }
}

void diskClean(void) {
  pthread_mutex_destroy(&m);
  pthread_cond_destroy(&c);
}