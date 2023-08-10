#include <pthread.h>

#include "pss.h"
#include "spinlocks.h"
#include "h2o.h"

// ... defina aca variables globales ...  
Queue *queues[2];
int sl;
// int busy;

typedef struct {
  void *atom;
  int *sl;
  H2O *water;
} Request;


void initH2O(void) {
  queues[0] = makeQueue(); // Cola de Oxygeno
  queues[1] = makeQueue(); // Cola de Hydrogeno
  sl=OPEN;

}

void endH2O(void) {
  destroyQueue(queues[0]);
  destroyQueue(queues[1]);

}

H2O *combineOxy(Oxygen *o) {
  spinLock(&sl);
  H2O *water = NULL;
  
  if(queueLength(queues[1])>=2) {
    Request *h1 = get(queues[1]); // Pedir hydrogenos de la cola 1
    Request *h2 = get(queues[1]); // Pedir hydrogenos de la cola 1
    water = makeH2O(h1->atom, h2->atom, o);
    h1->water = water;
    h2->water = water;
    spinUnlock(h1->sl);
    spinUnlock(h2->sl);
  }
  else {
      int w=CLOSED; // spinlock del request
      Request oxy = {o, &w, NULL};
      put(queues[0], &oxy);
      spinUnlock(&sl);
      // esperar
      spinLock(oxy.sl);
      // despiertan
      spinLock(&sl);
      water = oxy.water;
  }
  
  spinUnlock(&sl);
  return water;
}

H2O *combineHydro(Hydrogen *h) {
  spinLock(&sl);
  H2O *water = NULL;
  
  if(queueLength(queues[0])>=1 && queueLength(queues[1])>=1) { 
    Request *o1 = get(queues[0]); // Pedir oxygenos de la cola 0
    Request *h1 = get(queues[1]); // Pedir hydrogenos de la cola 1
    water = makeH2O(h1->atom, h, o1->atom);
    o1->water = water;
    h1->water = water;
    spinUnlock(o1->sl);
    spinUnlock(h1->sl);
  }
  else {
      int w=CLOSED; // spinlock del request
      Request hydro = {h, &w, NULL};
      put(queues[1], &hydro);
      spinUnlock(&sl);
      // esperar
      spinLock(hydro.sl);
      // despiertan
      spinLock(&sl);
      water = hydro.water;
  }
  
  spinUnlock(&sl);
  return water;
}
