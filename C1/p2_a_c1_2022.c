typedef struct {
    int n_fichas;
    pthread_mutex_t m;
    Queue *solicitudes;
} Sem;

typedef struct {
    pthread_cond_t c;
} Request;

void iniSem(Sem *s){
    
}
void waitSem(Sem *s){
    pthread_mutex_lock(&s->m);
    // Si no quedan fichas
    while(s->n_fichas == 0){
        // Creo una solicitud
        Request *req = {PTHREAD_COND_INITIALIZER};
        // La pongo en la cola de solicitudes
        put(&s->solicitudes);
        // Espero
        pthread_cond_wait(&s->c);
    }
    // Saco una ficha
    s->n_fichas--;
    pthread_mutex_unlock(&s->m);
}
void postSem(Sem *s){
    pthread_mutex_lock(&s->m);
    Request *req = get(&s->solicitudes);
    // Si fila está vacía
    if(req == NULL){
        // Agrego una ficha
        s->n_fichas++;
    }
    // Sino, despierto al primero que pidió una ficha
    pthread_cond_signal(req->c);
    pthread_mutex_unlock(&s->m);
}
