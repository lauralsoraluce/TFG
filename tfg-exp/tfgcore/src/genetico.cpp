//----------------------------------------------------------------------
// genetico.Cpp
//----------------------------------------------------------------------
// Algoritmo Genético NSGA-II con límites por generación y tiempo.
//----------------------------------------------------------------------

#include "genetico.hpp"
#include "metrics.hpp"

#include<iomanip>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

//------------------------------------------------------------------
// Construcción aleatoria de expresiones (individuos)
//------------------------------------------------------------------
Expression build_random_expr(const vector<int>& conjs, const vector<Bitset>& F, const Bitset& U, int k, mt19937& rng){
    // Pool de nodos (expresiones parciales)
    struct Node{ Expression e; };
    vector<Node> pool; 
    pool.reserve(conjs.size());


    // Inicializar pool con conjuntos base
    for (int idx : conjs) {
        if (idx == -1){
            set<int> u_set = {};
            pool.push_back({Expression(U, "U", u_set, 0)});
        }
        else if (idx>=0){
            set<int> f_set = {idx};
            pool.push_back({Expression(F[idx], "F"+to_string(idx),f_set,0)});
        }
    }

    // Construir expresión combinando nodos aleatoriamente
    if (pool.empty()) return Expression(Bitset(), "∅", {}, 0);
    if (pool.size()==1) return pool.front().e;

    int intentos_fallidos = 0;
    const int max_intentos = 100;

    // Mientras haya más de un nodo en el pool, combinar dos aleatoriamente
    while (pool.size() > 1 && intentos_fallidos < max_intentos) {
        uniform_int_distribution<> pick(0, (int)pool.size()-1);
        int a = pick(rng), b = pick(rng); 

        
        if (a != b) {
            // Asegurar que a < b para eliminar sin invalidar
            if (a>b) swap(a,b);

            int op = uniform_int_distribution<>(0,2)(rng); 
            const char* op_str = (op==0) ?" ∪ ":(op==1)?" ∩ ":" \\ ";

            Bitset H;
            if (op==0) H = set_union (pool[a].e.conjunto, pool[b].e.conjunto);
            if (op==1) H = set_intersect(pool[a].e.conjunto, pool[b].e.conjunto);
            if (op==2) H = set_difference(pool[a].e.conjunto, pool[b].e.conjunto);

            // Verificar límite de operaciones
            int ops_new = pool[a].e.n_ops + pool[b].e.n_ops + 1;
            if (ops_new <= k) {
                // Construir nuevo nodo
                set<int> usados = pool[a].e.used_sets;
                usados.insert(pool[b].e.used_sets.begin(), pool[b].e.used_sets.end());
                string expr_str = "(" + pool[a].e.expr_str + op_str + pool[b].e.expr_str + ")";

                // Reemplazar a con el nuevo nodo y eliminar b
                pool[a].e = Expression(H, expr_str, usados, ops_new);
                pool.erase(pool.begin()+b);
                intentos_fallidos = 0; 
            } else {
                // Falló la condición ops_new > k
                intentos_fallidos++;
            }
        } else {
            // Falló la condición a == b
            intentos_fallidos++;
        }
    }
    // Devolver la única expresión restante
    return pool.front().e;
}

//------------------------------------------------------------------
// NSGA-II
//------------------------------------------------------------------
vector<Individuo> nsga2(
    const vector<Bitset>& F,
    const Bitset& U,
    const Bitset& G,
    int k,
    const GAParams& params)
{
    // Semilla: si es 0, usar tiempo actual (no determinista)
    uint64_t seed = params.seed 
        ? params.seed 
        : (uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count();
    mt19937 rng(seed);

    // Tiempo de inicio y límite
    auto start_time = chrono::steady_clock::now();
    auto time_limit = chrono::seconds(params.time_limit_sec);

    // Bloques base para mutación tipo 1
    vector<SolMO> bloques_base;
    for (size_t i = 0; i < F.size(); i++) {
        set<int> s = {(int)i}; 
        Expression e(F[i], "F"+to_string(i), s, 0);
        bloques_base.emplace_back(e, 0, M(e,G,Metric::SizeH), M(e,G,Metric::Jaccard));
    }
    set<int> s_u = {-1}; 
    Expression e_u(U, "U", s_u, 0);
    bloques_base.emplace_back(e_u, 0, M(e_u,G,Metric::SizeH), M(e_u,G,Metric::Jaccard));

    // Inicializar población
    vector<Individuo> poblacion = inicializar_poblacion(F, U, G, k, params.population_size, rng);

    int generation = 0;
    // Bucle principal
    while (generation < params.max_generations && 
           chrono::steady_clock::now() - start_time < time_limit) {
        
        
        vector<Individuo> offspring;
        offspring.reserve(params.population_size);
        
        // Generar descendencia
        unordered_set<string> seen_gen;
        seen_gen.reserve(params.population_size * 2);
        for (const auto& ind : poblacion) seen_gen.insert(ind.expr.expr_str);

        while ((int)offspring.size() < params.population_size) {
            // Selección por torneo de los dos padres
            Individuo p1 = torneo_seleccion(poblacion, params.tournament_size, rng);
            Individuo p2 = torneo_seleccion(poblacion, params.tournament_size, rng);


            Individuo hijo;
            // Cruzar
            if (uniform_real_distribution<>(0,1)(rng) < params.crossover_prob) {
                hijo = crossover(p1, p2, F, U, G, k, rng);
            } else {
                hijo = (uniform_int_distribution<>(0,1)(rng) == 0) ? p1 : p2;
            }
            // Mutar
            if (uniform_real_distribution<>(0,1)(rng) < params.mutation_prob) {
                mutar(hijo, F, U, G, k, rng, bloques_base);
            }
            // Añadir hijo si no hemos visto ya esa expresión
            if (seen_gen.insert(hijo.expr.expr_str).second) {
                offspring.push_back(move(hijo));
            }
        }

        // Combinar
        vector<Individuo> R = poblacion;
        R.insert(R.end(), offspring.begin(), offspring.end());

        // NSGA-II: frentes + crowding
        auto Flist = fast_non_dominated_sort(R);

        // Nueva población
        vector<Individuo> Pnext;
        Pnext.reserve(params.population_size);
        
        // Llenar Pnext con los frentes hasta completar el tamaño
        for (size_t i = 0; i < Flist.size(); ++i) {
            auto& Fi = Flist[i];

            // Añadir todo el frente Fi si cabe 
            if (Pnext.size() < (size_t)params.population_size && !Fi.empty()) {
                
                // Calcular crowding distance del frente Fi
                calcular_crowding_distance(Fi);
                
                // Capacidad restante en Pnext
                size_t remaining_capacity = params.population_size - Pnext.size();
                
                // Si el frente Fi cabe entero
                if (Fi.size() <= remaining_capacity) {
                    Pnext.insert(Pnext.end(), Fi.begin(), Fi.end());
                } 
                // Si el frente Fi no cabe entero, cortar
                else {
                    // Ordenar por crowding distance descendente
                    sort(Fi.begin(), Fi.end(),
                        [](const Individuo& a, const Individuo& b) {
                            if (isinf(a.crowd) && !isinf(b.crowd)) return true;
                            if (!isinf(a.crowd) && isinf(b.crowd)) return false;
                            return a.crowd > b.crowd;
                        });
                    
                    // Insertar solo el número necesario hasta llenar Pnext
                    Pnext.insert(Pnext.end(), Fi.begin(), Fi.begin() + remaining_capacity);
                }
            }
        }

        // Avanzar a la siguiente generación
        poblacion = move(Pnext);
        generation++;
    }
    // Devolver el frente de Pareto final
    return pareto_front(poblacion);
}

//------------------------------------------------------------------
// Fast Non-Dominated Sort
//------------------------------------------------------------------
vector<vector<Individuo>> fast_non_dominated_sort(vector<Individuo>& poblacion) {
    int n = (int)poblacion.size();
    vector<int> domination_count(n, 0);
    vector<vector<int>> dominated_set(n); 
    vector<vector<int>> frentes_idx;      

    // Construir los conjuntos dominados y contar dominaciones
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (dominates(poblacion[i], poblacion[j])) {
                dominated_set[i].push_back(j); // i domina al índice j
                domination_count[j]++;
            } else if (dominates(poblacion[j], poblacion[i])) {
                dominated_set[j].push_back(i); // j domina al índice i
                domination_count[i]++;
            }
        }
    }

    // Identificar el primer frente (rank 0)
    vector<int> frente0_idx;
    // Índices de individuos con domination_count == 0
    for (int i = 0; i < n; i++) {
        if (domination_count[i] == 0) {
            poblacion[i].rank = 0;
            frente0_idx.push_back(i); // Añadir el índice i
        }
    }
    // Añadir el primer frente si no está vacío
    if (!frente0_idx.empty()) frentes_idx.push_back(frente0_idx);

    // Construir los siguientes frentes
    int rank = 0;
    while (rank < (int)frentes_idx.size() && !frentes_idx[rank].empty()) {
        // Índices del siguiente frente
        vector<int> siguiente_idx; 
        // Para cada individuo en el frente actual
        for (int idx : frentes_idx[rank]) { 
            // 
            for (int j_idx : dominated_set[idx]) { 
                
                domination_count[j_idx]--;
                // Si ya no es dominado por nadie, pertenece al siguiente frente
                if (domination_count[j_idx] == 0) {
                    poblacion[j_idx].rank = rank + 1;
                    siguiente_idx.push_back(j_idx);
                }
            }
        }
        
        // Añadir el siguiente frente si no está vacío
        if (!siguiente_idx.empty()) {
             frentes_idx.push_back(siguiente_idx);
        }
        // Avanzar al siguiente rango
        rank++; 
    }

    // Convertir índices a individuos
    vector<vector<Individuo>> frentes;
    frentes.reserve(frentes_idx.size());
    for (const auto& frente_i : frentes_idx) {
        vector<Individuo> frente_ind;
        frente_ind.reserve(frente_i.size());
        for (int idx : frente_i) {
            frente_ind.push_back(poblacion[idx]);
        }
        frentes.push_back(frente_ind);
    }
    
    // Devolver los frentes
    return frentes;
}

//------------------------------------------------------------------
// Crowding Distance
//------------------------------------------------------------------
void calcular_crowding_distance(vector<Individuo>& frente) {
    // Inicializar distancias
    int n = (int)frente.size();
    if (n == 0) return;

    // Si solo hay un individuo, su crowding es infinito
    if (n == 1) {
        frente[0].crowd = INFINITY;
        return;
    }
    for (auto& ind : frente) ind.crowd = 0.0;

    // Jaccard (maximizar)
    sort(frente.begin(), frente.end(), [](const Individuo& a, const Individuo& b) {
        return a.jaccard > b.jaccard;
    });
    // Los extremos tienen crowding infinito
    frente[0].crowd = INFINITY;
    frente[n-1].crowd = INFINITY;

    double r0 = max(1e-12, frente[0].jaccard - frente[n-1].jaccard);
    // Calcular crowding para los del medio
    for (int i = 1; i < n - 1; i++) {
        frente[i].crowd += (frente[i - 1].jaccard - frente[i + 1].jaccard) / r0;
    }

    // SizeH (minimizar)
    sort(frente.begin(), frente.end(), [](const Individuo& a, const Individuo& b) {
        return a.sizeH < b.sizeH;
    });
    // Los extremos tienen crowding infinito
    frente[0].crowd = INFINITY;
    frente[n-1].crowd = INFINITY;
    double r1 = max(1e-12, (double)(frente[n-1].sizeH - frente[0].sizeH));
    // Calcular crowding para los del medio
    for (int i = 1; i < n - 1; i++) {
        frente[i].crowd += (double)(frente[i + 1].sizeH - frente[i - 1].sizeH) / r1;
    }

    // n_ops (minimizar)
    sort(frente.begin(), frente.end(), [](const Individuo& a, const Individuo& b) {
        return a.n_ops < b.n_ops;
    });
    // Los extremos tienen crowding infinito
    frente[0].crowd = INFINITY;
    frente[n-1].crowd = INFINITY;
    double r2 = max(1e-12, (double)(frente[n-1].n_ops - frente[0].n_ops));
    // Calcular crowding para los del medio
    for (int i = 1; i < n - 1; i++) {
        frente[i].crowd += (double)(frente[i + 1].n_ops - frente[i - 1].n_ops) / r2;
    }
}

//------------------------------------------------------------------
// Operadores Genéticos
//------------------------------------------------------------------
// Selección por torneo
Individuo torneo_seleccion(const vector<Individuo>& poblacion, int tournament_size, mt19937& rng) {
    uniform_int_distribution<> dist(0, (int)poblacion.size() - 1);
    
    // Seleccionar el mejor entre 'tournament_size' individuos aleatorios
    Individuo mejor = poblacion[dist(rng)];
    
    // Comparar con los demás candidatos
    for (int i = 1; i < tournament_size; i++) {
        Individuo cand = poblacion[dist(rng)];
        // Elegir el que tenga mejor rank o, en caso de empate, mayor crowding
        if (cand.rank < mejor.rank || (cand.rank == mejor.rank && cand.crowd > mejor.crowd)) {
            mejor = cand;
        }
    }
    return mejor;
}

// Cruce
Individuo crossover(const Individuo& p1, const Individuo& p2,
                    const vector<Bitset>& F, const Bitset& U, const Bitset& G,
                    int k, mt19937& rng) 
{
    const Individuo* left_parent;
    const Individuo* right_parent;
    // Elegir aleatoriamente qué padre va a la izquierda y cuál a la derecha
    if (uniform_int_distribution<>(0,1)(rng)==0){
        left_parent = &p1;
        right_parent = &p2;
    }
    else {
        left_parent = &p2;
        right_parent = &p1;
    }

    // Comprobar límite de operaciones
    int new_ops = left_parent->n_ops+right_parent->n_ops+1;
    // Si se supera el límite, devolver el mejor padre según Jaccard
    if(new_ops>k){
        bool p1_is_better = (p1.rank < p2.rank) || 
                            (p1.rank == p2.rank && p1.crowd > p2.crowd);
        
        return p1_is_better ? p1 : p2;
    }

    // Elegir operación aleatoriamente
    int op = uniform_int_distribution<>(0,2)(rng);
    string op_str = (op == 0) ? " ∪ " : (op == 1) ? " ∩ " : " \\ ";

    // Aplicar la operación
    Bitset new_set;
    if (op == 0) new_set = set_union(left_parent->expr.conjunto, right_parent->expr.conjunto);
    else if (op == 1) new_set = set_intersect(left_parent->expr.conjunto, right_parent->expr.conjunto);
    else new_set = set_difference(left_parent->expr.conjunto, right_parent->expr.conjunto);

    // Construir la nueva expresión
    string new_expr = "(" + left_parent->expr.expr_str + op_str + right_parent->expr.expr_str + ")";
    // Combinar los conjuntos usados
    set<int> combined_sets = left_parent->expr.used_sets;
    combined_sets.insert(right_parent->expr.used_sets.begin(), right_parent->expr.used_sets.end());

    // Crear el nuevo individuo
    Expression e_hijo(new_set, new_expr, combined_sets, new_ops);
    
    // Evaluar y devolver
    double j = M(e_hijo, G, Metric::Jaccard);
    int sizeH = M(e_hijo, G, Metric::SizeH); 
    
    return Individuo(e_hijo, new_ops, sizeH, j);
}

// Mutación
void mutar(Individuo& ind, const vector<Bitset>& F, const Bitset& U, const Bitset& G, int k, mt19937& rng, const vector<SolMO>& bloques_base)
{
    // Damos un 80% de probabilidad a mutación de crecimiento
    // y un 20% a mutación destructiva
    std::uniform_real_distribution<double> dist_tipo(0.0, 1.0);
    int tipo = (dist_tipo(rng) < 0.80) ? 1 : 0; 

    if (tipo == 0) {
        // Mutación Destructiva

        vector<int> conjs(ind.expr.used_sets.begin(), ind.expr.used_sets.end());
        uniform_int_distribution<> dist_idx(-1, (int)F.size() - 1);

        if (conjs.empty()) {
            conjs.push_back(dist_idx(rng));
        } else {
            // Añadir o reemplazar un gen aleatoriamente
            if (uniform_int_distribution<>(0,1)(rng) == 0 && !conjs.empty()) {
                // Reemplazar
                int pos = uniform_int_distribution<>(0,(int)conjs.size()-1)(rng);
                conjs[pos] = dist_idx(rng);
            } else {
                // Añadir
                conjs.push_back(dist_idx(rng));
            }
        }
        // Reconstruir expresión aleatoria
        shuffle(conjs.begin(), conjs.end(), rng);
        ind.expr = build_random_expr(conjs, F, U, k, rng);

    } else {
        // Mutación de crecimiento
 
        int new_ops = ind.n_ops + 1; 
        // Comprobar límite de operaciones
        if (new_ops > k) {
            return; 
        }
        // Elegir operación y bloque base aleatoriamente
        std::uniform_int_distribution<int> dist_op(0, 2);
        std::uniform_int_distribution<int> dist_base(0, bloques_base.size() - 1);
        
        // Realizar la operación
        int op = dist_op(rng);
        const SolMO& right = bloques_base[dist_base(rng)];
        string op_str = (op == 0) ? " ∪ " : (op == 1) ? " ∩ " : " \\ ";

        Bitset new_set;
        string new_expr;
        
        // Decidir el orden de los operandos aleatoriamente
        if (uniform_int_distribution<>(0,1)(rng) == 0) {
            // (Individuo op BloqueBase)
            if (op == 0) new_set = set_union(ind.expr.conjunto, right.expr.conjunto);
            else if (op == 1) new_set = set_intersect(ind.expr.conjunto, right.expr.conjunto);
            else new_set = set_difference(ind.expr.conjunto, right.expr.conjunto);
            new_expr = "(" + ind.expr.expr_str + op_str + right.expr.expr_str + ")";
        } else {
            // (BloqueBase op Individuo)
            if (op == 0) new_set = set_union(right.expr.conjunto, ind.expr.conjunto);
            else if (op == 1) new_set = set_intersect(right.expr.conjunto, ind.expr.conjunto);
            else new_set = set_difference(right.expr.conjunto, ind.expr.conjunto);
            new_expr = "(" + right.expr.expr_str + op_str + ind.expr.expr_str + ")";
        }

        // Combinar los conjuntos usados
        set<int> combined_sets = ind.expr.used_sets;
        combined_sets.insert(right.expr.used_sets.begin(), right.expr.used_sets.end());
        // Actualizar la expresión del individuo
        ind.expr = Expression(new_set, new_expr, combined_sets, new_ops);
    }
    
    // Recalcular métricas
    ind.jaccard = M(ind.expr, G, Metric::Jaccard);
    ind.sizeH   = (int)M(ind.expr, G, Metric::SizeH);
    ind.n_ops   = ind.expr.n_ops;
}

//------------------------------------------------------------------
// Inicialización aleatoria
//------------------------------------------------------------------
vector<Individuo> inicializar_poblacion(const vector<Bitset>& F, const Bitset& U,
                                        const Bitset& G, int k, int pop_size, mt19937& rng) {
    vector<Individuo> pop;
    pop.reserve(pop_size);

    unordered_set<string> vistos_expr;
    vistos_expr.reserve(pop_size * 2);

    // Construir individuos aleatorios hasta completar la población
    while ((int)pop.size() < pop_size) {
        // Elegir número aleatorio de conjuntos base a usar
        int max_sets = min<int>((int)F.size()+1, k+1);
        int num_conjs = uniform_int_distribution<>(1, max_sets)(rng);

        set<int> usados;

        uniform_int_distribution<> dist_idx(-1, (int)F.size()-1);
        for (int i = 0; i < num_conjs; ++i)
            usados.insert(dist_idx(rng));

        vector<int> conjs(usados.begin(), usados.end());
        
        // Solo continuar si hay conjuntos seleccionados
        if (!conjs.empty()) { 
            // Construir expresión aleatoria
            Expression e = build_random_expr(conjs, F, U, k, rng);
            auto keyE = e.expr_str;
            
            // Solo continuamos si la expresión no ha sido vista antes
            if (vistos_expr.insert(keyE).second) { 
                // Construir y añadir el individuo
                Individuo ind;
                ind.expr    = move(e);
                ind.jaccard = M(ind.expr, G, Metric::Jaccard);
                ind.sizeH   = (int)M(ind.expr, G, Metric::SizeH);
                ind.n_ops   = ind.expr.n_ops;
                ind.rank    = 0;
                ind.crowd   = 0.0;

                pop.push_back(move(ind));
            }
        }
    }

    // Devolver población inicial
    return pop;
}