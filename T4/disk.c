#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "nthread.h"

#include "disk.h"

... defina aca sus variables globales ...

... defina aca otras funciones si las necesita ...

void requestDisk(int track) {
  ... complete ...
}

void releaseDisk(void) {
  ... complete ...
}

void diskInit(void) {
  ... complete o deje vacio ...
}

void diskClean(void) {
  ... complete o deje vacio ...
}
