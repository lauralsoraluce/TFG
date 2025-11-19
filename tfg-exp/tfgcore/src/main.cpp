//------------------------------------------------------------------
// main.cpp
//------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "domain.hpp"
#include "generator.hpp"
#include "metrics.hpp"
#include "exhaustiva.hpp"
#include "solutions.hpp"
#include "greedy.hpp"
#include "genetico.hpp"
#include "ground_truth.hpp"

using namespace std;

// ------------------------------------------------------------------
// Estructura para almacenar resultados de los algoritmos
// ------------------------------------------------------------------
template<typename T>
struct ResultadoAlgoritmo {
    string nombre;
    vector<T> soluciones;
    long long tiempo_ms;
};

// Convierte un vector de Individuo a un vector de SolMO
inline std::vector<SolMO> individuos_a_solmos(const std::vector<Individuo>& v) {
    std::vector<SolMO> r; r.reserve(v.size());
    for (const auto& x : v) r.emplace_back(x.expr, x.n_ops, x.sizeH, x.jaccard);
    return r;
}

// ------------------------------------------------------------------
// Impresión sencilla de los conjuntos G y F
// ------------------------------------------------------------------
static void print_conjuntos(const Bitset& G, const std::vector<Bitset>& F) {
    cout << "=== CONJUNTOS ===" << endl;
    cout << "CONJUNTO_G: ";
    for (int i = 0; i < U_size; ++i) {
        if (G[i]) {
            cout << i << ",";
        }
    }
    cout << "\n\n";

    cout << "NUM_CONJUNTOS_F: " << F.size() << "\n";
    for (size_t j = 0; j < F.size(); ++j) {
        cout << "F" << j << ": ";
        for (int i = 0; i < U_size; ++i) {
            if (F[j][i]) {
                cout << i << ",";
            }
        }
        cout << "\n";
    }
    cout << "\n";
}

// ------------------------------------------------------------------
// MAIN
// ------------------------------------------------------------------
int main(int argc, char** argv) {
    // Parámetros por defecto
    int G_size_min= 10;
    int F_n_min= 5;
    int F_n_max= 15;
    int Fi_size_min= 5;
    int Fi_size_max= 40;
    int k= 3;
    int seed= 1002;
    bool ejecutar_exhaustiva= true;
    bool ejecutar_greedy= true;
    bool ejecutar_genetico= false;
    int pop_size=150;
    double mutation_prob= 0.5;      
    double crossover_prob= 0.8;     
    int tournament_size= 5;         
    int max_generations= 1e9;       
    int time_limit= 900;   
    bool modo_test= true; // modo test por defecto
    int seed_expr= (uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count();      
    
    // Conjunto universo U
    int n = U_size;
    Bitset U;
    for (int i = 0; i < n; ++i) U[i] = 1;

    // Procesar argumentos de línea de comandos
    for (int i = 1; i < argc; i++) {
        string a = argv[i];
        
        if (a == "--G") G_size_min=stoi(argv[++i]); // tamaño mínimo de G
        else if (a == "--Fmin") F_n_min=stoi(argv[++i]); // número mínimo de conjuntos en F
        else if (a == "--Fmax") F_n_max=stoi(argv[++i]); // número máximo de conjuntos en F
        else if (a == "--FsizeMin") Fi_size_min=stoi(argv[++i]); // tamaño mínimo de cada conjunto en F
        else if (a == "--FsizeMax") Fi_size_max=stoi(argv[++i]); // tamaño máximo de cada conjunto en F
        else if (a == "--k") k=stoi(argv[++i]); // número máximo de expresiones en la expresión
        else if (a == "--seed") seed=stoi(argv[++i]); // semilla para generación de conjuntos
        else if (a == "--pop_size") pop_size=stoi(argv[++i]); // tamaño población GA
        else if (a == "--mutation_prob") {mutation_prob=stod(argv[++i]);} // probabilidad de mutación GA
        else if (a == "--crossover_prob") {crossover_prob=stod(argv[++i]);} // probabilidad de cruce GA
        else if (a == "--tournament_size") {tournament_size=stoi(argv[++i]);} // tamaño torneo GA
        else if (a == "--max_generations") {max_generations=stoi(argv[++i]);} // número máximo de generaciones GA
        else if (a == "--time_limit") {time_limit=stoi(argv[++i]);} // límite de tiempo GA (segundos)
        else if (a == "--no-test") modo_test= false; // desactivar modo test
        else if (a == "--seed_expr") seed_expr=stoi(argv[++i]); // semilla para GA
        else if (a == "--algo") { // elegir algoritmo
            string algo = argv[++i];
            ejecutar_exhaustiva = (algo == "exhaustiva" || algo == "all");
            ejecutar_greedy = (algo == "greedy" || algo =="both" || algo == "all");
            ejecutar_genetico = (algo == "genetico" || algo == "both" || algo == "all");
        }
    }

    // Modo de prueba: generar conjuntos y ejecutar algoritmos seleccionados
    if (modo_test) {
        // Generar conjuntos
        Bitset G = generar_G(n, G_size_min, seed);
        std::vector<Bitset> F = generar_F(n, F_n_min, F_n_max, Fi_size_min, Fi_size_max, seed);

        cout << "Semilla: " << seed << "\n";
        cout << "U_size: " << U_size << "\n";
        print_conjuntos(G, F); 

        vector<ResultadoAlgoritmo<SolMO>> resultados;
        // Ejecutar algoritmos seleccionados
        if (ejecutar_exhaustiva) {
            // EXHAUSTIVA
            cout << "=== EXHAUSTIVA ===\n";
            auto t0 = chrono::high_resolution_clock::now();
            auto soluciones = exhaustive_search(F, U, G, k);
            auto t1 = chrono::high_resolution_clock::now();
            auto dur_ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

            cout <<"Tiempo_ejecucion_ms: " << dur_ms << "\n\n";
            print_pareto_front(soluciones);
            resultados.push_back({"Exhaustiva", soluciones, dur_ms});
        }
        if (ejecutar_greedy) {
            // GREEDY
            cout << "=== GREEDY ===\n";
            auto t0 = chrono::high_resolution_clock::now();
            auto soluciones = greedy_multiobjective_search(F, U, G, k);
            auto t1 = chrono::high_resolution_clock::now();
            auto dur_ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

            cout <<"Tiempo_ejecucion_ms: " << dur_ms << "\n\n";
            print_pareto_front(soluciones);
            resultados.push_back({"Greedy", soluciones, dur_ms});
        }
        if (ejecutar_genetico) {
            // GENETICO (NSGA-II)
            cout << "=== GENETICO (NSGA-II) ===\n";
            GAParams ga_params;
            ga_params.population_size   = pop_size;
            ga_params.max_generations   = max_generations;
            ga_params.time_limit_sec    = time_limit;
            ga_params.crossover_prob    = crossover_prob;
            ga_params.mutation_prob     = mutation_prob;
            ga_params.tournament_size   = tournament_size;
            ga_params.seed              = seed_expr;

            cout << "Semilla_GA: " << ga_params.seed << "\n";
            cout << "Población: " << ga_params.population_size << endl;
            cout << "Limite_tiempo (s): " << ga_params.time_limit_sec << endl;
            cout << "Prob. mutación: " << ga_params.mutation_prob << endl;
            cout << "Prob. cruce: " << ga_params.crossover_prob << endl;
            cout << "Tamaño torneo: " << ga_params.tournament_size << "\n";
            cout << "Generaciones máx.: " << ga_params.max_generations << "\n";

            auto t0 = chrono::high_resolution_clock::now();
            auto soluciones = nsga2(F, U, G, k, ga_params);
            auto t1 = chrono::high_resolution_clock::now();
            auto dur_ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

            cout <<"Tiempo_ejecucion_ms: " << dur_ms << "\n\n";
            print_pareto_front(soluciones);
            resultados.push_back({"Genetico_NSGA-II", individuos_a_solmos(soluciones), dur_ms});
        }
    }
    // Modo no test: generar instancia de ground truth y ejecutar NSGA-II y Greedy
    if (!modo_test){
        // Generar instancia de ground truth
        auto gt = make_groundtruth(U, U_size, F_n_min, F_n_max, Fi_size_min, Fi_size_max, k, seed);

        cout << "=== INSTANCIA ===\n";
        cout << "Semilla: " << gt.seed << "\n";
        cout << "U_size: " << U_size << "\n";
        cout << "k: " << k << "\n";
        cout << "Expresion de referencia: " << gt.gold_expr.expr_str << "\n";
        cout << "Jaccard_objetivo: " << M(gt.gold_expr, gt.G, Metric::Jaccard) << "\n";
        cout << "\n";
        
        // Datos para reproducibilidad (para el script)
        cout << "@@@REPRO_DATA_START@@@\n";
        print_conjuntos(gt.G, gt.F); 
        cout << "@@@REPRO_DATA_END@@@\n";

        // NSGA-II
        GAParams ga_params;
        ga_params.population_size   = pop_size;
        ga_params.max_generations   = max_generations;
        ga_params.time_limit_sec    = time_limit;
        ga_params.crossover_prob    = crossover_prob;
        ga_params.mutation_prob     = mutation_prob;
        ga_params.tournament_size   = tournament_size;
        ga_params.seed              = seed_expr;

        auto t0 = chrono::steady_clock::now();
        auto pareto = nsga2(gt.F, U, gt.G, k, ga_params);
        auto t1 = chrono::steady_clock::now();
        auto dur_ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

        cout << "=== GENÉTICO (NSGA-II) ===\n";
        cout << "Tiempo (ms): " << dur_ms << "\n";
        cout << "Población: " << ga_params.population_size
                << " | Limite_tiempo_s: " << ga_params.time_limit_sec
                << " | p_mut: " << ga_params.mutation_prob
                << " | p_cruce: " << ga_params.crossover_prob
                << " | torneo: " << ga_params.tournament_size << "\n"; 

        print_pareto_front(pareto);
        bool hit = any_of(pareto.begin(), pareto.end(), [](const auto& s) {
            return s.jaccard == 1.0; });
            
        // Alcanzó el objetivo (Jaccard = 1.0)?
        cout << "HIT_OBJETIVO: " << (hit ? "SI" : "NO") << "\n\n\n";
        
        // GREEDY
        cout << "=== GREEDY ===\n";
        auto t00 = chrono::high_resolution_clock::now();
        auto soluciones = greedy_multiobjective_search(gt.F, U, gt.G, k);
        auto t11 = chrono::high_resolution_clock::now();
        dur_ms = chrono::duration_cast<chrono::milliseconds>(t11 - t00).count();

        cout <<"Tiempo_ejecucion_ms: " << dur_ms << "\n";
        print_pareto_front(soluciones);
    }
    return 0;

}