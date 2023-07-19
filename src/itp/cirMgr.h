#ifndef CIRMGR_H
#define CIRMGR_H

#include "sat.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
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
        variable(char type, Var sub1, Var sub2)
            : _type(type), _sub1(sub1), _sub2(sub2), _busIndex(-1), _busSize(0),_suppSize(0), _inputUnateNum_p(0),_inputUnateNum_n(0),_outputUnateNum(0), _outputGroupingNum(0), _inputUnates(""){
        }
        ~variable() {}

        vector<variable*> _funcSupp_PI; // record those PO that this PI affect
        vector<variable*> _funcSupp;    // record the functional support of PO
        vector<string>    candidates;   // record valid match candidate (you can use u_name_index_ckt1, u_name_index_ckt2 and x, y, f, g to get its variable)
        vector<unordered_set<int>> symmetric_set; // record the symmetric set (only in output variable)

        char                                gettype() const { return _type; }
        string                              getname() const { return _name; }
        Var                                 getSub2() const { return _sub2; }
        Var                                 getVar() const { return _var; }
        Var                                 getSub1() const { return _sub1; }  
        Var                                 getVar2() const { return _var2; }
        Var                                 getVar3() const { return _var3; }
        int                                 busIndex() { return _busIndex; }
        size_t                              busSize() { return _busSize; }
        size_t                              suppSize() { return _suppSize; }
        unordered_map<size_t, size_t>&      suppBus() { return _suppBus;}
        vector<pair<size_t, size_t>>*&      suppBus_distribution(){return _suppBus_distribution;}
        size_t                              outputUnateNum() { return _outputUnateNum; }
        string                              inputUnates() { return _inputUnates; }
        size_t                              inputUnateNum_p() { return _inputUnateNum_p; }
        size_t                              inputUnateNum_n() { return _inputUnateNum_n; }
        size_t                              outputGroupingNum() { return _outputGroupingNum; }
        void                                settype(const char& v) { _type = v; }
        void                                setname(const string& name) { _name = name; }
        void                                setSub1(const Var& v) { _sub1 = v; }
        void                                setSub2(const Var& v) { _sub2 = v; }
        void                                setVar(const Var& v) { _var = v; }
        void                                setVar2(const Var& v) { _var2 = v; }
        void                                setVar3(const Var& v) { _var3 = v; }
        void                                setBusIndex(const int _i) { _busIndex = _i; }
        void                                setBusSize(const size_t _s) { _busSize = _s; }
        void                                setSuppSize(const size_t _s) { _suppSize = _s; }
        void                                setInputUnateNum_p(const size_t _s) { _inputUnateNum_p = _s; }
        void                                setInputUnateNum_n(const size_t _s) { _inputUnateNum_n = _s; }
        void                                setOutputUnateNum(const size_t _s) { _outputUnateNum = _s; }
        void                                setOutputGroupingNum(const size_t _s) { _outputGroupingNum = _s; }
        void                                setInputUnates(const string _s) { _inputUnates = _s; }
        void                                addInputUnateNum_p() { ++_inputUnateNum_p; }
        void                                addInputUnateNum_n() { ++_inputUnateNum_n; }

    private:
        char    _type;
        string  _name;
        Var     _sub1;     // _sub1 - 1 == the index in x , y, f, g
        Var     _sub2;     // if == PI or PO -> _sub2 = -1
        Var     _var;      // for solver1
        Var     _var2;     // for miter solver
        Var     _var3;     // for verification solver
        int     _busIndex; // bus_index denotes the position of the bus it
                          // belongs to in bus vector
        size_t _busSize;  // bus_size denotes the number of the elements in the
                          // bus it belongs to
        size_t _suppSize; // currently : functional support
        // size_t _inputUnateNum;  // how many outputs are unate w.r.t this PI 
        size_t _inputUnateNum_p;  // how many outputs are positive unate w.r.t this PI 
        size_t _inputUnateNum_n;  // how many outputs are negative unate w.r.t this PI 
        size_t _outputUnateNum; // the number of output unate variables(i.e., the number of input that makes this output unate)
        string _inputUnates; // the detail input unate of this output
        size_t _outputGroupingNum; // index of the output grouping
        // collect those buses that this output's support inputs lie in    
        //_SuppBus[i].first = the bus index; .second = number of inputs in that bus
        unordered_map<size_t, size_t> _suppBus;
        vector<pair<size_t, size_t>>*  _suppBus_distribution;  // the sorted ( non-increasing order )distribution of each bus in _suppBus
        // unsigned long long _suppBus; 
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
        Bus ():_unionSupportSize(0), _outputUnatenessNum(0), _busIndex(-1){}
        Bus (const size_t _portidx, const int _numBus):_unionSupportSize(0), _outputUnatenessNum(0), _busIndex(_portidx), _busNum(_numBus){
            _inputUnatenessNum = 0;
            _unionInputSupportSize = 0;
            _bus_matching = new bool[_numBus];
            for(size_t i = 0; i < _numBus; ++i)
                _bus_matching[i] = 0;
        }
        ~Bus(){delete []_bus_matching; _bus_matching = 0;}
        bool            getIsInput() const {return isInput;}
        bool            getIsCkt1 () const {return isCkt1 ;}
        size_t          getPortNum() const {return portNum;}
        size_t          unionSupportSize() const {return _unionSupportSize;}
        size_t          unionInputSupportSize() const {return _unionInputSupportSize;}
        size_t          outputUnatenessNum() const {return _outputUnatenessNum;}
        size_t          inputUnatenessNum() const {return _inputUnatenessNum;}
        size_t          busIndex() const {return _busIndex;}
        size_t          busNum() const {return _busNum;}
        bool*           busMatching() const {return _bus_matching;}
        unordered_map<variable*, bool>& unionSupport(){return _unionSupport;}
        unordered_map<variable*, bool>& unionInputSupport(){return _unionInputSupport;}
        // vector<size_t>  getIndexes() const {return indexes;}
        // vector<string>  getNames  () const {return names  ;}
        void            setIsInput (const bool v)  {isInput = v;}
        void            setIsCkt1  (const bool v)  {isCkt1  = v;}
        void            setPortNum (const size_t v){portNum = v;}
        void            setUnionSupportSize(const size_t v){_unionSupportSize = v;}
        void            setUnionInputSupportSize(const size_t v){_unionInputSupportSize = v;}
        void            setOutputUnatenessNum(const size_t v){_outputUnatenessNum = v;}
        void            setBusIndex(const size_t v){_busIndex = v;}
        void            addOutputUnatenessNum(const size_t v){_outputUnatenessNum += v;}
        void            addInputUnatenessNum(const size_t v){_inputUnatenessNum += v;}
        void            addUnionSupportSize(const size_t v){_unionSupportSize += v;}
        void            addInputUnionSupportSize(const size_t v){_unionInputSupportSize += v;}
        void            insertIndex(const int v){indexes.push_back(v);}
        void            insertName (const string v){names.push_back(v);}
        vector<int> indexes;
        vector<string> names;
    private:
        bool isInput;
        bool isCkt1;
        size_t _busIndex; // each input bus (output bus) should have a unique busIndex (to close specific pair of bus matching)
        size_t _busNum; // number of bus in this circuit
        size_t portNum;
        size_t _unionSupportSize; // the input support size(i.e., the number of inputs this output bus support) of this bus' outputs
        size_t _unionInputSupportSize; // the output support size(i.e., the number of outputs this input bus support) of this bus' inputs
        size_t _outputUnatenessNum; // the sum of the input unateness of a output bus
        size_t _inputUnatenessNum; // the sum of the output unateness of a input bus
        unordered_map<variable*, bool> _unionSupport;   // collect the union inputs that is supported by this output bus
        unordered_map<variable*, bool> _unionInputSupport;  // collect the union outputs that is supported by this input bus
        // unordered_map<variable*, bool> _unionInputUnateness;    //  the union input unateness of ouptut bus
        // boolean array to indicate which buses can't this bus match with 
        // e.g., if bus i can't match with bus j, then bus i 's bus_matching[j] == 1 && bus j 's bus_matching[i] == 1
        bool* _bus_matching; 
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
        void recordPIsupportPO(); // record those POs affected by each PI, record in each PI's _funcSupp_PI
        void readInputUnateness();  // mix positive / negative unate together
        void readSymmetric();
        void outputGrouping();
        void busSupportUnion(); // collect the union support size of output bus and close the infeasible bus matching
        void busInputSupportUnion();
        // void busInputUnatenessUnion(); // collect the union input unateness of ouptut bus
        // collect the union input unateness of output bus and close the infeasible bus matching
        // to close output bus matching
        void busOutputUnateness();
        void busInputUnateness();
        // void readSupp();
        // void readUnate();
        void readBus_class(SatSolver& s, string& inputfilename); 
        void feasibleBusMatching(); // calculate valid bus match permutation
        void supportBusClassification(); // calculate the bus distribution of each output's inputs
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
        // vector<vector<Var>> MO_no_pos_neg; // MO but without positive/negative match, half the size of MO
        vector<variable*>         x, y, f, g; // sub2 = -1
        vector<vector<bool>>      MI_valid, MO_valid; // record valid match (without +-)(true: can match, false: invalid match) (matrix version of candidates) (including constant)
        vector<vector<Var>>       MI_valid_Var, MO_valid_Var; // record valid match (without +-)(true: can match, false: invalid match) (matrix version of candidates) (including constant)
        vector<vector<Var>>       MIbus_Var, MObus_Var;       // record bus match Var, bus_ckt1_input * bus_ckt2_input
        vector<vector<bool>>      MIbus_valid, MObus_valid;   // record bus valid
        bool* exist_x;
        bool* exist_y;
        bool* exist_f;
        bool* exist_g; 
        vector<Bus*>              bus_ckt1_input, bus_ckt1_output, bus_ckt2_input, bus_ckt2_output;
        vector<vector<Bus*>>      valid_busMatch_ckt2_input, valid_busMatch_ckt2_output; // don't use anymore after using addCandidateBusConstraint // permute ckt2's bus to match ckt1's bus
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
        unordered_map<string, int> u_name_busIndex_input_ckt1, u_name_busIndex_output_ckt1, u_name_busIndex_input_ckt2, u_name_busIndex_output_ckt2; // index for bus_ckt1_input, bus_ckt2_input

        // helper function

        void _readbus(
            ifstream& fbus, vector<vector<string>>& bus_list_ckt,
            bool _isCkt1); // _ckti = 1 denotes is reading ckt1; = 0 -> ckt2
        void _readPreprocess(ifstream& fPreprocess, bool _isCkt1, preprocess _p);// _ckti = 1 denotes is reading ckt1; = 0 -> ckt2
        void _readInputUnateness(ifstream& , bool _isCkt1);
        void _busOutputUnateness(bool _isCkt1); // collect the union input unateness of output bus
        void _busInputUnateness(bool _isCkt1); // collect the union output unateness of input bus
        void _busSupportUnion(bool _isCkt1); // collect the union support size of output bus
        void _busInputSupportUnion(bool _isCkt1); // collect the union support size of input bus
        static bool _outputsorting(variable* a, variable* b);
        // void _readSupp(
        //     ifstream& fbus,
        //     bool _isCkt1); 
        // void _readUnate(
        //     ifstream& funate,
        //     bool _isCkt1); // _ckti = 1 denotes is reading ckt1; = 0 -> ckt2
             
        vector<vector<Bus*>> permute(vector<Bus*>& num){
            vector<vector<Bus*>> ans;
            int cnt = 0;
            permutation(num, 0, ans, cnt);
            return ans;
        }
        void permutation(vector<Bus*>& num, int begin, vector<vector<Bus*>>& ans, int& cnt){
            vector<Bus*>& b = num[0]->getIsInput() ? bus_ckt1_input : bus_ckt1_output;
            // cout << begin << endl;
            if(begin >= num.size()){
                ans.push_back(num);
                // cout << "valid permu" << cnt++ << endl;
                return;
            }
            for(int i=begin; i< num.size(); i++){
                swap(num[begin], num[i]);
                if(num[begin]->getPortNum() >= b[begin]->getPortNum() && num[i]->getPortNum() >= b[i]->getPortNum())
                    permutation(num, begin+1, ans, cnt);
                swap(num[begin], num[i]);
            }
        }
        void printMIMO_valid(){
            cout << "\nMI_valid: " << endl << "\t";
            for (int i = 0; i < x.size(); i++){
                cout << x[i]->getname() << "\t";
            }
            cout << endl;
            for (int j = 0; j < MI_valid[0].size(); j++){
                cout << y[j]->getname() << "\t";
                for (int i = 0; i < MI_valid.size(); i++){
                    cout << MI_valid[i][j] << "\t";
                }
                cout << endl;
            }

            cout << "\nMO_valid: " << endl << "\t";
            for (int i = 0; i < f.size(); i++){
                cout << f[i]->getname() << "\t";
            }
            cout << endl;
            for (int j = 0; j < MO_valid[0].size(); j++){
                cout << g[j]->getname() << "\t";
                for (int i = 0; i < MO_valid.size(); i++){
                    cout << MO_valid[i][j] << "\t";
                }
                cout << endl;
            }
        }
        void printMIMO_valid_notab(){
            cout << "\nMI_valid: " << endl;
            // for (int i = 0; i < x.size(); i++){
            //     cout << x[i]->getname() << "\t";
            // }
            cout << endl;
            for (int j = 0; j < MI_valid[0].size(); j++){
                cout << y[j]->getname() << "\t";
                for (int i = 0; i < MI_valid.size(); i++){
                    cout << MI_valid[i][j];
                }
                cout << endl;
            }

            cout << "\nMO_valid: " << endl;
            // for (int i = 0; i < f.size(); i++){
            //     cout << f[i]->getname() << "\t";
            // }
            cout << endl;
            for (int j = 0; j < MO_valid[0].size(); j++){
                cout << g[j]->getname() << "\t";
                for (int i = 0; i < MO_valid.size(); i++){
                    cout << MO_valid[i][j];
                }
                cout << endl;
            }
        }
        void printBusIndexes(){
                        //test cirmgr.bus_ckt1_output[i]->indexes
            for(int i = 0; i < bus_ckt1_input.size(); i++){
                cout << "bus_ckt1_input[" << i << "]: ";
                for(int j = 0; j < bus_ckt1_input[i]->indexes.size(); j++){
                    cout << bus_ckt1_input[i]->indexes[j] << " ";
                }
                cout << endl;
            }
            for(int i = 0; i < bus_ckt1_output.size(); i++){
                cout << "bus_ckt1_output[" << i << "]: ";
                for(int j = 0; j < bus_ckt1_output[i]->indexes.size(); j++){
                    cout << bus_ckt1_output[i]->indexes[j] << " ";
                }
                cout << endl;
            }
            for(int i = 0; i < bus_ckt2_input.size(); i++){
                cout << "bus_ckt2_input[" << i << "]: ";
                for(int j = 0; j < bus_ckt2_input[i]->indexes.size(); j++){
                    cout << bus_ckt2_input[i]->indexes[j] << " ";
                }
                cout << endl;
            }
            for(int i = 0; i < bus_ckt2_output.size(); i++){
                cout << "bus_ckt2_output[" << i << "]: ";
                for(int j = 0; j < bus_ckt2_output[i]->indexes.size(); j++){
                    cout << bus_ckt2_output[i]->indexes[j] << " ";
                }
                cout << endl;
            }

        }
};

#endif // CIRMGR_H