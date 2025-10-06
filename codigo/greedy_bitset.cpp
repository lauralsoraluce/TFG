#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>

using namespace std;
using namespace chrono;

// Tamaño máximo del universo (ajustar según necesidad)
const int MAX_N = 4096;
using Bitset = bitset<MAX_N>;

/* OPERACIONES BITSET (mucho más rápidas) */
inline Bitset set_union(const Bitset& A, const Bitset& B) {
    return A | B;
}

inline Bitset set_intersect(const Bitset& A, const Bitset& B) {
    return A & B;
}

inline Bitset set_difference(const Bitset& A, const Bitset& B) {
    return A & (~B);
}

inline Bitset apply_op(int op, const Bitset& A, const Bitset& B) {
    if (op == 0) return A | B;       // unión
    else if (op == 1) return A & B;  // intersección
    else return A & (~B);             // diferencia
}

inline string op_name(int op) {
    if (op == 0) return " ∪ ";
    else if (op == 1) return " ∩ ";
    else return " \\ ";
}

/* MEDIDAS */
enum class Metric { JACCARD, PRECISION, RECALL, F1, SIZE };

static Metric parse_metric(string s) {
    for (auto& c : s) c = tolower(c);
    if (s == "jaccard") return Metric::JACCARD;
    if (s == "precision") return Metric::PRECISION;
    if (s == "recall") return Metric::RECALL;
    if (s == "f1") return Metric::F1;
    if (s == "size") return Metric::SIZE;
    return Metric::JACCARD;
}

static const char* metric_name(Metric m) {
    switch(m){
        case Metric::JACCARD: return "jaccard";
        case Metric::PRECISION: return "precision";
        case Metric::RECALL: return "recall";
        case Metric::F1: return "f1";
        case Metric::SIZE: return "size";
    }
    return "unknown";
}

inline double safe_div(double num, double den) {
    return den == 0.0 ? 0.0 : (num / den);
}

// Función de scoring ULTRA OPTIMIZADA con bitsets
inline double M(const Bitset& A, const Bitset& G, int n, Metric metric) {
    // Operaciones en O(n/64) en lugar de O(n) con sets
    size_t tp = (A & G).count();           // |A ∩ G|
    size_t a_size = A.count();             // |A|
    size_t g_size = G.count();             // |G|
    size_t fp = a_size - tp;               // |A \ G|
    size_t fn = g_size - tp;               // |G \ A|
    
    double dtp = (double)tp;
    double dfp = (double)fp;
    double dfn = (double)fn;

    switch (metric) {
        case Metric::JACCARD:
            return safe_div(dtp, dtp + dfp + dfn);
        case Metric::PRECISION:
            return safe_div(dtp, dtp + dfp);
        case Metric::RECALL:
            return safe_div(dtp, dtp + dfn);
        case Metric::F1: {
            double prec = safe_div(dtp, dtp + dfp);
            double rec = safe_div(dtp, dtp + dfn);
            return safe_div(2.0 * prec * rec, prec + rec);
        }
        case Metric::SIZE:
            return -static_cast<double>(fn);
    }
    return 0.0;
}

/* CONVERSIÓN Y DISPLAY */
string bitset_to_str(const Bitset& bs, int n) {
    ostringstream oss;
    oss << "{";
    bool first = true;
    for (int i = 0; i < n; i++) {
        if (bs[i]) {
            if (!first) oss << ",";
            oss << i;
            first = false;
        }
    }
    oss << "}";
    return oss.str();
}

string family_to_str(const vector<Bitset>& F, int n) {
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < F.size(); ++i) {
        if (i > 0) oss << " ";
        oss << "F" << i << "=" << bitset_to_str(F[i], n);
    }
    oss << "]";
    return oss.str();
}

/* GENERACIÓN */
vector<Bitset> generar_F(int n, int num_subconjuntos, int tam_min, int tam_max, int seed=123) {
    vector<Bitset> F;
    
    mt19937 gen(seed);
    uniform_int_distribution<int> dist_tam(tam_min, tam_max);
    uniform_int_distribution<int> dist_idx(0, n - 1);
    
    int intentos = 0;
    int max_intentos = num_subconjuntos * 100;
    
    while (F.size() < num_subconjuntos && intentos < max_intentos) {
        Bitset subconjunto;
        int tam = dist_tam(gen);
        int count = 0;
        
        while (count < tam) {
            int idx = dist_idx(gen);
            if (!subconjunto[idx]) {
                subconjunto[idx] = 1;
                count++;
            }
        }
        
        // Verificar si ya existe (búsqueda lineal)
        bool duplicado = false;
        for (const auto& existing : F) {
            if (existing == subconjunto) {
                duplicado = true;
                break;
            }
        }
        
        if (!duplicado) {
            F.push_back(subconjunto);
        }
        
        intentos++;
    }
    
    return F;
}

Bitset generar_G(int n, int tam, int seed=456) {
    Bitset G;
    mt19937 gen(seed);
    uniform_int_distribution<int> dist_idx(0, n - 1);
    
    int count = 0;
    while (count < tam) {
        int idx = dist_idx(gen);
        if (!G[idx]) {
            G[idx] = 1;
            count++;
        }
    }
    
    return G;
}

/* ALGORITMO GREEDY */
pair<Bitset, string> greedy_search(
    const vector<Bitset>& F,
    const Bitset& G,
    int n,
    Metric metric,
    int k)
{
    Bitset G_tilde;  // Empieza vacío (todos ceros)
    string expresion = "";
    double prev_score = M(G_tilde, G, n, metric);
        
    for (int iter = 0; iter <= k; iter++) {
        double best_score = -numeric_limits<double>::infinity();
        int best_op = -1;
        int best_i = -1;
        Bitset best_conjunto;
        
        // Probar todas las combinaciones op × F[i]
        for (int op = 0; op < 3; op++) {
            for (int i = 0; i < F.size(); i++) {
                Bitset nuevo_conjunto = apply_op(op, G_tilde, F[i]);
                double score = M(nuevo_conjunto, G, n, metric);
                
                if (score >= best_score) {
                    best_score = score;
                    best_op = op;
                    best_i = i;
                    best_conjunto = nuevo_conjunto;
                }
            }
        }
        
        // Si no hay mejora, terminar
        if (best_score <= prev_score) {
            break;
        }

        if (iter == 0){
            expresion = "F" + to_string(best_i);
        } else {
            expresion = "(" + expresion + op_name(best_op) + "F" + to_string(best_i) + ")";
        }
        
        // Actualizar
        G_tilde = best_conjunto;
        prev_score = best_score;
    }
    
    return make_pair(G_tilde, expresion);
}

int main(int argc, char* argv[]) {
    // DEFAULTS
    int k = 5;
    int n = 100;
    int Fsize = -1; // número de subconjuntos en F
    string txt_file;
    unsigned seed = 123;
    Metric metric = Metric::JACCARD;

    // PARSE
    for (int i=1; i<argc; ++i){
        string a = argv[i];
        auto next = [&](){ return (i+1<argc) ? string(argv[++i]) : string(); };
        if (a == "--n") n = stoi(next());
        else if (a == "--k") k = stoi(next());
        else if (a == "--txt") txt_file = next();
        else if (a == "--seed") seed = (unsigned)stoul(next());
        else if (a == "--metric") metric = parse_metric(next());
        else if (a == "--f") Fsize = stoi(next());
        else if (a == "--help") {
            cerr << "Uso: ./prog [--n N] [--k K] [--txt file.txt] [--seed S] [--metric jaccard|precision|recall|f1|size]\n";
            cerr << "Máximo N=" << MAX_N << "\n";
            return 0;
        }
    }

    if (n > MAX_N) {
        cerr << "ERROR: n=" << n << " excede MAX_N=" << MAX_N << "\n";
        return 1;
    }

    // Generar F y G
    int tam_min = max(1, n/10);
    int tam_max = max(tam_min, n/3);
    if (Fsize == -1) Fsize = n; // Por defecto |F|=n
    vector<Bitset> F = generar_F(n, Fsize, tam_min, tam_max, seed);
    Bitset G = generar_G(n, n/2, seed + 1);

    // Ejecutar greedy
    auto t0 = high_resolution_clock::now();
    auto [resultado, expresion] = greedy_search(F, G, n, metric, k);
    auto t1 = high_resolution_clock::now();
    double ms = duration<double, milli>(t1 - t0).count();

    // SALIDA
    ostringstream line;
    line.setf(ios::fixed);
    line << setprecision(6);
    line << "n=" << n << " k=" << k << " |F|=" << F.size();
        
    if (n <= 20) {
        line << " F=" << family_to_str(F, n)
             << " |G|=" << G.count()
             << " G=" << bitset_to_str(G, n);
    }
    
    line << " time_ms=" << ms
         << " metric=" << metric_name(metric)
         << " best_score=" << M(resultado, G, n, metric)
         << " |best|=" << resultado.count()
         << " best_expr=" << expresion;
    
    if (n <= 50) {
        line << " best_set=" << bitset_to_str(resultado, n);
    }
    
    line << "\n";
    
    if (!txt_file.empty()) {  
        ofstream f(txt_file, ios::app);
        if (!f) {
            cerr << "ERROR: no se pudo abrir " << txt_file << "\n";
            return 1;
        }
        f << line.str();
    } else {
        cout << line.str();
    }

    return 0;
}