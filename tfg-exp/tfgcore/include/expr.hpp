//----------------------------------------------------------------------
// expr.hpp
//----------------------------------------------------------------------
// Define la estructura básica Expression utilizada en todo el proyecto.
//----------------------------------------------------------------------

#pragma once

#include <string>
#include <set>
#include "domain.hpp"

using namespace std;

//------------------------------------------------------------------
// Estructura de expresión
//------------------------------------------------------------------
struct Expression {
    Bitset conjunto;            // Resultado de evaluar la expresión
    string expr_str;            // Representación de la expresión
    set<int> used_sets;         // Índices de conjuntos base usados
    int n_ops = 0;              // Número de operaciones

    // Constructores
    Expression() = default;
    Expression(Bitset c, std::string s)
        : conjunto(std::move(c)), expr_str(std::move(s)) {}
    Expression(const Bitset& c, const std::string& s, const std::set<int>& sets, int ops=0)
        : conjunto(c), expr_str(s), used_sets(sets), n_ops(ops) {}
};
