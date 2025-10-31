//======================================================================
// greedy.cpp
//----------------------------------------------------------------------
// Heurística greedy para construir una expresión (≤ k ops) maximizando
// Jaccard y, a igualdad, minimizando |H| y nº de operaciones.
//======================================================================

#include "metrics.hpp"
#include "greedy.hpp"
#include "solutions.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

//------------------------------------------------------------------
// Greedy principal
//------------------------------------------------------------------
vector<SolMO> greedy(
    const vector<Bitset>& F,
    const Bitset& G,
    int k)
{
    Expression curr(Bitset(), "∅", set<int>{}, 0);
    vector<SolMO> candidatos;

    unordered_set<string> seen;
    auto try_push = [&](const Expression& e) {
        string key = e.conjunto.to_string();
        if (seen.insert(key).second) {
            double j     = M(e, G, Metric::Jaccard);
            int    sizeH = (int)M(e, G, Metric::SizeH);
            int    n_ops = (int)M(e, G, Metric::OpSize);
            candidatos.emplace_back(e, n_ops, sizeH, j);   
        }
    };

    // Punto de partida: vacío
    try_push(curr);

    for (int iter = 0; iter < k; ++iter) {
        bool found = false;
        double best_j = -1.0; 
        int best_sz = 1e9; 
        int best_ops = 1e9;
        Expression bestE;

        for (int op = 0; op < 3; ++op) {
            for (int i = 0; i <= (int)F.size(); ++i) {
                bool isU = (i==(int)F.size());

                Bitset rhs;
                string rhs_name;
                if (isU) {
                    rhs.set();      // U
                    rhs_name = "U";
                }
                else {
                    rhs = F[i];     // F_i
                    rhs_name = "F" + std::to_string(i);
                }

                Bitset H_new = apply_op(op, curr.conjunto, rhs);

                set<int> sets_new = curr.used_sets; 
                if (!isU) sets_new.insert(i);

                int ops_new = (curr.expr_str == "∅") ? 0 : (curr.n_ops + 1);

                const char* opstr = (op == 0) ? " ∪ " : (op == 1) ? " ∩ " : " \\ ";
                string expr_new = (curr.expr_str == "∅")
                    ? rhs_name
                    : ("(" + curr.expr_str + opstr + rhs_name + ")");

                Expression E_new(H_new, std::move(expr_new), sets_new, ops_new);
                try_push(E_new);

                double j  = M(E_new, G, Metric::Jaccard);
                int sH    = (int)M(E_new, G, Metric::SizeH);
                int nops  = (int)M(E_new, G, Metric::OpSize);

                if (j > best_j || (j == best_j && (sH < best_sz || (sH == best_sz && nops < best_ops)))) {
                    best_j = j; best_sz = sH; best_ops = nops; bestE = std::move(E_new); found = true;
                }
            }
        }
        if (!found) break;

        curr = move(bestE);
        try_push(curr);
    }
    
    // Devuelve el frente de Pareto
    return pareto_front(candidatos);
}