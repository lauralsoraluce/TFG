#include <iostream>
#include <vector>
#include <string>
#include "domain.hpp"
#include "generator.hpp"
#include "metrics.hpp"
#include <chrono>
#include "exhaustiva.hpp"
#include "solutions.hpp"
#include "greedy.hpp"
#include "genetico.hpp"
#include "spea2.hpp"

using namespace std;

template<typename T>
struct ResultadoAlgoritmo {
    string nombre;
    vector<T> soluciones;
    long long tiempo_ms;
    /*double Jaccard_promedio;
    double SizeH_promedio;
    double OpSize_promedio;*/
};

inline std::vector<SolMO> individuos_a_solmos(const std::vector<Individuo>& v) {
    std::vector<SolMO> r; r.reserve(v.size());
    for (const auto& x : v) r.emplace_back(x.expr, x.n_ops, x.sizeH, x.jaccard);
    return r;
}

int main(int argc, char* argv[]) {
    // ======= EXPERIMENTO EN LOTE: SOLO GENÉTICO (NSGA-II) =======
    // --- configuración “grande” (ajusta a tu gusto) ---
    int n = U_size;
    int G_size_min   = 10;
    int F_n_min      = 10;
    int F_n_max      = 100;
    int Fi_size_min  = 5;
    int Fi_size_max  = 100;

    
    std::vector<int> seeds   = {2001, 2002, 2003};   // 4–5 seeds
    std::vector<int> ks      = {10};                        // distintos k
    std::vector<int> budgets = {600};                       // p.ej. 2min y 5min

    int pop_size        = 600;   // para k grandes, poblaciones >400 suelen ir mejor
    double pm           = 0.6;   // mutation_prob
    double pc           = 0.8;   // crossover_prob
    int tournament_k    = 2;
    int max_generations = 1e9;   // límite real = tiempo

    // cabecera general
    std::cout << "=== BATCH: SOLO GENETICO (NSGA-II) ===\n"
              << "U=" << n
              << " | F_n in [" << F_n_min << "," << F_n_max << "]"
              << " | |Fi| in [" << Fi_size_min << "," << Fi_size_max << "]\n\n";

    for (int seed : seeds) {
        // 1) Generar UNA instancia por seed (misma G y F para todos los k y tiempos)
        Bitset U;
        for (int i = 0; i < n; ++i) U[i] = 1;

        Bitset G = generar_G(U.size(), G_size_min, seed);
        std::vector<Bitset> F = generar_F(U.size(), F_n_min, F_n_max, Fi_size_min, Fi_size_max, seed);

        std::cout << "---- SEED " << seed
                  << " | |G|=" << G.count()
                  << " | |F|=" << F.size() << " ----\n";

        for (int k : ks) {
            for (int tsec : budgets) {
                // 2) Parametrizar GA (semilla reproducible por (seed,k,t))
                GAParams params;
                params.population_size = pop_size;
                params.mutation_prob   = pm;
                params.crossover_prob  = pc;
                params.tournament_size = tournament_k;
                params.max_generations = max_generations;
                params.time_limit_sec  = tsec;
                // mezcla simple y reproducible
                params.seed = (static_cast<uint64_t>(seed) << 32)
                            ^ static_cast<uint64_t>(k * 1009)
                            ^ static_cast<uint64_t>(tsec * 7919);

                std::cout << "  [RUN] k=" << k
                          << " | t=" << tsec << "s"
                          << " | pop=" << pop_size
                          << " | pm=" << pm
                          << " | pc=" << pc
                          << " | rng=" << params.seed << "\n";

                auto t0 = std::chrono::high_resolution_clock::now();
                std::vector<Individuo> pareto_inds = nsga2(F, G, k, params);
                auto t1 = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

                // resumen de resultados
                
                auto a_solmo = [&](const std::vector<Individuo>& v){
                    std::vector<SolMO> r; r.reserve(v.size());
                    for (const auto& x : v) r.emplace_back(x.expr, x.n_ops, x.sizeH, x.jaccard);
                    return r;
                };
                auto soluciones = a_solmo(pareto_inds);

                // métrica rápida: mejor Jaccard del frente
                double bestJ = 0.0;
                for (const auto& s : soluciones) bestJ = std::max(bestJ, s.jaccard);

                std::cout << "    -> tiempo=" << ms << "ms"
                          << " | |Pareto|=" << soluciones.size()
                          << " | bestJ=" << bestJ << "\n";

                
                print_pareto_front(soluciones);
            }
        }
        std::cout << std::endl;
    }

    // salir aquí para no ejecutar el resto del main
    return 0;
// ======= FIN EXPERIMENTO EN LOTE =======

}