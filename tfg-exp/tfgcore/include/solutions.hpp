//----------------------------------------------------------------------
// solutions.hpp
//----------------------------------------------------------------------
// Utilidades comunes para soluciones multiobjetivo
//----------------------------------------------------------------------
#pragma once

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>

#include "expr.hpp"

//------------------------------------------------------------------
// Solución multiobjetivo
//------------------------------------------------------------------
struct SolMO {
    Expression expr;        // Expresión asociada
    int n_ops = 0;          // Número de operaciones
    int sizeH = 0;          // número de conjuntos distintos usados
    double jaccard = 0.0;   // Coeficiente de Jaccard

    // Constructores
    SolMO() = default;
    SolMO(const Expression& e, int ops, int size, double j)
        : expr(e), n_ops(ops), sizeH(size), jaccard(j) {}
};

//------------------------------------------------------------------
// Individuo (NSGA-II)
//------------------------------------------------------------------
struct Individuo : public SolMO{
    int rank = 0;       // Rango en el frente de Pareto
    double crowd = 0.0; // Distancia de aglomeración
    Individuo() = default;
    Individuo(const Expression& e, int ops, int size, double j)
        : SolMO(e, ops, size, j), rank(0), crowd(0.0) {}
    Individuo(const SolMO& s)
        : SolMO(s), rank(0), crowd(0.0) {}
};

//------------------------------------------------------------------
// Dominancia: max Jaccard, min n_ops, min |H|
//------------------------------------------------------------------
inline bool dominates(const SolMO& a, const SolMO& b) {
    bool ge = (a.jaccard >= b.jaccard) && (a.n_ops <= b.n_ops) && (a.sizeH <= b.sizeH);
    bool gt = (a.jaccard >  b.jaccard) || (a.n_ops <  b.n_ops) || (a.sizeH <  b.sizeH);
    return ge && gt;
}

//------------------------------------------------------------------
// Frente de Pareto genérico (SolMO / Individuo)
//------------------------------------------------------------------
template<typename T>
std::vector<T> pareto_front_generic(const std::vector<T>& v) {
    // Caso trivial
    if (v.empty()) return {};
    
    std::vector<T> nd; // El frente no dominado
    nd.reserve(v.size() / 10 + 1); 

    // Iterar sobre todas las soluciones
    for (const auto& sol : v) {
        bool sol_is_dominated = false;
        
        // Comprobar si 'sol' es dominada por alguna del frente actual
        int i = 0;
        while (i < nd.size()) { // Bucle interno: O(M)
            
            const auto& other = nd[i];

            // Comprobar dominancia entre 'sol' y 'other'
            if (dominates(other, sol)) {
                sol_is_dominated = true;
                break;
            }
            // 'other' no domina a 'sol', comprobar si 'sol' domina a 'other'
            if (dominates(sol, other)) {
                nd[i] = nd.back();
                nd.pop_back();
                
            } 
            // 'Ninguna dominancia', seguir con la siguiente
            else {
                i++;
            }
        }

        // Si 'sol' no fue dominado por nadie del frente actual,
        // entonces 'sol' pertenece al frente.
        if (!sol_is_dominated) {
            nd.push_back(sol);
        }
    }
    
    // Ordenar el frente de Pareto resultante
    std::stable_sort(nd.begin(), nd.end(), [](const T& a, const T& b) {
        if (a.jaccard != b.jaccard) return a.jaccard > b.jaccard; // descendente
        if (a.sizeH != b.sizeH) return a.sizeH < b.sizeH; // ascendente
        return a.n_ops < b.n_ops; // ascendente
    });
    
    return nd;
}

// ------------------------------------------------------------------
// Frente de Pareto específico (SolMO)
// ------------------------------------------------------------------
inline std::vector<SolMO> pareto_front(const std::vector<SolMO>& v) {
    return pareto_front_generic(v);
}
// ------------------------------------------------------------------
// Frente de Pareto específico (Individuo)
// ------------------------------------------------------------------
inline std::vector<Individuo> pareto_front(const std::vector<Individuo>& v) {
    return pareto_front_generic(v);
}

//------------------------------------------------------------------
// Impresión sencilla del frente de Pareto (general)
//------------------------------------------------------------------
template<typename T>
void print_pareto_front_generic(const vector<T>& pareto) {
    std::cout << "\n=== FRENTE DE PARETO ===" << std::endl;
    std::cout << "Total soluciones no dominadas: " << pareto.size() << "\n" << std::endl;
    
    for (size_t i = 0; i < pareto.size(); i++) {
        std::cout << "Solución " << (i+1) << ":" << std::endl;
        std::cout << "  Expresión: " << pareto[i].expr.expr_str << std::endl;
        std::cout << "  Jaccard: " << pareto[i].jaccard << std::endl;
        std::cout << "  Operaciones: " << pareto[i].n_ops << std::endl;
        std::cout << "  |H|: " << pareto[i].sizeH << std::endl;
        std::cout << std::endl;
    }
}

// ------------------------------------------------------------------
// Impresión sencilla del frente de Pareto (SolMO)
// ------------------------------------------------------------------
inline void print_pareto_front(const std::vector<SolMO>& pareto) {
    print_pareto_front_generic(pareto);
}

// ------------------------------------------------------------------
// Impresión sencilla del frente de Pareto (Individuo)
// ------------------------------------------------------------------
inline void print_pareto_front(const std::vector<Individuo>& pareto) {
    print_pareto_front_generic(pareto);
}