#include <iostream>
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

#define XOR_LINE_SIZE 4
#define NOT_LINE_SIZE 3
#define MIN_LINE_SIZE 4

using namespace std;

// <strumienie wejscia, wyjscia>
using gate = pair<vector<int>, int>;
// bramki w kolejnosci pojawiania sie na wejsciu
vector<gate> gates;

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

int main() {
    // wczytuje do (globalnych) wektora gates kolejne bramki oraz do wektora gate_types ich typy
    // z numeracją od 0 w kolejności pojawiania się na wejsciu
    // wypisuje bledy o zlym formacie linii oraz o powtarzajacych sie strumieniach wyjscia
    // zwraca informacje czy nalezy dalej przetwarzac uklad
    bool is_correct_data = read_input();
}
