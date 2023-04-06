// Esta es la solución incorrecta.

/* Correspodne a la solución ocupando el patron request, de tal forma que resuelve el problema en función del 
    orden de llegada del hincha.*/


#include <semaphore.h>

enum
{
    ROJO = 0,
    AZUL = 1
};

sem_t mutex;  // sem_init(&mutex, 0, 1);
sem_t sem[2]; // sem_init(&sem[ROJO], 0, 1); sem_init(&sem[AZUL], 0, 1);

int esperan[2] = {0, 0};

Queue * fila_bano[2] = // esto esta definido, usar makequeue()



int adentro[2] = {0, 0};

void entrar(int color)
{

    // el oponente del equipo AZUL es el equipo ROJO y viceversa.
    int oponente = !color;

    sem_wait(&mutex);

    // Si hay hinchas del otro equipo en el baño o en la cola, se debe esperar.
    if (adentro[oponente] > 0 || !emptyQueue(fila_bano[oponente]))
    {
        sem_t sem;
        sem_init(&sem, 0, 0);
        put(fila_bano[color], &sem);
        sem_post(&mutex);      >// Se debe soltar explicitamente el mutex porque vamos a esperar
        sem_wait(&sem);
        sem_destry(&sem); // Es importante porque es una semáforo que se creo arriba y se ocupa para esto.
    } else{
        adentro[color]++;
        sem_post(&mutex);
    }
}

void salir(int color)
{
    int oponente = !color;

    sem_wait(&mutex);

    adentro[color]--; // salimos del baño

    if (adentro[color] == 0)
    {
        // Despertar a los oponentes poniendo tantos tickets como son
        // threads hay en espera
        while(!emptyQueue(fila_bano[oponente])){
            sem_t * wait = get(fila_bano[oponente]);
            sem_post(wait);
            adentro[oponente]++;
        }
    }

    sem_post(&mutex);
}