sem_t mtx_damas;
sem_t llega_dama;
sem_t mtx_varones;
sem_t llega_varon;
char *nom_dama, *nom_varon;

 void nth_iniDisco() {
    sem_init(&mtx_damas,0,1);
    sem_init(&mtx_varones,0,1);
    sem_init(&sale_damas,0,1);
    sem_init(&sale_varones,0,1);
    sem_init(&llega_dama,0,0);
    sem_init(&llega_varon,0,0);
}
char *nDama(char *nom) {
    sem_wait(&mtx_damas);
    // Solo puede entrar una dama, si ya salió el varón que estaba adentro
    sem_wait(&sale_varon);
    nom_dama = nom;
    sem_post(&llega_dama);
    sem_wait(&llega_varon);
    char *pareja=nom_varon;
    // Avisamos que salió la dama
    sem_post(&sale_mujer);
    sem_post(&mtx_damas);
 return par
}

char *nVaron(char *nom) {
    sem_wait(&mtx_varones);
    // Solo puede entrar el varon, si salió el varon que estaba adentro
    sem_wait(&sale_mujer);
    nom_varon= nom;
    sem_post(&llega_varon);
    sem_wait(&llega_dama);
    char *pareja=nom_dama;
    // Avisamos que salió el varón
    sem_post(&sale_varon);
    sem_post(&mtx_varones);
    return pareja;
}
