//----------------------------------------------------------------------
// generator.cpp
//----------------------------------------------------------------------
// Generación aleatoria de instancias: conjunto objetivo G y familia F.
//----------------------------------------------------------------------

#include "generator.hpp"

using namespace std;

// -----------------------------------------------------------------------------
// Generar conjunto G ⊆ U 
// -----------------------------------------------------------------------------
Bitset generar_G(int n, int tam_min, int seed) {
    Bitset G;
    mt19937 gen(seed);
    uniform_int_distribution<int> dist_tam(tam_min, n);
    uniform_int_distribution<int> dist_idx(0, n - 1);

    // Seleccionar tamaño de G
    int count = 0;
    int tam = dist_tam(gen);

    // Rellenar G con elementos aleatorios
    while (count < tam) {
        int idx;
        do{
            idx = dist_idx(gen);
        } while (G[idx]);

        G[idx] = 1;
        count++;
    }

    return G;
}

// -----------------------------------------------------------------------------
// Generar familia F
// -----------------------------------------------------------------------------
vector<Bitset> generar_F(int n, int n_min, int n_max,
                              int tam_min, int tam_max, int seed) {
    vector<Bitset> F;

    mt19937 gen(seed);
    uniform_int_distribution<int> dist_tam_sub(tam_min, tam_max);
    uniform_int_distribution<int> dist_tam_f(n_min, n_max);
    uniform_int_distribution<int> dist_idx(0, n - 1);

    // Seleccionar tamaño de F
    int tam_f = dist_tam_f(gen);
    F.reserve(tam_f);

    // Rellenar F con conjuntos aleatorios
    for (int i=0; i<tam_f; i++) {
        Bitset sub; 
        int tam_sub = dist_tam_sub(gen);
        int count = 0;

        // Rellenar subconjunto con elementos aleatorios
        while (count < tam_sub) {
            int idx;

            do{
                idx = dist_idx(gen);
            } while (sub[idx]);

            sub[idx] = 1;
            count++;
        }
        // Añadir subconjunto a la familia
        F.push_back(sub);
    }
    
    return F;
}