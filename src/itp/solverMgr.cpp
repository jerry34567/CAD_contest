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
        for (vector<std::tuple<bool, Var, Var>>::iterator i = inputMatch.begin(); i != inputMatch.end(); i++) {
            ++cnt;
            satmgr.addBindClause(get<0>(*i), get<1>(*i), get<2>(*i));
        }
        cout << "input bind " << cnt << "times\n";
        cnt = 0;
        for (vector<std::tuple<bool, Var, Var>>::iterator i = outputMatch.begin(); i != outputMatch.end(); i++) { // np3v a little bit strange!!!
            cnt++;
            if (!satmgr.outputBind(get<1>(*i), get<0>(*i), get<2>(*i)))

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
        for (vector<std::tuple<bool, Var, Var>>::iterator i = inputMatch.begin(); i != inputMatch.end(); i++)
            cout << (get<0>(*i) ? '~' : ' ')
                 << verifierSolver.getValue(get<1>(*i)) << ' '
                 << verifierSolver.getValue(get<2>(*i)) << endl;
        cout << endl;
        for (vector<std::tuple<bool, Var, Var>>::iterator i = outputMatch.begin(); i != outputMatch.end(); i++)
            cout << (get<0>(*i) ? '~' : ' ')
                 << verifierSolver.getValue(get<1>(*i)) << ' '
                 << verifierSolver.getValue(get<2>(*i)) << endl;
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
    satmgr.cirmgr.readBus_class(satmgr.solver, inputFilename);
    satmgr.busMatchExactlyOne(satmgr.solver);
    // return; //test indexes
    satmgr.cirmgr.readPreporcess(outputUnateness);
    satmgr.cirmgr.readPreporcess(support);
    satmgr.cirmgr.recordPIsupportPO();
    satmgr.cirmgr.readInputUnateness();
    satmgr.cirmgr.readSymmetric();
    satmgr.cirmgr.outputGrouping();
    satmgr.cirmgr.busSupportUnion();
    satmgr.cirmgr.busInputSupportUnion();
    satmgr.cirmgr.busOutputUnateness();
    satmgr.cirmgr.busInputUnateness();
    satmgr.cirmgr.supportBusClassification();
    
    satmgr.addSuppConstraint();  
    satmgr.addSuppConstraint_input();  
    satmgr.addUnateConstraint(1); // add input unate constraint         // !(input # of p of ckt1 > input # of p of ckt2 && input # of n of ckt1 > input # of n of ckt2) -> 關掉正的match || !((input # of n of ckt1 > input # of p of ckt2) && (input # of p of ckt1 > input # of n of ckt2))-> 關掉負的match
    satmgr.addUnateConstraint(0); // add output unate constraint // test closeTTTTTTTTTTTTTTTTTTTTT
    satmgr.addOutputGroupingConstraint();
    satmgr.cirmgr.printMIMO_valid();
    // for(int i = 0; i < 1; i++) Var t = satmgr.solver.newVar(); // test var
    satmgr.addOutputConstraint_inputBusNum();  
    satmgr.addBusConstraint_inputUnateness();   
    satmgr.addBusConstraint_outputUnateness();   
    satmgr.addBusConstraint_inputSupportSize();
    satmgr.addBusConstraint_outputSupportSize();
    satmgr.addOutput0Constraint();
    // size_t bp = 0;
    
    size_t BusMatchIdx_I = 0, BusMatchIdx_O = 0;
    int totalBusMatch_I = satmgr.cirmgr.valid_busMatch_ckt2_input.size();
    int totalBusMatch_O = satmgr.cirmgr.valid_busMatch_ckt2_output.size();
    vec<Lit> BusMatchAssump;
    // vec<Lit> find_valid_output_assump, find_input_given_output_assump;
    bool finding_output = true; // if true: solver.assumpSolve(find_valid_output_assump), else: solver.assumpSolve(find_input_given_output_assump)
    // satmgr.addBusConstraint_match(0, 0, BusMatchAssump);
    // BusMatchAssump.copyTo(find_valid_output_assump);
    // BusMatchAssump.copyTo(find_input_given_output_assump);
    
    vector<vector<variable*>>&MI = satmgr.cirmgr.MI, &MO = satmgr.cirmgr.MO;

    // cout << "AAAAAAA110" << endl;
    // for(auto i: satmgr.cirmgr.x){
    //     cout << i->getname() << ": ";
    //     for(auto j: i->_funcSupp_PI){
    //         cout << j->getname() << " ";
    //     }
    //     cout << endl;
    // }
    // cout << "AAAAAAA117" << endl;
    // for(auto i: satmgr.cirmgr.y){
    //     cout << i->getname() << ": ";
    //     for(auto j: i->_funcSupp_PI){
    //         cout << j->getname() << " ";
    //     }
    //     cout << endl;
    // }
    // cout << "AAAAAAA125" << endl;
    satmgr.addSymmConstraint(satmgr.solver);
    // satmgr.addSameSuppSizeConstraint(satmgr.solver);
    cout << satmgr.cirmgr.inputNum_ckt1 << satmgr.cirmgr.inputNum_ckt2 << endl;
    cout << satmgr.cirmgr.outputNum_ckt1 << " " << satmgr.cirmgr.outputNum_ckt2 << endl;
// return;
    // for (int i = 0, n = satmgr.cirmgr.f[0]->symmetric_set.size(); i < n; i++) {
    //     for (auto j : satmgr.cirmgr.f[0]->symmetric_set[i]) {
    //         cout << j << " ";
    //     }
    //     cout << endl;
    // }

    // for (int i = 0, n = satmgr.cirmgr.f[0]->symmetric_set.size(); i < n; i++) {
    //     for (auto j : satmgr.cirmgr.f[0]->symmetric_set[i]) {
    //         cout << j << " ";
    //     }
    //     cout << endl;
    // }

    satmgr.addCandidateBusConstraint(satmgr.solver);
    satmgr.addBusValidConstraint(satmgr.solver);



    // for (int i = 0, n = satmgr.cirmgr.g[6]->symmetric_set.size(); i < n; i++) {
    //     for (auto j : satmgr.cirmgr.g[6]->symmetric_set[i]) {
    //         cout << j << " ";
    //     }
    //     cout << endl;
    // }

    // for level sovle (output sould at least some number)
    int level = -1;
    // int level = 2; // start from full match
    bool next_level = true;
    vector<Var> vec_var;
    // vector<pair<int, int>> MO_no_pos_neg_pair; // record match variable pair: [i][j] -> i, j. f[i] g[j] can obtain variable info. For funcSuppInputConstraint convenience
    for (int i = 0, n = MO.size(); i < n; i++){
        for (int j = 0, u = MO[0].size(); j < u; j++){
            vec_var.push_back(MO[i][j]->getVar());
        }
    }
    
    vec<Lit> close_constant_assump; bool enable_constant = false; // close constant matching initially, until no match then enable constant
    for(int j = 0; j < satmgr.cirmgr.MI_valid_Var[0].size(); j++){
        close_constant_assump.push(~Lit(satmgr.cirmgr.MI_valid_Var[satmgr.cirmgr.MI_valid_Var.size()-1][j]));
    }

    while (satmgr.point != (satmgr.cirmgr.outputNum_ckt1 + satmgr.cirmgr.outputNum_ckt2) && time < 3570) {
        if (level == 0 && next_level){
            cout << "a0" << endl;
            satmgr.solver.addAtLeast(vec_var, satmgr.cirmgr.outputNum_ckt2 / 4, 0, 0);
            satmgr.solver.MapClear();
            // level++;
            next_level = false;
            enable_constant = false;
        }
        else if (level == 1 && next_level){
            cout << "a" << endl;
            satmgr.solver.addAtLeast(vec_var, satmgr.cirmgr.outputNum_ckt2 / 2, 0, 0);
            satmgr.solver.MapClear();
            // level++;
            next_level = false;
            enable_constant = false;
        }
        else if (level == 2 && next_level){
            cout << "b" << endl;
            satmgr.solver.addAtLeast(vec_var, (satmgr.cirmgr.outputNum_ckt2 * 3) / 4, 0, 0);
            satmgr.solver.MapClear();
            // level++;
            next_level = false;
            enable_constant = false;
        }
        else if (level == 3 && next_level){
            cout << "c" << endl;
            satmgr.solver.addAtLeast(vec_var, satmgr.cirmgr.outputNum_ckt2, 0, 0);
            next_level = false;
            enable_constant = false;
        }

        bool                      SAT1_result, SAT2_result;
        if(enable_constant){
            cout << "enable_constant" << endl;
            SAT1_result = satmgr.solver.solve();
        }
        else{
            cout << "disable_constant" << endl;
            SAT1_result = satmgr.solver.assumpSolve(close_constant_assump);
        }
        /*
        SAT1_result = satmgr.solver.assumpSolve(BusMatchAssump);
        if (!SAT1_result) { // when find_valid_output_assump couldn't find feasible match, change bus match
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
            // BusMatchAssump.copyTo(find_valid_output_assump);
        }
        */
        if (!SAT1_result) { // use satmgr.addCandidateBusConstraint(satmgr.solver), no manual BusMatchAssump anymore
            if(enable_constant){ // even enable constant matching can't find answer.
                cout << "No match!!" << endl;
                break;
            }
            else{
                enable_constant = true;
            }
        }
        else {
            unordered_set<variable*>  circuit2_func_supp_union;
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
                        // cout << "MI i: " << i << "\tj: " << j << " val: "
                        //      << satmgr.solver.getValue(MI[i][j]->getVar())
                        //      << endl;
                        cout << "MI i:\t" << i << "\t" <<  ((i%2 == 0 || i >= MI.size() - 2) ? "" : "!") << ((i == MI.size() - 2) ? "0" : (i == MI.size() - 1) ? "1" : satmgr.cirmgr.x[i/2]->getname())
                             << "\tin bus:\t" << ((i < MI.size() - 2) ? to_string(satmgr.cirmgr.u_name_busIndex_input_ckt1[satmgr.cirmgr.x[i/2]->getname()]) : "")                    
                             << "\tj:\t"   << j << "\t" <<                             satmgr.cirmgr.y[j]->getname()
                             << "\tin bus:\t" << satmgr.cirmgr.u_name_busIndex_input_ckt2[satmgr.cirmgr.y[j]->getname()]                    
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
                            // if(finding_output){
                            //     find_input_given_output_assump.push(~Lit(satmgr.cirmgr.MO_no_pos_neg[i/2][j]));
                            // }
                        }
                    }
                    if (satmgr.solver.getValue(MO[i][j]->getVar()) == 1) {
                        Lit v1 = ~Lit(MO[i][j]->getVar());
                        be_searched.push(v1);

                        Lit v = Lit(MO[i][j]->getVar2());
                        assump.push(v);
                        // cout << "MO i: " << i << " j: " << j << " val: "
                        //      << satmgr.solver.getValue(MO[i][j]->getVar())
                        //      << endl;
                        cout << "MO i:\t" << i << "\t" <<  ((i%2 == 0) ? "" : "!") << satmgr.cirmgr.f[i/2]->getname()
                             << "\tin bus:\t" << satmgr.cirmgr.u_name_busIndex_output_ckt1[satmgr.cirmgr.f[i/2]->getname()]                    
                             << "\tj:\t"   << j << "\t" <<                             satmgr.cirmgr.g[j]->getname()
                             << "\tin bus:\t" << satmgr.cirmgr.u_name_busIndex_output_ckt2[satmgr.cirmgr.g[j]->getname()]                    
                             << endl;
                        // collect matched outputs' function support
                        for (int k = 0, n = satmgr.cirmgr.g[j]->_funcSupp.size(); k < n; k++) {
                            circuit2_func_supp_union.insert(satmgr.cirmgr.g[j]->_funcSupp[k]);
                            // cout << "cir2: " << satmgr.cirmgr.g[j]->_funcSupp[k]->getname() << endl;
                        }
                    
                    } else if (satmgr.solver.getValue(MO[i][j]->getVar()) == 0) {
                        Lit v1 = Lit(MO[i][j]->getVar());
                        be_searched.push(v1);

                        Lit v = ~Lit(MO[i][j]->getVar2());
                        assump.push(v);
                    }
                }
            }
            for (int i = 0; i < satmgr.cirmgr.MIbus_Var.size(); i++){
                for(int j = 0; j < satmgr.cirmgr.MIbus_Var[0].size(); j++){
                    if (satmgr.solver.getValue(satmgr.cirmgr.MIbus_Var[i][j]) == 1){
                        cout << "MIbus cir1:\t" << i << "\t<->\t" << "MIbus cir2:\t" << j << endl;
                    }
                }
            }
            for (int i = 0; i < satmgr.cirmgr.MObus_Var.size(); i++){
                for(int j = 0; j < satmgr.cirmgr.MObus_Var[0].size(); j++){
                    if (satmgr.solver.getValue(satmgr.cirmgr.MObus_Var[i][j]) == 1){
                        cout << "MObus cir1:\t" << i << "\t<->\t" << "MObus cir2:\t" << j << endl;
                    }
                }
            }

            // assign 0 to inrelevant cir2's input
            for (int i = 0, n = satmgr.cirmgr.y.size(); i < n; i++) {
                if (!circuit2_func_supp_union.count(satmgr.cirmgr.y[i])){
                    Lit v = ~Lit(satmgr.cirmgr.y[i]->getVar2());
                    // cout << satmgr.cirmgr.y[i]->getname() << endl;
                    // cout << satmgr.cirmgr.y[i]->getname() << endl;
                    assump.push(v);
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
                level++;

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
                    satmgr.record_input.clear();
                    satmgr.record_output.clear();
                    for (int i = 0, n = MI.size(); i < n; i++){
                        vector<int> temp;
                        for (int j = 0, u = MI[0].size(); j < u; j++){
                            if (satmgr.solver.getValue(MI[i][j]->getVar()) == 1){
                                temp.push_back(1);
                            }
                            else{
                                temp.push_back(0);
                            }
                        }
                        satmgr.record_input.push_back(temp);
                    }
                    for (int i = 0, n = MO.size(); i < n; i++){
                        vector<int> temp;
                        for (int j = 0, u = MO[0].size(); j < u; j++){
                            if (satmgr.solver.getValue(MO[i][j]->getVar()) == 1){
                                temp.push_back(1);
                            }
                            else{
                                temp.push_back(0);
                            }
                        }
                        satmgr.record_output.push_back(temp);
                    }
                    satmgr.reportResult(satmgr.solver);
                    cout << inputFilename << endl;
                    cout << "point : " << float(satmgr.point) / float(satmgr.cirmgr.outputNum_ckt1 + satmgr.cirmgr.outputNum_ckt2) << endl;
                }
            } else {
                // cout << "ELSE" << endl;
                satmgr.AddLearnedClause(satmgr.solver, satmgr.miterSolver);
                // AddLearnedClause_const(solver, miterSolver);
                // if(finding_output){
                //     satmgr.funcSuppInputConstraint(MO_no_pos_neg_pair, BusMatchIdx_I, BusMatchIdx_O, find_input_given_output_assump);
                //     finding_output = false;
                // }
            }
            assump.clear();
        }
    }
    satmgr.reportResult(satmgr.solver);
    cout << inputFilename << endl;
    cout << "point : " << float(satmgr.point) / float(satmgr.cirmgr.outputNum_ckt1 + satmgr.cirmgr.outputNum_ckt2) << endl;
}