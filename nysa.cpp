#include <iostream>
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <queue>
#include <stdlib.h>

#define XOR_LINE_SIZE 4
#define NOT_LINE_SIZE 3
#define MIN_LINE_SIZE 4

using namespace std;

// <strumienie wejscia, wyjscia>
using gate = pair<vector<int>, int>;
using signal = pair<int, vector<int> >;
// <bramka z której wychodzi, bramki do których wchodzi>
// jesli nie wychodzi z zadnej bramki to -1
// bramki w kolejnosci pojawiania sie na wejsciu
vector<gate> gates;
// sygnaly po numerach
unordered_map<int, signal> signals;
int num_of_gates;
// mój edytor nie obsluguje polskich znaków :(

enum gate_type{
    NOT,
    XOR,
    AND,
    NAND,
    OR,
    NOR
};
// nr bramki w kolejnosci wejscia, jej typ
unordered_map<int, gate_type> gate_types;


int count_words_in_line(string line) {
    int count = 0;
    stringstream buffer(line);
    string buffer_output;

    while (buffer >> buffer_output) {
        count++;
    }
    return count;
}

bool handle_short_circuit(int line_number, int output_stream, set<int> &output_streams) {
    if (output_streams.find(output_stream) != output_streams.end()) {
        cout << "Error in line " << line_number << ": signal " << output_stream
             << " is assigned to multiple outputs.\n";
        return false;
    } else {
        output_streams.emplace(output_stream);
        return true;
    }
}

bool handle_num_of_input_error(string line, int line_number, gate_type type) {
    int line_size = count_words_in_line(line);
    if ((type == XOR && line_size != XOR_LINE_SIZE) ||
        (type == NOT && line_size != NOT_LINE_SIZE) ||
        (type != XOR && type != NOT && line_size < MIN_LINE_SIZE)) {
        cout << "Error in line " << line_number << ": " << line << "\n";
        return false;
    }
    return true;
}

bool parse_gate(string line, stringstream &buffer, bool &correct_input, int line_number,
                set<int> &all_output_streams, gate_type type) {
    bool correct_gate = handle_num_of_input_error(line, line_number, type);
    int output_stream;
    buffer >> output_stream;
    // czy błędy się nakładają?
    if (!handle_short_circuit(line_number, output_stream, all_output_streams)) {
        correct_gate = false;
    }

    int input_stream;
    vector<int> input_streams;
    while (buffer >> input_stream) {
        input_streams.emplace_back(input_stream);
    }
    if (correct_gate) {
        gate curr_gate = {input_streams, output_stream};
        gates.emplace_back(curr_gate);
        gate_types[gates.size() - 1] = type;
    }

    if (!correct_gate) {
        correct_input = false;
    }
    return correct_gate;
}

bool determine_and_parse_gate(string line, bool &correct_input, int line_number,
                              set<int> &all_output_streams) {
    stringstream buffer;
    buffer << line;
    string logic_gate;
    buffer >> logic_gate;
    gate_type type;

    if (logic_gate == "XOR") {
        type = XOR;
    } else if (logic_gate == "NOT") {
        type = NOT;
    } else if (logic_gate == "AND") {
        type = AND;
    } else if (logic_gate == "NAND") {
        type = NAND;
    } else if (logic_gate == "OR") {
        type = OR;
    } else if (logic_gate == "NOR") {
        type = NOR;
    } else {
        cout << "Error in line " << line_number << ": " << line << "\n";
        return false;
    }
    return parse_gate(line, buffer, correct_input, line_number, all_output_streams, type);
}

bool read_input() {
    bool correct_input = true;
    int line_number = 1;
    set<int> all_output_streams;
    string line;
    regex pattern("(NOT|XOR|AND|NAND|OR|NOR)((\\s)+(\\d){1,9})+\\s*");

    while (getline(cin, line)) {
        smatch result;
        if (!regex_search(line, result, pattern)) {
            cout << "Error in line " << line_number << ": " << line << "\n";
            correct_input = false;
        } else {
            determine_and_parse_gate(line, correct_input, line_number, all_output_streams);
        }
        line_number++;
    }
    return correct_input;
}



vector< pair<vector <int>, vector<int> > > graph;
// wierzcholkami sa bramki, graf jest skierowany
// wektor krawedzi wchodzacych, potem wychodzacych
// krawedz od wierzcholka a do wierzcholka b istnieje wtedy i tylko wtedy,
// gdy istnieje sygnal wychodzacy z a i wchodzacy do b

// funkcja pomocnicza funkcji create_graph()
// dodaje krawedz od bramki a do bramki b
void add_edge(int a, int b)
{
    //fprintf(stderr, "tu dziala\n");
    graph[a].second.push_back(b);
    graph[b].first.push_back(a);
}

// na podstawie gates tworzy graf w obiekcie graph
void create_graph()
{
    // uzupelnianie obiektu signals
    unsigned int num_of_gates = gates.size();
    for (unsigned int i = 0; i < num_of_gates; i++)
    {
        signals[gates[i].second].first = i;
        for (unsigned int j = 0; j < gates[i].first.size(); j++)
        {
            if (signals.count(gates[i].first[j]) == 0)
            {
                signals[gates[i].first[j]].first = -1;
            }
            signals[gates[i].first[j]].second.push_back(i);
        }
    }
    /* for (auto i = signals.begin(); i != signals.end(); ++i)
    {
        fprintf(stderr, "%d: %d; ", i->first, i->second.first);
        for (int j = 0; j < i->second.second.size(); j++)
        {
            fprintf(stderr, "%d ", i->second.second[j]);
        }
        fprintf(stderr, "\n");
    } // wypisywanie obiektu signals, do debugowania */

    // tworzenie obiektu graph
    for (int i = 0; i < num_of_gates; i++)
    {
        // tworzenie pustych wektorów dla wierzcholków grafu
        graph.push_back(make_pair(vector<int>(), vector<int>()));
    }
    for (auto i = signals.begin(); i != signals.end(); ++i)
    {
        for (unsigned int j = 0; j < i->second.second.size(); j++)
        {
            if ((i->second).first != -1)
            {
                // fprintf(stderr, "dodajemy krawedz od %d do %d\n", (i->second).first, (i->second).second[j]);
                add_edge((i->second).first, (i->second).second[j]);
            }
        }
    }
}

// stany wierzcholków potrzebne do algorytmu wykrywania cyklu
enum visited_state
{
    FULLY_DONE,
    IN_PROGRESS,
    NOT_BEGAN
};
// nie mam pomyslu na lepsze nazwy :(

// rekurencyjna funkcja przetwarzania wierzcholka o numerze a
//  w algorytmie wykrywania cykli
bool visit(int a, visited_state* visited_states)
{
    visited_states[a] = IN_PROGRESS;
    unsigned int num_of_edges = graph[a].second.size();
    for (unsigned int i = 0; i < num_of_edges; i++)
    {
        if (visited_states[graph[a].second[i]] == IN_PROGRESS)
        {
            // znalezienie cyklu
            return false;
        }
        else if (visited_states[graph[a].second[i]] == NOT_BEGAN)
        {
            if (!visit(graph[a].second[i], visited_states))
            {
                // znalezienie cyklu w rekurencyjnych wywolaniach
                return false;
            }
        }
    }
    visited_states[a] = FULLY_DONE;
    return true;
}

// funkcja pomocnicza, ustawia wszystkie wierzcholki na nieodwidzone
void clear_table(visited_state* visited_states, unsigned int num_of_gates)
{
    for (unsigned int i = 0; i < num_of_gates; i++)
    {
        visited_states[i] = NOT_BEGAN;
    }
}

// sprawdza, czy graf przechowywany w graph jest cykliczny
bool is_cyclic()
{
    // https://eduinf.waw.pl/inf/alg/001_search/0132.php
    unsigned int num_of_gates = gates.size();
    visited_state visited_states[num_of_gates];
    clear_table(visited_states, num_of_gates);
    if (!visit(0, visited_states))
    {
        return true;
    }
    return false;
}

// dla danej poczatkowej konfiguracji sygnalów
// przechowuje obliczony stan wszystkich sygnalów
map<int, bool> truth_output;

// sprawdza, które sygnaly nie wychodza z zadnej bramki
// i zapamietuje je w input_signals
void detect_input_signals(vector<int>* input_signals)
{
    for (auto i = signals.begin(); i != signals.end(); ++i)
    {
        if (i->second.first == -1)
        {
            input_signals->push_back(i->first);
        }
    }
}


// ustawia tablice z konfiguracja sygnalów na same zera
void initialize_state_of_signals(bool* state_of_signals, int num_of_input_signals)
{
    for (int i = 0; i < num_of_input_signals; i++)
    {
        state_of_signals[i] = false;
    }
}

// zmienia tablice z konfiguracja sygnalów na nastepna
// zwraca false jesli byla to juz ostatnia
bool next_state_of_signals(bool* state_of_signals, int num_of_input_signals)
{
    for (int i = num_of_input_signals - 1; i >= 0; i--)
    {
        if (state_of_signals[i] == true)
        {
            state_of_signals[i] = false;
        }
        else
        {
            state_of_signals[i] = true;
            return true;
        }
    }
    return false;
}

// sortowanie topologiczne wierzcholków grafu
// zwraca posortowane wierzcholki w wektorze sorted_vertices
void toposort(vector<int>* sorted_vertices)
{
    queue <int> zero_degree_vertices;
    int vertex_degrees[num_of_gates];
    for (int i = 0; i < num_of_gates; i++)
    {
        vertex_degrees[i] = graph[i].second.size();
    }
    for (int i = 0; i < num_of_gates; i++)
    {
        if (vertex_degrees[i] == 0)
        {
            // fprintf(stderr, "%d jest wierzcholkiem koncowym\n", i);
            zero_degree_vertices.push(i);
        }
    }

    while (!zero_degree_vertices.empty())
    {
        int v = zero_degree_vertices.front();
        zero_degree_vertices.pop();
        sorted_vertices->push_back(v);
        int in_degree = graph[v].first.size();
        for (int i = 0; i < in_degree; i++)
        {
            vertex_degrees[graph[v].first[i]]--;
            if (vertex_degrees[graph[v].first[i]] == 0)
            {
                zero_degree_vertices.push(graph[v].first[i]);
            }
        }
    }
    reverse((*sorted_vertices).begin(), (*sorted_vertices).end());
}

// dla danej bramki oraz danej konfiguracji jej wejsciowych stanów
// wypelnia tablice prawdy dla jej wyjscia
void complete(int gate_number)
{
    //fprintf(stderr, "wypelniamy dla bramki nr %d\n", gate_number);
    int num_of_inputs = gates[gate_number].first.size();
    if (gate_types[gate_number] == NOT)
    {
        truth_output[gates[gate_number].second] = !truth_output[gates[gate_number].first[0]];
    }
    else if (gate_types[gate_number] == XOR)
    {
        truth_output[gates[gate_number].second] = (truth_output[gates[gate_number].first[0]] != truth_output[gates[gate_number].first[1]]);
    }
    else if (gate_types[gate_number] == AND)
    {
        truth_output[gates[gate_number].second] = true;
        for (int i = 0; i < num_of_inputs; i++)
        {
            if (!truth_output[gates[gate_number].first[i]])
            {
                truth_output[gates[gate_number].second] = false;
                break;
            }
        }
    }
    else if (gate_types[gate_number] == NAND)
    {
        truth_output[gates[gate_number].second] = false;
        for (int i = 0; i < num_of_inputs; i++)
        {
            if (!truth_output[gates[gate_number].first[i]])
            {
                truth_output[gates[gate_number].second] = true;
                break;
            }
        }
    }
    else if (gate_types[gate_number] == OR)
    {
        truth_output[gates[gate_number].second] = false;
        for (int i = 0; i < num_of_inputs; i++)
        {
            if (truth_output[gates[gate_number].first[i]])
            {
                truth_output[gates[gate_number].second] = true;
                break;
            }
        }
    }
    else if (gate_types[gate_number] == NOR)
    {
        truth_output[gates[gate_number].second] = true;
        for (int i = 0; i < num_of_inputs; i++)
        {
            if (truth_output[gates[gate_number].first[i]])
            {
                truth_output[gates[gate_number].second] = false;
                break;
            }
        }
    }
}

// oblicza jedna linijke tablicy prawdy dla wszystkich bramek, na podstawie poczatkowej konfiguracji
// oraz wektora posortowanych wierzcholków
void compute_output(vector<int>* sorted_gates)
{
    for (int i = 0; i < num_of_gates; i++)
    {
        complete((*sorted_gates)[i]);
    }
}

// wypisuje otrzymana liijke tablicy prawdy
void print_truth_output()
{
    for (auto i = truth_output.begin(); i != truth_output.end(); ++i)
    {
        if (i->second)
        {
            cout << "1";
        }
        else
        {
            cout << "0";
        }
    }
    cout << "\n";
}

// wypelnia znane od poczatku wartosci tablicy truth_output (te zakladane w danej linijce)
void complete_input_values(int num_of_input_signals, vector<int>* input_signals, bool* state_of_signals)
{
    for (auto i = truth_output.begin(); i != truth_output.end(); ++i)
    {
        i->second = false; //to chyba nie jest konieczne ale dla porzadku, moge to potem wywalic
    }
    for (int i = 0; i < num_of_input_signals; i++)
    {
        truth_output[(*input_signals)[i]] = state_of_signals[i];
    }
}

// tworzy i wypisuje tablice prawdy ukladu
void create_truth_table()
{
    // sprawdza, które sygnaly sa tylko wejsciowe
    vector<int> input_signals;
    detect_input_signals(&input_signals);
    unsigned int num_of_input_signals = input_signals.size();
    sort(input_signals.begin(), input_signals.end());
    /* fprintf(stderr, "input signals to:\n");
    for (int i = 0; i < num_of_input_signals; i++)
    {
        fprintf(stderr, "%d\n", input_signals[i]);
    } // wypisanie tylko wejsciowych sygnalów, pomocnicze */
    // tablica z obecnie sprawdzana konfiguracja sygnalow
    bool state_of_signals[num_of_input_signals];
    initialize_state_of_signals(state_of_signals, num_of_input_signals);

    // przejscie po wszystkich konfiguracjach
    vector<int> sorted_gates;
    toposort(&sorted_gates);
    do
    {
        complete_input_values(num_of_input_signals, &input_signals, state_of_signals);
        compute_output(&sorted_gates);
        print_truth_output();
    } while (next_state_of_signals(state_of_signals, num_of_input_signals));
    /* fprintf(stderr, "kolejnosc sygnalow w truth table:\n");
    for (auto i = truth_output.begin(); i != truth_output.end(); ++i)
    {
        // fprintf(stderr, "w pentli\n");
        fprintf(stderr, "%d\n", i->first);
    } // pomocnicze wypisanie, w jakiej kolejnosci rozwazamy sygnaly w truth table (powino byc w rosnacej) */
}


int main() {
    // wczytuje do (globalnych) wektora gates kolejne bramki oraz do wektora gate_types ich typy
    // z numeracją od 0 w kolejności pojawiania się na wejsciu
    // wypisuje bledy o zlym formacie linii oraz o powtarzajacych sie strumieniach wyjscia
    // zwraca informacje czy nalezy dalej przetwarzac uklad
    bool is_correct_data = read_input();
    num_of_gates = gates.size(); // to sie potem wlozy do jakiejs funkcji
    if (is_correct_data)
    {
        create_graph();
        // fprintf(stderr, "tu dziala\n");
        if (is_cyclic())
        {
            fprintf(stderr, "Error: sequential logic analysis has not yet been implemented.\n");
        }
        // fprintf(stderr, "tu dziala\n");
        else
        {
            create_truth_table();
        }
    }

    return 0;
}
