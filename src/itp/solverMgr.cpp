#include "solverMgr.h"

void
SolverMgr::verification(bool isManualBinded) {
    // cout << "inputMatch = \n";
    // for (auto i : inputMatch)
    //     cout << get<0>(i) << ' ' << get<1>(i) << ' ' << get<2>(i)
    //          << endl;
    // cout << "\noutputMatch = \n";
    // for (auto i : outputMatch)
    //     cout << get<0>(i) << ' ' << get<1>(i) << ' ' << get<2>(i)
    //          << endl;
    vec<Lit>                      lits;
    vector<Var>&                  xorVar         = satmgr.xorVar;
    SatSolver&                    verifierSolver = satmgr.verifierSolver;
    vector<tuple<bool, Var, Var>>&inputMatch     = satmgr.inputMatch,
                            &outputMatch         = satmgr.outputMatch;
    Var f =
        verifierSolver.newVar(); // f should be 0 for two equilivance circuit
    lits.push(~Lit(f));
    if (!isManualBinded) {
        for (auto i : inputMatch)
            satmgr.addBindClause(get<0>(i), get<1>(i), get<2>(i));

        for (auto i : outputMatch) { // np3v a little bit strange!!!
            if (!satmgr.outputBind(get<1>(i), get<0>(i), get<2>(i)))
                ;
            // cout << "outputBind fail : solverMgr.cpp:26\n";
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
}

void
SolverMgr::solveNP3() {
    cout << "tem" << endl;
    satmgr.reset();
    // SatSolver solver, miterSolver;
    satmgr.solver.initialize();
    satmgr.miterSolver.initialize();
    satmgr.verifierSolver.initialize();

    satmgr.cirmgr.readAAG();
    satmgr.cirmgr.readMAP();
    satmgr.initCircuit(satmgr.solver, satmgr.miterSolver,
                       satmgr.verifierSolver);
    satmgr.cirmgr.readCNF(satmgr.miterSolver, satmgr.verifierSolver);
    satmgr.cirmgr.readBus();
    // return;
    /*
    bool result;
    result = solver.solve();
    reportResult(solver, result);
    */

    while (true) {
        // for(int i = 0; i < 10; i++){
        vector<vector<variable*>>&MI = satmgr.cirmgr.MI, &MO = satmgr.cirmgr.MO;
        bool                      SAT1_result, SAT2_result;
        SAT1_result = satmgr.solver.solve();
        if (!SAT1_result) {
            cout << "No match!!" << endl;
            break;
        } else {
            vec<Lit>                  assump;
            vector<vector<variable*>>&MI = satmgr.cirmgr.MI,
            &MO                          = satmgr.cirmgr.MO;
            for (int i = 0; i < MI.size(); i++) {
                for (int j = 0; j < MI[0].size(); j++) {
                    if (satmgr.solver.getValue(MI[i][j]->getVar()) == 1) {
                        Lit v = Lit(MI[i][j]->getVar2());
                        assump.push(v);
                        cout << "MI i: " << i << " j: " << j << " val: "
                             << satmgr.solver.getValue(MI[i][j]->getVar())
                             << endl;
                    } else if (satmgr.solver.getValue(MI[i][j]->getVar()) ==
                               0) {
                        Lit v = ~Lit(MI[i][j]->getVar2());
                        assump.push(v);
                    }
                }
            }
            // cout << "below is MO " << endl;
            for (int i = 0; i < MO.size(); i++) {
                for (int j = 0; j < MO[0].size(); j++) {
                    if (i % 2 == 0) {
                        if ((satmgr.solver.getValue(MO[i][j]->getVar()) +
                             satmgr.solver.getValue(MO[i + 1][j]->getVar())) ==
                            0) {
                            assump.push(Lit(satmgr.big_or[j][i / 2]));
                        }
                    }
                    if (satmgr.solver.getValue(MO[i][j]->getVar()) == 1) {
                        Lit v = Lit(MO[i][j]->getVar2());
                        assump.push(v);
                        cout << "MO i: " << i << " j: " << j << " val: "
                             << satmgr.solver.getValue(MO[i][j]->getVar())
                             << endl;
                    } else if (satmgr.solver.getValue(MO[i][j]->getVar()) ==
                               0) {
                        Lit v = ~Lit(MO[i][j]->getVar2());
                        assump.push(v);
                    }
                }
            }
            cout << endl;

            string temp;
            // cin >> temp;

            SAT2_result = satmgr.miterSolver.assumpSolve(assump);
            if (!SAT2_result) {
                cout << "Match found!!" << endl;
                satmgr.reportResult(satmgr.solver, SAT1_result);

                break;
            } else {
                // cout << "ELSE" << endl;
                satmgr.AddLearnedClause(satmgr.solver, satmgr.miterSolver);
                // AddLearnedClause_const(solver, miterSolver);
            }
            assump.clear();
        }
    }
}