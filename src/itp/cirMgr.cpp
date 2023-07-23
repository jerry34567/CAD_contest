#include "cirMgr.h"
#include "sat.h"
#include <fstream>
#include <string>
#include <algorithm>
#include <bitset>
#include <sstream>
#include <iomanip>

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
    int output0counter = ip + 1;
    bool flag0 = 1;
    inputNum_ckt1  = ip;
    outputNum_ckt1 = op;

    faag2 >> aag >> maxidx >> ip >> latch >> op >> andgate;
    inputNum_ckt2  = ip;
    outputNum_ckt2 = op;

    while (true) {
        string tmp, tmp2;
        if(flag0 == 1)
            --output0counter;
        else 
            ++output0counter;
        if(output0counter == 0)
            flag0 = 0;
        faag1 >> tmp;
        if(tmp == "0")
            output0.push_back(output0counter);
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
                // vector<variable*>& vecVar2  = _isCkt1 ? x : y;
                size_t _offset = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2; // have to substract the input num if vecVar is the output port vector
                switch (_p){
                    case outputUnateness:
                        vecVar[k - _offset]->setOutputUnateNum(num);
                        break;
                    case support:
                        vecVar[k - _offset]->setSuppSize(num);
                        vector<variable*>& inputVec  = _isCkt1 ? x : y;
                        while(fPreprocess >> portName)
                        {
                            if(portName == ";")
                                break;
                            for (size_t j = 0; j < inputNum; ++j) {
                                if (portNameNumpairs_ckt[j].second == portName){
                                    // cout << portName << ' ';
                                    vecVar[k - _offset]->_funcSupp.push_back(inputVec[j]);
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
    // for(auto i : f)
    //     cout << "funcSupp size = "<< i->_funcSupp.size() << endl;
    return;
}
void
CirMgr::_readInputUnateness(ifstream& fInputUnateness, bool _isCkt1) {
    vector<variable*>& outputVariables = _isCkt1 ? f : g;
    size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
    string resultingOutputs = "";
    int num = 0;
    int cnt = 0;
    for(size_t i = 0; i < outputNum; ++i)
    {
        fInputUnateness >> resultingOutputs;
        outputVariables[i]->setInputUnates(resultingOutputs);
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
    return;
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
void CirMgr::recordPIsupportPO(){ // record those POs affected by each PI, record in each PI's _funcSupp_PI
    cout << "recordPIsupportPO" << endl;
    for(int i = 0; i < f.size(); i++){
        for(int j = 0; j < f[i]->_funcSupp.size(); j++){
            x[u_name_index_ckt1[f[i]->_funcSupp[j]->getname()]]->_funcSupp_PI.push_back(f[i]);
        }
    }
    for(int i = 0; i < g.size(); i++){
        for(int j = 0; j < g[i]->_funcSupp.size(); j++){
            y[u_name_index_ckt2[g[i]->_funcSupp[j]->getname()]]->_funcSupp_PI.push_back(g[i]);
        }
    }
} 
void CirMgr::busSupportUnion(){
    _busSupportUnion(1);
    _busSupportUnion(0);
    // int cnt = 0;
    // for(size_t i = 0, ni = bus_ckt1_output.size(); i < ni; ++i)
    //     for(size_t j = 0, nj = bus_ckt2_output.size(); j < nj; ++j){
    //         if(bus_ckt1_output[i]->unionSupportSize() > bus_ckt2_output[j]->unionSupportSize())
    //         {
    //             cnt++;
    //             bus_ckt1_output[i]->busMatching()[bus_ckt2_output[j]->busIndex()] = 1;
    //             bus_ckt2_output[j]->busMatching()[bus_ckt1_output[i]->busIndex()] = 1;
    //         }
    //     }
    return;
}
void CirMgr::busInputSupportUnion(){
    _busInputSupportUnion(1);
    _busInputSupportUnion(0);
    return;
}



void CirMgr::busOutputUnateness(){
    _busOutputUnateness(1);
    _busOutputUnateness(0);
    
}
void CirMgr::busInputUnateness(){
    _busInputUnateness(1);
    _busInputUnateness(0);
    
}

void CirMgr::readInputUnateness() {
    string   fInputUnateness_name = "./preprocess/inputUnateness.txt";
    ifstream fInputUnateness(fInputUnateness_name);
    _readInputUnateness(fInputUnateness, 1);    // read ckt1
    _readInputUnateness(fInputUnateness, 0);    // read ckt2
}

void CirMgr::readSymmetric() {
    string   fSymmetric_name = "./preprocess/symmetric.txt";
    ifstream fSymmetric(fSymmetric_name);
    int counter = 0;
    string temp;
    string temp2;
    while (counter != (outputNum_ckt1 + outputNum_ckt2)) {
        fSymmetric >> temp;
        if (counter < outputNum_ckt1){
            if (temp != "end") {
                fSymmetric >> temp2;
                bool has_been_inserted = false;
                for (int i = 0, n = f[counter]->symmetric_set.size(); i < n; i++) {
                    if (f[counter]->symmetric_set[i].count(stoi(temp))) {
                        has_been_inserted = true;
                        if (!f[counter]->symmetric_set[i].count(stoi(temp2))) {
                            f[counter]->symmetric_set[i].insert(stoi(temp2));
                        }
                    }
                }
                if (!has_been_inserted){
                    unordered_set<int> set;
                    set.insert(stoi(temp));
                    set.insert(stoi(temp2));
                    f[counter]->symmetric_set.push_back(set);
                }
                int tmpXIndex = -1;
                std::istringstream(temp) >> tmpXIndex;
                x[tmpXIndex]->setSymmOutput(counter);
                std::istringstream(temp2) >> tmpXIndex;
                x[tmpXIndex]->setSymmOutput(counter);
            }
            else {
                counter++;
            }
        }
        else {
            if (temp != "end") {
                fSymmetric >> temp2;
                bool has_been_inserted = false;
                for (int i = 0, n = g[counter - outputNum_ckt1]->symmetric_set.size(); i < n; i++) {
                    if (g[counter - outputNum_ckt1]->symmetric_set[i].count(stoi(temp))) {
                        has_been_inserted = true;
                        if (!g[counter - outputNum_ckt1]->symmetric_set[i].count(stoi(temp2))) {
                            g[counter - outputNum_ckt1]->symmetric_set[i].insert(stoi(temp2));
                        }
                    }
                }
                if (!has_been_inserted){
                    unordered_set<int> set;
                    set.insert(stoi(temp));
                    set.insert(stoi(temp2));
                    g[counter - outputNum_ckt1]->symmetric_set.push_back(set);
                }
                int tmpXIndex = -1;
                std::istringstream(temp) >> tmpXIndex;
                y[tmpXIndex]->setSymmOutput(counter - outputNum_ckt1);
                std::istringstream(temp2) >> tmpXIndex;
                y[tmpXIndex]->setSymmOutput(counter - outputNum_ckt1);
            }
            else {
                counter++;
            }
        }
    }
    cout << "x = " << endl;
    // size_t symmGroups_counter = -1;
    // symmOutputs* symm0 = new symmOutputs(outputNum_ckt1, 0); // used to check if a symmOutputs is 0 or not
    vector<symmOutputs*> tmp;
    // vector<variable*> symmGroup;
    for(size_t i = 0, n = x.size(); i < n; ++i){
        tmp.push_back(x[i]->symmOutput());
    }
    sort(tmp.begin(), tmp.end(), _increasing);
    // symmGroup.push_back(x[tmp[0]->index()]);
    // x[tmp[0]->index()]->setSymmGroupIndex(symmGroups_counter);
    // for(size_t i = 1, n = tmp.size(); i < n; ++i){
    //     if(tmp[i]->isEqual(symmGroup[0]->symmOutput()) == false){
    //         if(symm0->isEqual(symmGroup[0]->symmOutput()) == false && symmGroup.size() != 1){
    //             ++symmGroups_counter;
    //             for(size_t j = 0, nj = symmGroup.size(); j < nj; ++j){
    //                 symmGroup[j]->setIsInSymmGroup(1);
    //                 symmGroup[j]->setSymmGroupIndex(symmGroups_counter);
    //             }
    //                 // x[tmp[i]->index()]->setSymmGroupIndex(symmGroups_counter);
    //             symmGroups_x.push_back(symmGroup);
    //         }
    //         else{
    //             for(size_t j = 0, nj = symmGroup.size(); j < nj; ++j)
    //                 symmGroup[j]->setSymmGroupIndex(-1);
    //             // symmGroup[0]->setSymmGroupIndex(-1);
    //         }
    //         symmGroup.clear();
    //     }
    //     symmGroup.push_back(x[tmp[i]->index()]);
    // }
    for(auto i : tmp){
        for(auto j = i->getSymmOutput().rbegin(), nj = i->getSymmOutput().rend(); j != nj; ++j)
            cout << setw(3) <<i->index() << " : "<<std::bitset<64>((*j)) << " | ";
        cout << " end" << endl;
    }
    _symmGrouping(1, tmp);
    for(auto i : symmGroups_x){
        for(auto j : i){
            for(auto k = j->symmOutput()->getSymmOutput().rbegin(), nk = j->symmOutput()->getSymmOutput().rend(); k != nk; ++k)
                cout << setw(3) << j->getSub1() - 1 << " : "<<std::bitset<64>((*k)) << " | ";
            cout << " " << (j->isInSymmGroup() ? "true" : "false" )<<" "<<j->symmGroupIndex() <<" end" << endl;
        }
        cout << "\n ------- end Group -------- \n";
    }

    // symmGroups_counter = 0;
    // delete symm0;
    // symm0 = new symmOutputs(outputNum_ckt2, 0);
    tmp.clear();
    for(size_t i = 0, n = y.size(); i < n; ++i){
        tmp.push_back(y[i]->symmOutput());
    }
    sort(tmp.begin(), tmp.end(), _increasing);
    sort(tmp.begin(), tmp.end(), _increasing); // don't know why have to run ::sort twice to get the correct answer
    _symmGrouping(0, tmp);
    cout << "\ny = " << endl;
    for(auto i : symmGroups_y){
        for(auto j : i){
            for(auto k = j->symmOutput()->getSymmOutput().rbegin(), nk = j->symmOutput()->getSymmOutput().rend(); k != nk; ++k)
                cout << setw(3) << j->getSub1() - 1 << " : "<<std::bitset<64>((*k)) << " | ";
            cout << " " << (j->isInSymmGroup() ? "true" : "false" )<<" "<<j->symmGroupIndex() <<" end" << endl;
        }
        cout << "\n ------- end Group -------- \n";
    }
    for(auto i : tmp){
        for(int j = i->arrayLength() - 1; j >= 0; --j)
        // for(auto j = i->getSymmOutput().rbegin(), nj = i->getSymmOutput().rend(); j != nj; ++j)
            cout << i->index() << " : "<<std::bitset<64>(i->getSymmOutput()[j]) << " | ";
        cout << " end" << endl;
    }
    tmp.clear();
    // for(auto i : y){
    //     for(int j = i->symmOutput()->arrayLength() - 1, nj = 0; j >= nj; --j){
    //         cout << std::bitset<64>(i->symmOutput()->getSymmOutput()[j]) << " | ";
    //     }
    //     cout << " end" << endl;
    // }
    return;
}

void CirMgr::outputGrouping(){  // |f| = |g|
    // cout << "bus : \n";
    // for(auto j : bus_ckt1_input){

    //     for(auto i : j->names){
    //     cout << i << ' ';

    //     }
    // cout << endl;
    // }
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
        for(vector<variable *>::iterator j = f_groups[i].begin(); j != f_groups[i].end(); j++)
            (*j)->setOutputGroupingNum(i);
    for(size_t i = 0, n = g_groups.size(); i < n; ++i)
        for(vector<variable *>::iterator j = g_groups[i].begin(); j != g_groups[i].end(); j++)
            (*j)->setOutputGroupingNum(i);
    // exit(0);
}
bool CirMgr::_outputsorting(variable* a, variable* b)
{
    return a->suppSize() < b->suppSize();
}

void CirMgr::_busOutputUnateness(bool _isCkt1){
    vector<variable*>& outputVariable = _isCkt1 ? f : g;
    vector<Bus*>& bus_output = _isCkt1 ? bus_ckt1_output : bus_ckt2_output;
    size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
    for(size_t i = 0, ni = bus_output.size(); i < ni; ++i){
        cout << "names = ";
        for(vector<std::string>::iterator j = bus_output[i]->names.begin(); j != bus_output[i]->names.end(); j++)
            cout << (*j) << ' ';
        cout << endl;
        for(size_t j = 0, nj = bus_output[i]->indexes.size(); j < nj; ++j)
            bus_output[i]->addOutputUnatenessNum(outputVariable[bus_output[i]->indexes[j]]->outputUnateNum());

        // for(unordered_map<variable*, bool>::iterator it = bus_output[i]->unionSupport().begin(), _end = bus_output[i]->unionSupport().end(); it != _end; ++it)
        //         bus_output[i]->addOutputUnatenessNum((*it).first->inputUnateNum_p() + (*it).first->inputUnateNum_n());
        cout << "outputUnatenessnum = "<< (bus_output[i]->outputUnatenessNum())<< endl;
    }
    return;
}
void CirMgr::_busInputUnateness(bool _isCkt1){
    vector<variable*>& inputVariable = _isCkt1 ? x : y;
    vector<Bus*>& bus_input = _isCkt1 ? bus_ckt1_input : bus_ckt2_input;
    size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
    for(size_t i = 0, ni = bus_input.size(); i < ni; ++i){
        cout << "names = ";
        for(vector<std::string>::iterator j = bus_input[i]->names.begin(); j != bus_input[i]->names.end(); j++)
            cout << (*j) << ' ';
        cout << endl;
        for(size_t j = 0, nj = bus_input[i]->indexes.size(); j < nj; ++j){
            size_t unionInputUnateness = inputVariable[bus_input[i]->indexes[j]]->inputUnateNum_n() + inputVariable[bus_input[i]->indexes[j]]->inputUnateNum_p();
            bus_input[i]->addInputUnatenessNum(unionInputUnateness);
        }

        // for(unordered_map<variable*, bool>::iterator it = bus_input[i]->unionSupport().begin(), _end = bus_input[i]->unionSupport().end(); it != _end; ++it)
        //         bus_input[i]->addOutputUnatenessNum((*it).first->inputUnateNum_p() + (*it).first->inputUnateNum_n());
        cout << "inputUnatenessnum = "<< (bus_input[i]->inputUnatenessNum())<< endl;
    }
    return;
}

void CirMgr::_busSupportUnion(bool _isCkt1){
    vector<Bus*>& bus_output = _isCkt1 ? bus_ckt1_output : bus_ckt2_output;
    size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
    for(size_t i = 0, ni = bus_output.size(); i < ni; ++i){
        // int cnt = 0;
        // cout << "names = ";
        for(size_t j = 0, nj = bus_output[i]->indexes.size(); j < nj; ++j)
        {
            cout << bus_output[i]->names[j] << ' ';
            vector<variable*>& vecVar  = _isCkt1 ? f : g;
            vector<variable*>& functionalSupport = vecVar[bus_output[i]->indexes[j]]->_funcSupp;
            for(size_t k = 0, nk = functionalSupport.size(); k < nk; ++k)
                bus_output[i]->unionSupport().insert(pair<variable*, bool>(functionalSupport[k], 1));
        }
        cout << endl;
        // cout << "cnt = " << cnt << endl;
        // cout << "unionSupport = ";
        // for(auto mm : bus_output[i]->unionSupport())
        //     cout << mm.second << ' ';
        // cout << bus_output[i]->unionSupport().size() << endl;
        bus_output[i]->setUnionSupportSize(bus_output[i]->unionSupport().size());
        // cout << "size = " <<bus_output[i]->unionSupportSize() << endl;
        // cout << endl;
    }

}
void CirMgr::_busInputSupportUnion(bool _isCkt1){
    vector<Bus*>& bus_input = _isCkt1 ? bus_ckt1_input : bus_ckt2_input;
    size_t outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    size_t inputNum  = _isCkt1 ? inputNum_ckt1 : inputNum_ckt2;
    for(size_t i = 0, ni = bus_input.size(); i < ni; ++i){
        int cnt = 0;
        cout << "names = ";
        for(size_t j = 0, nj = bus_input[i]->indexes.size(); j < nj; ++j)
        {
            cout << bus_input[i]->names[j] << ' ';
            vector<variable*>& vecVar  = _isCkt1 ? x : y;
            vector<variable*>& functionalSupport = vecVar[bus_input[i]->indexes[j]]->_funcSupp_PI;
            for(size_t k = 0, nk = functionalSupport.size(); k < nk; ++k)
                bus_input[i]->unionInputSupport().insert(pair<variable*, bool>(functionalSupport[k], 1));
        }
        cout << endl;
        cout << "cnt = " << cnt << endl;
        cout << "unionInputSupport = ";
        for(unordered_map<variable *, bool>::iterator mm = bus_input[i]->unionInputSupport().begin(); mm != bus_input[i]->unionInputSupport().end(); mm++)
            cout << (*mm).first->getname() << ' ';
        // cout << bus_input[i]->unionInputSupport().size() << endl;
        bus_input[i]->setUnionInputSupportSize(bus_input[i]->unionInputSupport().size());
        cout << "size = " <<bus_input[i]->unionInputSupportSize() << endl;
        cout << endl;
    }
    return;
}

void CirMgr::_symmGrouping(bool _isCkt1, vector<symmOutputs*>& sortedSymmInputs){
    size_t symmGroups_counter = -1, outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    symmOutputs* symm0 = new symmOutputs(outputNum, 0); // used to check if a symmOutputs is 0 or not
    vector<variable*> symmGroup, &inputs = _isCkt1 ? x : y;
    vector<vector<variable*>> &symmGroups = _isCkt1 ? symmGroups_x : symmGroups_y;
    symmGroup.push_back(inputs[sortedSymmInputs[0]->index()]);
    inputs[sortedSymmInputs[0]->index()]->setSymmGroupIndex(symmGroups_counter);
    for(size_t i = 1, n = sortedSymmInputs.size(); i < n; ++i){
        if(sortedSymmInputs[i]->isEqual(symmGroup[0]->symmOutput()) == false){
            if(symm0->isEqual(symmGroup[0]->symmOutput()) == false && symmGroup.size() != 1){
                ++symmGroups_counter;
                for(size_t j = 0, nj = symmGroup.size(); j < nj; ++j){
                    symmGroup[j]->setIsInSymmGroup(1);
                    symmGroup[j]->setSymmGroupIndex(symmGroups_counter);
                }
                    // inputs[sortedSymmInputs[i]->index()]->setSymmGroupIndex(symmGroups_counter);
                symmGroups.push_back(symmGroup);
            }
            else{
                for(size_t j = 0, nj = symmGroup.size(); j < nj; ++j)
                    symmGroup[j]->setSymmGroupIndex(-1);
                // symmGroup[0]->setSymmGroupIndex(-1);
            }
            symmGroup.clear();
        }
        symmGroup.push_back(inputs[sortedSymmInputs[i]->index()]);
    }
    symmGroup.clear();
    delete symm0;
}
void CirMgr::feasibleBusMatching(){
    valid_busMatch_ckt2_input = permute(bus_ckt2_input);
    valid_busMatch_ckt2_output = permute(bus_ckt2_output);

    cout << "Vi size: " << valid_busMatch_ckt2_input.size() << endl;
    cout << "Vo size: " << valid_busMatch_ckt2_output.size() << endl;
    
    return;
}
void CirMgr::supportBusClassification(){
    for(size_t i = 0, ni = f.size(); i < ni; ++i){
        vector<variable*>& functionalSupport = f[i]->_funcSupp;
        for(size_t j = 0, nj = functionalSupport.size(); j < nj; ++j){
            if(functionalSupport[j]->busIndex() != -1){
                if(f[i]->suppBus().insert(pair<size_t, bool>(functionalSupport[j]->busIndex(), 1)).second == false)
                    ++f[i]->suppBus()[functionalSupport[j]->busIndex()];    // if not exist -> insert a new element; else, increment by one

            }
        }
        f[i]->suppBus_distribution() = new vector<pair<size_t, size_t>>(f[i]->suppBus().begin(), f[i]->suppBus().end());
        std::sort(f[i]->suppBus_distribution()->begin(), f[i]->suppBus_distribution()->end(), [](pair<size_t, size_t> a, pair<size_t, size_t> b){ 
            return a.second > b.second;});  // sort in non-increasing order
        // for(auto k : *(f[i]->suppBus_distribution()))
        //     cout<< "( " << k.first << " : " << k.second << " )";
        // cout << endl;
    }
    
    for(size_t i = 0, ni = g.size(); i < ni; ++i){
        vector<variable*>& functionalSupport = g[i]->_funcSupp;
        for(size_t j = 0, nj = functionalSupport.size(); j < nj; ++j){
            if(functionalSupport[j]->busIndex() != -1){
                if(g[i]->suppBus().insert(pair<size_t, bool>(functionalSupport[j]->busIndex(), 1)).second == false)
                    ++g[i]->suppBus()[functionalSupport[j]->busIndex()];    // if not exist -> insert a new element; else, increment by one

            }
        }
        g[i]->suppBus_distribution() = new vector<pair<size_t, size_t>>(g[i]->suppBus().begin(), g[i]->suppBus().end());
        std::sort(g[i]->suppBus_distribution()->begin(), g[i]->suppBus_distribution()->end(), [](pair<size_t, size_t> a, pair<size_t, size_t> b){ 
            return a.second > b.second;});  // sort in non-increasing order
        // for(auto k : *(g[i]->suppBus_distribution()))
        //     cout<< "( " << k.first << " : " << k.second << " )";
        // cout << endl;
    }
    return;
} 
void
CirMgr::
readBus_class(SatSolver& s, string& inputfilename){
    ifstream fbus(inputfilename);
    string port;
    size_t portidx = 0;
    int num_bus = 0, num_port = 0;
    
    fbus >> port >> num_bus;
    for(size_t i = 0; i < num_bus; i++){
        fbus >> num_port;
        Bus* newBus = new Bus(portidx++, num_bus);
        for(size_t j = 0; j < num_port; j++){
            fbus >> port;
            int busidx = 0;
            // cout << u_name_index_ckt1[port] << endl;
            if(u_name_isInput_ckt1[port]){
                exist_x[u_name_index_ckt1[port]] = true;
                u_name_busIndex_input_ckt1[port] = bus_ckt1_input.size();
                busidx = bus_ckt1_input.size();
                // cout << "ex x name: " << port << " idx: " << u_name_index_ckt1[port] << endl;
            }
            else{
                exist_f[u_name_index_ckt1[port]] = true;
                u_name_busIndex_output_ckt1[port] = bus_ckt1_output.size();
                busidx = bus_ckt1_output.size();
                // cout << "ex f name: " << port << " idx: " << u_name_index_ckt1[port] << endl;
            }
            newBus->setIsInput(u_name_isInput_ckt1[port]);
            newBus->setIsCkt1(true);
            newBus->setPortNum(num_port);
            newBus->insertIndex(u_name_index_ckt1[port]);
            // if(u_name_index_ckt1[port] > 1000 || u_name_index_ckt1[port] <= 0)
                cout << "idx: " << busidx << " " << newBus->getIsInput() << " u_name_index_ckt1[port] = " << u_name_index_ckt1[port] << endl;
            newBus->insertName(port);
            newBus->getIsInput() ? x[u_name_index_ckt1[port]]->setBusIndex(portidx - 1) : f[u_name_index_ckt1[port]]->setBusIndex(portidx - 1);
        }
        if(newBus->getIsInput()) bus_ckt1_input.push_back(newBus);
        else                     bus_ckt1_output.push_back(newBus);
    }
    fbus >> port >> num_bus;
    for(size_t i = 0; i < num_bus; i++){
        fbus >> num_port;
        Bus* newBus = new Bus(portidx++, num_bus);
        for(size_t j = 0; j < num_port; j++){
            fbus >> port;
            int busidx = 0;
            if(u_name_isInput_ckt2[port]){
                exist_y[u_name_index_ckt2[port]] = true;
                u_name_busIndex_input_ckt2[port] = bus_ckt2_input.size();
                busidx = bus_ckt2_input.size();
                // cout << "ex y name: " << port << " idx: " << u_name_index_ckt2[port] << endl;
            }
            else{
                exist_g[u_name_index_ckt2[port]] = true;
                u_name_busIndex_output_ckt2[port] = bus_ckt2_output.size();
                busidx = bus_ckt2_output.size();
                // cout << "ex g name: " << port << " idx: " << u_name_index_ckt2[port] << endl;
            }
            newBus->setIsInput(u_name_isInput_ckt2[port]);
            newBus->setIsCkt1(false);
            newBus->setPortNum(num_port);
            // if(u_name_index_ckt2[port] > 1000 || u_name_index_ckt2[port] <= 0 )
                cout << "idx: " << busidx << " " << newBus->getIsInput() << " u_name_index_ckt2[port] = "<< u_name_index_ckt2[port] << endl;
            newBus->insertIndex(u_name_index_ckt2[port]);
            newBus->insertName(port);
            newBus->getIsInput() ? y[u_name_index_ckt2[port]]->setBusIndex(portidx - 1) : g[u_name_index_ckt2[port]]->setBusIndex(portidx - 1);
        }
        if(newBus->getIsInput()) bus_ckt2_input.push_back(newBus);
        else                     bus_ckt2_output.push_back(newBus);
    }
    // cout << "portidx for bus_ckt1_input\n";
    // for(auto i : bus_ckt1_input)
    //     cout << i->busIndex() << ' ';
    // cout << endl;
    // cout << "busidx for x\n";
    // for(auto i : x)
    //     cout << i->busIndex() << ' ';
    // cout << endl;
    // cout << "portidx for bus_ckt1_output\n";
    // for(auto i : bus_ckt1_output)
    //     cout << i->busIndex() << ' ';
    // cout << endl;
    // cout << "busidx for f\n";
    // for(auto i : f)
    //     cout << i->busIndex() << ' ';
    // cout << endl;
    // cout << "portidx for bus_ckt2_input\n";
    // for(auto i : bus_ckt2_input)
    //     cout << i->busIndex() << ' ';
    // cout << endl;
    // cout << "busidx for y\n";
    // for(auto i : y)
    //     cout << i->busIndex() << ' ';
    // cout << endl;
    // cout << "portidx for bus_ckt2_output\n";
    // for(auto i : bus_ckt2_output)
    //     cout << i->busIndex() << ' ';
    // cout << endl;
    // cout << "busidx for g\n";
    // for(auto i : g)
    //     cout << i->busIndex() << ' ';
    // cout << endl;

    /* don't have to sort after using addCandidateBusConstraint
    sort(bus_ckt1_input.begin(), bus_ckt1_input.end(), [](Bus* a, Bus* b){return a->getPortNum() > b->getPortNum();});
    sort(bus_ckt1_output.begin(), bus_ckt1_output.end(), [](Bus* a, Bus* b){return a->getPortNum() > b->getPortNum();});
    sort(bus_ckt2_input.begin(), bus_ckt2_input.end(), [](Bus* a, Bus* b){return a->getPortNum() > b->getPortNum();});
    sort(bus_ckt2_output.begin(), bus_ckt2_output.end(), [](Bus* a, Bus* b){return a->getPortNum() > b->getPortNum();});
    */
    // cout << "BS: " << bus_ckt1_input.size() << " " << bus_ckt2_input.size() << " " << bus_ckt1_output.size() << " " << bus_ckt2_output.size() << endl;
    // for(int i = 0; i < 1; i++) Var t = s.newVar(); // test var
    
    // construct MIbus_Var and MIbus_valid
    // (!MIbus_Var[][] + !MI_valid_var[][] (invalid matching)) clauses
    for (int i = 0; i < bus_ckt1_input.size(); i++){
        vector<Var> tmp;
        vector<bool> tmp_b;
        for (int j = 0; j < bus_ckt2_input.size(); j++){
            Var v = s.newVar();
            tmp.push_back(v);
            tmp_b.push_back(true);
            // cout << "902" << endl;
            for(int i1 = 0; i1 < bus_ckt1_input[i]->indexes.size(); i1++){
                bool* exist = new bool[y.size()]{0};
                // cout << "905" << endl;
                for(int yidx = 0; yidx < bus_ckt2_input[j]->indexes.size(); yidx++) exist[bus_ckt2_input[j]->indexes[yidx]] = true;
                // cout << "907" << endl;
                for(int yidx = 0; yidx < y.size(); yidx++){
                    // cout << "909" << endl;
                    if(!exist[yidx]){
                        // cout << "911" << endl;
                        vec<Lit> invalid_match;
                        invalid_match.push(~Lit(v));
                        invalid_match.push(~Lit(MI_valid_Var[bus_ckt1_input[i]->indexes[i1]][yidx]));
                        s.addClause(invalid_match);
                        invalid_match.clear();
                    }
                }
                delete[] exist;
            }
        }
        MIbus_Var.push_back(tmp);
        MIbus_valid.push_back(tmp_b);
    }
    // construct MObus_Var and MObus_valid
    for (int i = 0; i < bus_ckt1_output.size(); i++){
        vector<Var> tmp;
        vector<bool> tmp_b;
        for (int j = 0; j < bus_ckt2_output.size(); j++){
            Var v = s.newVar();
            tmp.push_back(v);
            tmp_b.push_back(true);
            // cout << "929" << endl;
            for(int i1 = 0; i1 < bus_ckt1_output[i]->indexes.size(); i1++){
                bool* exist = new bool[g.size()]{0};
                for(int gidx = 0; gidx < bus_ckt2_output[j]->indexes.size(); gidx++) exist[bus_ckt2_output[j]->indexes[gidx]] = true;
                for(int gidx = 0; gidx < g.size(); gidx++){
                    if(!exist[gidx]){
                        vec<Lit> invalid_match;
                        invalid_match.push(~Lit(v));
                        invalid_match.push(~Lit(MI_valid_Var[bus_ckt1_output[i]->indexes[i1]][gidx]));
                        s.addClause(invalid_match);
                        invalid_match.clear();
                    }
                }
                delete[] exist;
            }
        }
        MObus_Var.push_back(tmp);
        MObus_valid.push_back(tmp_b);
    }







    cout << "test MIbus_valid" << endl;
    for(int i = 0; i < MIbus_valid.size(); i++){
        for(int j = 0; j < MIbus_valid[0].size(); j++){
            cout << MIbus_valid[i][j] << " ";
        }
        cout << endl;
    }
    cout << "test MObus_valid" << endl;
    for(int i = 0; i < MObus_valid.size(); i++){
        for(int j = 0; j < MObus_valid[0].size(); j++){
            cout << MObus_valid[i][j] << " ";
        }
        cout << endl;
    }




    return;
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