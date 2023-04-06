typedef sem_t;

// Escriba una solución correcta y eficiente para este problema utilizando 3 semáforos.
// No importa que en su solución algunos procesos sufran “hambruna”.
//                  No programar para solucionar hambruna 
// Hint: utilice la estructura de la solución incorrecta.

/* Notar que el mutext que se incluye no correpsonde a un mutex como tal, sino una variable. 
    El primer problema corresponde a si 2 hinchas pueden llegar al mimso tiempo y ocupar ambos el baño. 
    Ocupar diagramas de threads para ver el caso fácil, y cuando esta falla.*/

/* Notar que la hambruna sucede porque todos siempre pueden haber hinchas de un color entrando y saliendo del baño,
    de tal forma que para el otro equipo se crea una cola que nunca se ocupan los elementos.*/

enum
{
    ROJO = 0,
    AZUL = 1
};


sem_t mutex_zona_critica[2]; //sem_init(&mutex_zona_critica[0])
sem_t mutex_entrada_al_bano; //sem_init(&mutex_entrada_bano)

int cantidad[2] = {0, 0};
int mutex = 0; // este mutex representa el acceso al baño. El equipo que lo tiene es el que está adentro.

void entrar(int color)
{
    sem_wait(&mutex_zona_critica[color]);
    if (cantidad[color] == 0)
    {
        sem_wait(&mutex_entrada_al_bano);
    }
    cantidad[color]++;
    sem_post(&mutex_zona_critica[color]);
}

void salir(int color)
{
    cantidad[color]--;
    if (cantidad[color] == 0)
    {
        mutex = 0;
    }
}



