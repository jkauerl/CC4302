typedef unsigned long long Set;
Set buscar(int a[], int n) {
    Set comb= (1<<(n-1)<<1)-1; // 2
    for (Set k= 1; k<=comb; k++) {
        // k es el mapa de bits para el subconjunto { a[i] | bit ki de k es 1 }
        long long sum= 0;
        for (int i= 0; i<n; i++) {
            if ( k & ((Set)1<<i) ) // si bit ki de k es 1
                sum+= a[i];
        }
        if (sum==0) { // éxito: el subconjunto suma 0
            return k; // y el mapa de bits para el subconjunto es k
        }
    }
    return 0; // no existe subconjunto que sume 0
}