#include "cirMgr.h"
#include "sat.h"
#include <fstream>
#include <string>

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
            portnum_ckt1.push_back(tmp);
            faag1 >> tmp;
            portname_ckt1.push_back(tmp);
            portNameNumpairs_ckt1.push_back(pair<string, string>(
                portnum_ckt1.back(), portname_ckt1.back()));
            while (true) {
                faag1 >> tmp >> tmp2;
                if (tmp == "c") break;
                portnum_ckt1.push_back(tmp);
                portname_ckt1.push_back(tmp2);
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
            portnum_ckt2.push_back(tmp);
            faag2 >> tmp;
            portname_ckt2.push_back(tmp);
            portNameNumpairs_ckt2.push_back(pair<string, string>(
                portnum_ckt2.back(), portname_ckt2.back()));
            while (true) {
                faag2 >> tmp >> tmp2;
                if (tmp == "c") break;
                portnum_ckt2.push_back(tmp);
                portname_ckt2.push_back(tmp2);
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
                    cout << lits_veri.size() << endl;
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
    cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << endl;
}

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
    string port;
    int num = 0;
    int cnt = 0;
    for (size_t i = 0; i < outputNum; ++i) {
        fPreprocess >> port >> num;
        for (size_t k = inputNum, n = portNameNumpairs_ckt.size(); k < n; ++k) {
            if (portNameNumpairs_ckt[k].second == port) {
                cnt++;
                vector<variable*>& vecVar  = _isCkt1 ? f : g;
                size_t _offset = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2; // have to substract the input num if vecVar is the output port vector
                switch (_p){
                    case unateness:
                        vecVar[k - _offset]->setUnateSum(num);
                        break;
                    case support:
                        vecVar[k - _offset]->setSuppSize(num);
                        break;
                }
                break;
            }
        }
    }
    return;
}

void
CirMgr::readBus() {
    string   fbus_name = "./input";
    ifstream fbus(fbus_name);
    _readbus(fbus, bus_list_ckt1, 1);
    _readbus(fbus, bus_list_ckt2, 0);
}
// void
// CirMgr::readSupp() {
//     string   fsupp_name = "./funcsupp.txt";
//     ifstream fsupp(fsupp_name);
//     _readSupp(fsupp, 1);
//     _readSupp(fsupp, 0);
// }
// void
// CirMgr::readUnate() {
//     string   funate_name = "./unateness.txt";
//     ifstream funate(funate_name);
//     _readUnate(funate, 1);
//     _readUnate(funate, 0);
// }
void
CirMgr::readPreporcess(preprocess _p) {
    string f_name = _p == unateness ? "./unateness.txt" : "./funcsupp.txt";
    ifstream fPreprocess(f_name);
    _readPreprocess(fPreprocess, 1, _p);
    _readPreprocess(fPreprocess, 0, _p);
}