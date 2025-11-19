//------------------------------------------------------------------
// domain.hpp
//------------------------------------------------------------------
// Define la representación de conjuntos (Bitset) y operaciones básicas.
// El tamaño del universo (U_size) es configurable antes de compilar.
//------------------------------------------------------------------

#pragma once
#include <bitset>
#include <string>
#include <stdexcept>

//------------------------------------------------------------------
// Tamaño del universo
//------------------------------------------------------------------
#ifndef U_SIZE
#define U_SIZE 128
#endif
constexpr int U_size = U_SIZE;

// Nombre para el conjunto sobre el universo U
using Bitset = std::bitset<U_size>;

//------------------------------
// Operaciones básicas
//------------------------------

// Unión de conjuntos: A ∪ B
inline Bitset set_union(const Bitset& A, const Bitset& B)      { return A | B; }
// Intersección de conjuntos: A ∩ B
inline Bitset set_intersect(const Bitset& A, const Bitset& B)  { return A & B; }
// Diferencia de conjuntos: A \ B
inline Bitset set_difference(const Bitset& A, const Bitset& B) { return A & (~B); }
// Aplica la operación indicada
inline Bitset apply_op(const int op, const Bitset& A, const Bitset& B) {
    if (op == 0) return set_union(A, B);
    if (op == 1) return set_intersect(A, B);
    if (op == 2) return set_difference(A, B);
    throw std::invalid_argument("Operación inválida");
}