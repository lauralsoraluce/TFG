//----------------------------------------------------------------------
// metrics.cpp
//----------------------------------------------------------------------
// Definición de métricas comunes usadas por los algoritmos
//----------------------------------------------------------------------

#include "metrics.hpp"

using namespace std;

//------------------------------------------------------------------
// Coeficiente de Jaccard: |H ∩ G| / |H ∪ G|
//------------------------------------------------------------------
static inline double jaccard_coefficient(const Bitset& H, const Bitset& G) {
    const int intersection = (H & G).count();
    const int union_size = (H | G).count();
    if (union_size == 0) return 1.0;  // Ambos vacíos
    return static_cast<double>(intersection) / union_size;
}

//------------------------------------------------------------------
// Función principal de métrica
//------------------------------------------------------------------
double M(const Expression& H, const Bitset& G, Metric metric) {
    // Se pueden agregar más métricas aquí
    switch (metric) {
        case Metric::Jaccard:
            return jaccard_coefficient(H.conjunto, G);
        case Metric::SizeH:
            return static_cast<int>(H.used_sets.size());
        case Metric::OpSize:
            return H.n_ops;
    }
    throw invalid_argument("Métrica no implementada");
}