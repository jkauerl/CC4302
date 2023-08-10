#include "nthread.h"

#include "nexchange.h"

/* Importante: cuando su programa no funcione y de un error fatal,
 * use ddd (o gdb) poniendo un breakpoint en la funcion exit.
 * Esto le permitira determinar el contexto en donde se produjo
 * el error.
 */

typedef struct {
  pthread_t target;
  int delay;
  void *msg, *expected;
} TaskParam;

static void *task_proc(void *ptr) {
  TaskParam *pparam= ptr;
  pthread_t target= pparam->target;
  int delay= pparam->delay;
  void *msg= pparam->msg, *expected= pparam->expected;
  free(pparam);

  void *exch_msg;
  usleep(delay*1000);
  exch_msg= nExchange(target, msg, -1);
  if (expected!=exch_msg)
    nFatalError("task_proc", "No se recibio el mensaje esperado\n");
  return 0;
}

static void create_task(pthread_t *pself, pthread_t target,
                int delay, void *msg, void *expected) {
  TaskParam param= { target, delay, msg, expected};
  TaskParam *pparam= malloc(sizeof(TaskParam));
  *pparam= param;
  if (pthread_create(pself, NULL, task_proc, pparam)!=0)
    nFatalError("pthread_create", "No se puede crear thread\n");
}

static void *test1(void *ptr) {
  int msg1, msg2, *exch_msg;
  pthread_t t;
  create_task(&t, pthread_self(), 10, &msg1, &msg2);

  exch_msg= nExchange(t, &msg2, -1);
  if (exch_msg != &msg1)
    nFatalError("nMain", "no se recibio el mensaje esperado\n");
  pthread_join(t, NULL);
  return NULL;
}

static void *test2(void *ptr) {
  int msg1_a, msg2_a, msg1_b, msg2_b, *exch_msg_a, *exch_msg_b;
  pthread_t t_a, t_b;
  create_task(&t_a, pthread_self(), 10, &msg1_a, &msg2_a);
  create_task(&t_b, pthread_self(), 20, &msg1_b, &msg2_b);

  exch_msg_b= nExchange(t_b, &msg2_b, -1);
  if (exch_msg_b != &msg1_b)
    nFatalError("nMain", "no se recibio el mensaje esperado\n");

  exch_msg_a= nExchange(t_a, &msg2_a, -1);
  if (exch_msg_a != &msg1_a)
    nFatalError("nMain", "No se recibio el mensaje esperado\n");
  pthread_join(t_a, NULL);
  pthread_join(t_b, NULL);
  return NULL;
}

#ifdef SAN
#define NTASKS 1000
#define NITER  10000
#else
#define NTASKS 1000
#define NITER  100000
#endif

pthread_t tasks[NTASKS*3];

static int test_parallel() {
  int i, k;
  for (k=0; k<NTASKS; k++) {
    tasks[3*k]= NULL;
    tasks[3*k+1]= NULL;
    tasks[3*k+2]= NULL;
  }
   
  k= 0;
  for (i=0; i<NITER; i++) {
    if (tasks[3*k]!=NULL)
      pthread_join(tasks[3*k], NULL);
    if (pthread_create(&tasks[3*k], NULL, test1, NULL)!=0)
      nFatalError("pthread_create", "No se pudo crear el thread\n");
    if (tasks[3*k+1]!=NULL)
      pthread_join(tasks[3*k+1], NULL);
    if (pthread_create(&tasks[3*k+1], NULL, test2, NULL)!=0)
      nFatalError("pthread_create", "No se pudo crear el thread\n");
    if (tasks[3*k+2]!=NULL)
      pthread_join(tasks[3*k+2], NULL);
    if (pthread_create(&tasks[3*k+2], NULL, i%1==0 ? test1 : test2, NULL)!=0)
      nFatalError("pthread_create", "No se pudo crear el thread\n");
    k++;
    if (k==NTASKS)
      k= 0;
  }

  for (k=0; k<NTASKS; k++) {
    pthread_join(tasks[3*k], NULL);
    pthread_join(tasks[3*k+1], NULL);
    pthread_join(tasks[3*k+2], NULL);
  }
  return 0;
}

int main(int argc, int argv) {

  printf("Test 1: Verifica que los mensajes intercambiados sean correctos\n");
  test1(NULL);

  printf("Test 2: Verifica que el intercambio ocurra con la tarea correcta\n");
  test2(NULL);

  printf("Test 3: Test de robustez\n");
  test_parallel();

  printf("Felicitaciones! Su tarea paso todos los tests\n");

  return 0;
}
