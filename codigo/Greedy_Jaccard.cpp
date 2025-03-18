#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_set>

using namespace std;

// Los conjuntos van a ser sets de enteros
using Conjunto = set<int>;

Conjunto unir(const Conjunto& a, const Conjunto& b) {
    Conjunto res;
    // Iteramos sobre los elementos de a y b
    for (int elem : a) {
        res.insert(elem);
    }
    for (int elem : b) {
        res.insert(elem);
    }
    return res;
}

Conjunto intersectar(const Conjunto& a, const Conjunto& b) {
    Conjunto res;
    // Iteramos sobre los elementos de a
    for (int elem : a) {
        // Si el elemento está en b, lo agregamos al resultado
        if (b.count(elem) > 0) {
            res.insert(elem);
        }
    }
    return res;
}

Conjunto restar(const Conjunto& a, const Conjunto& b) {
    Conjunto res;
    // Iteramos sobre los elementos de a
    for (int elem : a) {
        // Si el elemento no está en b, lo agregamos al resultado
        if (b.count(elem) == 0) {
            res.insert(elem);
        }
    }
    return res;
}

/*Conjunto complementar(const Conjunto& a, int n) {
    Conjunto res;
    // Iteramos sobre los elementos de 0 a n-1
    for (int i = 0; i < n; i++) {
        // Si el elemento no está en a, lo agregamos al resultado
        if (a.count(i) == 0) {
            res.insert(i);
        }
    }
    return res;
}*/

Conjunto leerConjunto(int n) {
    Conjunto res;
    // Leemos los elementos del conjunto
    for (int i = 0; i < n; i++) {
        int elem;
        cin >> elem;
        res.insert(elem);
    }
    return res;
}

// Función para calcular el índice de Jaccard
double jaccard_index(const Conjunto& A, const Conjunto& B) {
    int interseccion = 0, union_ = A.size();
    for (int x : B) {
        if (A.count(x)) interseccion++;
        else union_++;
    }
    return (union_ == 0) ? 0.0 : (double)interseccion / union_;
}

Conjunto greedy_jaccard(const int k, const vector<Conjunto>& F, const Conjunto& U, const Conjunto& G) {
    int count = 0;
    Conjunto Covered;
    // vector<bool> used(F.size(), false); // Descomentar si no queremos repetir elementos
    
    double best_jaccard = jaccard_index(Covered, G);
    double maxJaccard=0.0;

    vector<Conjunto (*)(const Conjunto&, const Conjunto&)> ops = {unir, intersectar, restar};

    while (maxJaccard<1.0 - 1e-6 && count<k){
        int bestIndex = -1;
        int bestOp = -1;
        maxJaccard = best_jaccard;
        Conjunto bestSet;
        
        for (size_t i = 0; i < F.size(); i++){
            for (size_t j = 0; j < ops.size(); j++){
                Conjunto newSet = ops[j](Covered, F[i]);
                double newJaccard = jaccard_index(newSet, G);
                
                if (newJaccard > maxJaccard){
                    maxJaccard = newJaccard;
                    bestIndex = i;
                    bestOp = j;
                    bestSet = newSet;
                }
            }
        }
        if (bestIndex == -1){
            break;
        }
        Covered = bestSet;
        best_jaccard = maxJaccard;
        count++;
        
        // Mostrar información de la iteración
        cout << "Iteración " << count << ": Añadimos F[" << bestIndex << "] con operación " << bestOp << endl;
        cout << "Nuevo índice de Jaccard: " << best_jaccard << endl;
    }

    return Covered;
}

int main() {
    // Universo U
    int n = 10;
    Conjunto U;
    for (int i = 1; i <= n; i++) U.insert(i);

    // Familia de subconjuntos F
    vector<Conjunto> F = {
        {1, 2, 3},
        {2, 4, 5},
        {6, 7},
        {3, 5, 7, 8},
        {9, 10}
    };

    // Conjunto objetivo G
    Conjunto G = {1, 2, 3, 5, 7, 9};

    // Ejecutar algoritmo con k = 5
    Conjunto result = greedy_jaccard(5, F, U, G);

    // Mostrar resultado final
    cout << "Conjunto final obtenido: { ";
    for (int x : result) cout << x << " ";
    cout << "}" << endl;
    cout << "Índice de Jaccard final: " << jaccard_index(result, G) << endl;

    return 0;
}