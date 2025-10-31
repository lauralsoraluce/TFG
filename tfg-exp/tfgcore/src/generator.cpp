#include "generator.hpp"

// -----------------------------------------------------------------------------
// Generar conjunto G ⊆ U 
// -----------------------------------------------------------------------------
Bitset generar_G(int n, int tam_min, int seed) {
    Bitset G;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist_tam(tam_min, n);
    std::uniform_int_distribution<int> dist_idx(0, n - 1);

    int count = 0;
    int tam = dist_tam(gen);

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
std::vector<Bitset> generar_F(int n, int n_min, int n_max,
                              int tam_min, int tam_max, int seed) {
    std::vector<Bitset> F;

    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist_tam_sub(tam_min, tam_max);
    std::uniform_int_distribution<int> dist_tam_f(n_min, n_max);
    std::uniform_int_distribution<int> dist_idx(0, n - 1);

    int tam_f = dist_tam_f(gen);
    F.reserve(tam_f);

    for (int i=0; i<tam_f; i++) {
        Bitset sub; 
        int tam_sub = dist_tam_sub(gen);
        int count = 0;

        while (count < tam_sub) {
            int idx;

            do{
                idx = dist_idx(gen);
            } while (sub[idx]);

            sub[idx] = 1;
            count++;
        }
        F.push_back(sub);
    }
    return F;
}