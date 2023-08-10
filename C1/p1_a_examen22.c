typedef struc {
    int a;
    int n;
    int j;
    int k;
    int res;
} Args;

int palindro1(char *a, int n, int j, int k) {
    // Parte en j
    int i = j;
    // Llega hasta k
    while (i<k) {
        if(a[i]!=a[n-1-i])
            return 0;
        i++;
    }
    return 1;
}

void *threadFun(void *ptr){
    // Se castea
    Args *args = (Args *) ptr;
    // Se recuperan los argumentos
    int a = args->a;
    int n = args->n;
    int j = args->i;
    int k = args->k;
    // Se ejecuta palindro1
    args->res = palindro1(a, n, i, k);
    return NULL;
}

int palindro(char *a, int n){
    // Se crea un thread, que va a revisar hasta en [0, n/4] y [n-n/4-1,n-1]
    pthread_t second_thread;
    Args ptr = {a, n, 0, n/4};
    // Se lanza el thread
    pthred_create(&second_thread, NULL, threadFun, &ptr);
    // Se ejecuta palindro 1 en [n/4 + 1, n/2-1] y [n/2, n-n/4-2]
    int res = palindro1(a, n, n/4 + 1, n/2);
    // Se entierra el segundo thread
    pthread_join(second_thread, NULL);
    // Si suman 2, es palindromo
    if(ptr.res + res == 2){ 
        return 1;
    }
    // Sino, no
    return 0;
}
