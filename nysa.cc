#include <iostream>
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <queue>
#include <stdlib.h>

using namespace std;

constexpr int32_t xor_line_size = 4;
constexpr int32_t not_line_size = 3;
constexpr int32_t min_line_size = 4;

// Bramka reprezentowana jest przez parę - strumienie wejściowe, strumień wyjściowy.
using gate = pair<vector<int32_t>, int32_t>;

// Sygnał reprezentowany jest przez parę - bramka z której wychodzi, bramka do której wchodzi.
// Jeśli nie wychodzi z żadnej bramki, pierwsza wartość z pary to -1.
using signal = pair<int32_t, vector<int32_t>>;

enum class gate_type {
    NOT,
    XOR,
    AND,
    NAND,
    OR,
    NOR
};

int32_t count_words_in_line(string line) {
    int32_t count = 0;
    stringstream buffer(line);
    string buffer_output;

    while (buffer >> buffer_output) {
        count++;
    }
    return count;
}

bool contains(vector<int32_t> &vec, int32_t value) {
    return find(vec.begin(),vec.end(), value) != vec.end();
}

// Sprawdza, czy dany numer strumienia pojawił się wcześniej jako strumień wyjściowy.
// Jeśli tak, wypisuje błąd. Zapamiętuje, że rozpatrywany strumień jest wyjściowy.
bool handle_short_circuit(int32_t line_number, int32_t output_stream, vector<int32_t> &all_output_streams) {
    if (contains(all_output_streams, output_stream)) {
        cerr << "Error in line " << line_number << ": signal " << output_stream << " is assigned to multiple outputs.\n";
        return false;
    } else {
        all_output_streams.emplace_back(output_stream);
        return true;
    }
}

bool handle_num_of_input_error(string line, gate_type type) {
    int32_t line_size = count_words_in_line(line);
    if ((type == gate_type::XOR && line_size != xor_line_size) ||
        (type == gate_type::NOT && line_size != not_line_size) ||
        (type != gate_type::XOR && type != gate_type::NOT && line_size < min_line_size)) {
        return false;
    }
    return true;
}

bool parse_gate(string line, stringstream &buffer, bool &correct_input, int32_t line_number,
                vector<int32_t> &all_output_streams, gate_type type, unordered_map<int32_t, gate_type> &gate_types,
                vector<gate> &gates) {
    bool correct_gate = handle_num_of_input_error(line, type);
    int32_t output_stream;
    buffer >> output_stream;
    // Numer strumienia musi być liczbą dodatnią.
    if (output_stream == 0) {
        correct_gate = false;
    }

    int32_t input_stream;
    vector<int32_t> input_streams;
    while (buffer >> input_stream) {
        // Numer strumienia musi być liczbą dodatnią.
        if (input_stream == 0) {
            correct_gate = false;
        }
        input_streams.emplace_back(input_stream);
    }

    bool short_circuit_occured = false;
    if (correct_gate) {
        if (!handle_short_circuit(line_number, output_stream, all_output_streams)) {
            short_circuit_occured = true;
            correct_gate = false;
        }
    }

    if (correct_gate) {
        gate curr_gate = {input_streams, output_stream};
        gates.emplace_back(curr_gate);
        gate_types[gates.size() - 1] = type;
    } else {
        if (!short_circuit_occured) {
            cerr << "Error in line " << line_number << ": " << line << '\n';
        }
    }

    if (!correct_gate) {
        correct_input = false;
    }
    return correct_gate;
}

// Rozpoznaje typ bramki oraz odpowiednio wywołuje funkcję która ją przetworzy lub zwraca błąd.
bool determine_and_parse_gate(string line, bool &correct_input, int32_t line_number,
                              vector<int32_t> &all_output_streams, unordered_map<int32_t, gate_type> &gate_types,
                              vector<gate> &gates) {
    stringstream buffer;
    buffer << line;
    string logic_gate;
    buffer >> logic_gate;
    gate_type type;

    if (logic_gate == "XOR") {
        type = gate_type::XOR;
    } else if (logic_gate == "NOT") {
        type = gate_type::NOT;
    } else if (logic_gate == "AND") {
        type = gate_type::AND;
    } else if (logic_gate == "NAND") {
        type = gate_type::NAND;
    } else if (logic_gate == "OR") {
        type = gate_type::OR;
    } else if (logic_gate == "NOR") {
        type = gate_type::NOR;
    } else {
        cerr << "Error in line " << line_number << ": " << line << '\n';
        return false;
    }
    return parse_gate(line, buffer, correct_input, line_number, all_output_streams, type, gate_types, gates);
}

// Wczytuje do wektora gates kolejne bramki oraz do wektora gate_types ich typy
// w kolejności pojawiania się na wejściu, numerowane od 0.
// Wypisuje błędy o złym formacie linii oraz o powtarzających się strumieniach wyjścia.
// Zwraca informację, czy należy dalej przetwarzać układ.
bool read_input(unordered_map<int32_t, gate_type> &gate_types, vector<gate> &gates) {
    bool correct_input = true;
    int32_t line_number = 1;
    vector<int32_t> all_output_streams;
    string line;
    regex pattern("^\\s*(NOT|XOR|AND|NAND|OR|NOR)((\\s)+(\\d){1,9})+\\s*$");

    while (getline(cin, line)) {
        smatch result;
        if (!regex_search(line, result, pattern)) {
            cerr << "Error in line " << line_number << ": " << line << '\n';
            correct_input = false;
        } else {
            determine_and_parse_gate(line, correct_input, line_number, all_output_streams, gate_types, gates);
        }
        line_number++;
    }
    return correct_input;
}

// Funkcja pomocnicza funkcji create_graph().
// Dodaje krawędź od bramki a do bramki b.
void add_edge(int32_t a, int32_t b, vector<pair<vector<int32_t>, vector<int32_t>>> &graph) {
    graph[a].second.push_back(b);
    graph[b].first.push_back(a);
}

// Na podstawie wektora kolejno wczytanych bramek gates tworzy graf w obiekcie graph.
void create_graph(vector<pair<vector<int32_t>, vector<int32_t>>> &graph, unordered_map<int32_t, signal> &signals,
                  size_t num_of_gates, vector<gate> &gates) {
    // Uzupełnianie obiektu signals.
    for (size_t i = 0; i < num_of_gates; i++) {
        signals[gates[i].second].first = i;
        for (size_t j = 0; j < gates[i].first.size(); j++) {
            if (signals.count(gates[i].first[j]) == 0) {
                signals[gates[i].first[j]].first = -1;
            }
            signals[gates[i].first[j]].second.push_back(i);
        }
    }

    // Tworzenie obiektu graph.
    for (size_t i = 0; i < num_of_gates; i++) {
        // Tworzenie pustych wektorów dla wierzcholków grafu.
        graph.push_back(make_pair(vector<int32_t>(), vector<int32_t>()));
    }
    for (auto i = signals.begin(); i != signals.end(); ++i) {
        for (size_t j = 0; j < i->second.second.size(); j++) {
            if ((i->second).first != -1) {
                add_edge((i->second).first, (i->second).second[j], graph);
            }
        }
    }
}

// Stany wierzchołków potrzebne do algorytmu wykrywania cyklu.
enum visited_state {
    FULLY_DONE,
    IN_PROGRESS,
    NOT_BEGAN
};

// Rekurencyjna funkcja przetwarzania wierzchołka o numerze a
// w algorytmie wykrywania cykli.
bool visit(int32_t a, visited_state* visited_states, vector<pair<vector<int32_t>, vector<int32_t>>> &graph) {
    visited_states[a] = IN_PROGRESS;
    size_t num_of_edges = graph[a].second.size();
    for (size_t i = 0; i < num_of_edges; i++) {
        if (visited_states[graph[a].second[i]] == IN_PROGRESS) {
            // Znalezienie cyklu.
            return false;
        } else if (visited_states[graph[a].second[i]] == NOT_BEGAN) {
            if (!visit(graph[a].second[i], visited_states, graph)) {
                // Znalezienie cyklu w rekurencyjnych wywołaniach.
                return false;
            }
        }
    }
    visited_states[a] = FULLY_DONE;
    return true;
}

// Funkcja pomocnicza, ustawia wszystkie wierzchołki jako nieodwiedzone.
void clear_table(visited_state* visited_states, size_t num_of_gates) {
    for (size_t i = 0; i < num_of_gates; i++) {
        visited_states[i] = NOT_BEGAN;
    }
}

// Sprawdza, czy graf przechowywany w obiekcie graph jest cykliczny.
bool is_cyclic(size_t num_of_gates, vector<pair<vector<int32_t>, vector<int32_t>>> &graph) {
    // https://eduinf.waw.pl/inf/alg/001_search/0132.php
    visited_state visited_states[num_of_gates];
    clear_table(visited_states, num_of_gates);
    if (!visit(0, visited_states, graph)) {
        return true;
    }
    return false;
}

// Sprawdza, które sygnały nie wychodzą z ąadnej bramki
// i zapamiętuje je w wektorze input_signals.
void detect_input_signals(vector<int32_t>* input_signals, unordered_map<int32_t, signal> &signals) {
    for (auto i = signals.begin(); i != signals.end(); ++i) {
        if (i->second.first == -1) {
            input_signals->push_back(i->first);
        }
    }
}

// Ustawia tablice z konfiguracją sygnałów na zawierającą same zera.
void initialize_state_of_signals(bool* state_of_signals, size_t num_of_input_signals) {
    for (size_t i = 0; i < num_of_input_signals; i++) {
        state_of_signals[i] = false;
    }
}

// Zmienia tablicę z konfiguracją sygnałów na nastepną
// Zwraca false jeśli była to już ostatnia.
bool next_state_of_signals(bool* state_of_signals, size_t num_of_input_signals) {
    for (int32_t i = num_of_input_signals - 1; i >= 0; i--) {
        if (state_of_signals[i]) {
            state_of_signals[i] = false;
        } else {
            state_of_signals[i] = true;
            return true;
        }
    }
    return false;
}

// Sortowanie topologiczne wierzchołków grafu.
// Zwraca posortowane wierzchołki w wektorze sorted_vertices.
void toposort(vector<int32_t>* sorted_vertices, size_t num_of_gates, vector<pair<vector<int32_t>,
        vector<int32_t>>> &graph) {
    queue<int32_t> zero_degree_vertices;
    int32_t vertex_degrees[num_of_gates];
    for (size_t i = 0; i < num_of_gates; i++) {
        vertex_degrees[i] = graph[i].second.size();
    }
    for (size_t i = 0; i < num_of_gates; i++) {
        if (vertex_degrees[i] == 0) {
            zero_degree_vertices.push(i);
        }
    }

    while (!zero_degree_vertices.empty()) {
        int32_t v = zero_degree_vertices.front();
        zero_degree_vertices.pop();
        sorted_vertices->push_back(v);
        size_t in_degree = graph[v].first.size();
        for (size_t i = 0; i < in_degree; i++) {
            vertex_degrees[graph[v].first[i]]--;
            if (vertex_degrees[graph[v].first[i]] == 0) {
                zero_degree_vertices.push(graph[v].first[i]);
            }
        }
    }
    reverse((*sorted_vertices).begin(), (*sorted_vertices).end());
}

// Dla danej bramki oraz danej konfiguracji jej wejściowych stanów
// wypełnia tablice prawdy dla jej wyjścia.
void complete(int32_t gate_number, vector<gate> &gates, unordered_map<int32_t, gate_type> &gate_types,
              map<int32_t, bool> &truth_output) {
    size_t num_of_inputs = gates[gate_number].first.size();
    if (gate_types[gate_number] == gate_type::NOT) {
        truth_output[gates[gate_number].second] = !truth_output[gates[gate_number].first[0]];
    } else if (gate_types[gate_number] == gate_type::XOR) {
        truth_output[gates[gate_number].second] = (truth_output[gates[gate_number].first[0]] !=
                                                   truth_output[gates[gate_number].first[1]]);
    } else if (gate_types[gate_number] == gate_type::AND) {
        truth_output[gates[gate_number].second] = true;
        for (size_t i = 0; i < num_of_inputs; i++) {
            if (!truth_output[gates[gate_number].first[i]]) {
                truth_output[gates[gate_number].second] = false;
                break;
            }
        }
    } else if (gate_types[gate_number] == gate_type::NAND) {
        truth_output[gates[gate_number].second] = false;
        for (size_t i = 0; i < num_of_inputs; i++) {
            if (!truth_output[gates[gate_number].first[i]]) {
                truth_output[gates[gate_number].second] = true;
                break;
            }
        }
    } else if (gate_types[gate_number] == gate_type::OR) {
        truth_output[gates[gate_number].second] = false;
        for (size_t i = 0; i < num_of_inputs; i++) {
            if (truth_output[gates[gate_number].first[i]]) {
                truth_output[gates[gate_number].second] = true;
                break;
            }
        }
    } else if (gate_types[gate_number] == gate_type::NOR) {
        truth_output[gates[gate_number].second] = true;
        for (size_t i = 0; i < num_of_inputs; i++) {
            if (truth_output[gates[gate_number].first[i]]) {
                truth_output[gates[gate_number].second] = false;
                break;
            }
        }
    }
}

// Oblicza jedną linijkę tablicy prawdy dla wszystkich bramek na podstawie początkowej konfiguracji
// oraz wektora posortowanych wierzchołków.
void compute_output(vector<int32_t>* sorted_gates, size_t num_of_gates, vector<gate> &gates,
                    unordered_map<int32_t, gate_type> &gate_types, map<int32_t, bool> &truth_output) {
    for (size_t i = 0; i < num_of_gates; i++) {
        complete((*sorted_gates)[i], gates, gate_types, truth_output);
    }
}

// Wypisuje otrzymaną linijkę tablicy prawdy.
void print_truth_output(map<int32_t, bool> &truth_output) {
    for (auto i = truth_output.begin(); i != truth_output.end(); ++i) {
        if (i->second) {
            cout << "1";
        } else {
            cout << "0";
        }
    }
    cout << "\n";
}

// Wypełnia znane od początku wartości tablicy truth_output.
void complete_input_values(size_t num_of_input_signals, vector<int32_t>* input_signals,
                           bool* state_of_signals, map<int32_t, bool> &truth_output) {
    for (auto i = truth_output.begin(); i != truth_output.end(); ++i) {
        i->second = false;
    }
    for (size_t i = 0; i < num_of_input_signals; i++) {
        truth_output[(*input_signals)[i]] = state_of_signals[i];
    }
}

// Tworzy i wypisuje tablicę prawdy układu
void create_truth_table(unordered_map<int32_t, signal> &signals, size_t num_of_gates,
                        vector<pair<vector<int32_t>, vector<int32_t>>> &graph, vector<gate> &gates,
                        unordered_map<int32_t, gate_type> &gate_types) {
    // Sprawdza, które sygnały są tylko wejściowe.
    vector<int32_t> input_signals;
    detect_input_signals(&input_signals, signals);
    size_t num_of_input_signals = input_signals.size();
    sort(input_signals.begin(), input_signals.end());

    // Tablica z obecnie sprawdzaną konfiguracją sygnałów.
    bool state_of_signals[num_of_input_signals];
    initialize_state_of_signals(state_of_signals, num_of_input_signals);

    // Przejście po wszystkich konfiguracjach.
    vector<int32_t> sorted_gates;
    toposort(&sorted_gates, num_of_gates, graph);

    // Dla danej początkowej konfiguracji sygnałów
    // przechowuje obliczony stan wszystkich sygnałów.
    map<int32_t, bool> truth_output;
    do {
        complete_input_values(num_of_input_signals, &input_signals, state_of_signals, truth_output);
        compute_output(&sorted_gates, num_of_gates, gates, gate_types, truth_output);
        print_truth_output(truth_output);
    } while (next_state_of_signals(state_of_signals, num_of_input_signals));
}


int main() {
    unordered_map<int32_t, gate_type> gate_types;
    vector<gate> gates;
    bool is_correct_data = read_input(gate_types, gates);

    if (!is_correct_data) {
        return 0;
    }

    size_t num_of_gates = gates.size();

    // Wierzchołkami są bramki, graf jest skierowany.
    // Wektor krawędzi wchodzących, potem wychodzących.
    // Krawędź od wierzchołka a do wierzchołka b istnieje wtedy i tylko wtedy,
    // gdy istnieje sygnał wychodzący z a i wchodzący do b.
    vector<pair<vector<int32_t>, vector<int32_t>>> graph;

    // Sygnały ponumerowane kolejnością pojawiania się.
    unordered_map<int32_t, signal> signals;

    create_graph(graph, signals, num_of_gates, gates);

    if (is_cyclic(num_of_gates, graph)) {
       cerr<< "Error: sequential logic analysis has not yet been implemented.\n";
    }
    else {
        create_truth_table(signals, num_of_gates, graph, gates, gate_types);
    }

    return 0;
}

