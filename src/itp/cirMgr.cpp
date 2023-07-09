#include "cirMgr.h"
#include "sat.h"
#include <fstream>
#include <string>
#include <algorithm>

void
CirMgr::reset() {
    portnum_ckt1.clear();
    portname_ckt1.clear();
    portnum_ckt2.clear();
    portname_ckt2.clear();
    inputNum_ckt1 = outputNum_ckt1 = inputNum_ckt2 = outputNum_ckt2 = 0;

    umapNum_ckt1.clear();
    umapName_ckt1.clear();
    umapNum_ckt2.clear();
    umapName_ckt2.clear();

    umapInterVar_ckt1.clear();
    umapInterVar_ckt1_veri.clear();
    umapInterVar_ckt2.clear();
    umapInterVar_ckt2_veri.clear();

    MI.clear();
    MO.clear();
    x.clear();
    y.clear();
    f.clear();
    g.clear();
}
void
CirMgr::readAAG() {
    ifstream faag1, faag2;
    string   faag1_name = "", faag2_name = "";
    // faag1_name = faag2_name = "./files/aag/";
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
            int cnt = 0;
            portnum_ckt1.push_back(tmp);
            faag1 >> tmp2;
            portname_ckt1.push_back(tmp2);
            u_name_index_ckt1[tmp2] = cnt;
            if(tmp[0] == 'i') u_name_isInput_ckt1[tmp2] = true;
            else u_name_isInput_ckt1[tmp2] = false;
            portNameNumpairs_ckt1.push_back(pair<string, string>(
                portnum_ckt1.back(), portname_ckt1.back()));
            
            cnt ++;
            while (true) {
                faag1 >> tmp >> tmp2;
                if (tmp == "c") break;
                portnum_ckt1.push_back(tmp);
                portname_ckt1.push_back(tmp2);
                if(tmp == "o0") cnt = 0;
                u_name_index_ckt1[tmp2] = cnt++;
                if(tmp[0] == 'i') u_name_isInput_ckt1[tmp2] = true;
                else u_name_isInput_ckt1[tmp2] = false;
                portNameNumpairs_ckt1.push_back(pair<string, string>(
                    portnum_ckt1.back(), portname_ckt1.back()));
            }
            break;
        }
    }
    while (true) {
        string tmp, tmp2;
        faag2 >> tmp;
        if (tmp == "i0") { // first line. ex: i0 a
            int cnt = 0;
            portnum_ckt2.push_back(tmp);
            faag2 >> tmp2;
            portname_ckt2.push_back(tmp2);
            u_name_index_ckt2[tmp2] = cnt;
            if(tmp[0] == 'i') u_name_isInput_ckt2[tmp2] = true;
            else u_name_isInput_ckt2[tmp2] = false;
            portNameNumpairs_ckt2.push_back(pair<string, string>(
                portnum_ckt2.back(), portname_ckt2.back()));
            cnt ++;
            while (true) {
                faag2 >> tmp >> tmp2;
                if (tmp == "c") break;
                portnum_ckt2.push_back(tmp);
                portname_ckt2.push_back(tmp2);
                if(tmp == "o0") cnt = 0;
                u_name_index_ckt2[tmp2] = cnt++;
                if(tmp[0] == 'i') u_name_isInput_ckt2[tmp2] = true;
                else u_name_isInput_ckt2[tmp2] = false;
                portNameNumpairs_ckt2.push_back(pair<string, string>(
                    portnum_ckt2.back(), portname_ckt2.back()));
            }
            break;
        }
    }
}
void
CirMgr::readMAP() {

    ifstream fmap1, fmap2;
    string   fmap1_name = "", fmap2_name = "";
    // fmap1_name = fmap2_name = "./files/map/";
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
}
void
CirMgr::readCNF(SatSolver& s_miter,
                SatSolver& s_verifier) { // read cnf file: top1.cnf, top2.cnf
    // read CNF into miter solver to constraint functionality
    // correctness
    ifstream fcnf1, fcnf2;
    string   fcnf1_name = "", fcnf2_name = "";
    // fcnf1_name = fcnf2_name = "./files/cnf/";
    fcnf1_name += "top1.cnf";
    fcnf2_name += "top2.cnf";
    fcnf1.open(fcnf1_name);
    fcnf2.open(fcnf2_name);
    string tmp, cnf;
    int    varnum, clsnum, litTmp;
    size_t ct1 = 0, ct2 = 0;
    while (true) {

        fcnf1 >> tmp;
        if (tmp == "p") {
            fcnf1 >> cnf >> varnum >> clsnum;
            // cout<<"CC: " << cnf << " " << varnum  << " " << clsnum <<
            // endl;
            vec<Lit> lits, lits_veri;

            vector<Var> test; // test addClause

            while (fcnf1 >> litTmp) {

                // cout << "LL: " << litTmp << endl;
                if (litTmp != 0) {
                    if (umapNum_ckt1.find(abs(litTmp)) ==
                        umapNum_ckt1.end()) { // doesn't exist
                        umapNum_ckt1[abs(litTmp)] =
                            "n" + to_string(abs(litTmp));
                        Var nV                         = s_miter.newVar();
                        Var nV_veri                    = s_verifier.newVar();
                        umapInterVar_ckt1[abs(litTmp)] = nV;
                        umapInterVar_ckt1_veri[abs(litTmp)] = nV_veri;
                        Lit li, li_veri;
                        if (litTmp > 0) {
                            li      = Lit(nV);
                            li_veri = Lit(nV_veri);
                        } else {
                            li      = ~Lit(nV);
                            li_veri = ~Lit(nV_veri);
                        }

                        lits.push(li);
                        lits_veri.push(li_veri);
                        test.push_back(nV);
                    } else {
                        string portNumTmp =
                            umapNum_ckt1[abs(litTmp)]; // i0, i1 ..
                        // cout << "PPP: " << portNumTmp << endl;
                        // add clause to s_miter according to i/o and
                        // num. ex: i0 --->  x[0]->getVar2()
                        if (portNumTmp[0] == 'i') { // those var in x
                            Lit li, li_veri;
                            if (litTmp > 0) {
                                li = Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar2());
                                li_veri = Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar3());

                            } else {
                                li = ~Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar2());
                                li_veri = ~Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar3());
                            }
                            lits.push(li);
                            lits_veri.push(li_veri);
                            test.push_back(
                                x[stoi(portNumTmp.substr(1))]->getVar2());
                        } else if (portNumTmp[0] == 'o') { // o
                            Lit li, li_veri;
                            if (litTmp > 0) {
                                li = Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar2());
                                li_veri = Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar3());

                            } else {
                                li = ~Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar2());
                                li_veri = ~Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar3());
                            }
                            lits.push(li);
                            lits_veri.push(li_veri);
                            test.push_back(
                                f[stoi(portNumTmp.substr(1))]->getVar2());
                        } else { // n (intervariable)
                            Lit li, li_veri;
                            if (litTmp > 0) {
                                li = Lit(umapInterVar_ckt1[abs(litTmp)]);
                                li_veri =
                                    Lit(umapInterVar_ckt1_veri[abs(litTmp)]);

                            } else {
                                li = ~Lit(umapInterVar_ckt1[abs(litTmp)]);
                                li_veri =
                                    ~Lit(umapInterVar_ckt1_veri[abs(litTmp)]);
                            }

                            lits.push(li);
                            lits_veri.push(li_veri);
                            test.push_back(umapInterVar_ckt1[abs(litTmp)]);
                        }
                    }
                } else {
                    ++ct1;
                    // cout << lits_veri.size() << endl;
                    s_miter.addClause(lits);
                    s_verifier.addClause(lits_veri);
                    lits.clear();
                    lits_veri.clear();
                    // for(int i=0;i<test.size();i++){
                    //    cout << test[i] << " ";
                    // }
                    // cout << "\n" << "testclause ---" << endl;
                    test.clear();
                }
            }

            break;
        }
    }
    while (true) {

        fcnf2 >> tmp;
        if (tmp == "p") {
            fcnf2 >> cnf >> varnum >> clsnum;
            // cout<<"CC: " << cnf << " " << varnum  << " " << clsnum <<
            // endl;
            vec<Lit> lits, lits_veri;
            while (fcnf2 >> litTmp) {

                // cout << "LL: " << litTmp << endl;
                if (litTmp != 0) {
                    if (umapNum_ckt2.find(abs(litTmp)) ==
                        umapNum_ckt2.end()) { // doesn't exist
                        umapNum_ckt2[abs(litTmp)] =
                            "n" + to_string(abs(litTmp));
                        Var nV                         = s_miter.newVar();
                        Var nV_veri                    = s_verifier.newVar();
                        umapInterVar_ckt2[abs(litTmp)] = nV;
                        umapInterVar_ckt2_veri[abs(litTmp)] = nV_veri;
                        Lit li, li_veri;
                        if (litTmp > 0) {
                            li      = Lit(nV);
                            li_veri = Lit(nV_veri);
                        } else {
                            li      = ~Lit(nV);
                            li_veri = ~Lit(nV_veri);
                        }

                        lits.push(li);
                        lits_veri.push(li_veri);
                    } else {
                        string portNumTmp =
                            umapNum_ckt2[abs(litTmp)]; // i0, i1 ..
                        // cout << "PPP: " << portNumTmp << endl;
                        // add clause to s_miter according to i/o and
                        // num. ex: i0 --->  x[0]->getVar2()
                        if (portNumTmp[0] == 'i') {
                            Lit li, li_veri;
                            if (litTmp > 0) {
                                li = Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar2());
                                li_veri = Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar3());

                            } else {
                                li = ~Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar2());
                                li_veri = ~Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar3());
                            }

                            lits.push(li);
                            lits_veri.push(li_veri);
                        } else if (portNumTmp[0] == 'o') { // o
                            Lit li, li_veri;
                            if (litTmp > 0) {
                                li = Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar2());
                                li_veri = Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar3());

                            } else {
                                li = ~Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar2());
                                li_veri = ~Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar3());
                            }

                            lits.push(li);
                            lits_veri.push(li_veri);
                        } else { // n (intervariable)
                            Lit li, li_veri;
                            if (litTmp > 0) {
                                li = Lit(umapInterVar_ckt2[abs(litTmp)]);
                                li_veri =
                                    Lit(umapInterVar_ckt2_veri[abs(litTmp)]);

                            } else {
                                li = ~Lit(umapInterVar_ckt2[abs(litTmp)]);
                                li_veri =
                                    ~Lit(umapInterVar_ckt2_veri[abs(litTmp)]);
                            }

                            lits.push(li);
                            lits_veri.push(li_veri);
                        }
                    }
                } else {
                    ++ct2;
                    s_miter.addClause(lits);
                    s_verifier.addClause(lits_veri);
                    lits.clear();
                    lits_veri.clear();
                }
            }
            break;
        }
    }
    // cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << endl;
}
/*
void
CirMgr::_readbus(ifstream& fbus, vector<vector<string>>& bus_list_ckt,
                 bool _isCkt1) {
    vector<pair<string, string>>& portNameNumpairs_ckt =
        (_isCkt1 ? portNameNumpairs_ckt1 : portNameNumpairs_ckt2);

    string port;
    int    num_bus = 0, num_port = 0; // number of bus & port
    fbus >> port >>
        num_bus; // this port is to buffer the "circuit1.v" & "circuit2.v"
    bus_list_ckt.reserve(num_bus);
    for (size_t i = 0; i < num_bus; ++i) {
        vector<string> temp;
        fbus >> num_port;
        temp.reserve(num_port);
        for (int j = 0; j < num_port; ++j) {
            fbus >> port;
            for (size_t k = 0, n = portNameNumpairs_ckt.size(); k < n; ++k) {
                if (portNameNumpairs_ckt[k].second == port) {
                    vector<variable*>& vecVar =
                        ((portNameNumpairs_ckt[k].first[0] == 'i')
                             ? (_isCkt1 ? x : y)
                             : (_isCkt1 ? f : g));
                    size_t _offset =
                        (portNameNumpairs_ckt[k].first[0] == 'i')
                            ? 0
                            : (_isCkt1
                                   ? inputNum_ckt1
                                   : inputNum_ckt2); // have to substract the
                                                     // input num if vecVar is
                                                     // the output port vector
                    vecVar[k - _offset]->setBusSize(num_port);
                    vecVar[k - _offset]->setBusIndex(i);
                    break;
                }
            }
            // for (auto k : x) {
            //     if (k->getName() == port) {
            //         k->setBusSize(num_port);
            //         k->setBusIndex(i);
            //     }
            // }
            temp.push_back(port);
        }
        bus_list_ckt.push_back(temp);
    }
}
*/
// void
// CirMgr::_readSupp(ifstream& fsupp, bool _isCkt1) {
//     vector<pair<string, string>>& portNameNumpairs_ckt =
//         (_isCkt1 ? portNameNumpairs_ckt1 : portNameNumpairs_ckt2);
//     size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
//     size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
//     string port;
//     int  num_supp = 0; // number of bus & port
//     for (size_t i = 0; i < outputNum; ++i) {
//         fsupp >> port >> num_supp;
//         for (size_t k = inputNum, n = portNameNumpairs_ckt.size(); k < n; ++k) {
//             if (portNameNumpairs_ckt[k].second == port) {
//                 vector<variable*>& vecVar  = _isCkt1 ? f : g;
//                 size_t _offset = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2; // have to substract the input num if vecVar is the output port vector
//                 vecVar[k - _offset]->setSuppSize(num_supp);
//                 break;
//             }
//         }
//     }
// }
// void
// CirMgr::_readUnate(ifstream& funate, bool _isCkt1) {
//     vector<pair<string, string>>& portNameNumpairs_ckt =
//         (_isCkt1 ? portNameNumpairs_ckt1 : portNameNumpairs_ckt2);
//     size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
//     size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
//     string port;
//     int num_unate = 0; // number of bus & port
//     for (size_t i = 0; i < outputNum; ++i) {
//         funate >> port >> num_unate;
//         for (size_t k = inputNum, n = portNameNumpairs_ckt.size(); k < n; ++k) {
//             if (portNameNumpairs_ckt[k].second == port) {
//                 vector<variable*>& vecVar  = _isCkt1 ? f : g;
//                 size_t _offset = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2; // have to substract the input num if vecVar is the output port vector
//                 vecVar[k - _offset]->setUnateSum(num_unate);
//                 break;
//             }
//         }
//     }
//     return;
// }
void
CirMgr::_readPreprocess(ifstream& fPreprocess, bool _isCkt1, preprocess _p) {
    vector<pair<string, string>>& portNameNumpairs_ckt =
        (_isCkt1 ? portNameNumpairs_ckt1 : portNameNumpairs_ckt2);
    size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
    string port, portName = "";
    int num = 0;
    int cnt = 0;
    for (size_t i = 0; i < outputNum; ++i) {
        fPreprocess >> port >> num;
        cout << port << ' ' << num << ' ';
        for (size_t k = inputNum, n = portNameNumpairs_ckt.size(); k < n; ++k) {
            if (portNameNumpairs_ckt[k].second == port) {
                cnt++;
                vector<variable*>& vecVar  = _isCkt1 ? f : g;
                size_t _offset = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2; // have to substract the input num if vecVar is the output port vector
                switch (_p){
                    case outputUnateness:
                        vecVar[k - _offset]->setOutputUnateNum(num);
                        break;
                    case support:
                        vecVar[k - _offset]->setSuppSize(num);
                        while(fPreprocess >> portName)
                        {
                            if(portName == ";")
                                break;
                            for (size_t j = 0; j < inputNum; ++j) {
                                if (portNameNumpairs_ckt[j].second == portName){
                                    // cout << portName << ' ';
                                    vecVar[k - _offset]->funcSupp().push_back(vecVar[j]);
                                    break;
                                }
                            }
                        }
                        // cout << endl;
                        break;
                }
                break;
            }
        }
    }
    for(auto i : f)
        cout << "funcSupp size = "<<i->funcSupp().size() << endl;
    return;
}
void
CirMgr::_readInputUnateness(ifstream& fInputUnateness, bool _isCkt1) {
    vector<pair<string, string>>& portNameNumpairs_ckt =
        (_isCkt1 ? portNameNumpairs_ckt1 : portNameNumpairs_ckt2);
    size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
    string resultingOutputs = "";
    int num = 0;
    int cnt = 0;
    for(size_t i = 0; i < outputNum; ++i)
    {
        fInputUnateness >> resultingOutputs;
        for(size_t j = 0, nj = resultingOutputs.length(); j < nj; ++j)
            if(resultingOutputs[j] == 'p')
            {
                vector<variable*>& vecVar  = _isCkt1 ? x : y;
                vecVar[j]->addInputUnateNum_p();
            }
            else if(resultingOutputs[j] == 'n')
            {
                vector<variable*>& vecVar  = _isCkt1 ? x : y;
                vecVar[j]->addInputUnateNum_n();
            }

    }
    // vector<variable*>& vecVar  = _isCkt1 ? x : y;
    // for(auto i : vecVar)
    //     cout << "( "<<i->inputUnateNum_p() << " , " << i->inputUnateNum_n() << " ) ; ";
    // cout << endl;
}
// void
// CirMgr::readBus(string& inputFileName) {
//     ifstream fbus(inputFileName);
//     _readbus(fbus, bus_list_ckt1, 1);
//     _readbus(fbus, bus_list_ckt2, 0);
// }
void
CirMgr::readPreporcess(preprocess _p) {
    string f_name = "";
    switch(_p)
    {
        case outputUnateness:
            f_name = "./preprocess/outputUnateness.txt" ;
            break;
        case support:
            f_name = "./preprocess/funcsupp.txt" ;
            break;
    }
    ifstream fPreprocess(f_name);
    _readPreprocess(fPreprocess, 1, _p);
    _readPreprocess(fPreprocess, 0, _p);
}


void
CirMgr::readInputUnateness() {
    string   fInputUnateness_name = "./preprocess/inputUnateness.txt";
    ifstream fInputUnateness(fInputUnateness_name);
    _readInputUnateness(fInputUnateness, 1);    // read ckt1
    _readInputUnateness(fInputUnateness, 0);    // read ckt2
}

void CirMgr::outputGrouping(){  // |f| = |g|
    vector<variable*> f_sorted = f, g_sorted = g; // sorted by (functional supp), structral support, fanin size
    vector<vector<variable*>> f_groups({{}}), g_groups({{}}); // set of groups of f & g
    std::sort(f_sorted.begin(), f_sorted.end(), _outputsorting);
    std::sort(g_sorted.begin(), g_sorted.end(), _outputsorting);
    for(size_t i = 0, n = f.size(); i < n; ++i)
    {
        if(i != 0 && f_sorted[i]->suppSize() > g_sorted[i - 1]->suppSize())
        {
            f_groups.push_back({});
            g_groups.push_back({});
        }
        f_groups.back().push_back(f_sorted[i]);
        g_groups.back().push_back(g_sorted[i]);
    }
    for(size_t i = 0, n = f_groups.size(); i < n; ++i)
        for(auto j : f_groups[i])
            j->setOutputGroupingNum(i);
    for(size_t i = 0, n = g_groups.size(); i < n; ++i)
        for(auto j : g_groups[i])
            j->setOutputGroupingNum(i);
    // exit(0);
}
bool CirMgr::_outputsorting(variable* a, variable* b)
{
    return a->suppSize() < b->suppSize();
}



void
CirMgr::readBus_class(string& inputfilename){
    ifstream fbus(inputfilename);
    string port;
    int num_bus = 0, num_port = 0;
    
    fbus >> port >> num_bus;
    for(size_t i = 0; i < num_bus; i++){
        fbus >> num_port;
        Bus* newBus = new Bus();
        for(size_t j = 0; j < num_port; j++){
            fbus >> port;
            // cout << u_name_index_ckt1[port] << endl;
            if(u_name_isInput_ckt1[port]){
                exist_x[u_name_index_ckt1[port]] = true;
                cout << "ex x name: " << port << " idx: " << u_name_index_ckt1[port] << endl;
            }
            else{
                exist_f[u_name_index_ckt1[port]] = true;
                cout << "ex f name: " << port << " idx: " << u_name_index_ckt1[port] << endl;
            }
            newBus->setIsInput(u_name_isInput_ckt1[port]);
            newBus->setIsCkt1(true);
            newBus->setPortNum(num_port);
            newBus->insertIndex(u_name_index_ckt1[port]);
            newBus->insertName(port);
        }
        if(newBus->getIsInput()) bus_ckt1_input.push_back(newBus);
        else                     bus_ckt1_output.push_back(newBus);
    }
    fbus >> port >> num_bus;
    for(size_t i = 0; i < num_bus; i++){
        fbus >> num_port;
        Bus* newBus = new Bus();
        for(size_t j = 0; j < num_port; j++){
            fbus >> port;
            if(u_name_isInput_ckt2[port]){
                exist_y[u_name_index_ckt2[port]] = true;
                cout << "ex y name: " << port << " idx: " << u_name_index_ckt2[port] << endl;
            }
            else{
                exist_g[u_name_index_ckt2[port]] = true;
                cout << "ex g name: " << port << " idx: " << u_name_index_ckt2[port] << endl;
            }
            newBus->setIsInput(u_name_isInput_ckt2[port]);
            newBus->setIsCkt1(false);
            newBus->setPortNum(num_port);
            newBus->insertIndex(u_name_index_ckt2[port]);
            newBus->insertName(port);
        }
        if(newBus->getIsInput()) bus_ckt2_input.push_back(newBus);
        else                     bus_ckt2_output.push_back(newBus);
    }

    sort(bus_ckt1_input.begin(), bus_ckt1_input.end(), [](Bus* a, Bus* b){return a->getPortNum() > b->getPortNum();});
    sort(bus_ckt1_output.begin(), bus_ckt1_output.end(), [](Bus* a, Bus* b){return a->getPortNum() > b->getPortNum();});
    sort(bus_ckt2_input.begin(), bus_ckt2_input.end(), [](Bus* a, Bus* b){return a->getPortNum() > b->getPortNum();});
    sort(bus_ckt2_output.begin(), bus_ckt2_output.end(), [](Bus* a, Bus* b){return a->getPortNum() > b->getPortNum();});

    valid_busMatch_ckt2_input = permute(bus_ckt2_input);
    valid_busMatch_ckt2_output = permute(bus_ckt2_output);

    cout << "Vi size: " << valid_busMatch_ckt2_input.size() << endl;
    cout << "Vo size: " << valid_busMatch_ckt2_output.size() << endl;

    /*
    //test
    cout << "b 1 i " << bus_ckt1_input.size() << endl;
    for(int i=0; i<bus_ckt1_input.size(); i++){
        cout << bus_ckt1_input[i]->getIsInput() << " " << bus_ckt1_input[i]->getIsCkt1() << " " << bus_ckt1_input[i]->getPortNum() <<  endl;
        for(int j=0; j<bus_ckt1_input[i]->names.size(); j++){
            cout << bus_ckt1_input[i]->names[j] << endl;
        }
    }
    cout << "\nb 1 o " << bus_ckt1_output.size()  << endl;
    for(int i=0; i<bus_ckt1_output.size(); i++){
        cout << bus_ckt1_output[i]->getIsInput() << " " << bus_ckt1_output[i]->getIsCkt1() << " " << bus_ckt1_output[i]->getPortNum() <<  endl;
        for(int j=0; j<bus_ckt1_output[i]->names.size(); j++){
            cout << bus_ckt1_output[i]->names[j] << endl;
        }
    }
    cout << "\nb 2 i " << bus_ckt2_input.size() << endl;
    for(int i=0; i<bus_ckt2_input.size(); i++){
        cout << bus_ckt2_input[i]->getIsInput() << " " << bus_ckt2_input[i]->getIsCkt1() << " " << bus_ckt2_input[i]->getPortNum() <<  endl;
        for(int j=0; j<bus_ckt2_input[i]->names.size(); j++){
            cout << bus_ckt2_input[i]->names[j] << endl;
        }
    }
    cout << "\nb 2 o " << bus_ckt2_output.size()  << endl;
    for(int i=0; i<bus_ckt2_output.size(); i++){
        cout << bus_ckt2_output[i]->getIsInput() << " " << bus_ckt2_output[i]->getIsCkt1() << " " << bus_ckt2_output[i]->getPortNum() <<  endl;
        for(int j=0; j<bus_ckt2_output[i]->names.size(); j++){
            cout << bus_ckt2_output[i]->names[j] << endl;
        }
    }
    cout << "b4 permute" << endl;
    cout << "valid bus match" << endl;
    
    for(int i=0; i < valid_busMatch_ckt2_input.size(); i++){
        cout << "Group" << endl;
        for(int j=0; j < valid_busMatch_ckt2_input[0].size(); j++){
            for(auto k: valid_busMatch_ckt2_input[i][j]->names){
                cout << k << " ";
            }
            cout << endl;
        }
    }
    for(int i=0; i < valid_busMatch_ckt2_output.size(); i++){
        cout << "Group" << endl;
        for(int j=0; j < valid_busMatch_ckt2_output[0].size(); j++){
            for(auto k: valid_busMatch_ckt2_output[i][j]->names){
                cout << k << " ";
            }
            cout << endl;
        }
    }
    cout << "RB end" << endl;
    */
}