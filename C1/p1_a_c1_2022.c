/*
    Vamos a agregar a el campo nombre a el descriptor
*/
n_thQueue *nth_varonesQueue;
n_thQueue *nth_damasQueue;
char *nDama(char *nombre){
    START_CRITICAL
    nThread this_th = nSelf();
    this_th->nombre = nombre;
    nThread varon_th = nth_getFront(nth_varonesQueue);
    // La cola está vacía
    while(varon_th == NULL){
        nth_putBack(nth_damasQueue, this_th);
        suspend(WAIT_VARON);    
        schedule();
        varon_th = nth_getFront(nth_varonesQueue);
    }
    nombre = varon_th->nombre;
    setReady(this_th);
    END_CRITICAL
    return nombre;
}

char *nVaron(char *nombre){
    START_CRITICAL
    nThread this_th = nSelf();
    this_th->nombre = nombre;
    // La cola está vacía
    nThread dama_th = nth_getFront(nth_damasQueue);
    while(dama_th == NULL){
        nth_putBack(nth_varonesQueue, this_th);
        suspend(WAIT_DAMA);    
        schedule();
        dama_th = nth_getFront(nth_damasQueue);
    }
    nombre = dama_th->nombre;
    setReady(this_th);
    END_CRITICAL
    return nombre;
}

void nth_iniDisco(){
    nth_varonesQueue = makeQueue();
    nth_damasQueue = makeQueue();
}