//======================================================================
// generator.hpp
//----------------------------------------------------------------------
// Generación aleatoria de instancias: conjunto objetivo G y familia F.
//  - G: subconjunto aleatorio del universo U.
//  - F: colección de subconjuntos aleatorios F_i ⊆ U.
//======================================================================

#pragma once

#include <vector>
#include <random>
#include "domain.hpp"

// -----------------------------------------------------------------------------
// Parámetros de generación
// -----------------------------------------------------------------------------
struct GenConfig {
    int G_size_min = 10;    // Tamaño mínimo de G
    int F_n_min = 10;       // Número mínimo de conjuntos en F
    int F_n_max = 100;      // Número máximo de conjuntos en F
    int Fi_size_min = 1;    // Tamaño mínimo de cada conjunto F_i
    int Fi_size_max = 64;   // Tamaño máximo de cada conjunto F_i
};

// -----------------------------------------------------------------------------
// Generar conjunto G ⊆ U 
// -----------------------------------------------------------------------------
Bitset generar_G(int n, int tam_min, int seed = 456);

// -----------------------------------------------------------------------------
// Generar familia F
// -----------------------------------------------------------------------------
std::vector<Bitset> generar_F(int n, int n_min, int n_max,
                              int tam_min, int tam_max, int seed = 123);