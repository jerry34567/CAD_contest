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
    int cnt = 0;
    if (!isManualBinded) {
        for (auto i : inputMatch) {
            ++cnt;
            satmgr.addBindClause(get<0>(i), get<1>(i), get<2>(i));
        }
        cout << "input bind " << cnt << "times\n";
        cnt = 0;
        for (auto i : outputMatch) { // np3v a little bit strange!!!
            cnt++;
            if (!satmgr.outputBind(get<1>(i), get<0>(i), get<2>(i)))

                cout << "outputBind fail : solverMgr.cpp:26\n";
            // xorVar.push_back(verifierSolver.newVar());
            // verifierSolver.addXorCNF(xorVar.back(), get<1>(i),
            //                          get<0>(i), get<2>(i), 0);
        }
        cout << "output bind " << cnt << "times\n";
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
SolverMgr::solveNP3(string& inputFilename) {
    double time;
    clock_t start, stop;
    start = clock();

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
    satmgr.cirmgr.readBus_class(inputFilename);
    // satmgr.cirmgr.readBus(inputFilename);
    satmgr.cirmgr.readPreporcess(outputUnateness);
    satmgr.cirmgr.readPreporcess(support);
    satmgr.cirmgr.readInputUnateness();
    satmgr.cirmgr.outputGrouping();
    // satmgr.addBusConstraint();
    satmgr.addSuppConstraint();  
    satmgr.addUnateConstraint(1); // add input unate constraint         // !(input # of p of ckt1 > input # of p of ckt2 && input # of n of ckt1 > input # of n of ckt2) -> 關掉正的match || !((input # of n of ckt1 > input # of p of ckt2) && (input # of p of ckt1 > input # of n of ckt2))-> 關掉負的match
    satmgr.addUnateConstraint(0); // add output unate constraint
    satmgr.addOutputGroupingConstraint();
    
    size_t BusMatchIdx_I = 0, BusMatchIdx_O = 0;
    int totalBusMatch_I = satmgr.cirmgr.valid_busMatch_ckt2_input.size();
    int totalBusMatch_O = satmgr.cirmgr.valid_busMatch_ckt2_output.size();
    vec<Lit> BusMatchAssump;
    satmgr.addBusConstraint_match(0, 0, BusMatchAssump);
    vector<vector<variable*>>&MI = satmgr.cirmgr.MI, &MO = satmgr.cirmgr.MO;

    // for level sovle (output sould at least some number)
    int level = 0;
    bool next_level = true;
    vector<Var> vec_var;
    for (int i = 0, n = MO.size(); i < n; i++){
        for (int j = 0, u = MO[0].size(); j < u; j++){
            vec_var.push_back(MO[i][j]->getVar());
        }
    }

    while (satmgr.point != (satmgr.cirmgr.outputNum_ckt1 + satmgr.cirmgr.outputNum_ckt2) && time < 3570) {
        if (level == 0 && next_level){
            cout << "a" << endl;
            satmgr.solver.addAtLeast(vec_var, satmgr.cirmgr.outputNum_ckt2 / 2, 0, 0);
            satmgr.solver.MapClear();
            level++;
            next_level = false;
        }
        else if (level == 1 && next_level){
            cout << "b" << endl;
            satmgr.solver.addAtLeast(vec_var, (satmgr.cirmgr.outputNum_ckt2 * 3) / 4, 0, 0);
            satmgr.solver.MapClear();
            level++;
            next_level = false;
        }
        else if (level == 2 && next_level){
            cout << "c" << endl;
            satmgr.solver.addAtLeast(vec_var, satmgr.cirmgr.outputNum_ckt2, 0, 0);
            // next_level = false;
        }

        bool                      SAT1_result, SAT2_result;
        // SAT1_result = satmgr.solver.solve();
        SAT1_result = satmgr.solver.assumpSolve(BusMatchAssump);
        // cout << "BS: " << BusMatchAssump.size() << endl;
        if (!SAT1_result) {
            cout << "!sat1 result: BI: " << BusMatchIdx_I << " BO: " << BusMatchIdx_O << endl;
            if(BusMatchIdx_I == totalBusMatch_I-1 && BusMatchIdx_O == totalBusMatch_O-1){
                cout << "No match!!" << endl;
                break;
            }
            else if(BusMatchIdx_O == totalBusMatch_O-1){
                BusMatchIdx_I++;
                BusMatchIdx_O = 0;
            }
            else{
                BusMatchIdx_O++;
            }
            satmgr.addBusConstraint_match(BusMatchIdx_I, BusMatchIdx_O, BusMatchAssump);
        } else {
            vec<Lit>                  assump;
            vec<Lit>                  be_searched;
            vector<vector<variable*>>&MI = satmgr.cirmgr.MI,
            &MO                          = satmgr.cirmgr.MO;
            for (int i = 0; i < MI.size(); i++) {
                for (int j = 0; j < MI[0].size(); j++) {
                    if (satmgr.solver.getValue(MI[i][j]->getVar()) == 1) {
                        Lit v1 = ~Lit(MI[i][j]->getVar());
                        be_searched.push(v1);

                        Lit v = Lit(MI[i][j]->getVar2());
                        assump.push(v);
                        cout << "MI i: " << i << " j: " << j << " val: "
                             << satmgr.solver.getValue(MI[i][j]->getVar())
                             << endl;
                    }
                    else if (satmgr.solver.getValue(MI[i][j]->getVar()) ==0) {
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
                             satmgr.solver.getValue(MO[i + 1][j]->getVar())) == 0) {
                            assump.push(Lit(satmgr.big_or[j][i / 2]));
                        }
                    }
                    if (satmgr.solver.getValue(MO[i][j]->getVar()) == 1) {
                        Lit v1 = ~Lit(MO[i][j]->getVar());
                        be_searched.push(v1);

                        Lit v = Lit(MO[i][j]->getVar2());
                        assump.push(v);
                        cout << "MO i: " << i << " j: " << j << " val: "
                             << satmgr.solver.getValue(MO[i][j]->getVar())
                             << endl;
                    } else if (satmgr.solver.getValue(MO[i][j]->getVar()) == 0) {
                        Lit v1 = Lit(MO[i][j]->getVar());
                        be_searched.push(v1);

                        Lit v = ~Lit(MO[i][j]->getVar2());
                        assump.push(v);
                    }
                }
            }

            satmgr.solver.addClause(be_searched);
            be_searched.clear();
            cout << endl;

            SAT2_result = satmgr.miterSolver.assumpSolve(assump);
            stop = clock();
            time = double(stop - start) / CLOCKS_PER_SEC;
            if (!SAT2_result) {
                cout << "Match found!!" << endl;
                next_level = true;

                int temp_point = 0;
                for (int i = 0, n = MO.size(); i < n; i = i + 2){
                    bool temp_bool = false;
                    for (int j = 0, u = MO[i].size(); j < u; j++){
                        if ((satmgr.solver.getValue(MO[i][j]->getVar()) +
                             satmgr.solver.getValue(MO[i + 1][j]->getVar())) == 1) {
                            temp_bool = true;
                            temp_point++;
                        }
                    }
                    if (temp_bool){
                        temp_point++;
                    }
                }

                if (temp_point > satmgr.point){
                    satmgr.point = temp_point;
                    satmgr.record_input = MI;
                    satmgr.record_output = MO;
                }
            } else {
                // cout << "ELSE" << endl;
                satmgr.AddLearnedClause(satmgr.solver, satmgr.miterSolver);
                // AddLearnedClause_const(solver, miterSolver);
            }
            assump.clear();
        }
    }
    satmgr.reportResult(satmgr.solver);
}