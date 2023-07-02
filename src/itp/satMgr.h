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
        SatMgr() {}
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
        void constraint3(SatSolver& s);
        void constraint4_miter(SatSolver& s);
        void constraint5_miter(SatSolver& s);
        void AddLearnedClause(SatSolver& s, SatSolver& s_miter);
        void AddLearnedClause_const(SatSolver& s, SatSolver& s_miter);
        void addBusConstraint();
        void addSuppConstraint(); // currently : structral support, |SSo1| <= |SSo2|
        void addUnateOutputConstraint(); // |Uo1| <= |Uo2|

        // only bind input
        // use int _port1, int _port2 to check if _port1 < 0 and if _port2 < 0
        // _p1, _p2 is for command to bind two port with their mnemonics
        bool addBindClause(bool isnegate, int _port1, int _port2,
                           string _p1 = "", string _p2 = "");
        bool outputBind(int _port1, bool _isPositive1, int _port2,
                        string _p1 = "", string _p2 = "");
        void printMatrix(const SatSolver& s, vector<vector<variable*>> const& M,
                         int IO);
        void recordResult(const SatSolver&                 s,
                          vector<vector<variable*>> const& M, int IO);
        void generateResult();
        void reportResult(const SatSolver& solver) {
            solver.printStats();
            printMatrix(solver, record_input, 1);
            printMatrix(solver, record_output, 2);
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
                cout << Arr[i]->getName() << " " << Arr[i]->getSub1() << " "
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
            for (auto const& pair : m) {
                std::cout << "{" << pair.first << ": " << pair.second << "}\n";
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
        vector<vector<variable*>> record_input, record_output;
        unsigned point = 0;
};

#endif // SATMGR_H
