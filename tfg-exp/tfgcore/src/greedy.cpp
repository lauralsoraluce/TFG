//----------------------------------------------------------------------
// greedy.cpp
//----------------------------------------------------------------------
// Algoritmo greedy 
//----------------------------------------------------------------------

#include "metrics.hpp"
#include "greedy.hpp"
#include "solutions.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

// ------------------------------------------------------------------
// Búsqueda greedy multi-objetivo
// ------------------------------------------------------------------
vector<SolMO> greedy_multiobjective_search(
    const vector<Bitset>& F,
    const Bitset& U,
    const Bitset& G,
    int k)
{
    // Frente global de soluciones
    vector<SolMO> frente_global;
    // Bloques base
    vector<SolMO> bloques_base;
    bloques_base.reserve(F.size()+1);

    // Generar los bloques base
    for (size_t i = 0; i < F.size(); i++) {
        set<int> sets = {static_cast<int>(i)};
        Expression e(F[i], "F" + to_string(i), sets, 0); 
        double j = M(e, G, Metric::Jaccard);
        int sizeH = M(e, G, Metric::SizeH);
        bloques_base.emplace_back(e, 0, sizeH, j);
    }
    set<int> u_set = {}; 
    Expression e_u(U, "U", u_set, 0);
    double j_u = M(e_u, G, Metric::Jaccard);
    int sizeH_u = M(e_u, G, Metric::SizeH);
    bloques_base.emplace_back(e_u, 0, sizeH_u, j_u);

    // Calcular frente de Pareto del nivel 0 (bloques base)
    vector<SolMO> frente_nivel_0 = pareto_front(bloques_base);
    frente_global = frente_nivel_0;

    // Construcción de soluciones de niveles superiores
    vector<SolMO> frente_para_construir = frente_nivel_0;

    int s=1; 
    // Mientras queden niveles por construir y no se haya alcanzado k operaciones
    while (s <= k && !frente_para_construir.empty()) {
        vector<SolMO> candidatos_s;
        
        // Generar todas las combinaciones de expresiones con s operaciones
        for (int op = 0; op < 3; op++) {
            string op_str = (op == 0) ? " ∪ " : (op == 1) ? " ∩ " : " \\ ";
            
            // Combinar cada expresión del frente actual con cada bloque base
            for (const SolMO& sol_left : frente_para_construir) {
                for (const SolMO& sol_right : bloques_base) {
                    
                    const Expression& left = sol_left.expr;
                    const Expression& right = sol_right.expr;

                    Bitset new_set;
                    if (op == 0) new_set = set_union(left.conjunto, right.conjunto);
                    else if (op == 1) new_set = set_intersect(left.conjunto, right.conjunto);
                    else new_set = set_difference(left.conjunto, right.conjunto);
                    
                    string new_expr = "(" + left.expr_str + op_str + 
                                     right.expr_str + ")";
                    
                    set<int> combined_sets = left.used_sets;
                    combined_sets.insert(right.used_sets.begin(), right.used_sets.end());
                    
                    Expression e_nueva(new_set, new_expr, combined_sets, s); 
                    
                    double j = M(e_nueva, G, Metric::Jaccard);
                    int sizeH = M(e_nueva, G, Metric::SizeH);
                    
                    candidatos_s.emplace_back(e_nueva, s, sizeH, j);
                }
            }
        }

        // Calcular el frente de Pareto local de las nuevas candidatas
        vector<SolMO> frente_local_s = pareto_front(candidatos_s);

        // Combinar el frente global con el local
        vector<SolMO> combined_front = frente_global;
        combined_front.insert(combined_front.end(), 
                              frente_local_s.begin(), frente_local_s.end());
        
        // Calcular el nuevo frente de Pareto global
        vector<SolMO> new_global_front = pareto_front(combined_front);

        // Filtrar las soluciones del nuevo frente global que tienen s operaciones
        vector<SolMO> frente_siguiente;
        for (const auto& sol : new_global_front) {
            if (sol.expr.n_ops == s) {
                frente_siguiente.push_back(sol);
            }
        }
        
        // Actualizar el frente global y el frente para construir
        frente_global = std::move(new_global_front);
        frente_para_construir = std::move(frente_siguiente);
        s++;
    }
    // Devolver el frente global final
    return frente_global;
}