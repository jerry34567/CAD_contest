#include "cirMgr.h"
#include "sat.h"
#include <fstream>
#include <string>
#include <algorithm>
#include <bitset>
#include <sstream>
#include <iomanip>

bool variable::checkSymmSign(map<size_t, size_t>& _ss, size_t _p, size_t _q){   //_ss is the symmSign of y[l], _p is fp, _q is gq
    bool tmp1 = _ss.find(_q) == _ss.end(), tmp2 = _symmSign.find(_p) == _symmSign.end();
    if(tmp1 == 1 && tmp2 == 1)  // both not symmetric to this output respectively
        return 1;
    else if((tmp1 == 0 && tmp2 == 1) || (tmp1 == 1 && tmp2 == 0))
        return 0;
    return (_ss.find(_q)->second == _symmSign.find(_p)->second);
}

void variable::inputSymmGroupClassification(size_t _input1, size_t _input2){
    if(_inputSymmGroup.find(_input1) != _inputSymmGroup.end())
        _inputSymmGroup.insert(pair<size_t, size_t>(_input2, _inputSymmGroup[_input1]));
    else if( _inputSymmGroup.find(_input2) != _inputSymmGroup.end())
        _inputSymmGroup.insert(pair<size_t, size_t>(_input1, _inputSymmGroup[_input2]));
    else{
        _inputSymmGroup.insert(pair<size_t, size_t>(_input2, _maxInputSymmGroupIndex));
        _inputSymmGroup.insert(pair<size_t, size_t>(_input1, _maxInputSymmGroupIndex++));
    }
}

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
CirMgr::readCNF(SatSolver& s_miter, SatSolver& s_cir1, SatSolver& s_cir2, SatSolver& s_verifier) {
    // read cnf file: top1.cnf, top2.cnf
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
            vec<Lit> lits, lits1, lits_veri;

            vector<Var> test; // test addClause

            while (fcnf1 >> litTmp) {

                // cout << "LL: " << litTmp << endl;
                if (litTmp != 0) {
                    if (umapNum_ckt1.find(abs(litTmp)) ==
                        umapNum_ckt1.end()) { // doesn't exist
                        umapNum_ckt1[abs(litTmp)] =
                            "n" + to_string(abs(litTmp));
                        Var nV                         = s_miter.newVar();
                        Var nV1                        = s_cir1.newVar();
                        Var nV_veri                    = s_verifier.newVar();
                        umapInterVar_ckt1[abs(litTmp)] = nV;
                        mapInterVar_cir1[abs(litTmp)] = nV1;
                        umapInterVar_ckt1_veri[abs(litTmp)] = nV_veri;
                        Lit li, li1, li_veri;
                        if (litTmp > 0) {
                            li      = Lit(nV);
                            li1     = Lit(nV1);
                            li_veri = Lit(nV_veri);
                        } else {
                            li      = ~Lit(nV);
                            li1     = ~Lit(nV1);
                            li_veri = ~Lit(nV_veri);
                        }

                        lits.push(li);
                        lits1.push(li1);
                        lits_veri.push(li_veri);
                        test.push_back(nV);
                    } else {
                        string portNumTmp =
                            umapNum_ckt1[abs(litTmp)]; // i0, i1 ..
                        // cout << "PPP: " << portNumTmp << endl;
                        // add clause to s_miter according to i/o and
                        // num. ex: i0 --->  x[0]->getVar2()
                        if (portNumTmp[0] == 'i') { // those var in x
                            Lit li, li1, li_veri;
                            if (litTmp > 0) {
                                li = Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar2());
                                li1 = Lit(x[stoi(portNumTmp.substr(1))]->getVar4());
                                li_veri = Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar3());

                            } else {
                                li = ~Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar2());
                                li1 = ~Lit(x[stoi(portNumTmp.substr(1))]->getVar4());
                                li_veri = ~Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar3());
                            }
                            lits.push(li);
                            lits1.push(li1);
                            lits_veri.push(li_veri);
                            test.push_back(
                                x[stoi(portNumTmp.substr(1))]->getVar2());
                        } else if (portNumTmp[0] == 'o') { // o
                            Lit li, li1, li_veri;
                            if (litTmp > 0) {
                                li = Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar2());
                                li1 = Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar4());
                                li_veri = Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar3());

                            } else {
                                li = ~Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar2());
                                li1 = ~Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar4());
                                li_veri = ~Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar3());
                            }
                            lits.push(li);
                            lits1.push(li1);
                            lits_veri.push(li_veri);
                            test.push_back(
                                f[stoi(portNumTmp.substr(1))]->getVar2());
                        } else { // n (intervariable)
                            Lit li, li1, li_veri;
                            if (litTmp > 0) {
                                li = Lit(umapInterVar_ckt1[abs(litTmp)]);
                                li1 = Lit(mapInterVar_cir1[abs(litTmp)]);
                                li_veri =
                                    Lit(umapInterVar_ckt1_veri[abs(litTmp)]);

                            } else {
                                li = ~Lit(umapInterVar_ckt1[abs(litTmp)]);
                                li1 = ~Lit(mapInterVar_cir1[abs(litTmp)]);
                                li_veri =
                                    ~Lit(umapInterVar_ckt1_veri[abs(litTmp)]);
                            }

                            lits.push(li);
                            lits1.push(li1);
                            lits_veri.push(li_veri);
                            test.push_back(umapInterVar_ckt1[abs(litTmp)]);
                        }
                    }
                } else {
                    ++ct1;
                    // cout << lits_veri.size() << endl;
                    s_miter.addClause(lits);
                    s_cir1.addClause(lits1);
                    s_verifier.addClause(lits_veri);
                    lits.clear();
                    lits1.clear();
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
            vec<Lit> lits, lits2, lits_veri;
            while (fcnf2 >> litTmp) {

                // cout << "LL: " << litTmp << endl;
                if (litTmp != 0) {
                    if (umapNum_ckt2.find(abs(litTmp)) ==
                        umapNum_ckt2.end()) { // doesn't exist
                        umapNum_ckt2[abs(litTmp)] =
                            "n" + to_string(abs(litTmp));
                        Var nV                         = s_miter.newVar();
                        Var nV2                        = s_cir2.newVar();
                        Var nV_veri                    = s_verifier.newVar();
                        umapInterVar_ckt2[abs(litTmp)] = nV;
                        mapInterVar_cir2[abs(litTmp)] = nV2;
                        umapInterVar_ckt2_veri[abs(litTmp)] = nV_veri;
                        Lit li, li2, li_veri;
                        if (litTmp > 0) {
                            li      = Lit(nV);
                            li2     = Lit(nV2);
                            li_veri = Lit(nV_veri);
                        } else {
                            li      = ~Lit(nV);
                            li2     = ~Lit(nV2);
                            li_veri = ~Lit(nV_veri);
                        }

                        lits.push(li);
                        lits2.push(li2);
                        lits_veri.push(li_veri);
                    } else {
                        string portNumTmp =
                            umapNum_ckt2[abs(litTmp)]; // i0, i1 ..
                        // cout << "PPP: " << portNumTmp << endl;
                        // add clause to s_miter according to i/o and
                        // num. ex: i0 --->  x[0]->getVar2()
                        if (portNumTmp[0] == 'i') {
                            Lit li, li2, li_veri;
                            if (litTmp > 0) {
                                li = Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar2());
                                li2 = Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar4());
                                li_veri = Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar3());

                            } else {
                                li = ~Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar2());
                                li2 = ~Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar4());
                                li_veri = ~Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar3());
                            }

                            lits.push(li);
                            lits2.push(li2);
                            lits_veri.push(li_veri);
                        } else if (portNumTmp[0] == 'o') { // o
                            Lit li, li2, li_veri;
                            if (litTmp > 0) {
                                li = Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar2());
                                li2 = Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar4());
                                li_veri = Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar3());

                            } else {
                                li = ~Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar2());
                                li2 = ~Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar4());
                                li_veri = ~Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar3());
                            }

                            lits.push(li);
                            lits2.push(li2);
                            lits_veri.push(li_veri);
                        } else { // n (intervariable)
                            Lit li, li2, li_veri;
                            if (litTmp > 0) {
                                li = Lit(umapInterVar_ckt2[abs(litTmp)]);
                                li2 = Lit(mapInterVar_cir2[abs(litTmp)]);
                                li_veri =
                                    Lit(umapInterVar_ckt2_veri[abs(litTmp)]);

                            } else {
                                li = ~Lit(umapInterVar_ckt2[abs(litTmp)]);
                                li2 = ~Lit(mapInterVar_cir2[abs(litTmp)]);
                                li_veri =
                                    ~Lit(umapInterVar_ckt2_veri[abs(litTmp)]);
                            }

                            lits.push(li);
                            lits2.push(li2);
                            lits_veri.push(li_veri);
                        }
                    }
                } else {
                    ++ct2;
                    s_miter.addClause(lits);
                    s_cir2.addClause(lits2);
                    s_verifier.addClause(lits_veri);
                    lits.clear();
                    lits2.clear();
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
        for(size_t j = 0, nj = resultingOutputs.length(); j < nj; ++j){
            if(resultingOutputs[j] == '.'){
                outputVariables[i]->addOutputBinateNum();
            }
        }
            // if(resultingOutputs[j] == 'p')
            // {
            //     vector<variable*>& vecVar  = _isCkt1 ? x : y;
            //     vecVar[j]->addInputUnateNum_p();
            // }
            // else if(resultingOutputs[j] == 'n')
            // {
            //     vector<variable*>& vecVar  = _isCkt1 ? x : y;
            //     vecVar[j]->addInputUnateNum_n();
            // }
    }
    // return;
    for(auto i : outputVariables)
        cout << "( "<<i->outputBinateNum()  << " ) ; ";
    cout << endl;
    return;
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
                int tmpXIndex1 = -1, tmpXIndex2 = -1;
                std::istringstream(temp) >> tmpXIndex1;
                x[tmpXIndex1]->setSymmOutput(counter);
                f[counter]->setSymmInput(tmpXIndex1);
                std::istringstream(temp2) >> tmpXIndex2;
                x[tmpXIndex2]->setSymmOutput(counter);
                f[counter]->setSymmInput(tmpXIndex2);
                f[counter]->inputSymmGroupClassification(tmpXIndex1, tmpXIndex2);
                x[tmpXIndex1]->setInputSymmGroupIndex(f[counter]->inputSymmGroup()[tmpXIndex1]);
                x[tmpXIndex2]->setInputSymmGroupIndex(f[counter]->inputSymmGroup()[tmpXIndex2]);
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
                int tmpXIndex1 = -1, tmpXIndex2 = -1;
                std::istringstream(temp) >> tmpXIndex1;
                y[tmpXIndex1]->setSymmOutput(counter - outputNum_ckt1);
                g[counter - outputNum_ckt1]->setSymmInput(tmpXIndex1);
                std::istringstream(temp2) >> tmpXIndex2;
                y[tmpXIndex2]->setSymmOutput(counter - outputNum_ckt1);
                g[counter - outputNum_ckt1]->setSymmInput(tmpXIndex2);
                g[counter - outputNum_ckt1]->inputSymmGroupClassification(tmpXIndex1, tmpXIndex2);
                y[tmpXIndex1]->setInputSymmGroupIndex(g[counter - outputNum_ckt1]->inputSymmGroup()[tmpXIndex1]);
                y[tmpXIndex2]->setInputSymmGroupIndex(g[counter - outputNum_ckt1]->inputSymmGroup()[tmpXIndex2]);
            }
            else {
                counter++;
            }
        }
    }
    // cout << "inputSymmGroup = " << endl;
    // for(auto i : g[63]->inputSymmGroup())
    //     cout << i.first << ' ' << i.second << endl;
    for(size_t i = 0, ni = f.size(); i < ni; ++i){
        f[i]->inputSymmGroupSize().reserve(f[i]->maxInputSymmGroupIndex());
        for(size_t j = 0, nj = f[i]->maxInputSymmGroupIndex(); j < nj; ++j)
            f[i]->inputSymmGroupSize().push_back(0);
        for(map<size_t, size_t>::iterator it = f[i]->inputSymmGroup().begin(), _end = f[i]->inputSymmGroup().end(); it != _end; ++it){
            ++f[i]->inputSymmGroupSize()[(*it).second];
}
    }
    for(size_t i = 0, ni = g.size(); i < ni; ++i){
        g[i]->inputSymmGroupSize().reserve(g[i]->maxInputSymmGroupIndex());
        for(size_t j = 0, nj = g[i]->maxInputSymmGroupIndex(); j < nj; ++j)
            g[i]->inputSymmGroupSize().push_back(0);
        for(map<size_t, size_t>::iterator it = g[i]->inputSymmGroup().begin(), _end = g[i]->inputSymmGroup().end(); it != _end; ++it){
            ++g[i]->inputSymmGroupSize()[(*it).second];
        }
    }
    // cout << "size = " << endl;
    // for(auto i : g[63]->inputSymmGroupSize())
    //     cout << i << endl;
    cout << "x = " << endl;
    // size_t symmGroups_counter = -1;
    // symmObj* symm0 = new symmObj(outputNum_ckt1, 0); // used to check if a symmObj is 0 or not
    vector<symmObj*> tmp;
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
    _symmGrouping(1, tmp);

    // symmGroups_counter = 0;
    // delete symm0;
    // symm0 = new symmObj(outputNum_ckt2, 0);
    tmp.clear();
    for(size_t i = 0, n = y.size(); i < n; ++i){
        tmp.push_back(y[i]->symmOutput());
    }
    sort(tmp.begin(), tmp.end(), _increasing);
    // sort(tmp.begin(), tmp.end(), _increasing); // don't know why have to run ::sort twice to get the correct answer
    _symmGrouping(0, tmp);
    tmp.clear();
    for(size_t i = 0, ni = f.size(); i < ni; ++i){
        f[i]->symmInput()->countNumberOfInputs();
        // cout << setw(3) <<i << " : "<<g[i]->symmInput()->numOfInputs() << endl;
    }
    for(size_t i = 0, ni = g.size(); i < ni; ++i){
        g[i]->symmInput()->countNumberOfInputs();
        // cout << setw(3) <<i << " : "<<g[i]->symmInput()->numOfInputs() << endl;
    }
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
void CirMgr::symmSign(){
    _symmSign(1);
    _symmSign(0);
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

void CirMgr::_symmGrouping(bool _isCkt1, vector<symmObj*>& sortedSymmInputs){
    size_t symmGroups_counter = -1, outputNum = _isCkt1 ? outputNum_ckt1 : outputNum_ckt2;
    symmObj* symm0 = new symmObj(outputNum, 0); // used to check if a symmObj is 0 or not
    vector<variable*> symmGroup, &inputs = _isCkt1 ? x : y;
    vector<vector<variable*> > &symmGroups = _isCkt1 ? symmGroups_x : symmGroups_y;
    sortedSymmInputs.push_back(symm0);
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
    sortedSymmInputs.pop_back();
}
void CirMgr::_symmSign(bool _isCkt1){
    vector<variable*>& inputs = _isCkt1 ? x : y, &outputs = _isCkt1 ? f : g;
    for(size_t i = 0, ni = inputs.size(); i < ni; ++i){
        for(size_t j = 0, nj = inputs[i]->symmOutput()->getSymmOutput().size(); j < nj; ++j){
            unsigned long long symm_j = inputs[i]->symmOutput()->getSymmOutput()[j];
            size_t counter = j * sizeof(unsigned long long) * 8;
            // cout << bitset<64>(symm_j) << endl;
            while(symm_j){
                if(symm_j & 1ULL){
                    inputs[i]->symmSign().insert(pair<size_t, size_t>(counter, outputs[counter]->inputSymmGroupSize().at(outputs[counter]->inputSymmGroup().at(i))));
		        }
                symm_j = symm_j >> 1;
                ++counter;
	        }
        }
    }
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
                        invalid_match.push(~Lit(MO_valid_Var[bus_ckt1_output[i]->indexes[i1]][gidx]));
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

void CirMgr::sortSuppDiff(){  // initially read in MO_valid and sort Supp Difference for MO_suppdiff, sort MO_suppdiff_row
    // MO_suppdiff.reserve(MO_valid.size());
    for(int i = 0; i < MO_valid.size(); i++){
        vector<Supp_Difference> tmp;
        // tmp.reserve(MO_valid[0].size());
        for(int j = 0; j < MO_valid[0].size(); j++){
            if(!MO_valid[i][j]) tmp.push_back(Supp_Difference(i, j, -1, MO_valid_Var[i][j]));
            else tmp.push_back(Supp_Difference(i, j, (g[j]->_funcSupp.size()-f[i]->_funcSupp.size()), MO_valid_Var[i][j]));
        }
        MO_suppdiff.push_back(tmp);
    }
    
    for(int j = 0; j < MO_valid[0].size(); j++){
        vector<Supp_Difference> tmp; // store row for sorting
        for(int i = 0; i < MO_valid.size(); i++){
            tmp.push_back(MO_suppdiff[i][j]);
        }
        sort(tmp.begin(), tmp.end(), _suppdiff_increasing);
        for(int i = 0; i < MO_valid.size(); i++){
            MO_suppdiff[i][j] = tmp[i];
        }
    }

    for(int j = 0; j < MO_valid[0].size(); j++){
        Supp_Diff_Row newRow(j, MO_suppdiff[MO_suppdiff.size()-1][j].suppdiff);
        newRow.suppdiff_cnt_arr.reserve(newRow.max_diff+2);
        cout << "newRow.suppdiff_cnt_arr.capacity()" << newRow.suppdiff_cnt_arr.capacity() << endl;
        for(int k = 0; k < newRow.max_diff+2; k++){
            newRow.suppdiff_cnt_arr.push_back(0);
        }
        for(int i = 0; i < MO_valid.size(); i++){
            ++(newRow.suppdiff_cnt_arr[MO_suppdiff[i][j].suppdiff +1]);
        }
        MO_suppdiff_row.push_back(newRow);
    }

    sort(MO_suppdiff_row.begin(), MO_suppdiff_row.end(), _suppdiff_cnt_arr_decreasing);
    
    // MO_suppdiff_chosen_col_idxes.reserve(MO_valid[0].size());
    for(int i = 0; i < MO_valid[0].size(); i++){
        MO_suppdiff_chosen_col_idxes.push_back(MO_suppdiff_row[i].suppdiff_cnt_arr[0]);
    }
    MO_suppdiff_chosen_row = 0;
    
}

void CirMgr::outputHeuristicMatching(vec<Lit>& output_heuristic_assump){ 
    while(!_inside_outputHeuristicMatching(output_heuristic_assump)){
        updateOutputHeuristic_Fail();
    }
}
bool CirMgr::_inside_outputHeuristicMatching(vec<Lit>& output_heuristic_assump){ // output match according to MO_suppdiff_row[MO_suppdiff_chosen_row], MO_suppdiff -> output match -> support input port match (can't match outside)

    // output match according to MO_suppdiff_row[MO_suppdiff_chosen_row], MO_suppdiff -> output match -> support input port match (can't match outside)
    // cout << "outputHeuristicMatching" << endl;
    output_heuristic_assump.clear();
    cir1_func_supp_union.clear();
    cir2_func_supp_union.clear();
    cir1_not_func_supp_union.clear();
    cir2_not_func_supp_union.clear();

    for(int j = 0; j <= MO_suppdiff_chosen_row; j++){
        int ori_row_index = MO_suppdiff_row[j].original_row_index;
        int idx1 = MO_suppdiff[MO_suppdiff_chosen_col_idxes[j]][ori_row_index].original_idx1;
        int idx2 = MO_suppdiff[MO_suppdiff_chosen_col_idxes[j]][ori_row_index].original_idx2;
        // cout << "ori_row_index: " << ori_row_index << "  idx1: " << idx1 << "  idx2: " << idx2 << endl;

        bool* exist = new bool[MI_valid.size()];
        for(int i = 0; i < MI_valid.size(); i++) exist[i] = false;
        for(int i = 0; i < f[idx1]->_funcSupp.size(); i++){
            exist[u_name_index_ckt1[f[idx1]->_funcSupp[i]->getname()]] = true;
            cir1_func_supp_union.insert(f[idx1]->_funcSupp[i]);
        }
        exist[MI_valid.size()-1] = true; // enable constant

        for(int k = 0; k < g[idx2]->_funcSupp.size(); k++){
            cir2_func_supp_union.insert(g[idx2]->_funcSupp[k]);
            for(int i = 0; i < MI_valid.size(); i++){
                if(!exist[i]){
                    output_heuristic_assump.push(~Lit(MI_valid_Var[i][u_name_index_ckt2[g[idx2]->_funcSupp[k]->getname()]]));
                }
            }
        }
        delete[] exist;
        
        for(int i = 0; i < MO_valid.size(); i++){
            if(i == idx1){
                output_heuristic_assump.push(Lit(MO_valid_Var[i][idx2]));
                // cout << "T " << i << " " << idx2 << endl; 
            }
            else{
                output_heuristic_assump.push(~Lit(MO_valid_Var[i][idx2]));
                // cout << "F " << i << " " << idx2 << endl; 
            } 
        }
    }
    for(int j = MO_suppdiff_chosen_row+1; j < MO_valid[0].size(); j++){
        int ori_row_index = MO_suppdiff_row[j].original_row_index;
        for(int i = 0; i < MO_valid.size(); i++){
            output_heuristic_assump.push(~Lit(MO_valid_Var[i][ori_row_index]));
            // cout << "F " << i << " " << ori_row_index << endl;
        }
    }
    for(int i = 0; i < x.size(); i++){
        if(!cir1_func_supp_union.count(x[i])){
            cir1_not_func_supp_union.insert(x[i]);
        }
    }
    for(int i = 0; i < y.size(); i++){
        if(!cir2_func_supp_union.count(y[i])){
            cir2_not_func_supp_union.insert(y[i]);
        }
    }
    if(cir1_func_supp_union.size() > cir2_func_supp_union.size() || cir1_not_func_supp_union.size() > cir2_not_func_supp_union.size()) return false;
    
    for(set<variable*>::iterator it = cir2_not_func_supp_union.begin(); it != cir2_not_func_supp_union.end(); ++it){
        output_heuristic_assump.push(~Lit(MI[MI.size()-1][(*it)->getSub1()-1]->getVar()));
    }
    // for(set<variable*>::iterator it1 = cir1_not_func_supp_union.begin(), it2 = cir2_not_func_supp_union.begin(), n1 = cir1_not_func_supp_union.end(), n2 = cir2_not_func_supp_union.end(); ; ++it1, ++it2){
    //     if(it1 == n1){
    //         set<variable*>::iterator i0 = cir1_not_func_supp_union.begin();
    //         for(set<variable*>::iterator it3 = it2; it3 != n2; ++it3){
    //             output_heuristic_assump.push(Lit(MI_valid_Var[(*i0)->getSub1()-1][(*it3)->getSub1()-1]));
    //             output_heuristic_assump.push(Lit(MI[((*i0)->getSub1()-1)*2][(*it3)->getSub1()-1]->getVar()));
    //             output_heuristic_assump.push(~Lit(MI[((*i0)->getSub1()-1)*2+1][(*it3)->getSub1()-1]->getVar()));
    //         }
    //         break;
    //     }
    //     else{
    //         output_heuristic_assump.push(Lit(MI_valid_Var[(*it1)->getSub1()-1][(*it2)->getSub1()-1]));
    //         output_heuristic_assump.push(Lit(MI[((*it1)->getSub1()-1)*2][(*it2)->getSub1()-1]->getVar()));
    //         output_heuristic_assump.push(~Lit(MI[((*it1)->getSub1()-1)*2+1][(*it2)->getSub1()-1]->getVar()));
    //     }
    // }

    return true;
}

void CirMgr::updateOutputHeuristic_Success(){
    cout << "updateOutputHeuristic_Success row: " << MO_suppdiff_chosen_row << endl;
    MO_suppdiff_chosen_row++;
}

bool CirMgr::updateOutputHeuristic_Fail(){
    cout << "updateOutputHeuristic_Fail row: " << MO_suppdiff_chosen_row << " col: " << MO_suppdiff_chosen_col_idxes[MO_suppdiff_chosen_row] << endl;
    
    if(MO_suppdiff_chosen_col_idxes[MO_suppdiff_chosen_row] == MO_valid.size()-1){
        MO_suppdiff_chosen_col_idxes[MO_suppdiff_chosen_row] = MO_suppdiff_row[MO_suppdiff_chosen_row].suppdiff_cnt_arr[0];
        if(MO_suppdiff_chosen_row == 0) return false;
        else{
            throwToLastRow(MO_suppdiff_chosen_row);
            MO_suppdiff_chosen_row--;
            // MO_suppdiff_chosen_col_idxes[MO_suppdiff_chosen_row]++;
            if (updateOutputHeuristic_Fail()) return true;
            else return false;
        }
    }
    else{
        MO_suppdiff_chosen_col_idxes[MO_suppdiff_chosen_row]++;
    }
    return true;
    // MO_suppdiff_row[MO_suppdiff_chosen_row].original_row_index
}
void CirMgr::throwToLastRow(int row){
    int tmp = MO_suppdiff_chosen_col_idxes[row];
    MO_suppdiff_chosen_col_idxes.erase(MO_suppdiff_chosen_col_idxes.begin() + row);
    MO_suppdiff_chosen_col_idxes.push_back(tmp);

    Supp_Diff_Row t = MO_suppdiff_row[row];
    MO_suppdiff_row.erase(MO_suppdiff_row.begin() + row);
    MO_suppdiff_row.push_back(t);
}