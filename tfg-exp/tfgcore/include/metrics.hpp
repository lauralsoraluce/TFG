//----------------------------------------------------------------------
// metrics.hpp
//----------------------------------------------------------------------
// Definición de métricas comunes usadas por los algoritmos
//----------------------------------------------------------------------

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
    Jaccard,     // índice de Jaccard (IoU) (a maximizar)
    SizeH,       // Número de subconjuntos de F_i distintos (a minimizar)
    OpSize       // Número de operaciones (a minimizar)
};

//------------------------------------------------------------------
// Parseo desde string (para reconocerlas en línea de comandos)
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
    return "Desconocida";
}

//------------------------------------------------------------------
// Indica si la métrica se maximiza o no
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
