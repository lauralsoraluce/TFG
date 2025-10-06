#!/usr/bin/env bash
# ==========================================
# Script para compilar y ejecutar pruebas
# Greedy TFG - Laura Lázaro
# ==========================================

set -euo pipefail

# --- Configuración ---
SRC="greedy_bitset.cpp"     # archivo fuente
OUT="Greedy"         # ejecutable
OUTFILE="resultados_Greedy.txt" # fichero donde se guardan los resultados
CXX="g++"                # compilador

# --- Comprobaciones básicas ---
if ! command -v "$CXX" >/dev/null 2>&1; then
  echo "❌ No se encontró $CXX en el PATH."
  exit 1
fi

if [ ! -f "$SRC" ]; then
  echo "❌ No existe el archivo fuente: $SRC"
  exit 1
fi

# --- Compilar ---
echo "🔧 Compilando $SRC..."
"$CXX" -std=c++17 -O2 -march=native -Wall -Wextra -o "$OUT" "$SRC"
echo "✅ Compilación correcta."

# --- Inicializar archivo de resultados ---
echo "🧾 Guardando resultados en $OUTFILE"
echo "==== RESULTADOS ====" > "$OUTFILE"
date >> "$OUTFILE"
echo >> "$OUTFILE"

METRICS=(jaccard)
SEEDS=(123 456 789)

# COMPARACIÓN CON BÚSQUEDA EXHAUSTIVA

N_VALUES=(6)
K_VALUES=(3 4)

for n in "${N_VALUES[@]}"; do
  for k in "${K_VALUES[@]}"; do
    for seed in "${SEEDS[@]}"; do
      for metric in "${METRICS[@]}"; do
        echo "▶️  n=$n, k=$k, seed=$seed, metric=$metric"
        "./$OUT" --n "$n" --k "$k" --seed "$seed" --metric "$metric" --txt "$OUTFILE"
      done
    done
  done
done

echo

N_VALUES=(8 12 16 20)
K_VALUES=(3)

for n in "${N_VALUES[@]}"; do
  for k in "${K_VALUES[@]}"; do
    for seed in "${SEEDS[@]}"; do
      for metric in "${METRICS[@]}"; do
        echo "▶️  n=$n, k=$k, seed=$seed, metric=$metric"
        "./$OUT" --n "$n" --k "$k" --seed "$seed" --metric "$metric" --txt "$OUTFILE"
      done
    done
  done
done

echo 

# VALORES NUEVOS - ESCALADO EN N

N_VALUES=(200 400 800 1600 3200)
K_VALUES=(16)

for n in "${N_VALUES[@]}"; do
  for k in "${K_VALUES[@]}"; do
    for seed in "${SEEDS[@]}"; do
      for metric in "${METRICS[@]}"; do
        echo "▶️  n=$n, k=$k, seed=$seed, metric=$metric"
        "./$OUT" --n "$n" --k "$k" --seed "$seed" --metric "$metric" --txt "$OUTFILE" --verbose
      done
    done
  done
done

echo

# VALORES NUEVOS - ESCALADO EN F

n=800
K_VALUES=(16)
F_SIZES=(800 1600 4000 8000)

for f in "${F_SIZES[@]}"; do
  for k in "${K_VALUES[@]}"; do
    for seed in "${SEEDS[@]}"; do
      for metric in "${METRICS[@]}"; do
        echo "▶️  n=$n, k=$k, seed=$seed, metric=$metric, fsize=$f", 
        "./$OUT" --n "$n" --k "$k" --seed "$seed" --metric "$metric" --txt "$OUTFILE" --verbose
      done
    done
  done
done

echo

# VALORES ESCALADO EN K

N=3200
KMAXS=(8 12 16 24 32)

for k in "${KMAXS[@]}"; do
  for s in "${SEEDS[@]}"; do
    for mtr in "${METRICS[@]}"; do
      echo "▶️  n=$N, k=$k, seed=$s, metric=$mtr"
      "./$OUT" --n "$N" --k "$k" \
        --seed "$s" --metric "$mtr" \
        --txt "$OUTFILE"
    done
  done
done


echo
echo "✅ Pruebas completadas. Resultados añadidos a $OUTFILE"

