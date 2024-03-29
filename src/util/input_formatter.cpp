#include <fstream>
#include <iostream>
using namespace std;

bool
input_formatter(string src_name, string dst_name) {
    ifstream rf(src_name);
    ofstream wf(dst_name);
    if (!rf) {
        cout << "can't open file" << endl;
        return false;
    }
    // 0351
    string temp, garbage;
    while (rf >> temp) {
        wf << temp << " ";
        if (temp[temp.length() - 1] == ';') {
            wf << "\n";
        }
        if (temp == "and" || temp == "nor" || temp == "xnor" || temp == "or" ||
            temp == "not" || temp == "nand" || temp == "xor" || temp == "buf") {
            rf >> garbage;
        }
    }
    return true;
}
