#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>
  
#include "nthread.h"

#include "batch.h"

struct job {
  ... defina aca la estructura de Job ...
};

... defina aca sus variables globales ...

... defina las funciones auxiliares que necesite ...

void startBatch(int n) {
  ... complete ...
}

void stopBatch() {
  ... complete ...
}

Job *submitJob(JobFun fun, void *input) {
  ... complete ...
}

void *waitJob(Job *job) {
  ... complete ...
}

