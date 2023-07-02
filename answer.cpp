#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

// Structure to represent a gate in the circuit
struct Gate {
    string type; // gate type &,|,^ or ~
    string input1;
    string input2;
};

// Function to evaluate the circuit and get the output value
bool evaluateCircuit(const vector<pair<string,Gate>>& circuit, const unordered_map<string, bool>& inputs,const string& faultNode,bool exp) {
    unordered_map<string, int> values;
    for(const auto& it : inputs){
        // storing input values available at start to ease the calculation
        values[it.first]=it.second; 
    }
    for (const auto& gate : circuit) {
        const string& name = gate.first; // output of the operation for corresponding equation
        const Gate& g = gate.second; //gate type
        values[faultNode]=exp;
        if(name==faultNode) continue; // if name is faultnode then value is already stored in previous step
        else if (g.type == "&") {
            values[name] = values[g.input1] & values[g.input2];
        } else if (g.type == "|") {
            values[name] = values[g.input1] | values[g.input2];
        } else if (g.type == "~") {
            values[name] = !values[g.input1];
        } else if (g.type == "^") {
            values[name] = values[g.input1] ^ values[g.input2];
        }
        
    }

    return values["Z"]; // returning final output
}

// splitting equation and getting it in form of Gate class
void eval_circuit(string& a,string& b,vector<pair<string,Gate>>& circuit){
    int n=a.length();
    string t,j="",k="";
    int i=0;
    while(i<n){
        if(a[i]==' ') {
            i++;
            continue;
        }
        else if(a[i]=='&' || a[i]=='^' || a[i]=='~' || a[i]=='|'){
            t=a[i];
            i++;
        }
        else{
            string s="";
            string temp=a.substr(i);
            int space_pos=temp.find(' ');
            s=a.substr(i,space_pos);
            if(j=="") j=s;
            else k=s;
            i+=s.length();
        }
    }
    circuit.push_back({b,{t,j,k}});
}

// Function to parse the circuit file and build the circuit representation
vector<pair<string,Gate>> parseCircuitFile(const string& circuitFile) {
    vector<pair<string,Gate>> circuit;

    ifstream file(circuitFile);
    if (file.is_open()) {
        string line;
        // getting each line in the input file
        while (getline(file, line)) {
            // finding position of =
            size_t equalsPos = line.find('=');
            // cheching if it is present in string , if present then call eval_circuit
            if (equalsPos != string::npos) {
                string gateOutput = line.substr(0, equalsPos-1); // previous than = is output
                
                string gateExpression = line.substr(equalsPos + 2);// after = is the expression containing logic gates
                
                eval_circuit(gateExpression,gateOutput,circuit);// storing outputs and their Gate(class) representation
                
            }
        }
        file.close();
    } else {
        cout << "Failed to open circuit file." << endl;
    }

    return circuit;
}

// Function to write the test case to the output file
void writeTestCase(const vector<bool>& inputs, bool output, const string& outputFile) {
    ofstream file(outputFile, ios::app);
    if (file.is_open()) {
        file<<"[A, B, C, D] = ";
        file << "[";
        for (size_t i = 0; i < inputs.size(); ++i) {
            file << inputs[i];
            if (i < inputs.size() - 1) {
                file << ", ";
            }
        }
        file << "], Z = " << output << endl;
        file.close();
    } else {
        cout << "Failed to write to output file." << endl;
    }
}

// Function to identify the input vector required to detect a fault
void identifyFault(const string& circuitFile, const string& faultNode, const string& faultType, const string& outputFile) {
    vector<pair<string,Gate>> circuit = parseCircuitFile(circuitFile);
    // initialising inputs
    unordered_map<string, bool> inputs = {
        { "A", false },
        { "B", false },
        { "C", false },
        { "D", false }
    };

    // Determine the expected output for the fault node based on the fault type
    bool expectedOutput;
    if (faultType == "SA0") {
        expectedOutput = false;
    } else if (faultType == "SA1") {
        expectedOutput = true;
    } else {
        cout << "Invalid fault type. Please specify 'SA0' or 'SA1'." << endl;
        return;
    }
    vector<bool> inputVector;

    // Iterate through all possible input vectors
    for (size_t i = 0; i < 16; ++i) {
        // Set the input vector values based on the binary representation of 'i'
        inputs["A"] = (i & 1) != 0;
        inputs["B"] = (i & 2) != 0;
        inputs["C"] = (i & 4) != 0;
        inputs["D"] = (i & 8) != 0;
        
        // Evaluate the circuit with the current input vector
        bool output = evaluateCircuit(circuit, inputs,faultNode,expectedOutput);

        // Add the input vector to the list
        inputVector.push_back(inputs["A"]);
        inputVector.push_back(inputs["B"]);
        inputVector.push_back(inputs["C"]);
        inputVector.push_back(inputs["D"]);

        // Write the input vector and expected output to the output file
        writeTestCase(inputVector, output, outputFile);

        // Clear the input vector for the next iteration
        inputVector.clear();
    }
}

int main() {
    // defining inputs, we can take them as input using cin
    string circuitFile = "circuit.txt";
    string faultNode = "net_f";
    string faultType = "SA1";
    string outputFile = "output.txt";

    identifyFault(circuitFile, faultNode, faultType, outputFile);

    return 0;
}

