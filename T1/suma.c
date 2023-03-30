#include <pthread.h>
#include "suma.h"

typedef unsigned long long ulonglong;
typedef unsigned int unit;

// Defina aca las estructuras que necesite
typedef struct {
    int *a;
    unit start;
    unit end;
    unit res;
    unit n;
} Args;

// Defina aca la funcion que ejecutaran los threads

Set buscarSequencial(int a[], int start, int end, int n) {
    
    for (Set k=start; k<=end; k++) {
    // k es el mapa de bits para el subconjunto { a[i] | bit ki de k es 1 }
        long long sum= 0;
        for (int i=0; i<n; i++) {
            if ( k & ((Set)1<<i) ) // si bit ki de k es 1
                sum+= a[i];
        }
        if (sum==0) { // éxito: el subconjunto suma 0
            return k; // y el mapa de bits para el subconjunto es k
        }
    }
    return 0; // no existe subconjunto que sume 0
}

void *thread(void* p) {
    Args *args = (Args*) p;
    int *a = args->a;
    int n = args->n;
    unit start = args->start;
    unit end = args->end;

    args->res = buscarSequencial(a, start, end, n);
    return NULL;    
}

// Reprograme aca la funcion buscar
Set buscar(int a[], int n) {
    
    pthread_t pid[8];
    Args args[8];

    Set comb = (1<<(n-1)<<1)-1;
    int intervalo = (comb-1)/8;
    for(int k=0; k<8; k++) {
        args[k].a = a;
        args[k].n = n;
        args[k].start = 1 + intervalo*k;
        args[k].end = 1 + intervalo*(k+1) - 1; 

        pthread_create(&pid[k], NULL, thread, &args[k]);
    }
    int ans = 0;
    for(int k=0; k<8; k++) {
        pthread_join(pid[k], NULL);
        if(args[k].res != 0) {
            ans = args[k].res;
        }
    }
    return ans;
}
