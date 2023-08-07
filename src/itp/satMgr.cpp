#include "satMgr.h"
#include "sat.h"
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <unordered_map>
#include <vector>

void
SatMgr::reset() {
    cirmgr.reset();
    verifierSolver.reset();
    solver.reset();
    miterSolver.reset();

    big_or.clear();

    inputMatch.clear();

    outputMatch.clear();

    xorVar.clear();
}

int
SatMgr::string2Var(string _s, bool isckt1, bool isinput) {
    if (isckt1) {
        for (size_t i = 0, n = cirmgr.portname_ckt1.size(); i < n; ++i) {
            if (cirmgr.portname_ckt1[i] == _s) {
                return isinput ? cirmgr.x[i]->getVar3()
                               : cirmgr.f[i - cirmgr.inputNum_ckt1]->getVar3();
            }
        }
    } else {
        for (size_t i = 0, n = cirmgr.portname_ckt2.size(); i < n; ++i) {
            if (cirmgr.portname_ckt2[i] == _s) {
                return isinput ? cirmgr.y[i]->getVar3()
                               : cirmgr.g[i - cirmgr.inputNum_ckt2]->getVar3();
            }
        }
    }
    return -100;
}
bool
SatMgr::outputBind(int _port1, bool _isPositive1, int _port2, string _p1,
                   string _p2) {
    if (!_p1.empty()) _port1 = string2Var(_p1, 1, 0);
    if (!_p2.empty()) _port2 = string2Var(_p2, 0, 0);
    if (_port1 == -100 || _port2 == -100) {
        cout << "in outputBind : _port1 and _port2 = " << _port1 << " "
             << _port2 << endl;
        return 0;
    }
    xorVar.push_back(verifierSolver.newVar());
    verifierSolver.addXorCNF(xorVar.back(), _port1, _isPositive1, _port2, 0);
    if (!_p1.empty() || !_p2.empty())
        outputMatch.push_back(
            tuple<bool, Var, Var>(!_isPositive1, _port1, _port2));
    return 1;
}
/*void
SatMgr::verification(bool isManualBinded) {
    // cout << "inputMatch = \n";
    // for (auto i : inputMatch)
    //     cout << get<0>(i) << ' ' << get<1>(i) << ' ' << get<2>(i)
    //          << endl;
    // cout << "\noutputMatch = \n";
    // for (auto i : outputMatch)
    //     cout << get<0>(i) << ' ' << get<1>(i) << ' ' << get<2>(i)
    //          << endl;
    vec<Lit> lits;
    // vector<Var> xorVar;
    Var      f =
        verifierSolver.newVar(); // f should be 0 for two equilivance circuit
    lits.push(~Lit(f));
    if (!isManualBinded) {
        for (auto i : inputMatch)
            addBindClause(get<0>(i), get<1>(i), get<2>(i));

        for (auto i : outputMatch) {
            outputBind(get<1>(i), get<0>(i), get<2>(i));
            // xorVar.push_back(verifierSolver.newVar());
            // verifierSolver.addXorCNF(xorVar.back(), get<1>(i),
            //                          get<0>(i), get<2>(i), 0);
        }
    }
    for (size_t i = 0, n = xorVar.size(); i < n; ++i) {
        // cout << xorVar[i] << endl;
        vec<Lit> cls;
        lits.push(Lit(xorVar[i]));
        Lit _i = ~Lit(xorVar[i]), _f = Lit(f);
        cls.push(_i);
        cls.push(_f);
        verifierSolver.addClause(cls);
        cls.clear();
    }
    if (lits.size() != 1) verifierSolver.addClause(lits);
    verifierSolver.assumeProperty(f, 1); // 要negate嗎

    bool result = verifierSolver.assumpSolve();
    verifierSolver.printStats();
    cout << (result ? "SAT" : "UNSAT") << endl;
    if (result) {
        for (auto i : inputMatch)
            cout << (get<0>(i) ? '~' : ' ')
                 << verifierSolver.getValue(get<1>(i)) << ' '
                 << verifierSolver.getValue(get<2>(i)) << endl;
        cout << endl;
        for (auto i : outputMatch)
            cout << (get<0>(i) ? '~' : ' ')
                 << verifierSolver.getValue(get<1>(i)) << ' '
                 << verifierSolver.getValue(get<2>(i)) << endl;
    }
    // assump.clear();
}*/

// void
// SatMgr::solveNP3() {
//     reset();
//     cirmgr.reset();
//     // SatSolver solver, miterSolver;
//     solver.initialize();
//     miterSolver.initialize();
//     verifierSolver.initialize();

//     cirmgr.readAAG();
//     cirmgr.readMAP();
//     initCircuit(solver, miterSolver, verifierSolver);
//     cirmgr.readCNF(miterSolver, verifierSolver);
//     // return;
//     /*
//     bool result;
//     result = solver.solve();
//     reportResult(solver, result);
//     */

//     while (true) {
//         // for(int i = 0; i < 10; i++){
//         vector<vector<variable*>>&MI = cirmgr.MI, &MO = cirmgr.MO;
//         bool                      SAT1_result, SAT2_result;
//         SAT1_result = solver.solve();
//         if (!SAT1_result) {
//             cout << "No match!!" << endl;
//             break;
//         } else {
//             vec<Lit> assump;
//             for (int i = 0; i < MI.size(); i++) {
//                 for (int j = 0; j < MI[0].size(); j++) {
//                     if (solver.getValue(MI[i][j]->getVar()) == 1) {
//                         Lit v = Lit(MI[i][j]->getVar2());
//                         assump.push(v);
//                         cout << "i: " << i << " j: " << j
//                              << " val: " <<
//                              solver.getValue(MI[i][j]->getVar())
//                              << endl;
//                     } else if (solver.getValue(MI[i][j]->getVar()) == 0) {
//                         Lit v = ~Lit(MI[i][j]->getVar2());
//                         assump.push(v);
//                     }
//                 }
//             }
//             cout << "below is MO " << endl;
//             for (int i = 0; i < MO.size(); i++) {
//                 for (int j = 0; j < MO[0].size(); j++) {
//                     if (i % 2 == 0) {
//                         if ((solver.getValue(MO[i][j]->getVar()) +
//                              solver.getValue(MO[i + 1][j]->getVar())) == 0) {
//                             assump.push(Lit(big_or[j][i / 2]));
//                         }
//                     }
//                     if (solver.getValue(MO[i][j]->getVar()) == 1) {
//                         Lit v = Lit(MO[i][j]->getVar2());
//                         assump.push(v);
//                         cout << "i: " << i << " j: " << j
//                              << " val: " <<
//                              solver.getValue(MO[i][j]->getVar())
//                              << endl;
//                     } else if (solver.getValue(MO[i][j]->getVar()) == 0) {
//                         Lit v = ~Lit(MO[i][j]->getVar2());
//                         assump.push(v);
//                     }
//                 }
//             }
//             cout << endl;

//             string temp;
//             // cin >> temp;

//             SAT2_result = miterSolver.assumpSolve(assump);
//             if (!SAT2_result) {
//                 cout << "Match found!!" << endl;
//                 reportResult(solver, SAT1_result);

//                 break;
//             } else {
//                 // cout << "ELSE" << endl;
//                 AddLearnedClause(solver, miterSolver);
//                 // AddLearnedClause_const(solver, miterSolver);
//             }
//             assump.clear();
//         }
//     }
// }
// how to check if this MIMO is feasible solution fast?
//   void printMatrix(const SatSolver&                 s,
//                    vector<vector<variable*>> const& M) {
//       for (int j = 0; j < M.size(); j++) {
//           for (int i = 0; i < M[0].size(); i++) {
//               cout << M[j][i]->getName() << " " << M[j][i]->getSub1()
//                    << " " << M[j][i]->getSub2() << " ";
//               cout << M[j][i]->getVar() << " " << M[j][i]->getVar2()
//                    << "\tAssign: ";
//               cout << M[j][i];
//               cout << endl;
//           }
//       }
//       cout << endl;
//   }

void
SatMgr::recordResult(const SatSolver& s, vector<vector<int>> const& M,
                     int IO) { // IO: 1 -> MI, 2 -> MO
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    for (int j = 0; j < M.size(); j++) {
        for (int i = 0; i < M[0].size(); i++) {
            // cout << M[j][i]->getName() << " " << M[j][i]->getSub1()
            // << " " << M[j][i]->getSub2() << " "; cout <<
            // M[j][i]->getVar() << " " << M[j][i]->getVar2() <<
            // "\tAssign: "; cout << M[j][i]; cout
            // << endl;
            if (M[j][i] == 1 && IO == 1) {
                // input match
                if (j == 2 * cirmgr.inputNum_ckt1) {
                    // port2 == CONST 0
                    if (constGroup
                            .insert(
                                pair<string, bool>(cirmgr.portname_ckt2[i], 0))
                            .second == false) {
                        cout << "failed to bind " << cirmgr.portname_ckt2[i]
                             << "with const 0 !!!" << endl;
                    }
                    // inputMatch.push_back(
                    //     tuple<bool, Var, Var>(0, -1, y[i]->getVar3()));
                } else if (j == 2 * cirmgr.inputNum_ckt1 + 1) {
                    // port2 == CONST 1
                    // cout << cirmgr.portname_ckt2[i] << " ==  " << 1 << endl;
                    if (constGroup
                            .insert(
                                pair<string, bool>(cirmgr.portname_ckt2[i], 1))
                            .second == false) {
                        cout << "failed to bind " << cirmgr.portname_ckt2[i]
                             << "with const 1 !!!" << endl;
                    }
                    // inputMatch.push_back(
                    //     tuple<bool, Var, Var>(0, -2, y[i]->getVar3()));
                } else if (j % 2 == 0) {
                    // port1 == port2
                    // cout << cirmgr.portname_ckt2[i]
                    //      << " ==  " << cirmgr.portname_ckt1[j / 2] << endl;
                    if (inputGroup.find(cirmgr.portname_ckt1[j / 2]) ==
                        inputGroup
                            .end()) // if inputGroup[cirmgr.portname_ckt1[j /
                                    // 2]] has not yet insert
                        inputGroup[cirmgr.portname_ckt1[j / 2]] = Group();
                    inputGroup[cirmgr.portname_ckt1[j / 2]].group().push_back(
                        pair<string, bool>(cirmgr.portname_ckt2[i], 1));
                    // inputMatch.push_back(tuple<bool, Var, Var>(
                    //     0, x[j / 2]->getVar3(), y[i]->getVar3()));
                } else {
                    // port1 == ! port2
                    // cout << cirmgr.portname_ckt2[i] << " == !"
                    //      << cirmgr.portname_ckt1[j / 2] << endl;
                    if (inputGroup.find(cirmgr.portname_ckt1[j / 2]) ==
                        inputGroup
                            .end()) // if inputGroup[cirmgr.portname_ckt1[j /
                                    // 2]] has not yet insert
                        inputGroup[cirmgr.portname_ckt1[j / 2]] = Group();
                    inputGroup[cirmgr.portname_ckt1[j / 2]].group().push_back(
                        pair<string, bool>(cirmgr.portname_ckt2[i], 0));
                    // inputMatch.push_back(tuple<bool, Var, Var>(
                    //     1, x[j / 2]->getVar3(), y[i]->getVar3()));
                }
            } else if (M[j][i] == 1 && IO == 2) {
                // output match
                if (j % 2 == 0) {
                    // port1 == port2
                    // cout << cirmgr.portname_ckt2[i + cirmgr.inputNum_ckt2]
                    //      << " ==  "
                    //      << cirmgr.portname_ckt1[j / 2 +
                    //      cirmgr.inputNum_ckt1]
                    //      << endl;
                    if (outputGroup.find(
                            cirmgr
                                .portname_ckt1[j / 2 + cirmgr.inputNum_ckt1]) ==
                        outputGroup
                            .end()) // if inputGroup[cirmgr.portname_ckt1[j /
                                    // 2]] has not yet insert
                        outputGroup[cirmgr.portname_ckt1
                                        [j / 2 + cirmgr.inputNum_ckt1]] =
                            Group();
                    outputGroup[cirmgr.portname_ckt1[j / 2 +
                                                     cirmgr.inputNum_ckt1]]
                        .group()
                        .push_back(pair<string, bool>(
                            cirmgr.portname_ckt2[i + cirmgr.inputNum_ckt2], 1));
                    // outputMatch.push_back(tuple<bool, Var, Var>(
                    //     0, f[j / 2]->getVar3(), g[i]->getVar3()));

                } else {
                    // port1 == ! port2
                    // cout << cirmgr.portname_ckt2[i + cirmgr.inputNum_ckt2]
                    //      << " == !"
                    //      << cirmgr.portname_ckt1[j / 2 +
                    //      cirmgr.inputNum_ckt1]
                    //      << endl;
                    if (outputGroup.find(
                            cirmgr
                                .portname_ckt1[j / 2 + cirmgr.inputNum_ckt1]) ==
                        outputGroup
                            .end()) // if inputGroup[cirmgr.portname_ckt1[j /
                                    // 2]] has not yet insert
                        outputGroup[cirmgr.portname_ckt1
                                        [j / 2 + cirmgr.inputNum_ckt1]] =
                            Group();
                    outputGroup[cirmgr.portname_ckt1[j / 2 +
                                                     cirmgr.inputNum_ckt1]]
                        .group()
                        .push_back(pair<string, bool>(
                            cirmgr.portname_ckt2[i + cirmgr.inputNum_ckt2], 0));
                    // outputMatch.push_back(tuple<bool, Var, Var>(
                    //     1, f[j / 2]->getVar3(), g[i]->getVar3()));
                }
            }
        }
    }
    cout << endl;
}

void
SatMgr::generateResult() {
    ofstream fout;
    fout.open("output.txt");
    if (!inputGroup.empty()) {
        for (unordered_map<string, Group>::iterator i = inputGroup.begin(); i != inputGroup.end(); i++) {
            fout << "INGROUP" << endl;
            fout << "1 + " << (*i).first << endl;
            for (vector<std::pair<std::string, bool>>::iterator j = (*i).second.group().begin(); j != (*i).second.group().end(); j++)
                fout << "2" << ((*j).second ? " + " : " - ") << (*j).first << endl;
            fout << "END" << endl;
        }
    }
    if (!outputGroup.empty()) {
        for (unordered_map<string, Group>::iterator i = outputGroup.begin(); i != outputGroup.end(); i++) {
            fout << "OUTGROUP" << endl;
            fout << "1 + " << (*i).first << endl;
            for (vector<std::pair<std::string, bool>>::iterator j = (*i).second.group().begin(); j != (*i).second.group().end(); j++)
                fout << "2" << ((*j).second ? " + " : " - ") << (*j).first << endl;
            fout << "END" << endl;
        }
    }
    if (!constGroup.empty()) {
        fout << "CONSTGROUP" << endl;
        for (unordered_map<string, bool>::iterator i = constGroup.begin(); i != constGroup.end(); i++)
            fout << ((*i).second ? "- " : "+ ") << (*i).first << endl;
        fout << "END" << endl;
    }
    fout.close();
}
void
SatMgr::printMatrix(const SatSolver& s, vector<vector<int>> const& M,
                    int IO) { // IO: 1 -> MI, 2 -> MO
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    for (int j = 0; j < M.size(); j++) {
        for (int i = 0; i < M[0].size(); i++) {
            // cout << M[j][i]->getName() << " " << M[j][i]->getSub1()
            // << " " << M[j][i]->getSub2() << " "; cout <<
            // M[j][i]->getVar() << " " << M[j][i]->getVar2() <<
            // "\tAssign: "; cout << M[j][i]; cout
            // << endl;
            if (M[j][i] == 1 && IO == 1) {
                // input match
                if (j == 2 * cirmgr.inputNum_ckt1) {
                    // port2 == CONST 0
                    cout << cirmgr.portname_ckt2[i] << " ==  " << 0 << endl;
                    inputMatch.push_back(
                        tuple<bool, Var, Var>(0, -1, y[i]->getVar3()));
                } else if (j == 2 * cirmgr.inputNum_ckt1 + 1) {
                    // port2 == CONST 1
                    cout << cirmgr.portname_ckt2[i] << " ==  " << 1 << endl;
                    inputMatch.push_back(
                        tuple<bool, Var, Var>(0, -2, y[i]->getVar3()));
                } else if (j % 2 == 0) {
                    // port1 == port2
                    cout << cirmgr.portname_ckt2[i]
                         << " ==  " << cirmgr.portname_ckt1[j / 2] << endl;
                    inputMatch.push_back(tuple<bool, Var, Var>(
                        0, x[j / 2]->getVar3(), y[i]->getVar3()));
                } else {
                    // port1 == ! port2
                    cout << cirmgr.portname_ckt2[i] << " == !"
                         << cirmgr.portname_ckt1[j / 2] << endl;
                    inputMatch.push_back(tuple<bool, Var, Var>(
                        1, x[j / 2]->getVar3(), y[i]->getVar3()));
                }
            } else if (M[j][i] == 1 && IO == 2) {
                // output match
                if (j % 2 == 0) {
                    // port1 == port2
                    cout << cirmgr.portname_ckt2[i + cirmgr.inputNum_ckt2]
                         << " ==  "
                         << cirmgr.portname_ckt1[j / 2 + cirmgr.inputNum_ckt1]
                         << endl;
                    outputMatch.push_back(tuple<bool, Var, Var>(
                        0, f[j / 2]->getVar3(), g[i]->getVar3()));

                } else {
                    // port1 == ! port2
                    cout << cirmgr.portname_ckt2[i + cirmgr.inputNum_ckt2]
                         << " == !"
                         << cirmgr.portname_ckt1[j / 2 + cirmgr.inputNum_ckt1]
                         << endl;
                    outputMatch.push_back(tuple<bool, Var, Var>(
                        1, f[j / 2]->getVar3(), g[i]->getVar3()));
                }
            }
        }
    }
    cout << endl;
}
// void
// SatMgr::printArr(const SatSolver& s, vector<variable*> const& Arr) {
//     for (int i = 0; i < Arr.size(); i++) {
//         cout << Arr[i]->getName() << " " << Arr[i]->getSub1() << " "
//              << Arr[i]->getSub2() << " ";
//         cout << Arr[i]->getVar() << " " << Arr[i]->getVar2() << "\tAssign: ";
//         cout << s.getValue(Arr[i]->getVar());
//         cout << endl;
//     }
//     cout << endl;
// }

// template <typename K, typename V>
// void
// SatMgr::print_map(unordered_map<K, V> const& m) {
//     for (auto const& pair : m) {
//         std::cout << "{" << pair.first << ": " << pair.second << "}\n";
//     }
// }

void
SatMgr::constraint2(
    SatSolver& s) { // for solver1, j = 1 ~ mO :sum(cij+dij)  <= 1, for
                    // all i = 1 ~ nO
                    // could be improved by pseudo boolean constraint
    // cout <<  "MO " << MO.size() << " " << MO[0].size() << endl;
    vector<vector<variable*>>& MO = cirmgr.MO;
    for (int i = 0; i < MO[0].size(); i++) {
        /*test*/
        vec<Lit> lits_n0; // c_i 1 + d_i 1 + c_i 2 + d_i 2 + ... to
                          // prevent all zero
        /*test*/

        for (int j1 = 0; j1 < MO.size(); j1++) {
            for (int j2 = j1 + 1; j2 < MO.size(); j2++) {
                vec<Lit> lits;
                // (cij' + cij'), (cij' + dij') could be omitted by
                // constraint 5
                // && !(j1%2 == 0 && j2 == j1 + 1)
                Lit      a = ~Lit(MO[j1][i]->getVar());
                lits.push(a);
                Lit b = ~Lit(MO[j2][i]->getVar());
                lits.push(b);
                s.addClause(lits);
                lits.clear();
            }
            /*test*/
            Lit c = Lit(MO[j1][i]->getVar());
            lits_n0.push(c);
            /*test*/
        }
        /*test*/
        s.addClause(lits_n0);
        lits_n0.clear();
        /*test*/
    }
    // cout << "GN" << MI[0][0]->getName() << endl;
}
void
SatMgr::constraint_Cmdr(SatSolver& s, vector<vector<variable*>>& M, bool is_MO) {
    cout << "MO0size: " << M[0].size() << endl;
    cout << "MO size: " << M.size() << endl;
    for (size_t i = 0; i < M[0].size(); i++) {

        // construct Cmdr Subcord
        cirmgr.Cmdr_level.clear();
        for (size_t j = 0; j < M.size(); j++) {
            Cmdr* newCmdr = new Cmdr(M[j][i]->getVar(), true);
            // cout << j << " " << i << " " << M[j][i]->getVar() << endl;
            cirmgr.Cmdr_level.push_back(newCmdr);
        }
        // cout << "CMDRLEVEL size: " << cirmgr.Cmdr_level.size() << endl;
        constructCmdr(s);
        Cmdr*    root = cirmgr.Cmdr_level[0];
        vec<Lit> lits;
        Lit      a = Lit(root->getVar());
        lits.push(a);
        s.addClause(lits);
        lits.clear();
        cmdrExactlyOne(s, root, is_MO);
    }
}
void SatMgr::constraint_Cmdr_control(SatSolver& s, Var control, vector<Var>& v){ // for functional support, if control == 1 -> only one Var in vector<Var> will be 1
    cirmgr.Cmdr_level.clear();
    for(int i = 0; i < v.size(); i++){
        Cmdr* newCmdr = new Cmdr(v[i], true);
        cirmgr.Cmdr_level.push_back(newCmdr);
    }
    constructCmdr(s);
    Cmdr*    root = cirmgr.Cmdr_level[0];
    vec<Lit> lits;
    Lit      a = Lit(root->getVar());
    lits.push(a);
    lits.push(~Lit(control)); // if control == true -> only one Var in vector<Var> will be 1
    s.addClause(lits);
    lits.clear();
    cmdrExactlyOne(s, root, false); // is_MO == false: exactly one will be true
}
void SatMgr::constraint_Cmdr_nocontrol(SatSolver& s, vector<Var>& v, bool isAtmostOne){ // only one Var in vector<Var> will be 1
    cirmgr.Cmdr_level.clear();
    for(int i = 0; i < v.size(); i++){
        Cmdr* newCmdr = new Cmdr(v[i], true);
        cirmgr.Cmdr_level.push_back(newCmdr);
    }
    constructCmdr(s);
    Cmdr*    root = cirmgr.Cmdr_level[0];
    vec<Lit> lits;
    Lit      a = Lit(root->getVar());
    lits.push(a);
    s.addClause(lits);
    lits.clear();
    cmdrExactlyOne(s, root, isAtmostOne); // is_MO == false: exactly one will be true
}
void SatMgr::busMatchExactlyOne(SatSolver& s){
    for(int i = 0; i < cirmgr.MIbus_Var.size(); i++){
        constraint_Cmdr_nocontrol(s, cirmgr.MIbus_Var[i], true);
    }
    for(int i = 0; i < cirmgr.MObus_Var.size(); i++){
        constraint_Cmdr_nocontrol(s, cirmgr.MObus_Var[i], true);
    }
    
    for(int j = 0; j < cirmgr.MIbus_Var[0].size(); j++){
        vector<Var> tmp;
        for(int i = 0; i < cirmgr.MIbus_Var.size(); i++){
            tmp.push_back(cirmgr.MIbus_Var[i][j]);
        }
        constraint_Cmdr_nocontrol(s, tmp, true);
        tmp.clear();
    }
    for(int j = 0; j < cirmgr.MObus_Var[0].size(); j++){
        vector<Var> tmp;
        for(int i = 0; i < cirmgr.MObus_Var.size(); i++){
            tmp.push_back(cirmgr.MObus_Var[i][j]);
        }
        constraint_Cmdr_nocontrol(s, tmp, true);
        tmp.clear();
    }

}
void
SatMgr::constructCmdr(SatSolver& s) {
    vector<Cmdr*> Cmdr_level_temp;
    size_t        group_index = 0;
    if (cirmgr.Cmdr_level.size() == 1) return;

    Cmdr* newCmdr;
    for (size_t i = 0; i < cirmgr.Cmdr_level.size(); i++) {
        if (i % 3 == 0) {
            newCmdr = new Cmdr(s.newVar(), false);
            Cmdr_level_temp.push_back(newCmdr);
        }
        newCmdr->Subords.push_back(cirmgr.Cmdr_level[i]);
    }
    cirmgr.Cmdr_level.clear();
    for (size_t i = 0; i < Cmdr_level_temp.size(); i++) {
        cirmgr.Cmdr_level.push_back(Cmdr_level_temp[i]);
    }
    constructCmdr(s);
    return;
}

void
SatMgr::cmdrExactlyOne(SatSolver& s, Cmdr* parent, bool is_MO) {
    // cout << "p: " << parent->getVar() << " ";
    // for (int i = 0; i < parent->Subords.size(); i++) {
    //     cout << "s" << i << ": " << parent->Subords[i]->getVar() << " ";
    // }
    // cout << endl;
    if (parent->getIsVar()) return;
    if (parent->Subords.size() == 1) {
        // cout << "E1" << endl;
        vec<Lit> lits;
        Lit      np = ~Lit(parent->getVar());
        Lit      n0 = ~Lit(parent->Subords[0]->getVar());
        Lit      pp = Lit(parent->getVar());
        Lit      p0 = Lit(parent->Subords[0]->getVar());
        lits.push(pp);
        lits.push(n0);
        s.addClause(lits);
        lits.clear();
        if (!is_MO){
            lits.push(np);
            lits.push(p0);
            s.addClause(lits);
            lits.clear();
        }
        cmdrExactlyOne(s, parent->Subords[0], is_MO);
    } else if (parent->Subords.size() == 2) {
        // cout << "E2" << endl;
        vec<Lit> lits;
        Lit      np = ~Lit(parent->getVar());
        Lit      n0 = ~Lit(parent->Subords[0]->getVar());
        Lit      n1 = ~Lit(parent->Subords[1]->getVar());
        Lit      pp = Lit(parent->getVar());
        Lit      p0 = Lit(parent->Subords[0]->getVar());
        Lit      p1 = Lit(parent->Subords[1]->getVar());
        lits.push(n0);
        lits.push(n1);
        s.addClause(lits);
        lits.clear();
        if (!is_MO){
            lits.push(np);
            lits.push(p0);
            lits.push(p1);
            s.addClause(lits);
            lits.clear();
        }
        lits.push(pp);
        lits.push(n0);
        s.addClause(lits);
        lits.clear();
        lits.push(pp);
        lits.push(n1);
        s.addClause(lits);
        lits.clear();
        cmdrExactlyOne(s, parent->Subords[0], is_MO);
        cmdrExactlyOne(s, parent->Subords[1], is_MO);
    } else if (parent->Subords.size() == 3) {
        // cout << "E3" << endl;
        vec<Lit> lits;
        Lit      np = ~Lit(parent->getVar());
        Lit      n0 = ~Lit(parent->Subords[0]->getVar());
        Lit      n1 = ~Lit(parent->Subords[1]->getVar());
        Lit      n2 = ~Lit(parent->Subords[2]->getVar());
        Lit      pp = Lit(parent->getVar());
        Lit      p0 = Lit(parent->Subords[0]->getVar());
        Lit      p1 = Lit(parent->Subords[1]->getVar());
        Lit      p2 = Lit(parent->Subords[2]->getVar());
        lits.push(n0);
        lits.push(n1);
        s.addClause(lits);
        lits.clear();
        lits.push(n1);
        lits.push(n2);
        s.addClause(lits);
        lits.clear();
        lits.push(n0);
        lits.push(n2);
        s.addClause(lits);
        lits.clear();
        if (!is_MO){
            lits.push(np);
            lits.push(p0);
            lits.push(p1);
            lits.push(p2);
            s.addClause(lits);
            lits.clear();
        }
        lits.push(pp);
        lits.push(n0);
        s.addClause(lits);
        lits.clear();
        lits.push(pp);
        lits.push(n1);
        s.addClause(lits);
        lits.clear();
        lits.push(pp);
        lits.push(n2);
        s.addClause(lits);
        lits.clear();
        cmdrExactlyOne(s, parent->Subords[0], is_MO);
        cmdrExactlyOne(s, parent->Subords[1], is_MO);
        cmdrExactlyOne(s, parent->Subords[2], is_MO);
    }
}

void
SatMgr::constraint3(
    SatSolver& s) { // for solver1, j = 1 ~ mI+1 :sum(aij+bij)  = 1, for
                    // all i = 1 ~ nI
                    // could be improved by pseudo boolean constraint
    vector<vector<variable*>>& MI = cirmgr.MI;
    for (int i = 0; i < MI[0].size(); i++) {
        vec<Lit> lits_n0; // a_i 1 + b_i 1 + a_i 2 + b_i 2 + ... to
                          // prevent all zero
        for (int j1 = 0; j1 < MI.size(); j1++) {
            for (int j2 = j1 + 1; j2 < MI.size(); j2++) {
                vec<Lit> lits;
                // (aij' + aij'), (aij' + bij') could be omitted by
                // constraint 5
                Lit      a = ~Lit(MI[j1][i]->getVar());
                lits.push(a);
                Lit b = ~Lit(MI[j2][i]->getVar());
                lits.push(b);
                s.addClause(lits);
                lits.clear();
            }
            Lit c = Lit(MI[j1][i]->getVar());
            lits_n0.push(c);
        }
        s.addClause(lits_n0);
        lits_n0.clear();
    }
}
void
SatMgr::constraint4_miter(SatSolver& s) { // for mitersolver, intentionally
                                          // make cji -> fi != gj
    vec<Lit>    lit_vec;
    vec<Lit>    lit_vec2; // let (result + constant 1), this can prevent
                          // that if I assume result = 0 when cij + dij = 0
    vector<Var> temp;
    vector<vector<variable*>>& MO = cirmgr.MO;
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    // cout << "miter: " << endl;
    for (int j = 0; j < MO[0].size(); j++) {
        for (int i = 0; i < MO.size(); i = i + 2) {
            Var temp1  = s.newVar(); // fi xor gj = temp1
            Var temp2  = s.newVar(); // ~temp1 and c_ij = temp2
            Var temp3  = s.newVar(); // temp1 and d_ij = temp3
            Var result = s.newVar(); // ~temp2 and ~temp3 = result
            s.addXorCNF(temp1, f[i / 2]->getVar2(), false, g[j]->getVar2(),
                        false);
            s.addAigCNF(temp2, temp1, true, MO[i][j]->getVar2(), false);
            s.addAigCNF(temp3, temp1, false, MO[i + 1][j]->getVar2(), false);
            s.addAigCNF(result, temp2, true, temp3, true);
            Var enable = s.newVar();
            lit_vec2.push(Lit(result));
            lit_vec2.push(Lit(enable));
            s.addClause(lit_vec2);
            lit_vec2.clear();
            // cout << f[i/2]->getVar2() << " " << g[j]->getVar2() << "
            // " << MO[i][j]->getVar2() << " " << MO[i+1][j]->getVar2()
            // << endl;
            lit_vec.push(~Lit(enable));
            temp.push_back(enable);
            // cout << "enable: " << enable << endl;
        }
        big_or.push_back(temp);
        temp.clear();
    }
    s.addClause(lit_vec);
    lit_vec.clear();
}
void
SatMgr::constraint5_miter(
    SatSolver& s) { // for mitersolver, aji -> xi == yj;  bji -> xi !=
    // yj;  aj MI+1 -> 0 == yj; bj MI+1 -> 1 == yj;
    vector<vector<variable*>>& MI = cirmgr.MI;
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    for (int j = 0; j < MI[0].size(); j++) {
        vec<Lit> lit_const;

        // constant 0 binding
        Lit a_const = ~Lit(MI[MI.size() - 2][j]->getVar2());
        lit_const.push(a_const);
        Lit y_neg = ~Lit(y[j]->getVar2());
        lit_const.push(y_neg);
        s.addClause(lit_const);
        lit_const.clear();

        // constant 1 binding
        Lit b_const = ~Lit(MI[MI.size() - 1][j]->getVar2());
        lit_const.push(b_const);
        Lit y_pos = Lit(y[j]->getVar2());
        lit_const.push(y_pos);
        s.addClause(lit_const);
        lit_const.clear();

        for (int i = 0; i < MI.size() - 2; i++) {
            if (i % 2 == 0) {
                vec<Lit> lits;
                Lit      ab = ~Lit(MI[i][j]->getVar2());
                lits.push(ab);
                Lit xv = Lit(x[i / 2]->getVar2());
                lits.push(xv);
                Lit yv = ~Lit(y[j]->getVar2());
                lits.push(yv);
                s.addClause(lits);
                lits.clear();

                lits.push(ab);
                xv = ~Lit(x[i / 2]->getVar2());
                lits.push(xv);
                yv = Lit(y[j]->getVar2());
                lits.push(yv);
                s.addClause(lits);
                lits.clear();
            } else {
                vec<Lit> lits;
                Lit      ab = ~Lit(MI[i][j]->getVar2());
                lits.push(ab);
                Lit xv = Lit(x[i / 2]->getVar2());
                lits.push(xv);
                Lit yv = Lit(y[j]->getVar2());
                lits.push(yv);
                s.addClause(lits);
                lits.clear();

                lits.push(ab);
                xv = ~Lit(x[i / 2]->getVar2());
                lits.push(xv);
                yv = ~Lit(y[j]->getVar2());
                lits.push(yv);
                s.addClause(lits);
                lits.clear();
            }
        }
    }
}


void SatMgr::addSuppConstraint()
{
    vector<variable*>&f = cirmgr.f, &g = cirmgr.g;
    int cnt = 0;
    vec<Lit> lits;
    for (size_t i = 0, ni = f.size(); i < ni; ++i) {
        for (size_t j = 0, nj = g.size(); j < nj; ++j) {
            if (f[i]->suppSize() > g[j]->suppSize()) {
                closeMatching(lits, i * 2, j, 0);    //close output positve matching
                closeMatching(lits, i * 2 + 1, j, 0);    //close output negative matching
                // lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                // solver.addClause(lits);
                // lits.clear();
                // lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                // solver.addClause(lits);
                // lits.clear();
                cirmgr.MO_valid[i][j] = false;
            }
        }
    }
    return;

}
void SatMgr::addSuppConstraint_input()
{
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y;
    int cnt = 0;
    vec<Lit> lits;
    for (size_t i = 0, ni = x.size(); i < ni; ++i) {
        
        for (size_t j = 0, nj = y.size(); j < nj; ++j) {
            // if (x[i]->_funcSupp_PI.size() != y[j]->_funcSupp_PI.size()) { // disable constant matching, case04 can be done in 13 seconds, should add into close_constant_assump..
            if (x[i]->_funcSupp_PI.size() > y[j]->_funcSupp_PI.size()) { // enable constant matching, case04 can't be done
                closeMatching(lits, i * 2, j, 1);    //close output positve matching
                closeMatching(lits, i * 2 + 1, j, 1);    //close output negative matching
                // lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                // solver.addClause(lits);
                // lits.clear();
                // lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                // solver.addClause(lits);
                // lits.clear();
                cirmgr.MI_valid[i][j] = false;
            }
        }
    }
    return;

}
void SatMgr::addUnateConstraint(bool _isInput)// |Uo1| <= |Uo2| ; |Ui1| <= |Ui2|
{
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    int cnt = 0;
    vec<Lit> lits;
            if(_isInput){
        for(size_t i = 0, ni = x.size(); i < ni; ++i){
            for(size_t j = 0, nj = y.size(); j < nj; ++j){
                size_t x_unateNum_p = x[i]->inputUnateNum_p(), x_unateNum_n = x[i]->inputUnateNum_n(),
                       y_unateNum_p = y[j]->inputUnateNum_p(), y_unateNum_n = y[j]->inputUnateNum_n();
                int mo_valid = 0;
                if((x_unateNum_p >= y_unateNum_p && x_unateNum_n > y_unateNum_n )|| (x_unateNum_p > y_unateNum_p && x_unateNum_n >= y_unateNum_n)){
                    closeMatching(lits, i * 2, j, 1);   //close input positve matching
                    // ++mo_valid;
                }
                if((x_unateNum_n >= y_unateNum_p && x_unateNum_p > y_unateNum_n )||( x_unateNum_n > y_unateNum_p && x_unateNum_p >= y_unateNum_n)){
                    closeMatching(lits, i * 2 + 1, j, 1);   //close input negative matching
                    // ++mo_valid;
                }
                // if(mo_valid == 2)
                //     cirmgr.MO_valid[i][j] = false;
            }
        }
            }
            else{
        for (size_t i = 0, ni = f.size(); i < ni; ++i) {
            for (size_t j = 0, nj = g.size(); j < nj; ++j) {
                size_t f_unateNum = f[i]->outputUnateNum(), g_unateNum = g[j]->outputUnateNum();
                if (f_unateNum > g_unateNum) {
                    closeMatching(lits, i * 2, j, 0);    //close output positve matching
                    closeMatching(lits, i * 2 + 1, j, 0);    //close output negative matching
                            // cirmgr.MO_valid[i][j] = false;
                }
            }
        }
    }
            // size_t _isInput ? f[i]->inputUnateNum() : f[i]->outputUnateNum(),
            //        g_unateNum = _isInput ? g[j]->inputUnateNum() : g[j]->outputUnateNum();
            // if (f_unateNum > g_unateNum) {
            //     ++cnt;
            //     vec<Lit> lits;
            //     closeMatching(lits, i * 2, j, 0);    //close output positve matching
            //     closeMatching(lits, i * 2 + 1, j, 0);    //close output negative matching
            //     // lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
            //     // solver.addClause(lits);
            //     // lits.clear();
            //     // lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
            //     // solver.addClause(lits);
            //     // lits.clear();

            // }
        // }
    // }
    return;
}
void SatMgr::addOutput0Constraint(){
    vector<variable*>&f = cirmgr.f;
    vec<Lit> lits;
    for(size_t i = 0, ni = cirmgr.output0.size(); i < ni; ++i){
        for(size_t j = 0, nj = cirmgr.MO[cirmgr.output0[i]].size(); j < nj; ++j){
            closeMatching(lits, cirmgr.output0[i], j, 0);
        }
    }
    return;

}

void SatMgr::addSymmConstraint(SatSolver& s)
{
    vector<variable*>&f = cirmgr.f, &g = cirmgr.g;

    for (int i = 0, n = f.size(); i < n; i++) {
        for (int j = 0, n1 = g.size(); j < n1; j++) {
            if (cirmgr.MO_valid[i][j]) {
                if (f[i]->suppSize() == g[j]->suppSize()){ // if f's functional support size == g's functional support size
                    cout << i << " " << j << endl;
                    if (f[i]->symmetric_set.size() > g[j]->symmetric_set.size()) {
                        continue; // the number of symmetric group of f[i] > the number of symmetric group of g[j]
                    }

                    cout << f[i]->symmetric_set.size() << " " << g[j]->symmetric_set.size() << endl;
                    for (int a = 0, n2 = f[i]->symmetric_set.size(); a < n2; a++) {
                        for (int b = 0, n3 = g[j]->symmetric_set.size(); b < n3; b++) {
                            cout << "size: " << f[i]->symmetric_set[a].size() << " " << g[j]->symmetric_set[b].size() << endl;
                            if (f[i]->symmetric_set[a].size() == g[j]->symmetric_set[b].size()) {
                                for (unordered_set<int>::iterator c = f[i]->symmetric_set[a].begin(); c != f[i]->symmetric_set[a].end(); c++) {
                                    for (unordered_set<int>::iterator d = g[j]->symmetric_set[b].begin(); d != g[j]->symmetric_set[b].end(); d++) {
                                        for (unordered_set<int>::iterator e = f[i]->symmetric_set[a].begin(); e != f[i]->symmetric_set[a].end(); e++) {
                                            if (c == e) {
                                                continue;
                                            }
                                            else {
                                                cout << "abc:  ";
                                                vec<Lit> v;
                                                v.push(~Lit(cirmgr.MO_valid_Var[i][j]));
                                                v.push(~Lit(cirmgr.MI_valid_Var[*c][*d]));
                                                for (unordered_set<int>::iterator m = g[j]->symmetric_set[b].begin(); m != g[j]->symmetric_set[b].end(); m++) {
                                                    if (d == m) {
                                                        continue;
                                                    }
                                                    else {
                                                        v.push(Lit(cirmgr.MI_valid_Var[*e][*m]));
                                                        cout << "MO: " << i << " " << j << " MI: " << *c << " " << *d << " R: " << *e << " " << *m << endl;
                                                    }
                                                }
                                                s.addClause(v);
                                                v.clear();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void SatMgr::addSameSuppSizeConstraint(SatSolver& s)
{
    vector<variable*>&f = cirmgr.f, &g = cirmgr.g;
    
    for (int i = 0, n = f.size(); i < n; i++) {
        for (int j = 0, n1 = g.size(); j < n1; j++) {
            if (cirmgr.MO_valid[i][j]) {
                if (f[i]->suppSize() == g[j]->suppSize()){ // if f's functional support size == g's functional support size
                    unordered_set<int> g_temp;
                    unordered_set<int> f_temp;
                    for (int b = 0, n3 = g[j]->_funcSupp.size(); b < n3; b++) {
                        g_temp.insert(cirmgr.u_name_index_ckt2[g[j]->_funcSupp[b]->getname()]);
                    }
                    for (int a = 0, n2 = f[i]->_funcSupp.size(); a < n2; a++) {
                        f_temp.insert(cirmgr.u_name_index_ckt1[f[i]->_funcSupp[a]->getname()]);
                    }
                    for (int b = 0, n3 = g[j]->_funcSupp.size(); b < n3; b++) {
                        for (int c = 0, n4 = cirmgr.inputNum_ckt1; c < n4; c++) {
                            vec<Lit> v;
                            v.push(~Lit(cirmgr.MO_valid_Var[i][j]));
                            if (!f_temp.count(c)) {
                                v.push(~Lit(cirmgr.MI_valid_Var[c][cirmgr.u_name_index_ckt2[g[j]->_funcSupp[b]->getname()]]));
                                s.addClause(v);
                                v.clear();
                            }
                        }
                    }
                }
            }
        }
    }
}

void SatMgr::addOutputGroupingConstraint()  // will not match outputs with different group number if |f| != |g|
{
    vector<variable*>&f = cirmgr.f, &g = cirmgr.g;
    vec<Lit> lits;
    for (size_t i = 0, ni = f.size(); i < ni; ++i) {
        for (size_t j = 0, nj = g.size(); j < nj; ++j) {
            if (f[i]->outputGroupingNum() != g[j]->outputGroupingNum()) {
                closeMatching(lits, i * 2, j, 0);    //close output positve matching
                closeMatching(lits, i * 2 + 1, j, 0);    //close output negative matching
                // lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                // solver.addClause(lits);
                // lits.clear();
                // lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                // solver.addClause(lits);
                // lits.clear();
                cirmgr.MO_valid[i][j] = false;
            }
        }
    }
    return;
}
void SatMgr::addBusConstraint_inputUnateness(){
    vector<Bus*>& bus_ckt1_output = cirmgr.bus_ckt1_output, &bus_ckt2_output = cirmgr.bus_ckt2_output;
    vec<Lit> lits;
    int cnt = 0;
    cout << "\nin addBusConstraint_inputUnateness\n";
    for(size_t i = 0, ni = bus_ckt1_output.size(); i < ni; ++i){
        for(size_t j = 0, nj = bus_ckt2_output.size(); j < nj; ++j){
            // cout << "bus_ckt1_input[i]->outputUnatenessNum() = " << bus_ckt1_output[i]->outputUnatenessNum();
            // cout << " ; bus_ckt2_input[j]->outputUnatenessNum() = " << bus_ckt2_output[j]->outputUnatenessNum() << endl;
            if(bus_ckt1_output[i]->outputUnatenessNum() > bus_ckt2_output[j]->outputUnatenessNum()){
                cnt++;
                closeMatching_bus(lits, i , j, 0);
            }
        }
    }
    cout << "cnt = " << cnt << endl;
}
void SatMgr::addBusConstraint_outputUnateness(){
    vector<Bus*>& bus_ckt1_input = cirmgr.bus_ckt1_input, &bus_ckt2_input = cirmgr.bus_ckt2_input;
    vec<Lit> lits;
    int cnt = 0;
    cout << " \nin addBusConstraint_outputUnateness \n";
    for(size_t i = 0, ni = bus_ckt1_input.size(); i < ni; ++i){
        for(size_t j = 0, nj = bus_ckt2_input.size(); j < nj; ++j){
            // cout << "bus_ckt1_input[i]->inputUnatenessNum() = " << bus_ckt1_input[i]->inputUnatenessNum();
            // cout << " ; bus_ckt2_input[j]->inputUnatenessNum() = " << bus_ckt2_input[j]->inputUnatenessNum() << endl;
            if(bus_ckt1_input[i]->inputUnatenessNum() > bus_ckt2_input[j]->inputUnatenessNum()){
                cnt++;
                closeMatching_bus(lits, i , j, 1);
            }
        }
    }
    cout << "cnt = " << cnt << endl;
}
void SatMgr::addBusConstraint_outputSupportSize(){
    vector<Bus*>& bus_ckt1_output = cirmgr.bus_ckt1_output, &bus_ckt2_output = cirmgr.bus_ckt2_output;
    vec<Lit> lits;
    int cnt = 0;
    cout << "\n in addBusConstraint_outputSupportSize \n";
    for(size_t i = 0, ni = bus_ckt1_output.size(); i < ni; ++i){
        for(size_t j = 0, nj = bus_ckt2_output.size(); j < nj; ++j){
            // cout << "bus_ckt1_output[i]->unionSupportSize() = " << bus_ckt1_output[i]->unionSupportSize();
            // cout << " ; bus_ckt2_output[j]->unionSupportSize() = " << bus_ckt2_output[j]->unionSupportSize() << endl;
            if(bus_ckt1_output[i]->unionSupportSize() > bus_ckt2_output[j]->unionSupportSize()){
                cnt++;
                closeMatching_bus(lits, i , j, 0);
            }
        }
    }
    cout << "cnt = " << cnt << endl;

}
void SatMgr::addBusConstraint_inputSupportSize(){
    vector<Bus*>& bus_ckt1_input = cirmgr.bus_ckt1_input, &bus_ckt2_input = cirmgr.bus_ckt2_input;
    vec<Lit> lits;
    int cnt = 0;
    cout << "\n in addBusConstraint_inputSupportSize \n";
    for(size_t i = 0, ni = bus_ckt1_input.size(); i < ni; ++i){
        for(size_t j = 0, nj = bus_ckt2_input.size(); j < nj; ++j){
            // cout << "bus_ckt1_input[i]->unionInputSupportSize() = " << bus_ckt1_input[i]->unionInputSupportSize();
            // cout << " ; bus_ckt2_input[j]->unionInputSupportSize() = " << bus_ckt2_input[j]->unionInputSupportSize() << endl;
            if(bus_ckt1_input[i]->unionInputSupportSize() > bus_ckt2_input[j]->unionInputSupportSize()){
                cnt++;
                closeMatching_bus(lits, i , j, 1);
            }
        }
    }
    cout << "cnt = " << cnt << endl;
}
void SatMgr::addOutputConstraint_inputBusNum(){
    vector<variable*>&f = cirmgr.f, &g = cirmgr.g;
    vec<Lit> lits;
    int cnt = 0;
    for (size_t i = 0, ni = f.size(); i < ni; ++i) {
        for (size_t j = 0, nj = g.size(); j < nj; ++j) {
            bool _close = 0;
            // if (f[i]->suppBus_distribution()->size() > g[j]->suppBus_distribution()->size()) // g's # of bus should >= f's
            // _close = 1;
            for(size_t k = 0, nk = f[i]->suppBus_distribution()->size() < g[j]->suppBus_distribution()->size() ? f[i]->suppBus_distribution()->size() : g[j]->suppBus_distribution()->size(); k < nk; ++k)
                if(f[i]->suppBus_distribution()->at(k).second > g[j]->suppBus_distribution()->at(k).second){
                    _close = 1;
                    break;   
                }
            if(_close == 1){
                cout << "_close" << endl;cnt++;
                closeMatching(lits, i * 2, j, 0);    //close output positve matching
                closeMatching(lits, i * 2 + 1, j, 0);    //close output negative matching
                cirmgr.MO_valid[i][j] = false;
            }
        }
    }
    cout << "cnt = " << cnt << endl;
    return;
    /*int cnt = 0;
    for (size_t i = 0, ni = f.size(); i < ni; ++i) {
        for (size_t j = 0, nj = g.size(); j < nj; ++j) {
            bool _close = 0;
            if (f[i]->suppBus_size().size() > g[j]->suppBus_size().size()) // g's # of bus should >= f's
                _close = 1;
            else{
                for(size_t k = 0, nk = f[i]->suppBus_size().size() < g[j]->suppBus_size().size() ? f[i]->suppBus_size().size() : g[j]->suppBus_size().size(); k < nk; ++k)
                    if(f[i]->suppBus_size()[k] > g[j]->suppBus_size()[k])   // g's bus's # of input should >= f's bus's
                        _close = 1;
            }
            if(_close == 1){
                cout << "_close" << endl;cnt++;
                closeMatching(lits, i * 2, j, 0);    //close output positve matching
                closeMatching(lits, i * 2 + 1, j, 0);    //close output negative matching
            }
        }
    }
    cout << "cnt = "<<cnt << endl;
    return;*/
}
// cirmgr.inputNum_ckt1, outputNum_ckt1, cirmgr.inputNum_ckt2, outputNum_ckt2;
// Construct PHI<0>  (Preprocess not implemented yet.)
void SatMgr::addSymmSignConstraint(){
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    for(size_t i = 0, ni = f.size(); i < ni; ++i){
        for(size_t j = 0, nj = g.size(); j < nj; ++j){
            if(f[i]->_funcSupp.size() == g[j]->_funcSupp.size() && cirmgr.MO_valid[i][j] == true){
                for(size_t k = 0, nk = x.size(); k < nk; ++k){
                    for(size_t l = 0, nl = y.size(); l < nl; ++l){
                        if(x[k]->checkSymmSign(y[l]->symmSign(), i, j) == false && cirmgr.MI_valid[k][l] == true){
                            cout << "f" << i << " <-> " << "g" << j << " ; " << "( " << k << " ," << l << " )" << endl;
                            vec<Lit> lits;
                            lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                            lits.push(~Lit(cirmgr.MI[k * 2][l]->getVar()));
                            solver.addClause(lits);
                            lits.clear();
                            lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                            lits.push(~Lit(cirmgr.MI[k * 2 + 1][l]->getVar()));
                            solver.addClause(lits);
                            lits.clear();
                            lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                            lits.push(~Lit(cirmgr.MI[k * 2][l]->getVar()));
                            solver.addClause(lits);
                            lits.clear();
                            lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                            lits.push(~Lit(cirmgr.MI[k * 2 + 1][l]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                        }
                    }
                }
            }
        }
    }
}

void SatMgr::addSupportConstraint_whenInputMatch(){
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    for(size_t k = 0, nk = x.size(); k < nk; ++k){
        for(size_t l = 0, nl = y.size(); l < nl; ++l){
            if(cirmgr.MI_valid[k][l] == true){
                for(size_t i = 0, ni = f.size(); i < ni; ++i){
                    for(size_t j = 0, nj = g.size(); j < nj; ++j){
                        bool cond1 = std::find(x[k]->_funcSupp_PI.begin(), x[k]->_funcSupp_PI.end(), f[i]) == x[k]->_funcSupp_PI.end(); // == true -> not found; == false -> found
                        bool cond2 = std::find(y[l]->_funcSupp_PI.begin(), y[l]->_funcSupp_PI.end(), g[j]) == y[l]->_funcSupp_PI.end(); // == true -> not found; == false -> found
                        if((!cond1 && cond2) || (cond1 && !cond2)){
                            vec<Lit> lits;
                            lits.push(~Lit(cirmgr.MI[k * 2][l]->getVar()));
                            lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                            lits.push(~Lit(cirmgr.MI[k * 2][l]->getVar()));
                            lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                            lits.push(~Lit(cirmgr.MI[k * 2 + 1][l]->getVar()));
                            lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                            lits.push(~Lit(cirmgr.MI[k * 2 + 1][l]->getVar()));
                            lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                        }
                    }
                }
            }
        }
    }
}
void SatMgr::addSupportConstraint_whenOutputMatch(){
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    for(size_t i = 0, ni = f.size(); i < ni; ++i){
        for(size_t j = 0, nj = g.size(); j < nj; ++j){
            if(cirmgr.MO_valid[i][j] == true){
                for(size_t k = 0, nk = x.size(); k < nk; ++k){
                    for(size_t l = 0, nl = y.size(); l < nl; ++l){
                        bool cond1 = std::find(f[i]->_funcSupp.begin(), f[i]->_funcSupp.end(), x[k]) == f[i]->_funcSupp.end(); // == true -> not found; == false -> found
                        bool cond2 = std::find(g[j]->_funcSupp.begin(), g[j]->_funcSupp.end(), y[l]) == g[j]->_funcSupp.end(); // == true -> not found; == false -> found
                        if((!cond1 && cond2) || (cond1 && !cond2)){
                            vec<Lit> lits;
                            lits.push(~Lit(cirmgr.MI[k * 2][l]->getVar()));
                            lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                            lits.push(~Lit(cirmgr.MI[k * 2][l]->getVar()));
                            lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                            lits.push(~Lit(cirmgr.MI[k * 2 + 1][l]->getVar()));
                            lits.push(~Lit(cirmgr.MO[i * 2][j]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                            lits.push(~Lit(cirmgr.MI[k * 2 + 1][l]->getVar()));
                            lits.push(~Lit(cirmgr.MO[i * 2 + 1][j]->getVar()));
                            solver.addClause(lits);
                            lits.clear();

                        }
                    }
                }
            }
        }
    }
}
void SatMgr::addBinateConstraint(){
    vector<variable*>&f = cirmgr.f, &g = cirmgr.g;
    for(size_t i = 0, ni = f.size(); i < ni; ++i){
        for(size_t j = 0, nj = g.size(); j < nj; ++j){
            if(f[i]->outputBinateNum() < g[j]->outputBinateNum()){
                vec<Lit> lits;
                closeMatching(lits, i * 2, j, 0);
                closeMatching(lits, i * 2 + 1, j, 0);
                cirmgr.MO_valid[i][j] = false;
            }
        }
    }
}

void
SatMgr::initCircuit(SatSolver& s, SatSolver& s_miter, SatSolver& s_cir1, SatSolver& s_cir2, SatSolver& s_verifier) { // create 2D array ETs
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    // construct x, y
    for (int i = 1; i <= cirmgr.inputNum_ckt1; i++) {
        variable* tmpX = new variable('x', i, -1);
        tmpX->newSymmOutput(cirmgr.outputNum_ckt1, i - 1);
        tmpX->setname(cirmgr.portname_ckt1[i-1]);
        cirmgr.u_name_busIndex_input_ckt1[tmpX->getname()] = -1; // initialize doesn't exist in any bus, after readBus_class will overwrite with corresponding bus index, we can know those "not in any bus" port: -1
        tmpX->setVar(s.newVar());
        tmpX->setVar2(s_miter.newVar());
        tmpX->setVar3(s_verifier.newVar());
        tmpX->setVar4(s_cir1.newVar());
        x.push_back(tmpX);
    }
    for (int i = 1; i <= cirmgr.inputNum_ckt2; i++) {
        variable* tmpY = new variable('y', i, -1);
        tmpY->newSymmOutput(cirmgr.outputNum_ckt2, i - 1);
        tmpY->setname(cirmgr.portname_ckt2[i - 1]);
        cirmgr.u_name_busIndex_input_ckt2[tmpY->getname()] = -1; // initialize doesn't exist in any bus, after readBus_class will overwrite with corresponding bus index, we can know those "not in any bus" port: -1
        tmpY->setVar(s.newVar());
        tmpY->setVar2(s_miter.newVar());
        tmpY->setVar3(s_verifier.newVar());
        tmpY->setVar4(s_cir2.newVar());
        y.push_back(tmpY);
    }

    // construct f, g
    for (int i = 1; i <= cirmgr.outputNum_ckt1; i++) {
        variable* tmpF = new variable('f', i, -1);
        tmpF->setname(cirmgr.portname_ckt1[cirmgr.inputNum_ckt1 + i - 1]);
        tmpF->newSymmInput(cirmgr.inputNum_ckt1, i - 1);
        cirmgr.u_name_busIndex_output_ckt1[tmpF->getname()] = -1; // initialize doesn't exist in any bus, after readBus_class will overwrite with corresponding bus index, we can know those "not in any bus" port: -1
        tmpF->setVar(s.newVar());
        tmpF->setVar2(s_miter.newVar());
        tmpF->setVar3(s_verifier.newVar());
        tmpF->setVar4(s_cir1.newVar());
        f.push_back(tmpF);
    }
    for (int i = 1; i <= cirmgr.outputNum_ckt2; i++) {
        variable* tmpG = new variable('g', i, -1);
        tmpG->setname(cirmgr.portname_ckt2[cirmgr.inputNum_ckt2 + i - 1]);
        tmpG->newSymmInput(cirmgr.inputNum_ckt2, i - 1);
        cirmgr.u_name_busIndex_output_ckt2[tmpG->getname()] = -1; // initialize doesn't exist in any bus, after readBus_class will overwrite with corresponding bus index, we can know those "not in any bus" port: -1
        tmpG->setVar(s.newVar());
        tmpG->setVar2(s_miter.newVar());
        tmpG->setVar3(s_verifier.newVar());
        tmpG->setVar4(s_cir2.newVar());
        g.push_back(tmpG);
    }

    cirmgr.exist_x = new bool[x.size()]{0};
    cirmgr.exist_y = new bool[y.size()]{0};
    cirmgr.exist_f = new bool[f.size()]{0};
    cirmgr.exist_g = new bool[g.size()]{0};

    // construct MI matrix
    for (int j = 1; j <= 2 * (cirmgr.inputNum_ckt1 + 1); j++) { // mI
        vector<variable*> col;
        for (int i = 1; i <= cirmgr.inputNum_ckt2; i++) { // nI
            variable* tmp =
                new variable((j % 2 == 1) ? 'a' : 'b', i, (j + 1) / 2);
            tmp->setVar(s.newVar());
            tmp->setVar2(s_miter.newVar());
            col.push_back(tmp);
        }
        cirmgr.MI.push_back(col);
    }

    // construct MO matrix
    vec<Lit> lits;
    for (int j = 1; j <= 2 * cirmgr.outputNum_ckt1; j++) { // mO
        vector<variable*> col;
        for (int i = 1; i <= cirmgr.outputNum_ckt2; i++) { // nO
            variable* tmp =
                new variable((j % 2 == 1) ? 'c' : 'd', i, (j + 1) / 2);
            Var nV   = s.newVar();
            Var nV_m = s_miter.newVar();
            tmp->setVar(nV);
            tmp->setVar2(nV_m);
            col.push_back(tmp);
            Lit l = Lit(nV);
            lits.push(l);
        }
        cirmgr.MO.push_back(col);
    }

    // construct MO_no_pos_neg matrix
    // vec<Lit> lits_p_or_n; // (positive match variable + negatvie match variable) = Var in MO_no_pos_neg
    // for(int i = 0; i < (cirmgr.MO.size() / 2); i++){
    //     vector<Var> col;
    //     for(int j = 0; j < cirmgr.MO[0].size(); j++){
    //         Var tmp = s.newVar();
    //         col.push_back(tmp);

    //         lits_p_or_n.push(~Lit(tmp)); lits_p_or_n.push(Lit(cirmgr.MO[i*2][j]->getVar())); lits_p_or_n.push(Lit(cirmgr.MO[i*2+1][j]->getVar())); s.addClause(lits_p_or_n); lits_p_or_n.clear(); 
    //         lits_p_or_n.push(Lit(tmp)); lits_p_or_n.push(~Lit(cirmgr.MO[i*2][j]->getVar())); s.addClause(lits_p_or_n); lits_p_or_n.clear();
    //         lits_p_or_n.push(Lit(tmp)); lits_p_or_n.push(~Lit(cirmgr.MO[i*2+1][j]->getVar())); s.addClause(lits_p_or_n); lits_p_or_n.clear();
    //     }
    //     cirmgr.MO_no_pos_neg.push_back(col);
    // }

    // // construct MI MO_valid, to show closed:0 /open:1 match
    // cirmgr.MI_valid = new bool*[cirmgr.MI.size()];
    // for (int i = 0; i < cirmgr.MI.size(); i++){
    //     cirmgr.MI_valid[i] = new bool[cirmgr.MI[0].size()];
    // }
    // for (int i = 0; i < cirmgr.MI.size(); i++){
    //     for (int j = 0; j < cirmgr.MI[0].size(); j++){
    //         cirmgr.MI_valid[i][j] = true;
    //     }
    // }
    // cirmgr.MO_valid = new bool*[cirmgr.MO.size()];
    // for (int i = 0; i < cirmgr.MO.size(); i++){
    //     cirmgr.MO_valid[i] = new bool[cirmgr.MO[0].size()];
    // }
    // for (int i = 0; i < cirmgr.MO.size(); i++){
    //     for (int j = 0; j < cirmgr.MO[0].size(); j++){
    //         cirmgr.MO_valid[i][j] = true;
    //     }
    // }


    s.addClause(lits);
    lits.clear(); // constraint 1.  sum(sum(c_ij + d_ij)) >0

    for (int i = 0; i < cirmgr.MI.size()/2; i++){
        vector<bool> tmp;
        vector<Var>  tmp_var;
        for (int j = 0; j < cirmgr.MI[0].size(); j++){
            tmp.push_back(true);
            Var c = s.newVar();
            tmp_var.push_back(c);
            vec<Lit> v;
            v.push(~Lit(c)); v.push(Lit(cirmgr.MI[i*2][j]->getVar())); v.push(Lit(cirmgr.MI[i*2+1][j]->getVar())); s.addClause(v); v.clear();
            v.push(Lit(c)); v.push(~Lit(cirmgr.MI[i*2][j]->getVar())); s.addClause(v); v.clear();
            v.push(Lit(c)); v.push(~Lit(cirmgr.MI[i*2+1][j]->getVar())); s.addClause(v); v.clear();
        }
        cirmgr.MI_valid.push_back(tmp);
        cirmgr.MI_valid_Var.push_back(tmp_var);
    }
    // for(int i = 0; i < 100000; i++) Var t = s.newVar(); // test var
    for (int i = 0; i < cirmgr.MO.size()/2; i++){
        vector<bool> tmp;
        vector<Var>  tmp_var;
        for (int j = 0; j < cirmgr.MO[0].size(); j++){
            tmp.push_back(true);
            Var c = s.newVar();
            tmp_var.push_back(c);
            vec<Lit> v;
            v.push(~Lit(c)); v.push(Lit(cirmgr.MO[i*2][j]->getVar())); v.push(Lit(cirmgr.MO[i*2+1][j]->getVar())); s.addClause(v); v.clear();
            v.push(Lit(c)); v.push(~Lit(cirmgr.MO[i*2][j]->getVar())); s.addClause(v); v.clear();
            v.push(Lit(c)); v.push(~Lit(cirmgr.MO[i*2+1][j]->getVar())); s.addClause(v); v.clear();
        }
        cirmgr.MO_valid.push_back(tmp);
        cirmgr.MO_valid_Var.push_back(tmp_var);
    }


    // test cmdr
    //  constraint2(s);
    constraint_Cmdr(s, cirmgr.MO, true); // commander version of constraint 2
    constraint_Cmdr(s, cirmgr.MI, false); // commander version of constraint 3

    // constraint3(s);
    // solver(solver1 is used for determining feasible a, b, c, d
    // (permutations), has nothing to do with x, y, f, g)

    constraint4_miter(s_miter);
    constraint5_miter(s_miter);
}
/*void
SatMgr::readAAG() { // read aag file: top1.aag, top2.aag
    ifstream faag1, faag2;
    string   faag1_name = "", faag2_name = "";
    faag1_name = faag2_name = "./files/aag/";
    faag1_name += "top1.aag";
    faag2_name += "top2.aag";
    faag1.open(faag1_name);
    faag2.open(faag2_name);

    string aag;
    int    maxidx, ip, latch, op, andgate;
    faag1 >> aag >> maxidx >> ip >> latch >> op >> andgate;
    inputNum_ckt1  = ip;
    outputNum_ckt1 = op;

    faag2 >> aag >> maxidx >> ip >> latch >> op >> andgate;
    inputNum_ckt2  = ip;
    outputNum_ckt2 = op;

    while (true) {
        string tmp, tmp2;

        faag1 >> tmp;
        if (tmp == "i0") { // first line. ex: i0 a
            portnum_ckt1.push_back(tmp);
            faag1 >> tmp;
            portname_ckt1.push_back(tmp);
            while (true) {
                faag1 >> tmp >> tmp2;
                if (tmp == "c") break;
                portnum_ckt1.push_back(tmp);
                portname_ckt1.push_back(tmp2);
            }
            break;
        }
    }
    while (true) {
        string tmp, tmp2;
        faag2 >> tmp;
        if (tmp == "i0") { // first line. ex: i0 a
            portnum_ckt2.push_back(tmp);
            faag2 >> tmp;
            portname_ckt2.push_back(tmp);
            while (true) {
                faag2 >> tmp >> tmp2;
                if (tmp == "c") break;
                portnum_ckt2.push_back(tmp);
                portname_ckt2.push_back(tmp2);
            }
            break;
        }
    }
}*/
/*void
SatMgr::readMAP() { // read map     : map1.txt, map2.txt
    ifstream fmap1, fmap2;
    string   fmap1_name = "", fmap2_name = "";
    fmap1_name = fmap2_name = "./files/map/";
    fmap1_name += "map1";
    fmap2_name += "map2";
    fmap1.open(fmap1_name);
    fmap2.open(fmap2_name);
    int inout, aagVar, cnfVar;
    int i = 0, j = 0;
    while (fmap1 >> inout >> aagVar >> cnfVar) {
        // inout: 2 -> input, 3 -> output. But portnum has inout info,
        // don't need to store inout. aagVar doesn't matter as well.
        umapNum_ckt1[cnfVar + 1]  = portnum_ckt1[i];
        umapName_ckt1[cnfVar + 1] = portname_ckt1[i];
        i++;
    }
    while (fmap2 >> inout >> aagVar >> cnfVar) {
        // inout: 2 -> input, 3 -> output. But portnum has inout info,
        // don't need to store inout. aagVar doesn't matter as well.
        umapNum_ckt2[cnfVar + 1]  = portnum_ckt2[j];
        umapName_ckt2[cnfVar + 1] = portname_ckt2[j];
        j++;
    }
}*/
/*void
SatMgr::readCNF(SatSolver& s_miter,
                SatSolver& s_verifier) { // read cnf file: top1.cnf, top2.cnf
//     // read CNF into miter solver to constraint functionality
//     // correctness
//     ifstream fcnf1, fcnf2;
//     string   fcnf1_name = "", fcnf2_name = "";
//     fcnf1_name = fcnf2_name = "./files/cnf/";
//     fcnf1_name += "top1.cnf";
//     fcnf2_name += "top2.cnf";
//     fcnf1.open(fcnf1_name);
//     fcnf2.open(fcnf2_name);
//     string tmp, cnf;
//     int    varnum, clsnum, litTmp;
//     size_t ct1 = 0, ct2 = 0;
//     while (true) {

//         fcnf1 >> tmp;
//         if (tmp == "p") {
//             fcnf1 >> cnf >> varnum >> clsnum;
//             // cout<<"CC: " << cnf << " " << varnum  << " " << clsnum <<
//             // endl;
//             vec<Lit> lits, lits_veri;

//             vector<Var> test; // test addClause

//             while (fcnf1 >> litTmp) {

//                 // cout << "LL: " << litTmp << endl;
//                 if (litTmp != 0) {
//                     if (umapNum_ckt1.find(abs(litTmp)) ==
//                         umapNum_ckt1.end()) { // doesn't exist
//                         umapNum_ckt1[abs(litTmp)] =
//                             "n" + to_string(abs(litTmp));
//                         Var nV                         = s_miter.newVar();
//                         Var nV_veri                    = s_verifier.newVar();
//                         umapInterVar_ckt1[abs(litTmp)] = nV;
//                         umapInterVar_ckt1_veri[abs(litTmp)] = nV_veri;
//                         Lit li, li_veri;
//                         if (litTmp > 0) {
//                             li      = Lit(nV);
//                             li_veri = Lit(nV_veri);
//                         } else {
//                             li      = ~Lit(nV);
//                             li_veri = ~Lit(nV_veri);
//                         }

//                         lits.push(li);
//                         lits_veri.push(li_veri);
//                         test.push_back(nV);
//                     } else {
//                         string portNumTmp =
//                             umapNum_ckt1[abs(litTmp)]; // i0, i1 ..
//                         // cout << "PPP: " << portNumTmp << endl;
//                         // add clause to s_miter according to i/o and
//                         // num. ex: i0 --->  x[0]->getVar2()
//                         if (portNumTmp[0] == 'i') { // those var in x
//                             Lit li, li_veri;
//                             if (litTmp > 0) {
//                                 li = Lit(
// x[stoi(portNumTmp.substr(1))]->getVar2());
//                                 li_veri = Lit(
// x[stoi(portNumTmp.substr(1))]->getVar3());

//                             } else {
//                                 li = ~Lit(
// x[stoi(portNumTmp.substr(1))]->getVar2());
//                                 li_veri = ~Lit(
// x[stoi(portNumTmp.substr(1))]->getVar3());
//                             }
//                             lits.push(li);
//                             lits_veri.push(li_veri);
//                             test.push_back(
//                                 x[stoi(portNumTmp.substr(1))]->getVar2());
//                         } else if (portNumTmp[0] == 'o') { // o
//                             Lit li, li_veri;
//                             if (litTmp > 0) {
//                                 li = Lit(
// f[stoi(portNumTmp.substr(1))]->getVar2());
//                                 li_veri = Lit(
// f[stoi(portNumTmp.substr(1))]->getVar3());

//                             } else {
//                                 li = ~Lit(
// f[stoi(portNumTmp.substr(1))]->getVar2());
//                                 li_veri = ~Lit(
// f[stoi(portNumTmp.substr(1))]->getVar3());
//                             }
//                             lits.push(li);
//                             lits_veri.push(li_veri);
//                             test.push_back(
//                                 f[stoi(portNumTmp.substr(1))]->getVar2());
//                         } else { // n (intervariable)
//                             Lit li, li_veri;
//                             if (litTmp > 0) {
//                                 li = Lit(umapInterVar_ckt1[abs(litTmp)]);
//                                 li_veri =
//                                     Lit(umapInterVar_ckt1_veri[abs(litTmp)]);

//                             } else {
//                                 li = ~Lit(umapInterVar_ckt1[abs(litTmp)]);
//                                 li_veri =
// ~Lit(umapInterVar_ckt1_veri[abs(litTmp)]);
//                             }

//                             lits.push(li);
//                             lits_veri.push(li_veri);
//                             test.push_back(umapInterVar_ckt1[abs(litTmp)]);
//                         }
//                     }
//                 } else {
//                     ++ct1;
//                     cout << lits_veri.size() << endl;
//                     s_miter.addClause(lits);
//                     s_verifier.addClause(lits_veri);
//                     lits.clear();
//                     lits_veri.clear();
//                     // for(int i=0;i<test.size();i++){
//                     //    cout << test[i] << " ";
//                     // }
//                     // cout << "\n" << "testclause ---" << endl;
//                     test.clear();
//                 }
//             }

//             break;
//         }
//     }
//     while (true) {

//         fcnf2 >> tmp;
//         if (tmp == "p") {
//             fcnf2 >> cnf >> varnum >> clsnum;
//             // cout<<"CC: " << cnf << " " << varnum  << " " << clsnum <<
//             // endl;
//             vec<Lit> lits, lits_veri;
//             while (fcnf2 >> litTmp) {

//                 // cout << "LL: " << litTmp << endl;
//                 if (litTmp != 0) {
//                     if (umapNum_ckt2.find(abs(litTmp)) ==
//                         umapNum_ckt2.end()) { // doesn't exist
//                         umapNum_ckt2[abs(litTmp)] =
//                             "n" + to_string(abs(litTmp));
//                         Var nV                         = s_miter.newVar();
//                         Var nV_veri                    = s_verifier.newVar();
//                         umapInterVar_ckt2[abs(litTmp)] = nV;
//                         umapInterVar_ckt2_veri[abs(litTmp)] = nV_veri;
//                         Lit li, li_veri;
//                         if (litTmp > 0) {
//                             li      = Lit(nV);
//                             li_veri = Lit(nV_veri);
//                         } else {
//                             li      = ~Lit(nV);
//                             li_veri = ~Lit(nV_veri);
//                         }

//                         lits.push(li);
//                         lits_veri.push(li_veri);
//                     } else {
//                         string portNumTmp =
//                             umapNum_ckt2[abs(litTmp)]; // i0, i1 ..
//                         // cout << "PPP: " << portNumTmp << endl;
//                         // add clause to s_miter according to i/o and
//                         // num. ex: i0 --->  x[0]->getVar2()
//                         if (portNumTmp[0] == 'i') {
//                             Lit li, li_veri;
//                             if (litTmp > 0) {
//                                 li = Lit(
// y[stoi(portNumTmp.substr(1))]->getVar2());
//                                 li_veri = Lit(
// y[stoi(portNumTmp.substr(1))]->getVar3());

//                             } else {
//                                 li = ~Lit(
// y[stoi(portNumTmp.substr(1))]->getVar2());
//                                 li_veri = ~Lit(
// y[stoi(portNumTmp.substr(1))]->getVar3());
//                             }

//                             lits.push(li);
//                             lits_veri.push(li_veri);
//                         } else if (portNumTmp[0] == 'o') { // o
//                             Lit li, li_veri;
//                             if (litTmp > 0) {
//                                 li = Lit(
// g[stoi(portNumTmp.substr(1))]->getVar2());
//                                 li_veri = Lit(
// g[stoi(portNumTmp.substr(1))]->getVar3());

//                             } else {
//                                 li = ~Lit(
// g[stoi(portNumTmp.substr(1))]->getVar2());
//                                 li_veri = ~Lit(
// g[stoi(portNumTmp.substr(1))]->getVar3());
//                             }

//                             lits.push(li);
//                             lits_veri.push(li_veri);
//                         } else { // n (intervariable)
//                             Lit li, li_veri;
//                             if (litTmp > 0) {
//                                 li = Lit(umapInterVar_ckt2[abs(litTmp)]);
//                                 li_veri =
//                                     Lit(umapInterVar_ckt2_veri[abs(litTmp)]);

//                             } else {
//                                 li = ~Lit(umapInterVar_ckt2[abs(litTmp)]);
//                                 li_veri =
// ~Lit(umapInterVar_ckt2_veri[abs(litTmp)]);
//                             }

//                             lits.push(li);
//                             lits_veri.push(li_veri);
//                         }
//                     }
//                 } else {
//                     ++ct2;
//                     s_miter.addClause(lits);
//                     s_verifier.addClause(lits_veri);
//                     lits.clear();
//                     lits_veri.clear();
//                 }
//             }

//             break;
//         }
//     }
//     cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << endl;
// }*/
void
SatMgr::AddLearnedClause(SatSolver& s, SatSolver& s_cir1, SatSolver& s_cir2, SatSolver& s_miter) {
    // cout << "AddLearnedClause start" << endl;
    vec<Lit>                  lits, lits_e; // ex: lits_e:  (not e + not c11)...
    // vec<Lit>                  temp_lits;
    vector<vector<variable*> >&MI = cirmgr.MI, &MO = cirmgr.MO;
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    for (int i = 0; i < f.size(); i++) {
        f[i]->partial.clear();
        vec<Lit> cir1_lits;
        Var output = f[i]->getVar4();
        cir1_lits.push(s_miter.getValue(f[i]->getVar2()) ? ~Lit(output) : Lit(output)); // negate output
        for (vector<variable*>::iterator it = f[i]->_funcSupp.begin(); it != f[i]->_funcSupp.end(); it++) {
            f[i]->partial.insert((*it));
        }

        for (int k = 0; k < f[i]->_funcSupp.size(); k++) {
            vec<Lit> temp_lits;
            cir1_lits.copyTo(temp_lits);
            for (int j = 0; j < f[i]->_funcSupp.size(); j++) {
                if (j == k) continue;
                else {
                    if (!f[i]->partial.count(f[i]->_funcSupp[j])) continue;
                    else {
                        temp_lits.push(s_miter.getValue(f[i]->_funcSupp[j]->getVar2()) ? Lit(f[i]->_funcSupp[j]->getVar4()) : ~Lit(f[i]->_funcSupp[j]->getVar4()));
                    }
                }
            }
            bool result = s_cir1.assumpSolve(temp_lits);
            if (!result) {
                f[i]->partial.erase(f[i]->_funcSupp[k]);
            }
            temp_lits.clear();
        }
        cir1_lits.clear();
    }

    for (int i = 0; i < g.size(); i++) {
        g[i]->partial.clear();
        vec<Lit> cir2_lits;
        Var output = g[i]->getVar4();
        cir2_lits.push(s_miter.getValue(g[i]->getVar2()) ? ~Lit(output) : Lit(output)); // negate output
        for (vector<variable*>::iterator it = g[i]->_funcSupp.begin(); it != g[i]->_funcSupp.end(); it++) {
            g[i]->partial.insert((*it));
        }

        for (int k = 0; k < g[i]->_funcSupp.size(); k++) {
            vec<Lit> temp_lits;
            cir2_lits.copyTo(temp_lits);
            for (int j = 0; j < g[i]->_funcSupp.size(); j++) {
                if (j == k) continue;
                else {
                    if (!g[i]->partial.count(g[i]->_funcSupp[j])) continue;
                    else {
                        temp_lits.push(s_miter.getValue(g[i]->_funcSupp[j]->getVar2()) ? Lit(g[i]->_funcSupp[j]->getVar4()) : ~Lit(g[i]->_funcSupp[j]->getVar4()));
                    }
                }
            }
            bool result = s_cir2.assumpSolve(temp_lits);
            if (!result) {
                g[i]->partial.erase(g[i]->_funcSupp[k]);
            }
            temp_lits.clear();
        }
        cir2_lits.clear();
    }

    for (int i = 0; i < f.size(); i++) {
        for (int j = 0; j < g.size(); j++) {
            if (s_miter.getValue(f[i]->getVar2()) == s_miter.getValue(g[j]->getVar2())) {
                Var dV = MO[2 * i + 1][j]->getVar();
                lits.push(~Lit(dV)); // ex: f2 == g5 -> not d2,5 f2@f[1];
                                          // g5@g[4]; d2,5@MI[9][1]
            }
            else {
                Var cV = MO[2 * i][j]->getVar();
                lits.push(~Lit(cV)); // ex: f2 != g5 -> not c2,5 f2@x[1];
                                          // g5@y[4]; c2,5@MI[8][1]
            }
            for (set<variable*>::iterator it = f[i]->partial.begin(); it != f[i]->partial.end(); it++) {
                for (set<variable*>::iterator it2 = g[j]->partial.begin(); it2 != g[j]->partial.end(); it2++) {
                    
                    int index1 = cirmgr.u_name_index_ckt1[(*it)->getname()];
                    int index2 = cirmgr.u_name_index_ckt2[(*it2)->getname()];
                    // cout << "i: " << (*it)->getname() << " " << index1 << "  j: " << (*it2)->getname() << " " << index2 << endl;
                    if (s_miter.getValue(x[index1]->getVar2()) == s_miter.getValue(y[index2]->getVar2())) {
                        lits.push(Lit(MI[2 * index1 + 1][index2]->getVar())); // ex: x2 == y5 -> b2,5 x2@x[1];
                                                        // y5@y[4]; b2,5@MI[9][1]
                    }
                    else {
                        lits.push(Lit(MI[2 * index1][index2]->getVar())); // ex: x2 != y5 -> a2,5 x2@x[1];
                                                    // y5@y[4]; b2,5@MI[8][1]
                    }
                }
            }

            for (set<variable*>::iterator it2 = g[j]->partial.begin(); it2 != g[j]->partial.end(); it2++) {
                int index2 = cirmgr.u_name_index_ckt2[(*it2)->getname()];
                if (s_miter.getValue(y[index2]->getVar2()) == 1) {
                    lits.push(Lit(MI[MI.size() - 2][index2]->getVar())); // ex: x2 == y5 -> b2,5 x2@x[1];
                                                    // y5@y[4]; b2,5@MI[9][1]
                } else {
                    lits.push(Lit(MI[MI.size() - 1][index2]->getVar())); // ex: x2 != y5 -> a2,5 x2@x[1];
                                                    // y5@y[4]; b2,5@MI[8][1]
                }
            }
            s.addClause(lits);
            lits.clear();
        }
    }
    // cout << "AddLearnedClause end" << endl;


/*
    for (int i = 0; i < f.size(); i++) {
        for (int j = 0; j < g.size(); j++) {
            if (s_miter.getValue(f[i]->getVar2()) == s_miter.getValue(g[j]->getVar2())) {
                Var dV = MO[2 * i + 1][j]->getVar();
                lits.push(~Lit(dV)); // ex: f2 == g5 -> not d2,5 f2@f[1];
                                          // g5@g[4]; d2,5@MI[9][1]
            }
            else {
                Var cV = MO[2 * i][j]->getVar();
                lits.push(~Lit(cV)); // ex: f2 != g5 -> not c2,5 f2@x[1];
                                          // g5@y[4]; c2,5@MI[8][1]
            }
            for (vector<variable*>::iterator it = f[i]->_funcSupp.begin(); it != f[i]->_funcSupp.end(); it++) {
                for (vector<variable*>::iterator it2 = g[j]->_funcSupp.begin(); it2 != g[j]->_funcSupp.end(); it2++) {
                    int index1 = cirmgr.u_name_index_ckt1[(*it)->getname()];
                    int index2 = cirmgr.u_name_index_ckt2[(*it2)->getname()];
                    if (s_miter.getValue(x[index1]->getVar2()) == s_miter.getValue(y[index2]->getVar2())) {
                        lits.push(Lit(MI[2 * index1 + 1][index2]->getVar())); // ex: x2 == y5 -> b2,5 x2@x[1];
                                                        // y5@y[4]; b2,5@MI[9][1]
                    }
                    else {
                        lits.push(Lit(MI[2 * index1][index2]->getVar())); // ex: x2 != y5 -> a2,5 x2@x[1];
                                                    // y5@y[4]; b2,5@MI[8][1]
                    }
                }
            }

            for (vector<variable*>::iterator it2 = g[j]->_funcSupp.begin(); it2 != g[j]->_funcSupp.end(); it2++) {
                int index2 = cirmgr.u_name_index_ckt2[(*it2)->getname()];
                if (s_miter.getValue(y[index2]->getVar2()) == 1) {
                    lits.push(Lit(MI[MI.size() - 2][index2]->getVar())); // ex: x2 == y5 -> b2,5 x2@x[1];
                                                    // y5@y[4]; b2,5@MI[9][1]
                } else {
                    lits.push(Lit(MI[MI.size() - 1][index2]->getVar())); // ex: x2 != y5 -> a2,5 x2@x[1];
                                                    // y5@y[4]; b2,5@MI[8][1]
                }
            }
            s.addClause(lits);
            lits.clear();
        }
    }


    /*
    vec<Lit>                  lits, lits_e; // ex: lits_e:  (not e + not c11)...
    vec<Lit>                  temp_lits;
    vector<vector<variable*>>&MI = cirmgr.MI, &MO = cirmgr.MO;
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    // Var eV = s.newVar();
    // Lit e  =  Lit(eV); // ex: e = c11' · c12' · d11' · d12'
    // Lit Ne = ~Lit(eV); // ex: e = c11' · c12' · d11' · d12'
    // cout << x.size() << " " << y.size() << " " << MI.size() << " " <<
    // MI[0].size() << endl;
    for (int i = 0; i < x.size(); i++) {
        for (int j = 0; j < y.size(); j++) {
            // cout << "xy" << endl;
            if (s_miter.getValue(x[i]->getVar2()) ==
                s_miter.getValue(y[j]->getVar2())) {
                // cout << "x" << i << ": " <<
                // s_miter.getValue(x[i]->getVar2())
                // << "  y" << j << ": " <<
                // s_miter.getValue(y[j]->getVar2()) << endl;
                lits.push(Lit(MI[2 * i + 1][j]
                                  ->getVar())); // ex: x2 == y5 -> b2,5 x2@x[1];
                                                // y5@y[4]; b2,5@MI[9][1]
            } else {
                lits.push(Lit(
                    MI[2 * i][j]->getVar())); // ex: x2 != y5 -> a2,5 x2@x[1];
                                              // y5@y[4]; b2,5@MI[8][1]
            }
        }
    }

    for (int j = 0; j < y.size(); j++) {
        // cout << "xy" << endl;
        if (s_miter.getValue(y[j]->getVar2()) == 1) {
            lits.push(Lit(MI[MI.size() - 2][j]
                              ->getVar())); // ex: x2 == y5 -> b2,5 x2@x[1];
                                            // y5@y[4]; b2,5@MI[9][1]
        } else {
            lits.push(Lit(MI[MI.size() - 1][j]
                              ->getVar())); // ex: x2 != y5 -> a2,5 x2@x[1];
                                            // y5@y[4]; b2,5@MI[8][1]
        }
    }

    // lits.push(e); s.addClause(lits); lits.clear();

    for (int i = 0; i < f.size(); i++) {
        for (int j = 0; j < g.size(); j++) {
            lits.copyTo(temp_lits);
            // cout << "f" << i << ": " <<
            // s_miter.getValue(f[i]->getVar2()) << "  g" << j << ": "
            // << s_miter.getValue(g[j]->getVar2()) << endl;
            if (s_miter.getValue(f[i]->getVar2()) ==
                s_miter.getValue(g[j]->getVar2())) {
                Var dV = MO[2 * i + 1][j]->getVar();
                temp_lits.push(~Lit(dV)); // ex: f2 == g5 -> not d2,5 f2@f[1];
                                          // g5@g[4]; d2,5@MI[9][1]
                // lits_e.push(Ne); lits_e.push(~Lit(dV));
                // s.addClause(lits_e); lits_e.clear(); for(int i = 0; i
                // < temp_lits.size(); i++){
                //    cout << temp_lits[i] << " ";
                // }
                // cout << endl;
                s.addClause(temp_lits);
                temp_lits.clear();

            } else {
                Var cV = MO[2 * i][j]->getVar();
                temp_lits.push(~Lit(cV)); // ex: f2 != g5 -> not c2,5 f2@x[1];
                                          // g5@y[4]; c2,5@MI[8][1]
                // lits_e.push(Ne); lits_e.push(~Lit(cV));
                // s.addClause(lits_e); lits_e.clear(); for(int i = 0; i
                // < temp_lits.size(); i++){
                //    cout << temp_lits[i] << " ";
                // }
                // cout << endl;
                s.addClause(temp_lits);
                temp_lits.clear();
            }
        }
    }
    // lits.push(e); s.addClause(lits); lits.clear();
    */
}
void
SatMgr::AddLearnedClause_const(SatSolver& s,
                               SatSolver& s_miter) { // for const 0, 1
    vector<vector<variable*>>&MI = cirmgr.MI, &MO = cirmgr.MO;
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    vec<Lit>          lits, lits_e; // ex: lits_e:  (not e + not c11)...
    vec<Lit>          temp_lits;
    // Var eV = s.newVar();
    // Lit e  =  Lit(eV); // ex: e = c11' · c12' · d11' · d12'
    // Lit Ne = ~Lit(eV); // ex: e = c11' · c12' · d11' · d12'
    // cout << x.size() << " " << y.size() << " " << MI.size() << " " <<
    // MI[0].size() << endl;
    for (int j = 0; j < y.size(); j++) {
        // cout << "xy" << endl;
        if (s_miter.getValue(y[j]->getVar2()) == 1) {
            lits.push(Lit(MI[2 * cirmgr.inputNum_ckt1][j]
                              ->getVar())); // ex: x2 == y5 -> b2,5 x2@x[1];
                                            // y5@y[4]; b2,5@MI[9][1]
        } else {
            lits.push(Lit(MI[2 * cirmgr.inputNum_ckt1 + 1][j]
                              ->getVar())); // ex: x2 != y5 -> a2,5 x2@x[1];
                                            // y5@y[4]; b2,5@MI[8][1]
        }
    }
    // lits.push(e); s.addClause(lits); lits.clear();

    for (int i = 0; i < f.size(); i++) {
        for (int j = 0; j < g.size(); j++) {
            lits.copyTo(temp_lits);
            if (s_miter.getValue(f[i]->getVar2()) ==
                s_miter.getValue(g[j]->getVar2())) {
                Var dV = MO[2 * i + 1][j]->getVar();
                temp_lits.push(~Lit(dV)); // ex: f2 == g5 -> not d2,5 f2@f[1];
                                          // g5@g[4]; d2,5@MI[9][1]
                // lits_e.push(Ne); lits_e.push(~Lit(dV));
                // s.addClause(lits_e); lits_e.clear();
                s.addClause(temp_lits);
                temp_lits.clear();
            } else {
                Var cV = MO[2 * i][j]->getVar();
                temp_lits.push(~Lit(cV)); // ex: f2 != g5 -> not c2,5 f2@x[1];
                                          // g5@y[4]; c2,5@MI[8][1]
                // lits_e.push(Ne); lits_e.push(~Lit(cV));
                // s.addClause(lits_e); lits_e.clear();
                s.addClause(temp_lits);
                temp_lits.clear();
            }
        }
    }
    // lits.push(e); s.addClause(lits); lits.clear();
}

// only bind input
// use int _port1, int _port2 to check if _port1 < 0 and if _port2 < 0
// _p1, _p2 is for command to bind two port with their mnemonics
bool
SatMgr::addBindClause(bool isnegate, int _port1, int _port2, string _p1,
                      string _p2) {
    vec<Lit> lits;
    if (!_p1.empty()) _port1 = string2Var(_p1, 1, 1);
    if (!_p2.empty()) _port2 = string2Var(_p2, 0, 1);
    if (_port1 == -100 || _port2 == -100) {
        cout << "_port1 and _port2 = " << _port1 << ' ' << _port2 << endl;
        return 0;
    }
    if (_port1 < 0) {
        verifierSolver.assumeProperty(
            _port2,
            !(_port1 == -1)); // port1 == -1 -> _port2 bind to constant 0
        inputMatch.push_back(tuple<bool, Var, Var>(0, _port1, _port2));
    } else if (isnegate) {
        // ~_port1 -> _port2
        lits.push(Lit(_port1));
        lits.push(Lit(_port2));
        verifierSolver.addClause(lits);
        lits.clear();
        // _port2 -> ~_port1
        lits.push(~Lit(_port1));
        lits.push(~Lit(_port2));

        verifierSolver.addClause(lits);
        lits.clear();
        inputMatch.push_back(tuple<bool, Var, Var>(1, _port1, _port2));
    } else {
        // _port1 -> _port2
        lits.push(~Lit(_port1));
        lits.push(Lit(_port2));
        verifierSolver.addClause(lits);
        lits.clear();
        // _port2 -> _port1
        lits.push(Lit(_port1));
        lits.push(~Lit(_port2));
        verifierSolver.addClause(lits);
        lits.clear();
        inputMatch.push_back(tuple<bool, Var, Var>(0, _port1, _port2));
    }
    return 1;
}


void
SatMgr::addBusConstraint_match(size_t idxI, size_t idxO, vec<Lit>& ans){
    ans.clear();
    vector<variable*>&x = cirmgr.x, &y = cirmgr.y, &f = cirmgr.f, &g = cirmgr.g;
    vector<Bus*>& i1 = cirmgr.bus_ckt1_input;
    vector<Bus*>& o1 = cirmgr.bus_ckt1_output;
    vector<Bus*>& i2 = cirmgr.valid_busMatch_ckt2_input[idxI];
    vector<Bus*>& o2 = cirmgr.valid_busMatch_ckt2_output[idxO];
    // cout << "v: " << cirmgr.valid_busMatch_ckt2_input.size() << endl;
    for (size_t i = 0, ni = i1.size(); i < ni; ++i) { 
        for (vector<Var>::iterator j = i1[i]->indexes.begin(); j != i1[i]->indexes.end(); j++) {
            vector<int>& i2_indexes = i2[i]->indexes;
            bool* exist = new bool[y.size()]{0};
            for(vector<Var>::iterator i2idx = i2_indexes.begin(); i2idx != i2_indexes.end(); i2idx++) exist[(*i2idx)] = true;
            for (size_t k = 0, nk = y.size(); k < nk; ++k) {
                if(!exist[k]){
                    ans.push(~Lit(cirmgr.MI[(*j)*2][k]->getVar()));
                    ans.push(~Lit(cirmgr.MI[(*j)*2+1][k]->getVar()));
                }
            }
            delete[] exist;
        }
        for (vector<Var>::iterator j = i2[i]->indexes.begin(); j != i2[i]->indexes.end(); j++) {
            vector<int>& i1_indexes = i1[i]->indexes;
            bool* exist = new bool[x.size()]{0};
            for(vector<Var>::iterator i1idx = i1_indexes.begin(); i1idx != i1_indexes.end(); i1idx++) exist[(*i1idx)] = true;
            for (size_t k = 0, nk = x.size(); k < nk; ++k) {
                if(!exist[k]){
                    ans.push(~Lit(cirmgr.MI[k*2][(*j)]->getVar()));
                    ans.push(~Lit(cirmgr.MI[k*2+1][(*j)]->getVar()));
                }
            }
            delete[] exist;
        }
    }
    for (size_t i = 0, ni = o1.size(); i < ni; ++i) { 
        for (vector<Var>::iterator j = o1[i]->indexes.begin(); j != o1[i]->indexes.end(); j++) {
            vector<int>& o2_indexes = o2[i]->indexes;
            bool* exist = new bool[g.size()]{0};
            for(vector<Var>::iterator o2idx = o2_indexes.begin(); o2idx != o2_indexes.end(); o2idx++) exist[(*o2idx)] = true;
            for (size_t k = 0, nk = g.size(); k < nk; ++k) {
                if(!exist[k]){
                    ans.push(~Lit(cirmgr.MO[(*j)*2][k]->getVar()));
                    ans.push(~Lit(cirmgr.MO[(*j)*2+1][k]->getVar()));
                }
            }
            delete[] exist;
        }
        for (vector<Var>::iterator j = o2[i]->indexes.begin(); j != o2[i]->indexes.end(); j++) {
            vector<int>& o1_indexes = o1[i]->indexes;
            bool* exist = new bool[f.size()]{0};
            for(vector<Var>::iterator o1idx = o1_indexes.begin(); o1idx != o1_indexes.end(); o1idx++) exist[(*o1idx)] = true;
            for (size_t k = 0, nk = f.size(); k < nk; ++k) {
                if(!exist[k]){
                    ans.push(~Lit(cirmgr.MO[k*2][(*j)]->getVar()));
                    ans.push(~Lit(cirmgr.MO[k*2+1][(*j)]->getVar()));
                }
            }
            delete[] exist;
        }

        // when single wire vs any buses -> assign 0
        // to do
        
    }

    // those don't include in any bus, will never connect to those in any bus
    for(size_t i = 0, ni = x.size(); i<ni; ++i){
        for(size_t j = 0, nj = y.size(); j<nj; ++j){
            if((cirmgr.exist_x[i] && !cirmgr.exist_y[j]) || (!cirmgr.exist_x[i] && cirmgr.exist_y[j])){
                ans.push(~Lit(cirmgr.MI[i*2][j]->getVar()));
                ans.push(~Lit(cirmgr.MI[i*2+1][j]->getVar()));
            }
        }
    }
    for(size_t i = 0, ni = f.size(); i<ni; ++i){
        for(size_t j = 0, nj = g.size(); j<nj; ++j){
            if((cirmgr.exist_f[i] && !cirmgr.exist_g[j]) || (!cirmgr.exist_f[i] && cirmgr.exist_g[j])){
                ans.push(~Lit(cirmgr.MO[i*2][j]->getVar()));
                ans.push(~Lit(cirmgr.MO[i*2+1][j]->getVar()));
            }
        }
    }

    // if(idxI == 0 && idxO == 0){
    //     cout << "X ex" << endl;
    //     for(int i = 0; i < x.size(); i++){
    //         cout << cirmgr.exist_x[i] << endl;
    //     }
    //     cout << endl;
    //     for(int i = 0; i < y.size(); i++){
    //         cout << cirmgr.exist_y[i] << endl;
    //     }
    //     cout << endl;
    //     for(int i = 0; i < f.size(); i++){
    //         cout << cirmgr.exist_f[i] << endl;
    //     }
    //     cout << endl;
    //     for(int i = 0; i < g.size(); i++){
    //         cout << cirmgr.exist_g[i] << endl;
    //     }

    // }
}

void SatMgr::addCandidateBusConstraint(SatSolver& s){
///////////below is testing, still debugging////    
    for(int j = 0; j < cirmgr.MI_valid[0].size(); j++){
        vec<Lit> atLeastOneCandidate; // "in bus" can't be candidate with "not in any bus" 
        vec<Lit> notInAnyBusCandidate; // match those candidate that both "not in any bus"
        if(cirmgr.u_name_busIndex_input_ckt2[cirmgr.y[j]->getname()] != -1){ // in some bus
            for(int i = 0; i < cirmgr.MI_valid.size(); i++){
                if(cirmgr.MI_valid[i][j]){
                    if(i != cirmgr.MI_valid.size() - 1){ // not constant, since constant has no bus infomation
                        if(cirmgr.u_name_busIndex_input_ckt1[cirmgr.x[i]->getname()] != -1){ // in some bus
                            atLeastOneCandidate.push(Lit(cirmgr.MI_valid_Var[i][j]));
                            cout << "MI:\t" << cirmgr.x[i]->getname() << " at bus " << cirmgr.u_name_busIndex_input_ckt1[cirmgr.x[i]->getname()] << "\t" << cirmgr.y[j]->getname() << " at bus " << cirmgr.u_name_busIndex_input_ckt2[cirmgr.y[j]->getname()] << endl;
                            vec<Lit> implyBusMatch;
                            Var busVar = cirmgr.MIbus_Var[cirmgr.u_name_busIndex_input_ckt1[cirmgr.x[i]->getname()]][cirmgr.u_name_busIndex_input_ckt2[cirmgr.y[j]->getname()]];
                            // cout << "1773" << endl;
                            Var candidate = cirmgr.MI_valid_Var[i][j];
                            implyBusMatch.push(~Lit(candidate));
                            implyBusMatch.push(Lit(busVar));
                            s.addClause(implyBusMatch);
                            implyBusMatch.clear();
                        }
                    }
                    else{ // constant
                        atLeastOneCandidate.push(Lit(cirmgr.MI_valid_Var[i][j]));
                    }
                }
            }
        }
        else{ // not in any bus
            for(int i = 0; i < cirmgr.MI_valid.size(); i++){
                if(cirmgr.MI_valid[i][j]){
                    if(i != cirmgr.MI_valid.size() - 1){ // not constant, since constant has no bus infomation
                        if(cirmgr.u_name_busIndex_input_ckt1[cirmgr.x[i]->getname()] == -1){ // not in any bus
                            cout << "MI:\t" << cirmgr.x[i]->getname() << "not in any bus\t" << cirmgr.y[j]->getname() << "not in any bus\t"  << endl;
                            notInAnyBusCandidate.push(Lit(cirmgr.MI_valid_Var[i][j]));
                        }
                    }
                    else{ // constant
                        notInAnyBusCandidate.push(Lit(cirmgr.MI_valid_Var[i][j]));
                    }
                }
            }
        }
        if(atLeastOneCandidate.size() != 0) s.addClause(atLeastOneCandidate);
        if(notInAnyBusCandidate.size() != 0) s.addClause(notInAnyBusCandidate);
        atLeastOneCandidate.clear();
        notInAnyBusCandidate.clear();
    }
    for(int j = 0; j < cirmgr.MO_valid[0].size(); j++){
        for(int i = 0; i < cirmgr.MO_valid.size(); i++){
            if(cirmgr.MO_valid[i][j]){
                if((cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()] == -1 && cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()] != -1) || (cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()] != -1 && cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()] == -1)){ // in some bus
                    // in some bus <> not in any bus  || not in any bus <> in some bus, close them
                    vec<Lit> closeInNOutBus;
                    closeInNOutBus.push(~Lit(cirmgr.MO_valid_Var[i][j]));
                    s.addClause(closeInNOutBus);
                    closeInNOutBus.clear();
                }
                else if(cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()] != -1 && cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()] != -1){
                    vec<Lit> implyBusMatch;
                    Var busVar = cirmgr.MObus_Var[cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()]][cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()]];
                    Var candidate = cirmgr.MO_valid_Var[i][j];
                    implyBusMatch.push(~Lit(candidate));
                    implyBusMatch.push(Lit(busVar));
                    s.addClause(implyBusMatch);
                    implyBusMatch.clear();                    
                }
            }
        }
    }
    /* // force MO at least one 1 in candidates, which conflicts with constraint 2
    for(int j = 0; j < cirmgr.MO_valid[0].size(); j++){
        vec<Lit> atLeastOneCandidate; // "in bus" can't be candidate with "not in any bus" 
        vec<Lit> notInAnyBusCandidate; // match those candidate that both "not in any bus"
        if(cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()] != -1){ // in some bus
            for(int i = 0; i < cirmgr.MO_valid.size(); i++){
                if(cirmgr.MO_valid[i][j]){
                    if(cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()] != -1){ // in some bus
                        atLeastOneCandidate.push(Lit(cirmgr.MO_valid_Var[i][j]));
                        cout << "MO:\t" << cirmgr.f[i]->getname() << " at bus " << cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()] << "\t" << cirmgr.g[j]->getname() << " at bus " << cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()] << endl;
                        vec<Lit> implyBusMatch;
                        Var busVar = cirmgr.MObus_Var[cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()]][cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()]];
                        // cout << "1773" << endl;
                        Var candidate = cirmgr.MO_valid_Var[i][j];
                        implyBusMatch.push(~Lit(candidate));
                        implyBusMatch.push(Lit(busVar));
                        s.addClause(implyBusMatch);
                        implyBusMatch.clear();
                    }
                }
            }
        }
        else{ // not in any bus
            for(int i = 0; i < cirmgr.MO_valid.size(); i++){
                if(cirmgr.MO_valid[i][j]){
                    if(cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()] == -1){ // not in any bus
                        cout << "MO:\t" << cirmgr.f[i]->getname() << "not in any bus\t" << cirmgr.g[j]->getname() << "not in any bus\t"  << endl;
                        notInAnyBusCandidate.push(Lit(cirmgr.MO_valid_Var[i][j]));
                    }
                }
            }
        }
        if(atLeastOneCandidate.size() != 0)s.addClause(atLeastOneCandidate);
        if(notInAnyBusCandidate.size() != 0)s.addClause(notInAnyBusCandidate);
        atLeastOneCandidate.clear();
        notInAnyBusCandidate.clear();

    }
    cout << "test" << endl;
    */
////////////////////////////below is original part, works for case04, doesn't work for case05(exist not in any bus port) ///////
    // for(int j = 0; j < cirmgr.MI_valid[0].size(); j++){
    //     vec<Lit> atLeastOneCandidate;
    //     for(int i = 0; i < cirmgr.MI_valid.size(); i++){
    //         if(cirmgr.MI_valid[i][j]){
    //             atLeastOneCandidate.push(Lit(cirmgr.MI_valid_Var[i][j]));
    //             if(i != cirmgr.MI_valid.size() - 1){ // not constant
    //                 cout << "MI:\t" << cirmgr.x[i]->getname() << " at bus " << cirmgr.u_name_busIndex_input_ckt1[cirmgr.x[i]->getname()] << "\t" << cirmgr.y[j]->getname() << " at bus " << cirmgr.u_name_busIndex_input_ckt2[cirmgr.y[j]->getname()] << endl;
    //                 // cout << cirmgr.x[i]->getname() << " at bus " << cirmgr.u_name_busIndex_input_ckt1[cirmgr.x[i]->getname()] << "\t" << cirmgr.y[j]->getname() << " at bus " << cirmgr.u_name_busIndex_input_ckt2[cirmgr.y[j]->getname()] << endl;
    //                 vec<Lit> implyBusMatch;
    //                 Var busVar = cirmgr.MIbus_Var[cirmgr.u_name_busIndex_input_ckt1[cirmgr.x[i]->getname()]][cirmgr.u_name_busIndex_input_ckt2[cirmgr.y[j]->getname()]];
    //                 Var candidate = cirmgr.MI_valid_Var[i][j];
    //                 implyBusMatch.push(~Lit(candidate));
    //                 implyBusMatch.push(Lit(busVar));
    //                 s.addClause(implyBusMatch);
    //                 implyBusMatch.clear();
    //             } 
    //         }
    //     }
    //     s.addClause(atLeastOneCandidate);
    //     atLeastOneCandidate.clear();
    // }
    // for(int j = 0; j < cirmgr.MO_valid[0].size(); j++){
    //     vec<Lit> atLeastOneCandidate;
    //     for(int i = 0; i < cirmgr.MO_valid.size(); i++){
    //         // cout << cirmgr.f[i]->getname() << " " << cirmgr.g[j]->getname() << endl;
    //         if(cirmgr.MO_valid[i][j]){
    //             cout << "MO:\t" << cirmgr.f[i]->getname() << " at bus " << cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()] << "\t" << cirmgr.g[j]->getname() << " at bus " << cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()] << endl;
    //             atLeastOneCandidate.push(Lit(cirmgr.MO_valid_Var[i][j]));
    //             vec<Lit> implyBusMatch;
    //             Var busVar = cirmgr.MObus_Var[cirmgr.u_name_busIndex_output_ckt1[cirmgr.f[i]->getname()]][cirmgr.u_name_busIndex_output_ckt2[cirmgr.g[j]->getname()]];
    //             Var candidate = cirmgr.MO_valid_Var[i][j];
    //             implyBusMatch.push(~Lit(candidate));
    //             implyBusMatch.push(Lit(busVar));
    //             s.addClause(implyBusMatch);
    //             implyBusMatch.clear();
    //         }
    //     }
    //     s.addClause(atLeastOneCandidate);
    //     atLeastOneCandidate.clear();
    // }




}

void SatMgr::addBusValidConstraint(SatSolver& s){ // close MIbus_valid, MObus_valid according to |cir1BusSize| <= |cir2BusSize|, also close MIbus_Var, MObus_Var according to MIbus_valid, MObus_valid
    cout << "in addBusValidConstraint" << endl;
    for(int i = 0; i < cirmgr.bus_ckt1_input.size(); i++){
        for(int j = 0; j < cirmgr.bus_ckt2_input.size(); j++){
            if(cirmgr.bus_ckt1_input[i]->getPortNum() > cirmgr.bus_ckt2_input[j]->getPortNum()){
                cirmgr.MIbus_valid[i][j] = false;
            }
        }
    }
    for(int i = 0; i < cirmgr.bus_ckt1_output.size(); i++){
        for(int j = 0; j < cirmgr.bus_ckt2_output.size(); j++){
            if(cirmgr.bus_ckt1_output[i]->getPortNum() > cirmgr.bus_ckt2_output[j]->getPortNum()){
                cirmgr.MIbus_valid[i][j] = false;
            }
        }
    }
    cout << "1913" << endl;
    for(int i = 0; i < cirmgr.MIbus_valid.size(); i++){
        for(int j = 0; j < cirmgr.MIbus_valid[0].size(); j++){
            if(!cirmgr.MIbus_valid[i][j]){
                vec<Lit> close;
                close.push(~Lit(cirmgr.MIbus_Var[i][j]));
                s.addClause(close);
                close.clear();
            }
             // bus match implies each port in this cir2bus must at least match to one of the port (or constant) in matched cir1bus 
            // // else{ // Bind control of each MIbus_Var/MObus_Var entry to MI_valid_var/MO_valid_var matching(only true(valid bus match) entry will add port matching clause to solver)
            // //     vec<Lit> busMatch;
            // //     for(int c2 = 0; c2 < cirmgr.bus_ckt2_input[j]->getPortNum(); c2++){
            // //         busMatch.push(~Lit(cirmgr.MIbus_Var[i][j]));
            // //         for(int c1 = 0; c1 < cirmgr.bus_ckt1_input[i]->getPortNum(); c1++){
            // //             busMatch.push(Lit(cirmgr.MI_valid_Var[cirmgr.bus_ckt1_input[i]->indexes[c1]][cirmgr.bus_ckt2_input[j]->indexes[c2]]));
            // //         }
            // //         busMatch.push(Lit(cirmgr.MI_valid_Var[cirmgr.MI_valid_Var.size()-1][cirmgr.bus_ckt2_input[j]->indexes[c2]])); // cir2 can bind to constant
                    
            // //         s.addClause(busMatch);
            // //         busMatch.clear();
            // //     }
            // // }

        }
        
    }
    cout << "1937" << endl;
    for(int i = 0; i < cirmgr.MObus_valid.size(); i++){
        for(int j = 0; j < cirmgr.MObus_valid[0].size(); j++){
            if(!cirmgr.MObus_valid[i][j]){
                vec<Lit> close;
                close.push(~Lit(cirmgr.MObus_Var[i][j]));
                s.addClause(close);
                close.clear();
            }
             // bus match implies each port in this cir2bus must at least match to one of the port in matched cir1bus
            
            /*
            else{ // Bind control of each MIbus_Var/MObus_Var entry to MI_valid_var/MO_valid_var matching(only true(valid bus match) entry will add port matching clause to solver)
                vec<Lit> busMatch;
                for(int c2 = 0; c2 < cirmgr.bus_ckt2_output[j]->getPortNum(); c2++){
                    busMatch.push(~Lit(cirmgr.MObus_Var[i][j]));
                    for(int c1 = 0; c1 < cirmgr.bus_ckt1_output[i]->getPortNum(); c1++){
                        busMatch.push(Lit(cirmgr.MO_valid_Var[cirmgr.bus_ckt1_output[i]->indexes[c1]][cirmgr.bus_ckt2_output[j]->indexes[c2]]));
                    }
                    
                    s.addClause(busMatch);
                    busMatch.clear();
                }
            }
            */
        }
    }
    cout << "out addBusValidConstraint" << endl;
}

//helper function
void
SatMgr::closeMatching(vec<Lit> &lits, size_t _i, size_t _j, bool _isInput){  // _isInput == 1 -> close input matching // positive symmetry only close negative connection(i % 2 == 1)
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
void SatMgr::fAtMostOneMatch(SatSolver& s){
    for(int i = 0; i < cirmgr.MO_valid.size(); i++){
        constraint_Cmdr_nocontrol(s, cirmgr.MO_valid_Var[i], true);
    }
}