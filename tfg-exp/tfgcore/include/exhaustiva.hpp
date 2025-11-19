//----------------------------------------------------------------------
// exhaustiva.hpp
//----------------------------------------------------------------------
// Implementa la búsqueda exhaustiva con profundidad limitada.
//----------------------------------------------------------------------

#ifndef EXHAUSTIVA_HPP
#define EXHAUSTIVA_HPP

#include <vector>
#include "expr.hpp"
#include "domain.hpp"
#include "solutions.hpp"

//------------------------------------------------------------------
/* Genera todas las expresiones posibles hasta profundidad k
    a partir de la familia F y el universo U. */
//------------------------------------------------------------------
std::vector<SolMO> exhaustive_search(
    const std::vector<Bitset>& F,
    const Bitset& U,
    const Bitset& G,
    int k);

//------------------------------------------------------------------
// EVALUACIÓN + FILTRADO PARETO (NO SE USA EN ESTA VERSIÓN)
//------------------------------------------------------------------
/* std::vector<SolMO> evaluar_subconjuntos(
    const std::vector<std::vector<Expression>>& expr,
    const Bitset& G,
    int k);
*/ 

#endif // EXHAUSTIVA_HPP