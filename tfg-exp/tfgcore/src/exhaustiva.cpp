//======================================================================
// exhaustiva.cpp
//----------------------------------------------------------------------
// Generación exhaustiva de expresiones hasta profundidad k y evaluación
// frente a un objetivo G.
//======================================================================

#include <unordered_set>
#include <set>
#include <string>
#include <vector>

#include "metrics.hpp"
#include "exhaustiva.hpp"
#include "solutions.hpp"

using namespace std;

/* BÚSQUEDA EXHAUSTIVA */
vector<vector<Expression>> exhaustive_search(
    const vector<Bitset>& F,
    const Bitset& U,
    int k)
{
    vector<vector<Expression>> expr(k + 1);
    
    // Nivel 0: conjuntos base + U
    expr[0].reserve(F.size() + 2);
    for (size_t i = 0; i < F.size(); i++) {
        set<int> sets = {static_cast<int>(i)};
        expr[0].emplace_back(F[i], "F" + to_string(i), sets, 0);
    }
    set<int> u_set = {-1};
    expr[0].emplace_back(U, "U", u_set, 0);

    // Para evitar duplicados de conjuntos generados
    /*unordered_set<string> seen_sets;
    for (const auto& e : expr[0]) {
        seen_sets.insert(e.conjunto.to_string());
    }*/
    
    // Generar expresiones con s operaciones (1...k)
    for (int s = 1; s <= k; s++) {
        for (int op = 0; op < 3; op++) {
            string op_str = (op == 0) ? " ∪ " : (op == 1) ? " ∩ " : " \\ ";
            
            for (int a = 0; a < s; a++) {
                int b = s - a - 1;

                for (const auto& left : expr[a]) {
                    for (const auto& right : expr[b]) {
                        
                        // Aplicamos operación
                        Bitset new_set;
                        if (op == 0) {
                            new_set = set_union(left.conjunto, 
                                               right.conjunto);
                        } else if (op == 1) {
                            new_set = set_intersect(left.conjunto, 
                                               right.conjunto);
                        } else {
                            new_set = set_difference(left.conjunto, 
                                               right.conjunto);
                        }
                        
                        // Evitamos duplicados
                        string set_str = new_set.to_string();
                        /*if (seen_sets.count(set_str)) continue;
                        seen_sets.insert(set_str);*/
                        
                        // Construimos expresión
                        string new_expr = "(" + left.expr_str + op_str + 
                                         right.expr_str + ")";
                        
                        set<int> combined_sets = left.used_sets;
                        combined_sets.insert(right.used_sets.begin(), right.used_sets.end());
                        
                        expr[s].emplace_back(new_set, new_expr, combined_sets, s);
                    }
                }
            }
        }
    }
    
    // Añadimos conjunto vacío a expr[0]
    set<int> empty_sets;
    expr[0].emplace_back(Bitset(), "∅", empty_sets, 0);
    
    return expr;
}

//------------------------------------------------------------------
// EVALUACIÓN CON RESPECTO A G Y FILTRADO PARETO
//------------------------------------------------------------------
vector<SolMO> evaluar_subconjuntos(
    const vector<vector<Expression>>& expr,
    const Bitset& G,
    int k)
{
    vector<SolMO> soluciones;

    for (int s=0; s<=k; s++){
        for (const auto& e : expr[s]) {
            double j = M(e, G, Metric::Jaccard);
            int sizeH = M(e, G, Metric::SizeH);
            int n_ops = M(e, G, Metric::OpSize);

            soluciones.emplace_back(e, n_ops, sizeH, j);
        }
    }
    return pareto_front(soluciones);
}