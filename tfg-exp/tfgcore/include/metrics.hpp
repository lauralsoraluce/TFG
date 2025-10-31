//======================================================================
// metrics.hpp
//----------------------------------------------------------------------
// Definición de métricas comunes usadas por los algoritmos:
//   - Jaccard (maximizar)
//   - SizeH  (minimizar |H|)
//   - OpSize (minimizar nº de operaciones)
//======================================================================

#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>
#include "domain.hpp"
#include "expr.hpp"

//------------------------------------------------------------------
// Tipos de métrica
//------------------------------------------------------------------
enum class Metric {
    Jaccard,
    SizeH,        // |H| (útil como objetivo a minimizar)
    OpSize       // Número de operaciones (útil como objetivo a minimizar)
};

//------------------------------------------------------------------
// Parseo desde string
//------------------------------------------------------------------
inline Metric parse_metric(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    if (s == "jaccard" || s == "iou")     return Metric::Jaccard;
    if (s == "sizeh" || s == "size")      return Metric::SizeH;
    if (s == "opsize" || s == "op_size")  return Metric::OpSize;
    throw std::invalid_argument("Metric desconocida: " + s);
}

//------------------------------------------------------------------
// Nombre textual
//------------------------------------------------------------------
inline const char* metric_name(Metric m) {
    switch (m) {
        case Metric::Jaccard:   return "Jaccard";
        case Metric::SizeH:     return "SizeH";
        case Metric::OpSize:    return "OpSize";
    }
    return "Unknown";
}

//------------------------------------------------------------------
// Indica si la métrica se maximiza
//------------------------------------------------------------------
inline bool is_maximization(Metric m) {
    switch (m) {
        case Metric::Jaccard:
            return true;
        case Metric::SizeH:
            return false;
        case Metric::OpSize:
            return false;  
    }
    return true;
}

//------------------------------------------------------------------
// Evaluación de la métrica
//-----------------------------------------------------------------
double M(const Expression& H, const Bitset& G, Metric metric);