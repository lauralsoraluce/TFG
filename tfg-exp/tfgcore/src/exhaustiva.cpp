//----------------------------------------------------------------------
// exhaustiva.cpp
//----------------------------------------------------------------------
// Implementa la búsqueda exhaustiva con profundidad limitada.
//----------------------------------------------------------------------

#include <unordered_set>
#include <set>
#include <string>
#include <vector>

#include "metrics.hpp"
#include "exhaustiva.hpp"
#include "solutions.hpp"

using namespace std;

//------------------------------------------------------------------
/* Genera todas las expresiones posibles hasta profundidad k
    a partir de la familia F y el universo U. */
//------------------------------------------------------------------
std::vector<SolMO> exhaustive_search(
    const vector<Bitset>& F,
    const Bitset& U,
    const Bitset& G,
    int k)
{
    vector<vector<Expression>> expr(k + 1);
    vector<SolMO> soluciones;

    // Nivel 0: conjuntos base + conjunto universo
    expr[0].reserve(F.size() + 1);

    // Nivel 0: añadir conjunto universo + conjuntos base
    for (int i = -1; i < (int)F.size(); i++) {
        if (i==-1) {
            set<int> u_set ={};
            expr[0].emplace_back(U, "U", u_set, 0);
        }
        else {
            set<int> sets = {i};
            expr[0].emplace_back(F[i], "F" + to_string(i), sets, 0);
        }

        // Evaluar cada expresión de nivel 0
        const Expression& e = expr[0].back();
        double j = M(e, G, Metric::Jaccard);
        int sizeH = M(e, G, Metric::SizeH);
        int n_ops = M(e, G, Metric::OpSize);
        soluciones.emplace_back(e, n_ops, sizeH, j);
    }

    // Generar expresiones con s operaciones (1...k)
    for (int s = 1; s <= k; s++) {
        for (int op = 0; op < 3; op++) {
            string op_str = (op == 0) ? " ∪ " : (op == 1) ? " ∩ " : " \\ ";
            
            for (int a = 0; a < s; a++) {
                int b = s - a - 1;

                for (const auto& left : expr[a]) {
                    for (const auto& right : expr[b]) {
                        
                        // Aplicar operación
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
                        
                        // Crear nueva expresión
                        string new_expr = "(" + left.expr_str + op_str + 
                                         right.expr_str + ")";
                        
                        // Combinar conjuntos usados
                        set<int> combined_sets = left.used_sets;
                        combined_sets.insert(right.used_sets.begin(), right.used_sets.end());
                        
                        // Almacenar nueva expresión
                        expr[s].emplace_back(new_set, new_expr, combined_sets, s);

                        // Evaluar nueva expresión
                        const Expression& e_nueva = expr[s].back();
                        double j = M(e_nueva, G, Metric::Jaccard);
                        int sizeH = M(e_nueva, G, Metric::SizeH);
                        soluciones.emplace_back(e_nueva, s, sizeH, j);
                    }
                }
            }
        }
    }
    // Filtrar frente de Pareto
    return pareto_front(soluciones);
}

//------------------------------------------------------------------
// EVALUACIÓN + FILTRADO PARETO (NO SE USA EN ESTA VERSIÓN)
//------------------------------------------------------------------
/* vector<SolMO> evaluar_subconjuntos(
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
} */ 