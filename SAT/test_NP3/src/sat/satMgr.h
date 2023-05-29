/****************************************************************************
  FileName     [ memMgr.h ]
  PackageName  [ cmd ]
  Synopsis     [ Define Memory Manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SATMGR_H
#define SATMGR_H

#include "sat.h"
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <unordered_map>
#include <vector>

using namespace std;

class variable;
class SatMgr;
class variable
{
    public:
        variable(char name, Var sub1, Var sub2)
            : _name(name), _sub1(sub1), _sub2(sub2) {}
        ~variable() {}
        char getName() const { return _name; }
        Var  getSub1() const { return _sub1; }
        Var  getSub2() const { return _sub2; }
        Var  getVar() const { return _var; }
        Var  getVar2() const { return _var2; }
        Var  getVar3() const { return _var3; }
        void setName(const char& v) { _name = v; }
        void setSub1(const Var& v) { _sub1 = v; }
        void setSub2(const Var& v) { _sub2 = v; }
        void setVar(const Var& v) { _var = v; }
        void setVar2(const Var& v) { _var2 = v; }
        void setVar3(const Var& v) { _var3 = v; }

    private:
        char _name;
        Var  _sub1;
        Var  _sub2;
        Var  _var;  // for solver1
        Var  _var2; // for miter solver
        Var  _var3; // for verification solver
                    // Var _aigVar; do we need this??
};
class SatMgr
{

    public:
        SatMgr() {}
        ~SatMgr() {}
        int  string2Var(string _s, bool isckt1, bool isinput);
        bool outputBind(int _port1, bool _isPositive1, int _port2,
                        string _p1 = "", string _p2 = "");
        void verification(bool isManualBinded = 0);
        void solveNP3();
        void printMatrix(const SatSolver& s, vector<vector<variable*>> const& M,
                         int IO);
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

        void constraint2(SatSolver& s);

        void constraint3(SatSolver& s);
        void constraint4_miter(SatSolver& s);
        void constraint5_miter(SatSolver& s);
        void initCircuit(SatSolver& s, SatSolver& s_miter,
                         SatSolver& s_verifier);
        void readAAG();
        void readMAP();
        void readCNF(SatSolver& s_miter, SatSolver& s_verifier);
        void AddLearnedClause(SatSolver& s, SatSolver& s_miter);
        void AddLearnedClause_const(SatSolver& s, SatSolver& s_miter);
        bool
             addBindClause(bool   isnegate, // only bind input
                           int    _port1, // use int to check if _port1 & _port2 < 0
                           int    _port2, // _p1, _p2 is for command to bind two
                                          // port with their mnemonics
                           string _p1 = "", string _p2 = "");
        void reportResult(const SatSolver& solver, bool result) {
            solver.printStats();
            cout << (result ? "SAT" : "UNSAT") << endl;
            if (result) {
                printMatrix(solver, MI, 1);
                printMatrix(solver, MO, 2);
                // printArr(solver, x);
                // printArr(solver, y);
                // printArr(solver, f);
                // printArr(solver, g);
            }
        }
        vector<tuple<bool, Var, Var>>& getInputMatch() { return inputMatch; }
        vector<tuple<bool, Var, Var>>& getOutputMatch() { return outputMatch; }

    private:
        /* Global */
        vector<string> portnum_ckt1, portname_ckt1, portnum_ckt2, portname_ckt2;
        int       inputNum_ckt1, outputNum_ckt1, inputNum_ckt2, outputNum_ckt2;
        SatSolver verifierSolver; // verifysolver is for verification
        // portnum: i0, i1.., o0, o1..
        // portname: a,  b... , g, f..

        unordered_map<int, string> umapNum_ckt1, umapName_ckt1, umapNum_ckt2,
            umapName_ckt2; // umap[real CNF_VAR] -> real input/output port name
        unordered_map<int, Var> umapInterVar_ckt1, umapInterVar_ckt1_veri,
            umapInterVar_ckt2,
            umapInterVar_ckt2_veri; // those intervariable that doesn't exist in
                                    // x/y/f/g
                                    // umapInterVar_ckti_veri is for
                                    // verifySolver
        vector<vector<variable*>> MI, MO;
        vector<variable*>         x, y, f, g; // sub2 = -1
        vector<vector<Var>>       big_or;

        vector<tuple<bool, Var, Var>>
            inputMatch; // record those match input pairs
        // tuple<isNegate, ckt1's input, ckt2's input>
        // if ckt2's input connect to 0(1) -> the second element
        // would be -1(-2)
        // isNegate == 0 -> negate
        vector<tuple<bool, Var, Var>>
                    outputMatch; // record those match output pairs
                         // tuple<isNegate, ckt1's output, ckt2's output>
        vector<Var> xorVar; // record the xorvar of two binded outputs' miter
};

#endif // SATMGR_H
