pthread_mutex_t m;
pthread_cond_t c;
int busy = 0; // Va a determinar si está o no ocupado el recurso
int n_1 = 0; // Cantidad de solicitantes con prioridad 1
void ocupar(int pri){
    pthread_mutex_lock(&m);
    if(pri == 1){ 
        n_1++; // Aumenta una solicitud
        // Espera si esta ocupado el recurso
        while(busy){ 
            pthread_cond_wait(&c, &m);
        }
        // Sale una solicitud
        n_1--;
    }
    else {
        // espera si está ocupado o hay alguna solicitud de prioridad 1
        while(busy && n_1 > 0){
            pthread_cond_wait(&c, &m);
        }
    }
    // Se marca como ocupado el recurso
    busy = 1; 
    pthread_mutex_unlock(&m);
}

void desocupar(void){
    pthread_mutex_lock(&m);
    // Se desocupa el recurso
    busy = 0;
    // Se despierta a todos los threads
    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&m);
}
