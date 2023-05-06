// funciones dadas
void modoBajoConsumo();
void modoUsoNormal();

//hay que implementar
void obtenerImpresora();
void devolverImpresora();
void inicializarImpresora();


/*
struct timespec { //definida en time.h
    time_t tv_sec;
    long tv_nsec;
};
*/


typedef struct {
    int ready;
    pthread_cond_t w;
} Request;

Queue *q;
int ocupada;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t obtener = PTHREAD_COND_INITIALIZER;
pthread_cond_t devolver = PTHREAD_COND_INITIALIZER;
pthread_t impresora;

void obtenerImpresora() {
    pthread_mutex_lock(&m);
    Request req = {FALSE, PTHREAD_COND_INITIALIZER};
    put(q, &req);
    
    pthread_cond_signal(&obtener);
    while(!req.ready)
        pthread_cond_wait(&req.w, &m);
    
    pthread_mutex_unlock(&m);
}

void devolverImpresora() {
    pthread_mutex_lock(&m);
    ocupada = FALSE;
    pthread_cond_signal(&devolver); //notifica a impresora    
    pthread_mutex_unlock(&m);
}

void inicializarImpresora() {
    q = MakeQueue();
    ocupada = FALSE;
    pthread_create(&impresora, NULL, impresoraServer, NULL);
}


void impresoraServer() {
    while(TRUE) {
        pthread_mutex_lock(&m);
        
        if (emptyQueue(q)) { //esperar 5 min a que llegue solicitud
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts); //definida en time.h
            ts.tv_sec += 5*60;
            
            while(emptyQueue(q)) {
                int rc = pthread_cond_timedwait(&obtener, &m, &ts);
                if (rc == ETIMEDOUT)
                    break;
            }
                  
            if (emptyQueue(q)) { //se cumplió el timeout
                modoBajoConsumo();
                while(emptyQueue(q))
                    pthread_cond_wait(&obtener, &m);
                modoUsoNormal();            
            }            
        }
        
        Request *pr = get(q);
        pr->ready = TRUE;
        ocupada = TRUE;
        pthread_cond_signal(&pr->w); 
        
        while(ocupada)
            pthread_cond_wait(&devolver);

        pthread_mutex_unlock();
    }
}