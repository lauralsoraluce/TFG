#include <iostream>
#include <vector>
#include <algorithm>
//#include <set>
#include <unordered_set>
#include <random>
#include <omp.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Los conjuntos van a ser sets de enteros
using Conjunto = unordered_set<int>; // Para evitar duplicados

vector<Conjunto> generar_subconjuntos(int m, const Conjunto& U, int tam_max = 0) {
    if (tam_max > U.size() || tam_max == 0) {
        tam_max = U.size();
    } // Tamaño máximo de subconjuntos

    random_device rd;
    mt19937 gen(rd());
    int min;
    if (m==1){
        min = 1;
    } else {
        min = m/10; // Tamaño mínimo de subconjuntos
    }
    
    uniform_int_distribution<int> dist_num_subs(1, m);
    //uniform_int_distribution<int> dist_num_subs(min, m);
    int num_subconjuntos = dist_num_subs(gen);

    vector<Conjunto> F(num_subconjuntos);
    vector<int> universo(U.begin(), U.end());  // Convertimos U a un vector para selección rápida
    uniform_int_distribution<int> dis(0, universo.size() - 1);

    // Generamos entre min y m subconjuntos aleatorios
    for (int i = 0; i < num_subconjuntos; i++) {
        int tam = dis(gen) % tam_max + 1;
        while(F[i].size() < tam) {
            F[i].insert(universo[dis(gen)]);
        }
    }
    return F;
}

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

double calcular_costo(const Conjunto& E, const Conjunto& G, double alpha, double beta) {
    int excess = 0;  // Elementos en E pero no en G
    int defect = 0;  // Elementos en G pero no en E

    for (auto& x : E) if (!G.count(x)) excess++;
    for (auto& x : G) if (!E.count(x)) defect++;
    return alpha * excess + beta * defect;
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

bool contiene(const vector<Conjunto>& F, const Conjunto& c) {
    return std::any_of(F.begin(), F.end(), [&](const Conjunto& x) { return x == c; });
}

/*vector<Conjunto> generar_algebra(const vector<Conjunto>& base, const Conjunto& universo, int max_size = 1000) {
    set<Conjunto> algebra(base.begin(), base.end());  // evita duplicados desde el inicio
    queue<Conjunto> cola(base.begin(), base.end());   // para construir nuevos conjuntos

    while (!cola.empty() && algebra.size() < max_size) {
        Conjunto a = cola.front();
        cola.pop();

        for (const Conjunto& b : algebra) {
            vector<Conjunto> nuevos = {
                intersectar(a, b),
                restar(a, b),
                unir(a, b)
            };

            for (const auto& nuevo : nuevos) {
                if (!nuevo.empty() && algebra.insert(nuevo).second) {
                    cola.push(nuevo);
                    if (algebra.size() >= max_size) break;
                }
            }
            if (algebra.size() >= max_size) break;
        }
    }

    return algebra;
}*/

vector<Conjunto> generar_anillo(const vector<Conjunto>& base) {
    vector<Conjunto> R = base;
    bool cambiado = true;

    while (cambiado) {
        cambiado = false;
        int n = R.size();
        for (int i = 0; i < n; ++i) {
            for (int j = i; j < n; ++j) {
                Conjunto u = unir(R[i], R[j]);
                Conjunto d1 = restar(R[i], R[j]);
                Conjunto d2 = restar(R[j], R[i]);
                for (auto& c : {u, d1, d2}) {
                    if (!contiene(R, c)) {
                        R.push_back(c);
                        cambiado = true;
                    }
                }
            }
        }
    }

    return R;
}

vector<Conjunto> generar_semianillo(const vector<Conjunto>& base) {
    vector<Conjunto> S = base;
    bool cambiado = true;

    while (cambiado) {
        cambiado = false;
        int n = S.size();
        for (int i = 0; i < n; ++i) {
            for (int j = i; j < n; ++j) {
                Conjunto inter = intersectar(S[i], S[j]);
                if (!contiene(S, inter)) {
                    S.push_back(inter);
                    cambiado = true;
                }

                Conjunto diff = restar(S[i], S[j]);
                if (!diff.empty() && !contiene(S, diff)) {
                    S.push_back(diff);
                    cambiado = true;
                }

                Conjunto diff2 = restar(S[j], S[i]);
                if (!diff2.empty() && !contiene(S, diff2)) {
                    S.push_back(diff2);
                    cambiado = true;
                }
            }
        }
    }

    return S;
}

Conjunto greedy_jaccard(const int k, const vector<Conjunto>& F, const Conjunto& G) {
    int count = 0;
    Conjunto Covered;
    
    double best_jaccard = jaccard_index(Covered, G);
    double maxJaccard=0.0;

    vector<Conjunto (*)(const Conjunto&, const Conjunto&)> ops = {unir, intersectar, restar};

    while (maxJaccard<1.0 - 1e-6 && count<k){
        int bestIndex = -1;
        int bestOp = -1;
        maxJaccard = best_jaccard;
        Conjunto bestSet;
        
        #pragma omp parallel
        {
            int localBestIndex = -1;
            int localBestOp = -1;
            double localMaxJaccard = best_jaccard;
            Conjunto localBestSet;

            #pragma omp for collapse(2) nowait
            for (size_t i = 0; i < F.size(); i++){
                for (size_t j = 0; j < ops.size(); j++){
                    Conjunto newSet = ops[j](Covered, F[i]);
                    double newJaccard = jaccard_index(newSet, G);
                    
                    if (newJaccard > localMaxJaccard){
                        localMaxJaccard = newJaccard;
                        localBestIndex = i;
                        localBestOp = j;
                        localBestSet= newSet;
                    }
                }
            }
            #pragma omp critical
            {
                if (localMaxJaccard > maxJaccard){
                    maxJaccard = localMaxJaccard;
                    bestIndex = localBestIndex;
                    bestOp = localBestOp;
                    bestSet = localBestSet;
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

// VERSIÓN 2 CON PAREJAS
Conjunto greedy_jaccard_parejas(int k, const vector<Conjunto>& F, const Conjunto& G) {
    int count = 0;
    Conjunto Covered;
    double best_jaccard = jaccard_index(Covered, G);
    double maxJaccard = 0.0;

    // Operaciones posibles
    vector<Conjunto (*)(const Conjunto&, const Conjunto&)> ops = {unir, intersectar, restar};

    while (maxJaccard < 1.0 - 1e-6 && count < k) {
        int best_i = -1, best_j = -1, best_op = -1;
        maxJaccard = best_jaccard;
        Conjunto bestSet;

        #pragma omp parallel
        {
            int local_best_i = -1, local_best_j = -1, local_best_op = -1;
            double local_maxJaccard = best_jaccard;
            Conjunto local_bestSet;

            #pragma omp for nowait
            for (size_t i = 0; i < F.size(); i++) {
                for (size_t j = i; j < F.size(); j++) {  // Evitamos repeticiones
                    for (size_t op = 0; op < ops.size(); op++) {
                        Conjunto newSet = ops[op](F[i], F[j]);  // Operación entre F[i] y F[j]
                        Conjunto candidate = unir(Covered, newSet);  // Unimos con Covered
                        double newJaccard = jaccard_index(candidate, G);

                        if (newJaccard > local_maxJaccard) {
                            local_maxJaccard = newJaccard;
                            local_best_i = i;
                            local_best_j = j;
                            local_best_op = op;
                            local_bestSet = candidate;
                        }
                    }
                }
            }

            #pragma omp critical
            {
                if (local_maxJaccard > maxJaccard) {
                    maxJaccard = local_maxJaccard;
                    best_i = local_best_i;
                    best_j = local_best_j;
                    best_op = local_best_op;
                    bestSet = local_bestSet;
                }
            }
        }

        if (best_i == -1) {
            break;
        }

        Covered = bestSet;
        best_jaccard = maxJaccard;
        count++;

        // Mostrar información de la iteración
        cout << "Iteración " << count << ": Combinamos F[" << best_i << "] y F[" << best_j << "] con operación " << best_op << endl;
        cout << "Nuevo índice de Jaccard: " << best_jaccard << endl;
    }

    return Covered;
}

Conjunto greedy_costo_parejas(int k, const vector<Conjunto>& F, const Conjunto& G, 
    double alpha = 1.0, double beta = 1.0) {
    int count = 0;
    Conjunto Covered;
    double best_cost = calcular_costo(Covered, G, alpha, beta);
    double minCost = best_cost;

    // Operaciones posibles
    vector<Conjunto (*)(const Conjunto&, const Conjunto&)> ops = {unir, intersectar, restar};

    while (minCost > 1e-6 && count < k) {  // Queremos minimizar el costo (hasta ~0)
        int best_i = -1, best_j = -1, best_op = -1;
        minCost = best_cost;
        Conjunto bestSet;

        #pragma omp parallel
        {
            int local_best_i = -1, local_best_j = -1, local_best_op = -1;
            double local_minCost = best_cost;
            Conjunto local_bestSet;

            #pragma omp for nowait
            for (size_t i = 0; i < F.size(); i++) {
                for (size_t j = i; j < F.size(); j++) {
                    for (size_t op = 0; op < ops.size(); op++) {
                        Conjunto newSet = ops[op](F[i], F[j]);
                        Conjunto candidate = unir(Covered, newSet);
                        double newCost = calcular_costo(candidate, G, alpha, beta);

                        if (newCost < local_minCost) {
                            local_minCost = newCost;
                            local_best_i = i;
                            local_best_j = j;
                            local_best_op = op;
                            local_bestSet = candidate;
                        }
                    }
                }
            }

            #pragma omp critical
            {
                if (local_minCost < minCost) {
                    minCost = local_minCost;
                    best_i = local_best_i;
                    best_j = local_best_j;
                    best_op = local_best_op;
                    bestSet = local_bestSet;
                }
            }
        }

        if (best_i == -1) break;

        Covered = bestSet;
        best_cost = minCost;
        count++;

        cout << "Iteración " << count << ": Combinamos F[" << best_i 
        << "] y F[" << best_j << "] con operación " << best_op 
        << "\nNuevo costo: " << best_cost << "\n";
    }

    return Covered;
}

vector<Conjunto> generar_recubrimiento(int m, const Conjunto& U) {
    vector<Conjunto> F;
    Conjunto elementos_cubiertos;
    vector<int> U_vector(U.begin(), U.end());

    // Primero aseguramos cobertura total
    while (elementos_cubiertos.size() < U.size()) {
        Conjunto nuevo_subconjunto;
        int elementos_necesarios = min(10, (int)U.size() - (int)elementos_cubiertos.size()); // Tamaño variable
        
        // Añadir elementos no cubiertos
        for (int elem : U_vector) {
            if (!elementos_cubiertos.count(elem) && rand() % 2 == 0) { // Probabilidad de inclusión
                nuevo_subconjunto.insert(elem);
                elementos_cubiertos.insert(elem);
                if (nuevo_subconjunto.size() >= elementos_necesarios) break;
            }
        }
        
        // Completar con elementos aleatorios si es necesario
        while (nuevo_subconjunto.size() < 5) { // Tamaño mínimo
            int random_elem = U_vector[rand() % U_vector.size()];
            nuevo_subconjunto.insert(random_elem);
            elementos_cubiertos.insert(random_elem);
        }
        
        F.push_back(nuevo_subconjunto);
    }

    // Completar hasta m subconjuntos con conjuntos aleatorios
    while (F.size() < m) {
        Conjunto random_set;
        for (int i = 0; i < 5 + rand() % 100; ++i) { // Tamaño entre 5 y 15
            random_set.insert(U_vector[rand() % U_vector.size()]);
        }
        F.push_back(random_set);
    }

    // Barajar para evitar sesgos
    random_shuffle(F.begin(), F.end());
    return F;
}

int main(int argc, char* argv[]) {
    omp_set_num_threads(4);

    // Universo U
    int n = (argc>1) ? atoi(argv[1]) : 2500;
    int m = (argc>2) ? atoi(argv[2]) : n/10;
    int tam_G = (argc>3) ? atoi(argv[3]) : n/3;
    int k = (argc>4) ? atoi(argv[4]) : 500;

    Conjunto U;
    for (int i = 1; i <= n; i++) U.insert(i);

    // Familia de subconjuntos F
    vector<Conjunto> F = generar_subconjuntos(m,U);
    cout << "Generado random" << endl;
    /*vector<Conjunto> F_base = generar_subconjuntos(10,U, m);
    cout << "Generado base" << endl;*/
    vector<Conjunto> F_recubrimiento = generar_recubrimiento(m, U);
    cout << "Generado recubrimiento" << endl;
    /*vector<Conjunto> F_algebra = generar_algebra(F_base, U);
    cout << "Generado algebra" << endl;
    vector<Conjunto> F_anillo = generar_anillo(F_base);
    cout << "Generado anillo" << endl;
    vector<Conjunto> F_semianillo = generar_semianillo(F_base);
    cout << "Generado semianillo" << endl; */

    // Conjunto objetivo G
    Conjunto G = generar_subconjuntos(1,U,tam_G)[0];

    // No tener en cuenta subconjuntos de F que no cubren G
    std::vector<Conjunto> F_util;
    for (const auto& f : F) {
        if (std::any_of(f.begin(), f.end(), [&](const auto& elem) { return G.count(elem) > 0; })) {
            F_util.push_back(f);
        }
    }
    F = std::move(F_util);
    /*Darle una vuelta a esto, igual nos interesa tenerlos en cuenta para hacer restas con ellos!!*/

    cout << "Total elementos U: " << U.size() << endl;
    cout << "Total elementos G: " << G.size() << endl;
    cout << "Total subconjuntos F: " << F.size() << endl;
    cout << "Total subconjuntos F_recubrimiento: " << F_recubrimiento.size() << endl;
    //cout << "Total subconjuntos F_algebra: " << F_algebra.size() << endl;
    //cout << "Total subconjuntos F_anillo: " << F_anillo.size() << endl;
    //cout << "Total subconjuntos F_semianillo: " << F_semianillo.size() << endl << endl;

    cout << "Greedy_jaccard" << endl;
    auto start = high_resolution_clock::now();
    Conjunto result = greedy_jaccard(k, F, G);
    auto end = high_resolution_clock::now();
    cout << "Índice de Jaccard final: " << jaccard_index(result, G) << endl;
    cout << "Tiempo: " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

    cout << "RECUBRIMIENTO: " << endl;
    start = high_resolution_clock::now();
    Conjunto result2 = greedy_jaccard(k, F_recubrimiento, G);
    end = high_resolution_clock::now();
    cout << "Índice de Jaccard final: " << jaccard_index(result2, G) << endl;
    cout << "Tiempo: " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

    /*cout << "ALGEBRA: " << endl;
    start = high_resolution_clock::now();
    Conjunto result3 = greedy_jaccard(k, F_algebra, G);
    end = high_resolution_clock::now();
    cout << "Índice de Jaccard final: " << jaccard_index(result3, G) << endl;
    cout << "Tiempo: " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

    cout << "ANILLO: " << endl;
    start = high_resolution_clock::now();
    Conjunto result4 = greedy_jaccard(k, F_anillo, G);
    end = high_resolution_clock::now();
    cout << "Índice de Jaccard final: " << jaccard_index(result4, G) << endl;
    cout << "Tiempo: " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

    cout << "SEMIANILLO: " << endl;
    start = high_resolution_clock::now();
    Conjunto result5 = greedy_jaccard(k, F_semianillo, G);
    end = high_resolution_clock::now();
    cout << "Índice de Jaccard final: " << jaccard_index(result5, G) << endl;
    cout << "Tiempo: " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;
*/
    return 0;
}