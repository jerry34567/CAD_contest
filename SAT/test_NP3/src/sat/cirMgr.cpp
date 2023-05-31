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
    faag1_name = faag2_name = "./files/aag/";
    faag1_name += "case01_1.aag";
    faag2_name += "case01_2.aag";
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
}
void
CirMgr::readMAP() {

    ifstream fmap1, fmap2;
    string   fmap1_name = "", fmap2_name = "";
    fmap1_name = fmap2_name = "./files/map/";
    fmap1_name += "case01map1";
    fmap2_name += "case01map2";
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
    fcnf1_name = fcnf2_name = "./files/cnf/";
    fcnf1_name += "case01_1.cnf";
    fcnf2_name += "case01_2.cnf";
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