#ifndef CIRMGR_H
#define CIRMGR_H

#include "sat.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
// #include "satMgr.h"

enum preprocess
{
    support,
    outputUnateness,
    inputUnateness
};
class variable
{
    public:
        variable(char name, Var sub1, Var sub2)
            : _name(name), _sub1(sub1), _sub2(sub2), _busIndex(0), _busSize(0),_suppSize(0), _inputUnateNum_p(0),_inputUnateNum_n(0),_outputUnateNum(0), _outputGroupingNum(0){
        }
        ~variable() {}
        char   getName() const { return _name; }
        Var    getSub1() const { return _sub1; }
        Var    getSub2() const { return _sub2; }
        Var    getVar() const { return _var; }
        Var    getVar2() const { return _var2; }
        Var    getVar3() const { return _var3; }
        size_t busIndex() { return _busIndex; }
        size_t busSize() { return _busSize; }
        size_t suppSize() { return _suppSize; }
        size_t outputUnateNum() { return _outputUnateNum; }
        size_t inputUnateNum_p() { return _inputUnateNum_p; }
        size_t inputUnateNum_n() { return _inputUnateNum_n; }
        size_t outputGroupingNum() { return _outputGroupingNum; }
        vector<variable*>& funcSupp(){return _funcSupp;}
        void   setName(const char& v) { _name = v; }
        void   setSub1(const Var& v) { _sub1 = v; }
        void   setSub2(const Var& v) { _sub2 = v; }
        void   setVar(const Var& v) { _var = v; }
        void   setVar2(const Var& v) { _var2 = v; }
        void   setVar3(const Var& v) { _var3 = v; }
        void   setBusIndex(const size_t _i) { _busIndex = _i; }
        void   setBusSize(const size_t _s) { _busSize = _s; }
        void   setSuppSize(const size_t _s) { _suppSize = _s; }
        void   setInputUnateNum_p(const size_t _s) { _inputUnateNum_p = _s; }
        void   setInputUnateNum_n(const size_t _s) { _inputUnateNum_n = _s; }
        void   setOutputUnateNum(const size_t _s) { _outputUnateNum = _s; }
        void   setOutputGroupingNum(const size_t _s) { _outputGroupingNum = _s; }
        void   addInputUnateNum_p() { ++_inputUnateNum_p; }
        void   addInputUnateNum_n() { ++_inputUnateNum_n; }
    private:
        char   _name;
        Var    _sub1;
        Var    _sub2;
        Var    _var;      // for solver1
        Var    _var2;     // for miter solver
        Var    _var3;     // for verification solver
        size_t _busIndex; // bus_index denotes the position of the bus it
                          // belongs to in bus vector
        size_t _busSize;  // bus_size denotes the number of the elements in the
                          // bus it belongs to
        size_t _suppSize; // currently : functional support
        // size_t _inputUnateNum;  // how many outputs are unate w.r.t this PI 
        size_t _inputUnateNum_p;  // how many outputs are positive unate w.r.t this PI 
        size_t _inputUnateNum_n;  // how many outputs are negative unate w.r.t this PI 
        size_t _outputUnateNum; // the number of output unate variables
        size_t _outputGroupingNum; // index of the output grouping
        vector<variable*> _funcSupp;    // record the functional support of PO
        // Var _aigVar; do we need this??
};

class Cmdr // commander variable encoding for constraint 2 and 3
{
    public:
        Cmdr(Var v, bool is_var) : _var(v), isVar(is_var) {}
        Cmdr() {}
        ~Cmdr() {}
        Var           getVar() const { return _var; }
        void          setVar(const Var& v) { _var = v; }
        bool          getIsVar() const { return isVar; }
        vector<Cmdr*> Subords;

    private:
        Var  _var;
        bool isVar; // 1: variable, 0: Subords
};
class Bus
{
    public:
        // Bus (bool I, bool C, size_t N): isInput(I), isCkt1(C), portNum(N) {}
        Bus (){}
        ~Bus(){}
        bool            getIsInput() const {return isInput;}
        bool            getIsCkt1 () const {return isCkt1 ;}
        size_t          getPortNum() const {return portNum;}
        // vector<size_t>  getIndexes() const {return indexes;}
        // vector<string>  getNames  () const {return names  ;}
        void            setIsInput (const bool v)  {isInput = v;}
        void            setIsCkt1  (const bool v)  {isCkt1  = v;}
        void            setPortNum (const size_t v){portNum = v;}
        void            insertIndex(const size_t v){indexes.push_back(v);}
        void            insertName (const string v){names.push_back(v);}
        vector<size_t> indexes;
        vector<string> names;
    private:
        bool isInput;
        bool isCkt1;
        size_t portNum;
};

class CirMgr
{

    public:
        friend class SatMgr;
        friend class SolverMgr;
        CirMgr(){};
        ~CirMgr(){};
        void reset();
        void readAAG();
        void readMAP();
        void readCNF(SatSolver& s_miter, SatSolver& s_verifier);
        // void readBus(string &);
        void readPreporcess(preprocess _p);
        void readInputUnateness();  // mix positive / negative unate together
        void outputGrouping();
        // void readSupp();
        // void readUnate();
        void readBus_class(string&); // also calculate valid bus match permutation

    private:
        int inputNum_ckt1, outputNum_ckt1, inputNum_ckt2, outputNum_ckt2;
        // portnum: i0, i1.., o0, o1..
        // portname: a,  b... , g, f..
        vector<string> portnum_ckt1, portname_ckt1, portnum_ckt2, portname_ckt2;
        vector<pair<string, string>> // first = i0, i1 ...; second = name
            portNameNumpairs_ckt1,   // for input, the index = the corresponding
                                     // element's index in x(y)
            portNameNumpairs_ckt2;   // for output, the index in
                                   // portNameNumpairs_ckt should substract the
                                   // inputNum_ckti to obtain the corresponding
                                   // index in f(g)
        vector<vector<variable*>> MI, MO;
        vector<variable*>         x, y, f, g; // sub2 = -1
        bool* exist_x;
        bool* exist_y;
        bool* exist_f;
        bool* exist_g; 
        vector<Bus*>              bus_ckt1_input, bus_ckt1_output, bus_ckt2_input, bus_ckt2_output;
        vector<vector<Bus*>>      valid_busMatch_ckt2_input, valid_busMatch_ckt2_output; // permute ckt2's bus to match ckt1's bus
        vector<vector<string>>    bus_list_ckt1,
            bus_list_ckt2; // record the bus (including input and output )info
                           // of circuit1 & circuit2
        // umap[real CNF_VAR] -> real input/output port name
        // those intervariable that doesn't exist in x/y/f/g
        // umapInterVar_ckti_veri is for verifySolver
        unordered_map<int, string> umapNum_ckt1, umapName_ckt1, umapNum_ckt2,
            umapName_ckt2;
        unordered_map<int, Var> umapInterVar_ckt1, umapInterVar_ckt1_veri,
            umapInterVar_ckt2, umapInterVar_ckt2_veri;
        vector<Cmdr*> Cmdr_level;

        unordered_map<string, int> u_name_index_ckt1, u_name_index_ckt2; // index can be used for x, y, f, g immediately
        unordered_map<string, bool> u_name_isInput_ckt1, u_name_isInput_ckt2;

        // helper function

        void _readbus(
            ifstream& fbus, vector<vector<string>>& bus_list_ckt,
            bool _isCkt1); // _ckti = 1 denotes is reading ckt1; = 0 -> ckt2
        void _readPreprocess(ifstream& fPreprocess, bool _isCkt1, preprocess _p);// _ckti = 1 denotes is reading ckt1; = 0 -> ckt2
        void _readInputUnateness(ifstream& , bool _isCkt1);
        static bool _outputsorting(variable* a, variable* b);
        // void _readSupp(
        //     ifstream& fbus,
        //     bool _isCkt1); 
        // void _readUnate(
        //     ifstream& funate,
        //     bool _isCkt1); // _ckti = 1 denotes is reading ckt1; = 0 -> ckt2
             
        vector<vector<Bus*>> permute(vector<Bus*>& num){
            vector<vector<Bus*>> ans;
            permutation(num, 0, ans);
            return ans;
        }
        int cnt = 0;
        void permutation(vector<Bus*>& num, int begin, vector<vector<Bus*>>& ans){
            vector<Bus*>& b = num[0]->getIsInput() ? bus_ckt1_input : bus_ckt1_output;
            // cout << begin << endl;
            if(begin >= num.size()){
                ans.push_back(num);
                cout << "valid permu" << cnt++ << endl;
                return;
            }
            for(int i=begin; i< num.size(); i++){
                swap(num[begin], num[i]);
                if(num[begin]->getPortNum() >= b[begin]->getPortNum() && num[i]->getPortNum() >= b[i]->getPortNum())
                    permutation(num, begin+1, ans);
                swap(num[begin], num[i]);
            }
        }
};

#endif // CIRMGR_H