#define _DEFAULT_SOURCE 1
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>

#include "nthread.h"

#include "batch.h"

#define FALSE 0
#define TRUE 1

#pragma GCC diagnostic ignored "-Wunused-function"

// ==================================================
// Funcion que entrega los milisegundos transcurridos desde
// que se invoco resetTime()

static int time0= 0;

static int getTime0() {
    struct timeval Timeval;
    gettimeofday(&Timeval, NULL);
    return Timeval.tv_sec*1000+Timeval.tv_usec/1000;
}

static void resetTime() {
  time0= getTime0();
}

static int getTime() {
  return getTime0()-time0;
}

// ==================================================
// Funcion que espera que transcurran milis milisegundos

static void esperar(useconds_t milis) {
  usleep(milis*1000);
}

// ==================================================
// Funcion para reportar un error y detener la ejecucion

void fatalError( char *procname, char *format, ... ) {
  va_list ap;

  fprintf(stderr,"Error Fatal en la rutina %s y la tarea \n", procname);
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);

  exit(1); /* shutdown */
}

// ==================================================

typedef struct {
  int delay;
  void *tag;
  int startTime;
} Test;

static pthread_mutex_t m= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond= PTHREAD_COND_INITIALIZER;
static int terminados;
static int lanzados;
static int chequeados;
static int waitTerminados;

static void *testFun(void *ptr) {
  Test *test= ptr;
  test->startTime= getTime();
  if (test->delay!=0)
    esperar(test->delay);
  else {
    pthread_mutex_lock(&m);
    terminados++;
    int punto= terminados%1000==0;
    pthread_mutex_unlock(&m);
    if (punto) printf(".");
  }
  return test;
}

static Job *submitTest(int delay) {
  Test *test= malloc(sizeof(Test));
  test->delay= delay;
  test->tag= test;
  return submitJob(testFun, test);
}

static void checkTest(Job *job) {
  Test *res= waitJob(job);
  if (res->tag!=res)
    fatalError("checkTest", "El resultado %p es incorrecto\n", res);
  free(res);
}

typedef struct {
  int first, step, nr;
  Job **jobs;
} SubmitParam;
  
void createSubmitThread(pthread_t *pth, void *(*fun)(void *),
         int first, int step, int nr, Job **jobs) {
  SubmitParam parm= { first, step, nr, jobs };
  SubmitParam *pparm= malloc(sizeof(SubmitParam));
  *pparm= parm;
  if (pthread_create(pth, NULL, fun, pparm)!=0) {
    perror("pthread_create");
    exit(1);
  }
}

void *submitFun(void *ptr) {
  SubmitParam *pparm= ptr;
  int first= pparm->first;
  int step= pparm->step;
  int nr= pparm->nr;
  Job **jobs= pparm->jobs;
  free(pparm);

  for (int i=first; i<nr; i+=step) {
    pthread_mutex_lock(&m);
    jobs[i]= submitTest(0);
    lanzados++;
    int dospuntos= lanzados%1000==0;
    pthread_mutex_unlock(&m);
    if (dospuntos) printf(":");
  }

  return NULL;
}

void *waitFun(void *ptr) {
  SubmitParam *pparm= ptr;
  int first= pparm->first;
  int step= pparm->step;
  int nr= pparm->nr;
  Job **jobs= pparm->jobs;
  free(pparm);

  for (int i= first; i<nr; i+=step) {
    pthread_mutex_lock(&m);
    while (jobs[i]==NULL)
      pthread_cond_wait(&cond, &m);// todavia no se crea el job, hay que esperar
    chequeados++;
    int guion= chequeados%1000==0;
    pthread_mutex_unlock(&m);
    checkTest(jobs[i]);
    if (guion) printf("-");
  }
  pthread_mutex_lock(&m);
  waitTerminados++;
  pthread_mutex_unlock(&m);
  return 0;
}

typedef struct {
  pthread_t *submitters;
  int nsubmitters;
} WaitParam;

void *waiterFun(void *ptr) {
  WaitParam *pparm= ptr;
  pthread_t *submitters= pparm->submitters;
  int nsubmitters= pparm->nsubmitters;
  free(pparm);

  for (int j= 0; j<nsubmitters; j++) {
    pthread_join(submitters[j], NULL);
  }
  printf("$");
  stopBatch();
  return 0;
}

void createWaiterThread(pthread_t *pth, pthread_t *submitters, int nsubmitters)
{
  WaitParam parm= { submitters, nsubmitters };
  WaitParam *pparm= malloc(sizeof(WaitParam));
  *pparm= parm;
  if (pthread_create(pth, NULL, waiterFun, pparm)!=0) {
    perror("pthread_create");
    exit(1);
  }
}

int main() {
  resetTime();
  printf("Test secuencial: 1 solo job a la vez\n");
  {
    startBatch(1);

    int ini= getTime();
    Job *r1= submitTest(100);
    Job *r2= submitTest(100);
    Job *r3= submitTest(100);

    checkTest(r1);
    checkTest(r2);
    checkTest(r3);
    int elapsed= getTime()-ini;
    printf("Tiempo transcurrido: %d milisegundos\n", elapsed);
    if (elapsed<300 || elapsed>350)
      fatalError("nMain",
          "Error: tiempo no esta entre [300, 350]\n",
          elapsed);

    printf("\nDeteniendo el sistema batch\n");
    stopBatch();
  }
  printf("----------------------------------------\n\n");

  printf("Test paralelo: 2 jobs simultaneos\n");
  {
    startBatch(2);

    int ini= getTime();
    Job *r1= submitTest(100);
    Job *r2= submitTest(100);
    Job *r3= submitTest(100);

    checkTest(r1);
    checkTest(r2);
    checkTest(r3);
    int elapsed= getTime()-ini;
    printf("Tiempo transcurrido: %d milisegundos\n", elapsed);
    if (elapsed<200 || elapsed>250)
      fatalError("nMain",
          "Error: tiempo no esta entre [200, 250]\n",
          elapsed);

    stopBatch();
  }
  printf("----------------------------------------\n\n");

  printf("Test paralelo: 100 jobs y 10 threads\n");
  {
    int nr= 100;
    int ncor= 10;
    startBatch(ncor);

    Job *jobs[nr];
    int ini= getTime();
    for (int i= 0; i<nr; i++) {
      jobs[i]= submitTest(100);
    }

    for (int i= 0; i<nr; i++) {
      checkTest(jobs[i]);
    }
    int elapsed= getTime()-ini;
    printf("Tiempo transcurrido: %d milisegundos\n", elapsed);
    if (elapsed<1000 || elapsed>1200)
      fatalError("nMain",
          "Error: tiempo no esta entre [1000, 1200]\n",
          elapsed);

    stopBatch();
  }
  printf("----------------------------------------\n\n");

  printf("Test del orden de llegada: 10 jobs separados por 100 milisegundos\n");
  {
    int nr= 10;
    startBatch(1);
 
    int ini= getTime();
    Job *jobs[nr];
    jobs[0]= submitTest(1100);
    for (int i= 1; i<nr; i++) {
      esperar(100);
      jobs[i]= submitTest(100);
    }

    int lastStartTime= 0;
    for (int i= 0; i<nr; i++) {
      Test *test= waitJob(jobs[i]);
      if (lastStartTime>test->startTime)
        fatalError("nMain", "no se respeta orden de llegada\n");
      lastStartTime= test->startTime;
      free(test);
    }

    int elapsed= getTime()-ini;
    printf("Tiempo transcurrido: %d milisegundos\n", elapsed);
    if (elapsed<2000 || elapsed>2200)
      fatalError("nMain",
          "Error: tiempo no esta entre [2000, 2200]\n",
          elapsed);

    stopBatch();
  }
  printf("----------------------------------------\n\n");

  printf("Cada '.' corresponde a 1000 jobs lanzados\n");
  printf("Cada ':' corresponde a 1000 jobs terminados\n");
  printf("Cada '-' corresponde a 1000 waitJob completados\n");
  printf("El '$' corresponde a la llamada de stopBatch\n\n");
  int njobs[]= {1000, 10000, 200000 };
  int ncores[]= {10, 100, 400 };

  for (int k= 0; k<3; k++) {
    int ncor= ncores[k]; // El numero de threads
    int nr= njobs[k];    // Numero de jobs
    printf("test de robustez con %d threads y %d jobs\n", ncor, nr);

    startBatch(ncor);    // Se lanza el sistema batch con ncor threads
    lanzados= 0;         // jobs creados
    terminados= 0;       // jobs terminados
    chequeados= 0;       // jobs completados
    int nsubmitters= 2*ncor; // 2*ncor tareas crearan jobs
                         // ncor tareas se encargaran de completarlos
    waitTerminados= 0;   // tareas que invocan a waitTest terminadas

    int ini= getTime(); // hora de inicio
    Job *jobs[nr];       // los nr jobs que se crearan
    for (int j= 0; j<nr; j++)
      jobs[j]= NULL;     // se necesita que partan en NULL
    printf("creando jobs\n");
    pthread_t submitters[nsubmitters], waitters[ncor];
    for (int j=0; j<nsubmitters; j++) {
      createSubmitThread(&submitters[j], submitFun, j, nsubmitters, nr, jobs);
    }
    for (int j=0; j<ncor; j++) {
      createSubmitThread(&waitters[j], waitFun, j, ncor, nr, jobs);
    }
    // Lanzamos una tarea encargada de enterrar los submitters y llamar
    // a stopBatch
    pthread_t waitsubtask;
    createWaiterThread(&waitsubtask, submitters, nsubmitters);
    // Parte truculenta: los waitters llaman a wait cuando encuentran
    // un job que aun no ha sido creado.  Cada 100 milisegundos llamamos
    // a broadcast para que vuelvan a intentar completar el job
    pthread_mutex_lock(&m);
    while (waitTerminados!=ncor) {
      pthread_cond_broadcast(&cond);
      pthread_mutex_unlock(&m);
      esperar(100);
      pthread_mutex_lock(&m);
    }
    pthread_mutex_unlock(&m);
    // Enterramos los waitters aqui mismo
    printf("\nesperando jobs\n");
    for (int j= 0; j<ncor; j++) {
      pthread_join(waitters[j], NULL);
    }
    pthread_join(waitsubtask, NULL);
    int elapsed= getTime() - ini;
    printf("\nTiempo transcurrido: %d milisegundos\n", elapsed);
  }

  printf("Felicitaciones.  Su tarea paso todos los tests\n");
  return 0;
}
