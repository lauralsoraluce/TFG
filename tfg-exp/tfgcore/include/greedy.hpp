//------------------------------------------------------------------
// greedy.hpp
//----------------------------------------------------------------------
// Algoritmo greedy 
//------------------------------------------------------------------

#ifndef GREEDY_HPP
#define GREEDY_HPP

#include <vector>
#include "expr.hpp"
#include "domain.hpp"
#include "solutions.hpp"

// ------------------------------------------------------------------
// Búsqueda greedy multi-objetivo
// ------------------------------------------------------------------ 
std::vector<SolMO> greedy_multiobjective_search(
    const std::vector<Bitset>& F,
    const Bitset& U,
    const Bitset& G,
    int k);

#endif // GREEDY_HPP