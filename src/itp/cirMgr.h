#ifndef CIRMGR_H
#define CIRMGR_H

#include "sat.h"
#include <string>
#include <unordered_map>
#include <vector>
// #include "satMgr.h"

class variable
{
    public:
        variable(char name, Var sub1, Var sub2)
            : _name(name), _sub1(sub1), _sub2(sub2), _busIndex(0), _busSize(0) {
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
        void   setName(const char& v) { _name = v; }
        void   setSub1(const Var& v) { _sub1 = v; }
        void   setSub2(const Var& v) { _sub2 = v; }
        void   setVar(const Var& v) { _var = v; }
        void   setVar2(const Var& v) { _var2 = v; }
        void   setVar3(const Var& v) { _var3 = v; }
        void   setBusIndex(const size_t _i) { _busIndex = _i; }
        void   setBusSize(const size_t _s) { _busSize = _s; }

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

        // Var _aigVar; do we need this??
};

class Cmdr // commander variable encoding for constraint 2 and 3
{
    public:
        Cmdr (Var v, bool is_var):_var(v), isVar(is_var) {}
        Cmdr () {}
        ~Cmdr() {}
        Var     getVar() const { return _var; }
        void    setVar(const Var& v) { _var = v; }
        bool    getIsVar() const { return isVar;}
        vector<Cmdr*> Subords;
        
    private:
        Var     _var;
        bool    isVar; // 1: variable, 0: Subords
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
        void readBus();

    private:
        int inputNum_ckt1, outputNum_ckt1, inputNum_ckt2, outputNum_ckt2;
        // portnum: i0, i1.., o0, o1..
        // portname: a,  b... , g, f..
        vector<string> portnum_ckt1, portname_ckt1, portnum_ckt2, portname_ckt2;
        vector<pair<string, string>>     // first = i0, i1 ...; second = name
            portNameNumpairs_ckt1, // for input, the index = the corresponding
                                   // element's index in x(y)
            portNameNumpairs_ckt2; // for output, the index in
                                   // portNameNumpairs_ckt should substract the
                                   // inputNum_ckti to obtain the corresponding
                                   // index in f(g)
        vector<vector<variable*>> MI, MO;
        vector<variable*>         x, y, f, g; // sub2 = -1
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
        vector<Cmdr* > Cmdr_level;

        // helper function

        void _readbus(
            ifstream& fbus, vector<vector<string>>& bus_list_ckt,
            bool _isCkt1); // _ckti = 1 denotes is reading ckt1; = 0 -> ckt2
};

#endif // CIRMGR_H