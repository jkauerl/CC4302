#define P 8
#define BUFF_SIZE 100

typedef struct {
    Tareas *tareas;
    int i, j;
    int *pi, *pj;
    int *min;
    pthread_mutex_t *pm;
} Job;

void masParecidas(Tareas *tareas, int n, int *pi, int *pj) {
    int min = INT_MAX; // #include <limits.h>
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    Buffer *buf = makeBuffer(BUFF_SIZE);
    
    pthread_t pid[8];
    for (int k=0; k<8; k++)
        pthread_create(&pid[k], NULL, threadFun, buf);

    for (int i=1; i<n; i++) {
        for (int j=0; j<i; j++) {
            Job job = {tareas, i, j, pi, pj, &min, &m};
            Job *pjob = malloc(sizeof(Job));
            *pjob = job;
            putBuf(buf, pjob);
        }
    }
    
    for (int k=0; k<8; k++) 
        putBuf(buf, NULL);
    
    for (int k=0; k<8; k++) 
        pthread_join(pid[k], NULL);
    
    destroyBuffer(buf);
}

void *threadFun(void *ptr){
    Buffer *buf = ptr;
    for (;;) {
        Job *pj = bufGet(buf);
        if (pj == NULL)
            return NULL;
        
        int s = compTareas(pj->tareas[pj->i], pj->tareas[pj-j]);
        pthread_mutex_lock(pj->pm);
        if (s < *pj->min) {
            *pj->min = s;
            *pj->pi = pj->i;
            *pj->pj = pj->j;
        }
        pthread_mutex_unlock(pj->pm);
        free(pj);  
    }
}
