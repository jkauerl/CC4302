// Escriba una solución correcta y eficiente para este problema utilizando 3 semáforos.
// No importa que en su solución algunos procesos sufran “hambruna”.
//                  No programar para solucionar hambruna 
// Hint: utilice la estructura de la solución incorrecta.

/* Notar que el mutext que se incluye no correpsonde a un mutex como tal, sino una variable. 
El primer problema corresponde a si 2 hinchas pueden llegar al mimso tiempo y ocupar ambos el baño. 
Ocupar diagramas de threads para ver el caso fácil, y cuando esta falla.*/

enum
{
    ROJO = 0,
    AZUL = 1
};

int cantidad[2] = {0, 0};
int mutex = 0; // este mutex representa el acceso al baño. El equipo que lo tiene es el que está adentro.

void entrar(int color)
{
    if (cantidad[color] == 0)
    {
        while (mutex)
            ;
        mutex = 1;
    }
    cantidad[color]++;
}

void salir(int color)
{
    cantidad[color]--;
    if (cantidad[color] == 0)
    {
        mutex = 0;
    }
}
