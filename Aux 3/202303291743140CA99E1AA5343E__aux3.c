#include<pthread.h>

// Estructuras de juguete
typedef struct {
    int a;
}Camion;

typedef struct {
    int a;
}Ciudad;

typedef struct {
    int a;
}Contenedor;

// Iniciamos unas variables
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; // herramienta de sincronización
Camion *camion;      // Camión
Ciudad *ubic= &Stgo; // Ciudad

/* Consideramos la empresa ha diseñado esta función para mover su unico camión */
void transportar(Contenedor *cont, Ciudad *orig, Ciudad *dest){
    pthread_muted_lock(&m);   // Asegurar exclusividad Mutua
    conducir(camion, ubic, orig);  // ASUMIR QUE EXISTE (tarda mucho tiempo)
    cargar(camion, cont);          //  ditto
    conducir(camion, orig, dest);  //  ditto
    descargar(camion, cont);       //  ditto
    ubic = dest;
    pthread_mutex_unlock(&m); // Ceder exclusividad
}

/* Con el tiempo la empresa creció y ahora tienen 8 camiones y por lo tanto
   Necesitan una nueva forma de manejar el transporte de estos para poder
   distribuir los camiones entree los encargos de carga.

   En particular se nos pide redefinir la función transporte, añadiendo funciones
   para:
      - poder buscar camiones disponibles
      - marcar que se está desocupando un camión

   La empresa nos dió lo siguiente para hacer el trabajo:
*/

#define P 8      // Número de camiones
#define R 100    // Número max de encargos
#define TRUE 1
#define FALSE 0

Camion *camiones[P];    // Los camiones
Ciudad *ubicaciones[P]; // Ciudades en las cuales se trabaja

int ocupados[P]; // Indica si los camiones estan ocupados

double distancia(Ciudad * orig, Ciudad *dest); // dest entre 2 ciudades

/* Patron Request
 * 
 * Cada thread tiene su propia condicion para asi evitar 
 * despertar a todos los threads e ir llamando espeficicos 
 * threads a despertarse cuando corresponde
**/

typedef struct {
    int ready;
    int camion_idx;
    Ciudad *orig;
    pthread_cond_t cond;
} Request

Request *peticiones[R];

int pedir_camion(Ciudad *orig){
    int camion_idx = -1;
    // Si hay camiones, elegimos el más cercano
    for(int i = 0; i<P, i++){
        if(!ocupados[i]){
            if(camion_idx == -1 || distancia(orig, ubicaciones[i]) < distancia(origen, ubicaciones[camion_idx])){
                camion_idx = i;
            }
        }
    }

    // Si no hay camiones, esperamos
    if (camion_idx == -1){
        // Generar la request con los datos necesario
        Request req = {FALSE, -1, orig, PTHREAD_COND_INITIALIZER};

        // Encolar la request
        for(int k = 0; k < R; k++){
            if(peticiones[k] == NULL)
                peticiones[k] = &req
        }

        // Esperar
        while(req.ready == FALSE){
            pthread_cond_wait(&req.cond, &m);
        }

        // Decir cual es el camion
        camion_idx = req.camion_idx;
    }

    return camion_idx;
}

void desocupar_camion(int camion_idx){
    // Si hay peticiones, elegimos la más cercana
    Request req_idx = -1;
    for(int k = 0; k < P; k++){
        (if peticiones[k] != NULL){
            if(
                req == NULL || 
                distancia(ubicaciones[camion_idx], peticiones[k]->orig) < 
                distancia(ubicaciones[camion_idx], peticiones[req_idx]->orig)
            )
                req_idx = k;
        }
    }

    if(req_idx != -1){
        peticiones[req_idx]->ready = TRUE;
        peticiones[req_idx]->camion_idx = camion_idx;
        pthread_cond_broadcast(peticiones[req_idx]->cond);
        peticiones[req_idx] = NULL
    } else{
        ocupados[camion_idx] = FALSE;
    }
}


void transportar(Contenedor *cont, Ciudad *orig, Ciudad *dest){

    pthread_muted_lock(&m);   // Asegurar exclusividad Mutua
    int camion_idx = pedir_camion(orig);
    ocupados[camion_idx] = TRUE;
    Camion *camion = camiones[camion_idx]
    pthread_mutex_unlock(&m); // Ceder exclusividad

    conducir(camion, ubic, orig);
    cargar(camion, cont);
    conducir(camion, orig, dest);
    descargar(camion, cont);
    
    pthread_muted_lock(&m);   // Asegurar exclusividad Mutua
    ubicaciones[camion_idx] = dest;
    desocupar_camion(camion_idx);
    pthread_mutex_unlock(&m); // Ceder exclusividad

}

// 1 camion 1 transporte
// No puede haber un camion libre si hay envios pendientes
// Elegir camion más cercano
// Para transportar es: 
//  - ir a ciudad origen
//  - cargar
//  - ir a ciudad destino
//  - descargar