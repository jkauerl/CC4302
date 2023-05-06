#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>
  
#include "pss.h"
#include "batch.h"

struct job {
  int ready;
  JobFun fun;
  void *input;
  void *result;
  pthread_cond_t w;
};

// ... defina aca sus variables globales ...

pthread_t *ppid=NULL;
int t=0; //threads
Queue *q;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c1 = PTHREAD_COND_INITIALIZER; // condicion de la cola, existen trabajos


// ... defina las funciones auxiliares que necesite ...

void *excuteFun(void *ptr) {
  while(1) {
    Queue *q = (Queue *) ptr;
    pthread_mutex_lock(&m);
    while(emptyQueue(q)) {
      pthread_cond_wait(&c1, &m);
    }

    Job *j = get(q);
    pthread_mutex_unlock(&m);
    if(j==NULL) {
      return NULL;
    }
    j->result = (*j->fun)(j->input);
    pthread_mutex_lock(&m);
    j->ready = 1;
    pthread_cond_signal(&j->w);
    pthread_mutex_unlock(&m);
  }
  return NULL;
}

void startBatch(int n) {
  pthread_mutex_init(&m, NULL);
  pthread_cond_init(&c1, NULL);
  t=n;
  pthread_t *pid = malloc(sizeof(pthread_t)*n);
  ppid=pid;
  q = makeQueue();
  for(int i=0; i<n; i++) {
    pthread_create(&pid[i], NULL, excuteFun, q);
  }
}

void stopBatch() {
  pthread_mutex_lock(&m);
  for(int i=0; i<t; i++) {
    put(q, NULL);
  }
  pthread_mutex_unlock(&m);
  pthread_cond_broadcast(&c1);
  for(int i=0; i<t; i++) {
    pthread_join(ppid[i], NULL);
  }
  destroyQueue(q);
  free(ppid);
}

Job *submitJob(JobFun fun, void *input) {
  Job job = {0, fun, input, NULL, PTHREAD_COND_INITIALIZER};
  Job *pjob = malloc(sizeof(Job));
  *pjob = job;
  pthread_mutex_lock(&m);
  put(q, pjob);
  pthread_cond_signal(&c1);
  pthread_mutex_unlock(&m);
  return pjob;
}

void *waitJob(Job *job) {
  pthread_mutex_lock(&m);
  while(job->ready == 0) {
    pthread_cond_wait(&job->w, &m); 
  }
  pthread_mutex_unlock(&m);
  void *result = job->result;
  free(job);
  return result;
} 

