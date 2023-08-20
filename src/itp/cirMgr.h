#ifndef CIRMGR_H
#define CIRMGR_H

#include "sat.h"
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cmath>
// #include "satMgr.h"

using namespace std;
enum preprocess
{
    support,
    outputUnateness,
    inputUnateness
};
class variable;

class Supp_Difference{ // Output heuristic pair need to sort support difference, while still able to obtain original idx of f, g (MO_valid_Var) 
    public:
        
        Supp_Difference(int i1, int i2, int sd, Var _v, variable* _f, variable* _g): original_idx1(i1), original_idx2(i2), suppdiff(sd), v(_v), f_var(_f), g_var(_g) {}
        Supp_Difference(){}
        Supp_Difference(const Supp_Difference &t): original_idx1(t.original_idx1), original_idx2(t.original_idx2), suppdiff(t.suppdiff), v(t.v), weight_sort(t.weight_sort) {}
        ~Supp_Difference(){}
        int original_idx1;
        int original_idx2;
        int suppdiff; // equal to (g[j]->_funcSupp.size() - f[i]->_funcSupp.size()), if(!MO_valid) -> -1
        Var v;
        double weight_sort; // suppdiff ^ 2 + f[original_idx1]->selected_times ^ 2, can add some weighted. To sort in row.
        variable* f_var;
        variable* g_var;
};
class Supp_Diff_Row{
    public:
        Supp_Diff_Row(int _ori_idx, int _max_diff, int _g, int _f): original_row_index(_ori_idx), max_diff(_max_diff), g_suppsize(_g), f_size(_f) {}    
        Supp_Diff_Row() {}    
        ~Supp_Diff_Row() {}    
        int original_row_index;
        int max_diff;
        int g_suppsize;
        int f_size;
        vector<int> suppdiff_cnt_arr; // suppdiff_cnt_arr[0]: number of X in a row, suppdiff_cnt_arr[1]: number of 0 in a row. Valid matching choice started from index: suppdiff_cnt_arr[0]
};
class symmObj{
    public:
        symmObj(size_t _outputNum, size_t _i): _arrayLength(_outputNum / (sizeof(unsigned long long) * 8) + 1), _index(_i), _numOfinputs(0){
            // if(_outputNum / (sizeof(unsigned long long) * 8) == 0)
            //     _symmOutput = new unsigned long long(0);
            // else{
                _symmOutput.reserve(_outputNum / (sizeof(unsigned long long) * 8) + 1);
                // _symmOutput = new unsigned long long[_outputNum / (sizeof(unsigned long long) * 8) + 1];
                for(size_t i = 0, n = (_outputNum / (sizeof(unsigned long long) * 8) + 1); i < n; ++i)
                    _symmOutput.push_back(0);
            // }
        }
        ~symmObj(){}
        void                        setSymmOutput(const unsigned long long _s){_symmOutput[(_s / (sizeof(unsigned long long) * 8))] = (_symmOutput[_s / ((sizeof(unsigned long long) * 8))] |
                                         ((unsigned long long)1 << (_s % ((sizeof(unsigned long long) * 8)))));
                                         return;}
        vector<unsigned long long>& getSymmOutput(){return _symmOutput;}
        size_t                      arrayLength(){return _arrayLength;}
        size_t                      numOfInputs(){return _numOfinputs;}
        size_t                      index(){return _index;}
        bool                        isEqual(symmObj* _s){
                                        for(size_t i = 0; i < _arrayLength; ++i)
                                            if(_s->getSymmOutput()[i] != _symmOutput[i])
                                                return 0;
                                        return 1;
                                    }
        void                        countNumberOfInputs(){
                                        size_t y = 0;
                                        for(size_t i = 0; i < _arrayLength; ++i){
                                            unsigned long long x = _symmOutput[i];
                                            while(x){
                                                y += x & 1ULL;
                                                x = x >> 1;
                                            }
                                        }
                                        _numOfinputs = y;
                                    }
    private:
        // unsigned long long* _symmOutput;
        vector<unsigned long long> _symmOutput;
        size_t _arrayLength;
        size_t _index; // the position in x / y
        size_t _numOfinputs; // the number of inputs w.r.t this output's positive symmetric group
};
class variable
{
    public:
        variable(char type, Var sub1, Var sub2)
            : _outputBinateNum(0),_inputSymmGroupIndex(-1),_maxInputSymmGroupIndex(0), _isInSymmGroup(0), _type(type), _sub1(sub1), _sub2(sub2), _busSize(0),_suppSize(0), _inputUnateNum_p(0),_inputUnateNum_n(0),_outputUnateNum(0), _outputGroupingNum(0), _inputUnates(""), _symmGroupIndex(-2){
        _busIndexs.push_back(-1);
        }
        ~variable() {delete _symmOutput;}

        vector<variable*> _funcSupp_PI; // record those PO that this PI affect
        vector<variable*> _funcSupp;    // record the functional support of PO
        vector<string>    candidates;   // record valid match candidate (you can use u_name_index_ckt1, u_name_index_ckt2 and x, y, f, g to get its variable)
        vector<unordered_set<int>> symmetric_set; // record the symmetric set (only in output variable)
        set<variable*> partial;

        char                                gettype() const { return _type; }
        bool                                isInSymmGroup()const{ return _isInSymmGroup;}
        bool                                checkSymmSign(map<size_t, size_t>& _s, size_t _p, size_t _q);
        string                              getname() const { return _name; }
        Var                                 getSub2() const { return _sub2; }
        Var                                 getVar() const { return _var; }
        Var                                 getSub1() const { return _sub1; }  
        Var                                 getVar2() const { return _var2; }
        Var                                 getVar3() const { return _var3; }
        Var                                 getVar4() const { return _var4; }
        // int                                 busIndex() { return _busIndex; }
        vector<int>&                        busIndexs(){return _busIndexs;}
        // int                                 busIndex(const int _i) { return find(_busIndexs.begin(), _busIndexs.end(), _i); }
        int                                 symmGroupIndex(){return _symmGroupIndex;}
        int                                 inputSymmGroupIndex(){return _inputSymmGroupIndex;}
        size_t                              busSize() { return _busSize; }
        size_t                              suppSize() { return _suppSize; }
        size_t                              maxInputSymmGroupIndex(){return _maxInputSymmGroupIndex;}
        map<size_t, size_t>&                suppBus() { return _suppBus;}
        vector<pair<size_t, size_t> >*&      suppBus_distribution(){return _suppBus_distribution;}
        vector<size_t>&                     inputSymmGroupSize(){return _inputSymmGroupSize;}
        symmObj*                            symmOutput(){return _symmOutput;}
        symmObj*                            symmInput(){return _symmInput;}
        map<size_t, size_t>&                symmSign(){return _symmSign;}
        map<size_t, size_t>&                inputSymmGroup(){return _inputSymmGroup;}
        size_t                              outputUnateNum() { return _outputUnateNum; }
        size_t                              outputBinateNum() { return _outputBinateNum; }
        string                              inputUnates() { return _inputUnates; }
        size_t                              inputUnateNum_p() { return _inputUnateNum_p; }
        size_t                              inputUnateNum_n() { return _inputUnateNum_n; }
        size_t                              outputGroupingNum() { return _outputGroupingNum; }
        int                                 getSelected_times() {return selected_times;}
        void                                settype(const char& v) { _type = v; }
        void                                setIsInSymmGroup(const bool _b){_isInSymmGroup = _b;}
        void                                setname(const string& name) { _name = name; }
        void                                setSub1(const Var& v) { _sub1 = v; }
        void                                setSub2(const Var& v) { _sub2 = v; }
        void                                setVar(const Var& v) { _var = v; }
        void                                setVar2(const Var& v) { _var2 = v; }
        void                                setVar3(const Var& v) { _var3 = v; }
        void                                setVar4(const Var& v) { _var4 = v; }
        // void                                setBusIndex(const int _i) { _busIndex = _i;}
        void                                setBusIndex(const int _i) { if(_busIndexs.front() == -1) _busIndexs.front() = _i; else _busIndexs.push_back(_i);}
        void                                setBusSize(const size_t _s) { _busSize = _s; }
        void                                setSuppSize(const size_t _s) { _suppSize = _s; }
        void                                setInputUnateNum_p(const size_t _s) { _inputUnateNum_p = _s; }
        void                                setInputUnateNum_n(const size_t _s) { _inputUnateNum_n = _s; }
        void                                setOutputUnateNum(const size_t _s) { _outputUnateNum = _s; }
        void                                setOutputBinateNum(const size_t _s) { _outputBinateNum = _s; }
        void                                setOutputGroupingNum(const size_t _s) { _outputGroupingNum = _s; }
        void                                setSymmGroupIndex(const int _s){_symmGroupIndex = _s;}
        void                                setInputSymmGroupIndex(const int _s){_inputSymmGroupIndex = _s;}
        void                                setInputUnates(const string _s) { _inputUnates = _s; }
        void                                setSymmOutput(const unsigned long long _s){_symmOutput->setSymmOutput(_s);}
        void                                setSymmInput(const unsigned long long _s){_symmInput->setSymmOutput(_s);}
        void                                newSymmOutput(const size_t _s, const size_t _i){_symmOutput = new symmObj(_s, _i);}
        void                                newSymmInput(const size_t _s, const size_t _i){_symmInput = new symmObj(_s, _i);}
        void                                addInputUnateNum_p() { ++_inputUnateNum_p; }
        void                                addInputUnateNum_n() { ++_inputUnateNum_n; }
        void                                addOutputBinateNum() { ++_outputBinateNum; }
        void                                inputSymmGroupClassification(size_t _input1, size_t _input2);
        void                                setSelected_times(int _s) {selected_times = _s;}

    private:
        char    _type;
        bool    _isInSymmGroup; // whehter this input is positive symm with other inputs
        string  _name;
        Var     _sub1;     // _sub1 - 1 == the index in x , y, f, g
        Var     _sub2;     // if == PI or PO -> _sub2 = -1
        Var     _var;      // for solver1
        Var     _var2;     // for miter solver
        Var     _var3;     // for verification solver
        Var     _var4;     // for circuit solver
        // int     _busIndex; // bus_index denotes the position of the bus it
                          // belongs to in bus vector
        vector<int> _busIndexs; // some inputs are in multiple buses simultaneously
        int    _symmGroupIndex; // the postiion in symmGroup
        int    _inputSymmGroupIndex; // the position in _inputSymmGroup (for symmSign)
        size_t _busSize;  // bus_size denotes the number of the elements in the
                          // bus it belongs to
        size_t _suppSize; // currently : functional support
        size_t _maxInputSymmGroupIndex; // record the current symm group index
        // size_t _inputUnateNum;  // how many outputs are unate w.r.t this PI
        size_t _outputBinateNum; // how many binate PIs w.r.t this output 
        size_t _inputUnateNum_p;  // how many outputs are positive unate w.r.t this PI 
        size_t _inputUnateNum_n;  // how many outputs are negative unate w.r.t this PI 
        size_t _outputUnateNum; // the number of output unate variables(i.e., the number of input that makes this output unate)
        string _inputUnates; // the detail input unate of this output
        size_t _outputGroupingNum; // index of the output grouping
        symmObj* _symmOutput; // record the outputs that this input variable is positive symmetric to
        symmObj* _symmInput; // record the inputs that is positive symmetric to this output variable
        // collect those buses that this output's support inputs lie in    
        //_SuppBus[i].first = the bus index; .second = number of inputs in that bus
        map<size_t, size_t> _suppBus;
        vector<pair<size_t, size_t>>*  _suppBus_distribution;  // the sorted ( non-increasing order )distribution of each bus in _suppBus
        map<size_t, size_t>  _symmSign; // the symmetry signature of each inputs; map[i].first == the output port index in f / g, map[i].second == the number of inputs in the symm group w.r.t f[map[i].first] / g[map[i].first]
        map<size_t, size_t> _inputSymmGroup; // to classify each input symmetry group of this output variable; map[i].first = the input port index in x / y, map[i]->second = the group index 
        vector<size_t> _inputSymmGroupSize; // the vector of input symmGroup size, the index is identical to the one in the map.second
        // unsigned long long _suppBus; 
        // Var _aigVar; do we need this??
        int     selected_times; // record selected times in output heuristic pair (only for f variables)
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
        void readCNF(SatSolver& s_miter, SatSolver& s_cir1, SatSolver& s_cir2, SatSolver& s_verifier);
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
        void symmSign(); // give each input variables a symmsign
        void sortSuppDiff(); // initially read in MO_valid and sort Supp Difference for MO_suppdiff, sort MO_suppdiff_row
        void outputHeuristicMatching(vec<Lit>& output_heuristic_assump); // output match according to MO_suppdiff_row[MO_suppdiff_chosen_row], MO_suppdiff -> output match -> support input port match (can't match outside)
        bool _inside_outputHeuristicMatching(vec<Lit>& output_heuristic_assump); // output match according to MO_suppdiff_row[MO_suppdiff_chosen_row], MO_suppdiff -> output match -> support input port match (can't match outside)
        void updateOutputHeuristic_Success(); // if Match Found -> call this to     MO_suppdiff_chosen_row++, update chosen output matching
        bool updateOutputHeuristic_Fail();    //return false if really NO MATCH!!!   if Match Not Found -> call this to chosen col idx++, if idx == MO_valid.size()-1 -> MO_suppdiff_chosen_row--, update chosen output matching. IF impossible(real No match) -> return false.
        void throwToLastRow(int row);  // throw MO_suppdiff_chosen_row row to last
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
        
        vector<vector<Supp_Difference* >> MO_suppdiff; //     (sort in row)       record row-sorted according to support difference 
        vector<Supp_Diff_Row>           MO_suppdiff_row; // (sort between rows) record each row's suppdiff count of each number for sorting: most X at top -> if same: most 0 at top...  this is to increase output matching success rate (heuristic)
        vector<int>                     MO_suppdiff_chosen_col_idxes; // store each row's currently chosen output heuristic matching column index
        int                             MO_suppdiff_chosen_row; // currently chosen output heuristic row
        // vector<int>                     MO_suppdiff_omit_rows; // store those
        int                             MO_suppdiff_omit_row; // store those
        // int                             MO_suppdiff_success_count; 
        // int                             MO_suppdiff_fail_count; 
        int                             MO_suppdiff_cons_fail_rows; // consecutive fail rows, if > a specific number, backtrack and choose next col, also reshuffle the following rows.

        vector<vector<Var>>       MIbus_Var, MObus_Var;       // record bus match Var, bus_ckt1_input * bus_ckt2_input
        vector<vector<bool>>      MIbus_valid, MObus_valid;   // record bus valid
        vector<size_t>            output0;  // record those output that equals to constant 0 after fraig 
        vector<vector<variable*> > symmGroups_x, symmGroups_y; // record the symmGroups of x & y respectively
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
        unordered_map<int, Var> umapInterVar_ckt1, umapInterVar_ckt1_veri, mapInterVar_cir1, mapInterVar_cir2,
            umapInterVar_ckt2, umapInterVar_ckt2_veri;
        vector<Cmdr*> Cmdr_level;

        unordered_map<string, int> u_name_index_ckt1, u_name_index_ckt2; // index can be used for x, y, f, g immediately
        unordered_map<string, bool> u_name_isInput_ckt1, u_name_isInput_ckt2;
        unordered_map<string, int> u_name_busIndex_input_ckt1, u_name_busIndex_output_ckt1, u_name_busIndex_input_ckt2, u_name_busIndex_output_ckt2; // index for bus_ckt1_input, bus_ckt2_input

        set<variable* > cir1_func_supp_union, cir2_func_supp_union, cir1_not_func_supp_union, cir2_not_func_supp_union;

        // output group
        vector<set<int> > cir1_output_group, cir2_output_group;

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
        void _symmGrouping(bool _isCkt1, vector<symmObj*>& tmp); // to collect those inputs with same positive symmtric outputs
        static bool _outputsorting(variable* a, variable* b);
        static bool _increasing(symmObj* a, symmObj* b){
            vector<unsigned long long>& asymm = a->getSymmOutput(), & bsymm = b->getSymmOutput();
            for(int i = a->arrayLength() - 1; i >= 0; --i){
                if(asymm[i] < bsymm[i])
                    return 1;
                else if(asymm[i] > bsymm[i])
                    return 0;
            }
            return 0;
        }
        static bool _suppdiff_increasing(Supp_Difference* a, Supp_Difference* b){return a->suppdiff < b->suppdiff;}
        static bool _suppdiff_cnt_arr_decreasing(Supp_Diff_Row a, Supp_Diff_Row b){
            if(a.suppdiff_cnt_arr[0] == a.f_size - 1 && b.suppdiff_cnt_arr[0] == b.f_size - 1) return a.g_suppsize < b.g_suppsize;
            for(int i = 0; i < (a.suppdiff_cnt_arr.size() < b.suppdiff_cnt_arr.size() ? a.suppdiff_cnt_arr.size() : b.suppdiff_cnt_arr.size()); i++){
                if(a.suppdiff_cnt_arr[i] != b.suppdiff_cnt_arr[i]) return a.suppdiff_cnt_arr[i] > b.suppdiff_cnt_arr[i];
            }
            return a.g_suppsize < b.g_suppsize;
        }
        static bool _suppdiff_weight_sort(Supp_Difference* a, Supp_Difference* b){return a->weight_sort < b->weight_sort;}
        void _symmSign(bool _isCkt1); // give each input variables a symmsign
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
        void print_ori_MO_suppDiff(){
            cout << "\nMO_suppDiff " << endl;
            // for (int i = 0; i < f.size(); i++){
            //     cout << f[i]->getname() << "\t";
            // }
            cout << endl;
            for (int j = 0; j < MO_valid[0].size(); j++){
                cout << g[j]->getname() << "\t";
                for (int i = 0; i < MO_valid.size(); i++){
                    // cout << MO_valid[i][j];
                    if(!MO_valid[i][j]) cout << "X\t";
                    else{
                        cout << g[j]->_funcSupp.size() - f[i]->_funcSupp.size() << "\t";
                    }
                }
                cout << endl;
            }
        }
        void print_ori_MO_suppDiff_notab(){
            cout << "\nMO_suppDiff " << endl;
            // for (int i = 0; i < f.size(); i++){
            //     cout << f[i]->getname() << "\t";
            // }
            cout << endl;
            for (int j = 0; j < MO_valid[0].size(); j++){
                cout << g[j]->getname() << "\t";
                for (int i = 0; i < MO_valid.size(); i++){
                    // cout << MO_valid[i][j];
                    if(!MO_valid[i][j]) cout << "X";
                    else{
                        cout << g[j]->_funcSupp.size() - f[i]->_funcSupp.size();
                    }
                }
                cout << endl;
            }
        }
        void printMO_suppDiff_notab(){
            cout << "\nsorted MO_suppDiff " << endl;
            for(int j = 0; j < MO_suppdiff[0].size(); j++){
                for(int i = 0; i < MO_suppdiff.size(); i++){
                    // if(MO_suppdiff[i][j].suppdiff == -1)
                    //     cout << "X";
                    // else
                    //     cout << MO_suppdiff[i][j].suppdiff; 
                    cout << int(g[j]->_funcSupp.size()) - int(f[i]->_funcSupp.size()) << "\t";
                }
                cout << endl;
            }
            cout << "original_idx1" << endl;
            for(int j = 0; j < MO_suppdiff[0].size(); j++){
                for(int i = 0; i < MO_suppdiff.size(); i++){
                    cout << MO_suppdiff[i][j]->original_idx1; 
                }
                cout << endl;
            }
            cout << "original_idx2" << endl;
            for(int j = 0; j < MO_suppdiff[0].size(); j++){
                for(int i = 0; i < MO_suppdiff.size(); i++){
                    cout << MO_suppdiff[i][j]->original_idx2; 
                }
                cout << endl;
            }
        }
        void print_suppdiff_cnt_arr(){
            cout << "MO_suppdiff_row suppdiff_cnt_arr " << MO_suppdiff_row.size() << endl;
            for(int i = 0; i < MO_suppdiff_row.size(); i++){
                cout << MO_suppdiff_row[i].original_row_index << "\t";
                for(int j = 0; j < MO_suppdiff_row[i].suppdiff_cnt_arr.size(); j++){
                    cout << MO_suppdiff_row[i].suppdiff_cnt_arr[j];
                }
                cout << endl;
            }
            for(int j = 0; j < MO_suppdiff[0].size(); j++){
                cout << g[MO_suppdiff_row[j].original_row_index]->getname() << " ";
                cout << g[MO_suppdiff_row[j].original_row_index]->_funcSupp.size() << " ";
                for(int i = 0; i < MO_suppdiff.size(); i++){
                    if(MO_suppdiff[i][MO_suppdiff_row[j].original_row_index]->suppdiff == -1)
                        cout << "X";
                    else
                        cout << MO_suppdiff[i][MO_suppdiff_row[j].original_row_index]->suppdiff; 
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
        void printSupp(){
            cout << "cir1 PI" << endl;
            for(int i = 0; i < x.size(); i++){
                cout << x[i]->getname() << "\t: " << x[i]->_funcSupp_PI.size() << "\t: ";
                for(int j = 0; j < x[i]->_funcSupp_PI.size(); j++){
                    cout << x[i]->_funcSupp_PI[j]->getname() << " ";
                }
                cout << endl;
            }
            cout << "cir2 PI" << endl;
            for(int i = 0; i < y.size(); i++){
                cout << y[i]->getname() << "\t: " << y[i]->_funcSupp_PI.size() << "\t: ";
                for(int j = 0; j < y[i]->_funcSupp_PI.size(); j++){
                    cout << y[i]->_funcSupp_PI[j]->getname() << " ";
                }
                cout << endl;
            }
            cout << "cir1 PO" << endl;
            for(int i = 0; i < f.size(); i++){
                cout << f[i]->getname() << "\t: " << f[i]->_funcSupp.size() << "\t: ";
                for(int j = 0; j < f[i]->_funcSupp.size(); j++){
                    cout << f[i]->_funcSupp[j]->getname() << " ";
                }
                cout << endl;
            }
            cout << "cir2 PO" << endl;
            for(int i = 0; i < g.size(); i++){
                cout << g[i]->getname() << "\t: " << g[i]->_funcSupp.size() << "\t: ";
                for(int j = 0; j < g[i]->_funcSupp.size(); j++){
                    cout << g[i]->_funcSupp[j]->getname() << " ";
                }
                cout << endl;
            }
        }
        void print_f_selected_times(){
            cout << "print_f_selected_times" << endl;
            for(int i = 0; i < f.size(); i++){
                if(f[i]->getSelected_times() != 0){
                    cout << f[i]->getname() << " " << f[i]->getSelected_times() << endl;
                }
            }
        }
};

#endif // CIRMGR_H
