#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <chrono>

using namespace std;

// Estructura para guardar conjunto + expresión que lo generó
struct Expression {
    set<int> conjunto;      // El conjunto resultante
    string expr_str;        // La expresión como string (para mostrar)
    
    Expression(set<int> c, string s) : conjunto(c), expr_str(s) {}
};

// Operaciones de conjuntos
set<int> set_union(const set<int>& A, const set<int>& B) {
    set<int> result;
    set_union(A.begin(), A.end(), B.begin(), B.end(), 
              inserter(result, result.begin()));
    return result;
}

set<int> set_intersect(const set<int>& A, const set<int>& B) {
    set<int> result;
    set_intersection(A.begin(), A.end(), B.begin(), B.end(), 
                     inserter(result, result.begin()));
    return result;
}

set<int> set_difference(const set<int>& A, const set<int>& B) {
    set<int> result;
    set_difference(A.begin(), A.end(), B.begin(), B.end(), 
                   inserter(result, result.begin()));
    return result;
}

double M(const Expression& e, const set<int>& G) {
    // Ejemplo: retorna el tamaño del conjunto
    return e.conjunto.size();
}

pair<Expression, set<int>> exhaustive_search(const vector<set<int>>& F, const set<int>& U, const set<int> G, int k) {

    vector<vector<Expression>> expr(k + 1);

    for (int i=0; i<F.size(); i++){
        expr[0].push_back(Expression(F[i], "F" + to_string(i)));
    }
    expr[0].push_back(Expression(U, "U")); // conjunto vacío

    double best_score = -99999;
    Expression e_best(set<int>(), "");
    
    // Generar expresiones con s operaciones
    for (int s=1; s<=k; s++) {
        // Para cada operación
        for (int op=0; op<3; op++){
            string op_str = (op==0) ? " ∪ " : (op==1) ? " ∩ " : " \\ ";
            // Para cada partición de s operaciones
            for (int a=0; a<s; a++){
                for (int i=0; i<expr[a].size(); i++){
                    for (int j=0; j<expr[s-a-1].size(); j++){
                        set<int> new_set;
                        if (op==0) {
                            new_set = set_union(expr[a][i].conjunto, expr[s-a-1][j].conjunto);
                        } else if (op==1) {
                            new_set = set_intersect(expr[a][i].conjunto, expr[s-a-1][j].conjunto);
                        } else if (op==2) {
                            new_set = set_difference(expr[a][i].conjunto, expr[s-a-1][j].conjunto);
                        }
                        string new_expr = "(" + expr[a][i].expr_str + op_str + expr[s-a-1][j].expr_str + ")";
                        expr[s].push_back(Expression(new_set, new_expr));
                    }
                }
            }
        }
    }
    expr[0].push_back(Expression(set<int>(), "∅"));

    for (int s=0; s<=k; s++) {
        for (int e=0; e<expr[s].size(); e++){
            double score = M(expr[s][e], G);

            if (score > best_score) {
                best_score = score;
                e_best = expr[s][e];
            }
        }
    }
    return make_pair(e_best, e_best.conjunto);
}

int main() {
    int k=2;  // Número máximo de operaciones

    // Universo
    set<int> U = {1,2,3,4,5,6};

    vector<set<int>> F = {
        {1,2,3},
        {2,3,4},
        {4,5,6}
    };

    set<int> G = {2,3,4,5};

    // Búsqueda exhaustiva
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto [e_best, G_best] = exhaustive_search(F, U, G, k);
    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    // Mostrar resultado
    cout << "\n=== RESULTADO ===" << endl;
    cout << "Mejor expresión: " << e_best.expr_str << endl;
    cout << "Conjunto resultante: { ";
    bool first = true;
    for (int x : G_best) {
        if (!first) cout << ", ";
        cout << x;
        first = false;
    }
    cout << " }" << endl;
    cout << "Tiempo de ejecución: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;

    return 0;
}
