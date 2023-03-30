#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "suma.h"

#ifdef OPT
#define N_INTENTOS 4
#define TOLERANCIA 1.5
#else
#define N_INTENTOS 1
#define TOLERANCIA 0.01
#endif


// ----------------------------------------------------
// Funcion que entrega el tiempo transcurrido desde el lanzamiento del
// programa en milisegundos

static int time0= 0;

static int getTime0() {
    struct timeval Timeval;
    gettimeofday(&Timeval, NULL);
    return Timeval.tv_sec*1000+Timeval.tv_usec/1000;
}

void resetTime() {
  time0= getTime0();
}

int getTime() {
  return getTime0()-time0;
}

// ----------------------------------------------------

Set buscarSeq(int a[], int n) {
  Set comb= (1<<(n-1)<<1)-1; // 2n-1: nro. de combinaciones
  for (Set k= 1; k<=comb; k++) {
    // k es el mapa de bits para el subconjunto { a[i] | bit ki de k es 1 }
    long long sum= 0;
    for (int i= 0; i<n; i++) {
      if ( k & ((Set)1<<i) ) // si bit ki de k es 1
        sum+= a[i];
    }
    if (sum==0) {  // exito: el subconjunto suma 0
      return k;    // y el mapa de bits para el subconjunto es k
  } }
  return 0;        // no existe subconjunto que sume 0
}

void mostrar(int a[], Set k, int n) {
  long long sum= 0;
  if (k==0) {
    printf("subconjunto no existe\n");
    return;
  }
  for (int i= 0; i<n; i++) {
    if ( k & ((Set)1<<i) ) {
      printf("%d ", a[i]);
      sum+= a[i];
    }
  }
  if (sum!=0) {
    fprintf(stderr, "El subconjunto suma %lld, no 0\n", sum);
  }
  printf("\n");
}

#define N 10000
#define M 20000
#define P 4

int main() {

  printf("Test 1: El ejemplo del enunciado\n");
  int a[]= { -7, -3, -2, 5, 8};
  Set kSeq= buscarSeq(a, 5);
  mostrar(a, kSeq, 5);
  Set k= buscar(a, 5);
  if (k!=kSeq) {
    fprintf(stderr, "La solucion debio ser %lld, no %lld\n", kSeq, k);
    exit(1);
  }
  mostrar(a, k, 5);
  printf("test 1 aprobado\n");
  printf("\n--------------------------------------------------\n\n");

  printf("Test 2: Uno ejemplo mas grande con n=26, sin solucion\n");
  int b[]= { 122737, -37364, 287373, -27267, 967923, -25383, 924973, -28973,
             278363, 28272, 98734, -26735, -983267, 674998, 72537, 116725,
             72537, 27263, 82739, 829276, -5383715, 675483, -28334, 38495,
             374943, 278367};

  int tiempo_sec, tiempo_par;
  double speedUp= 0;
  int i;
  
  printf("Calculando secuencialmente 2 veces\n");
  for (i= 0; i<2; i++) {
    resetTime();
    k= buscarSeq(b, 26);
    if (k!=0) {
      mostrar(b, k, 26);
      fprintf(stderr, "Bug del profesor: la solucion debio ser %d, no %lld\n",
                      0, k);
      exit(1);
    }
    tiempo_sec= getTime();
    printf("Tiempo secuencial= %d milisegundos\n", tiempo_sec);
  }
 
  printf("Calculando en paralelo hasta 5 veces\n");
  for (i= 0; i<N_INTENTOS; i++) {
    resetTime();
    k= buscar(b, 26);
    if (k!=0) {
      mostrar(b, k, 26);
      fprintf(stderr, "La solucion debio ser %d, no %lld\n", 0, k);
      exit(1);
    }
    tiempo_par= getTime();
    speedUp= (double)tiempo_sec/tiempo_par;
    printf("buscar par tiempo= %d miliseg., speedup= %f\n", tiempo_par, speedUp);
    if (speedUp>=TOLERANCIA)
      break;
  }
  if (i>=N_INTENTOS) {
    fprintf(stderr, "Despues de %d intentos no obtuvo un speedup de %f\n",
            N_INTENTOS, TOLERANCIA);
    fprintf(stderr, "Revise la paralelizacion.\n");
    exit(1);
  }
  printf("test 1 aprobado: speedup >= %f\n", TOLERANCIA);
  printf("\n--------------------------------------------------\n\n");

  printf("Uno ejemplo muy grande con n=29, con solucion\n");
  int c[]= { -3, -2, 5,
             122737, -37364, 287373, 27267, 967923, -25383, 924973, -28973,
             278363, 28272, 98734, -26735, 983267, 674998, 72537, 116725,
             72537, 27263, 82739, 829276, 5383715, 675483, -28334, 38495,
             374943, 278367};

  printf("Calculando secuencialmente 2 veces\n");
  resetTime();
  k= buscarSeq(c, 29);
  tiempo_sec= getTime();
  mostrar(c, k, 29);
  printf("Tiempo secuencial= %d milisegundos\n", tiempo_sec);
 
  printf("Calculando en paralelo\n");
  resetTime();
  int k29= buscar(c, 29);
  if (k!=k29) {
    printf("Solucion incorrecta:\n");
    mostrar(c, k29, 29);
    printf("Debio ser:\n");
    mostrar(c, k, 29);
    exit(1);
  }
  tiempo_par= getTime();
  speedUp= (double)tiempo_sec/tiempo_par;
  printf("buscar par tiempo= %d miliseg., speedup= %f\n", tiempo_par, speedUp);
  printf("No se preocupe.  Es normal que la version paralela se demore\n"
         "mucho mas.  Pero piense por que.\n\n");

  printf("Felicitaciones: su tarea funciona\n");
  
  return 0;
}

