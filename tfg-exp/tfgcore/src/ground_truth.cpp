
//----------------------------------------------------------------------
// ground_truth.cpp
//----------------------------------------------------------------------
// Generación de una instancia "ground truth" (de referencia)
//----------------------------------------------------------------------

#include "ground_truth.hpp"
#include "generator.hpp"
#include "genetico.hpp"

#include <numeric> 
#include <random>
#include <utility>

using namespace std;

//------------------------------------------------------------------
// Genera una instancia de ground truth
//------------------------------------------------------------------
GroundTruthInstance make_groundtruth(const Bitset& U, int n, int n_min, int n_max,
                                     int tam_min, int tam_max,
                                     int k, uint64_t seed_F)
{
    // Generar conjuntos base F
    vector<Bitset> F = generar_F(n, n_min, n_max, tam_min, tam_max, (int)seed_F);

    mt19937 rng(seed_F);
    vector<int> indices(F.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i;
    }

    // Generar expresión aleatoria que use hasta k conjuntos de F
    Expression gold = build_random_expr(indices, F, U, k, rng);

    // G es el resultado de evaluar esa expresión
    Bitset G = gold.conjunto;

    // Empaquetar todo
    GroundTruthInstance inst;
    inst.F = move(F);
    inst.G = move(G);
    inst.gold_expr = move(gold);
    inst.seed = seed_F;

    // Devolver
    return inst;
}
