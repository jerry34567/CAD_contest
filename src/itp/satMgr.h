/****************************************************************************
  FileName     [ memMgr.h ]
  PackageName  [ cmd ]
  Synopsis     [ Define Memory Manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SATMGR_H
#define SATMGR_H

#include "cirMgr.h"
#include "sat.h"
// #include <cassert>
// #include <fstream>
// #include <iomanip>
#include <iostream>
// #include <stdlib.h>
#include <unordered_map>
#include <vector>

using namespace std;

class variable;
class CirMgr;

class Group
{
        vector<pair<string, bool>> _group;

    public:
        Group() {}
        ~Group() {}
        vector<pair<string, bool>>& group() { return _group; }
};
class SatMgr
{

    public:
        friend class SolverMgr;
        SatMgr() {closeMatching_cnt = 0;}
        ~SatMgr() {}

        int string2Var(string _s, bool isckt1, bool isinput);

        // void readAAG();
        // void readMAP();
        // void readCNF(SatSolver& s_miter, SatSolver& s_verifier);
        void initCircuit(SatSolver& s, SatSolver& s_miter,
                         SatSolver& s_verifier);
        void reset();
        // void verification(bool isManualBinded = 0);
        // void solveNP3();

        void constructCmdr(
            SatSolver& s); // recursive construct Cmdr variable from bottom
        void cmdrExactlyOne(SatSolver& s, Cmdr* parent, bool is_MO); // recursive addClause
        void constraint2(SatSolver& s);
        void constraint_Cmdr(SatSolver& s, vector<vector<variable*>>& M, bool is_MO);
        void constraint_Cmdr_control(SatSolver& s, Var control, vector<Var>& v); // for functional support, if control == 1 -> only one Var in vector<Var> will be 1
        void constraint_Cmdr_nocontrol(SatSolver& s, vector<Var>& v, bool isAtmostOne); // for functional support, if isAtmostOne == 1 -> at most one Var in vector<Var> will be 1, if isAtmostOne == 0 -> exactly one Var in vector<Var> will be 1
        void busMatchExactlyOne(SatSolver& s); // bus matrix: at most one 1 in each row and each col
        void constraint3(SatSolver& s);
        void constraint4_miter(SatSolver& s);
        void constraint5_miter(SatSolver& s);
        void AddLearnedClause(SatSolver& s, SatSolver& s_miter);
        void AddLearnedClause_const(SatSolver& s, SatSolver& s_miter);
        void addSuppConstraint(); // currently : structral support, |SSo1| <= |SSo2|
        void addSuppConstraint_input(); // currently : structral support, |SSo1| <= |SSo2|, for PI constraint, using _funcSupp_PI.size() 
        void addUnateConstraint(bool _isInput); // _isInput == 1 -> inputUnateConstraint
        void addSymmConstraint(SatSolver& s);
        void addSameSuppSizeConstraint(SatSolver& s);
        void addOutputGroupingConstraint();
        void addBusConstraint_match(size_t idxI, size_t idxO, vec<Lit>& ans); // don't use this after using addCandidateBusConstraint
        // void funcSuppInputConstraint(vector<pair<int, int>>& MO_no_pos_neg_pair, int idxI,int idxO, vec<Lit>& as); // given bus match, output match(w/o +-) and funcsupp info, close invalid input match by adding assumption into find_input_given_output_assump
        void addOutputConstraint_inputBusNum(); // two output can only be matched if they have same amount of input bus
        void addBusConstraint_inputUnateness(); // two output bus can only be matched if the inputUnateness of the bus in f <= that of the bus in g
        void addBusConstraint_outputUnateness(); // two input bus can only be matched if the outputUnateness of the bus in x <= that of the bus in y
        void addBusConstraint_inputSupportSize(); // two input bus can only be matched if the output functional support size of the bus in f <= that of the bus of g
        void addBusConstraint_outputSupportSize(); // two output bus can only be matched if the input functional support size of the bus in x <= that of the bus of y        void addCandidateBusConstraint(SatSolver& s);
        void addCandidateBusConstraint(SatSolver& s);
        void addBusValidConstraint(SatSolver& s); // close MIbus_valid, MObus_valid according to |cir1BusSize| <= |cir2BusSize|, also close MIbus_Var, MObus_Var according to MIbus_valid, MObus_valid. Bind control of each MIbus_Var/MObus_Var entry to MI_valid_var/MO_valid_var matching(only true(valid bus match) entry will add port matching clause to solver).
        void addOutput0Constraint(); // close the MO of those output whose = 0 after fraig with other outputs

        // only bind input
        // use int _port1, int _port2 to check if _port1 < 0 and if _port2 < 0
        // _p1, _p2 is for command to bind two port with their mnemonics
        bool addBindClause(bool isnegate, int _port1, int _port2,
                           string _p1 = "", string _p2 = "");
        bool outputBind(int _port1, bool _isPositive1, int _port2,
                        string _p1 = "", string _p2 = "");
        void printMatrix(const SatSolver& s, vector<vector<int>> const& M,
                         int IO);
        void recordResult(const SatSolver&                 s,
                          vector<vector<int>> const& M, int IO);
        void generateResult();
        void reportResult(const SatSolver& solver) {
            solver.printStats();
            printMatrix(solver, record_input, 1);
            printMatrix(solver, record_output, 2);
            inputGroup.clear();
            outputGroup.clear();
            constGroup.clear();
            recordResult(solver, record_input, 1);
            recordResult(solver, record_output, 2);
            generateResult();
                // for (auto i : inputGroup)
                //     for (auto j : i.second.group())
                //         cout << i.first << ' ' << j.second << ' ' << j.first
                //              << endl;
                // cout << endl;
                // for (auto i : outputGroup)
                //     for (auto j : i.second.group())
                //         cout << i.first << ' ' << j.second << ' ' << j.first
                //              << endl;
                // cout << endl;
                // for (auto i : constGroup)
                //     cout << i.first << ' ' << i.second << endl;
                // printArr(solver, x);
                // printArr(solver, y);
                // printArr(solver, f);
                // printArr(solver, g);
        }
        void printArr(const SatSolver& s, vector<variable*> const& Arr) {
            for (int i = 0; i < Arr.size(); i++) {
                cout << Arr[i]->gettype() << " " << Arr[i]->getSub1() << " "
                     << Arr[i]->getSub2() << " ";
                cout << Arr[i]->getVar() << " " << Arr[i]->getVar2()
                     << "\tAssign: ";
                cout << s.getValue(Arr[i]->getVar());
                cout << endl;
            }
            cout << endl;
        }

        template <typename K, typename V>
        void print_map(unordered_map<K, V> const& m) {
            typename std::unordered_map<K, V>::const_iterator pair;
            for (pair = m.begin(); pair != m.end(); pair++) {
                std::cout << "{" << (*pair).first << ": " << (*pair).second << "}\n";
            }
        }
        vector<tuple<bool, Var, Var>>& getInputMatch() { return inputMatch; }
        vector<tuple<bool, Var, Var>>& getOutputMatch() { return outputMatch; }

    private:
        SatSolver solver, miterSolver,
            verifierSolver; // verifysolver is for verification
        CirMgr cirmgr;

        vector<vector<Var>> big_or;

        // record those matched input pairs
        // tuple<isNegate, ckt1's input, ckt2's input>
        // if ckt2's input connect to 0(1) -> the second element would be -1(-2)
        // isNegate == True -> negate
        vector<tuple<bool, Var, Var>> inputMatch;

        // record those match output pairs
        // tuple<isNegate, ckt1's output, ckt2's output>
        vector<tuple<bool, Var, Var>> outputMatch;

        vector<Var> xorVar; // record the xorvar of two binded outputs' miter

        // for inputGroup(outputGroup), inputGroup[<string var_name>] denotes
        // ckt1 variable, inputGroup[i][j] = (ckt2 variable, isPositive)
        unordered_map<string, Group> inputGroup, outputGroup;
        // for constGroup, constGroup denotes ckt2 variable bind to 0(1) if
        // constGroup[<string var_name>] = false(True)
        unordered_map<string, bool>  constGroup;

        // record the best match so far
        vector<vector<int>> record_input, record_output;
        unsigned point = 0;
        size_t closeMatching_cnt;

        //helper function
        void closeMatching(vec<Lit> &lits, size_t _i, size_t _j, bool _isInput){  // _isInput == 1 -> close input matching // positive symmetry only close negative connection(i % 2 == 1)
            if(_isInput){
                vector<variable*>&x = cirmgr.x, &y = cirmgr.y;
                lits.push(~Lit(cirmgr.MI[_i][_j]->getVar()));
                solver.addClause(lits);
                lits.clear();
                if(x[_i / 2]->isInSymmGroup() == true){
                    vector<variable*>& symmGroups_thisX = cirmgr.symmGroups_x[x[_i / 2]->symmGroupIndex()];
                    for(size_t i = 0, ni = symmGroups_thisX.size(); i < ni; ++i){
                        if(symmGroups_thisX[i]->getSub1() - 1 != (_i / 2) && symmGroups_thisX[i]->_funcSupp.size() == x[_i / 2]->_funcSupp.size() && _i % 2 == 1){
                            cout<< "closeMatching_cnt = " << closeMatching_cnt++ << endl;
                            //if(_i % 2 == 1)
                            lits.push(~Lit(cirmgr.MI[(symmGroups_thisX[i]->getSub1() - 1) * 2 + 1][_j]->getVar()));
                            /*else
                                lits.push(~Lit(cirmgr.MI[(symmGroups_thisX[i]->getSub1() - 1) * 2 + 1][_j]->getVar()));*/
                            solver.addClause(lits);
                            lits.clear();
                        }
                    }
                }
                if(y[_j]->isInSymmGroup() == true){
                    vector<variable*>& symmGroups_thisY = cirmgr.symmGroups_y[y[_j]->symmGroupIndex()];
                    for(size_t i = 0, ni = symmGroups_thisY.size(); i < ni; ++i){
                        if(symmGroups_thisY[i]->getSub1() - 1 != _j && symmGroups_thisY[i]->_funcSupp_PI.size() == y[_j]->_funcSupp_PI.size() && _i % 2 == 1){
                            cout<< "closeMatching_cnt = " << closeMatching_cnt++ << endl;
                            lits.push(~Lit(cirmgr.MI[_i][(symmGroups_thisY[i]->getSub1() - 1)]->getVar()));
                            solver.addClause(lits);
                            lits.clear();
                        }
                    }
                }
            }
            else{
                lits.push(~Lit(cirmgr.MO[_i][_j]->getVar()));
                solver.addClause(lits);
                lits.clear();
            }
        }
        void closeMatching_bus(vec<Lit> &lits, size_t _i, size_t _j, bool _isInput){  // _isInput == 1 -> close input matching
            if(_isInput == 1)
                lits.push(~Lit(cirmgr.MIbus_Var[_i][_j]));
            else
                lits.push(~Lit(cirmgr.MObus_Var[_i][_j]));
            solver.addClause(lits);
            lits.clear();
        }

};

#endif // SATMGR_H
