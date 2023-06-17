#include "sat.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

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
        void setName(const char& v) { _name = v; }
        void setSub1(const Var& v) { _sub1 = v; }
        void setSub2(const Var& v) { _sub2 = v; }
        void setVar(const Var& v) { _var = v; }
        void setVar2(const Var& v) { _var2 = v; }

    private:
        char _name;
        Var  _sub1;
        Var  _sub2;
        Var  _var;  // for solver1
        Var  _var2; // for miter solver
                    // Var _aigVar; do we need this??
};

/* Global */
vector<string> portnum_ckt1, portname_ckt1, portnum_ckt2, portname_ckt2;
int            inputNum_ckt1, outputNum_ckt1, inputNum_ckt2, outputNum_ckt2;
// portnum: i0, i1.., o0, o1..
// portname: a,  b... , g, f..

unordered_map<int, string> umapNum_ckt1, umapName_ckt1, umapNum_ckt2,
    umapName_ckt2; // umap[real CNF_VAR] -> real input/output port name
unordered_map<int, Var> umapInterVar_ckt1,
    umapInterVar_ckt2; // those intervariable that doesn't exist in x/y/f/g
vector<vector<variable*>> MI, MO;
vector<variable*>         x, y, f, g; // sub2 = -1
vector<vector<Var>>       big_or;

// how to check if this MIMO is feasible solution fast?
void
printMatrix(const SatSolver& s, vector<vector<variable*>> const& M) {
    for (int j = 0; j < M.size(); j++) {
        for (int i = 0; i < M[0].size(); i++) {
            cout << M[j][i]->getName() << " " << M[j][i]->getSub1() << " "
                 << M[j][i]->getSub2() << " ";
            cout << M[j][i]->getVar() << " " << M[j][i]->getVar2()
                 << "\tAssign: ";
            cout << s.getValue(M[j][i]->getVar());
            cout << endl;
        }
    }
    cout << endl;
}
void
printArr(const SatSolver& s, vector<variable*> const& Arr) {
    for (int i = 0; i < Arr.size(); i++) {
        cout << Arr[i]->getName() << " " << Arr[i]->getSub1() << " "
             << Arr[i]->getSub2() << " ";
        cout << Arr[i]->getVar() << " " << Arr[i]->getVar2() << "\tAssign: ";
        cout << s.getValue(Arr[i]->getVar());
        cout << endl;
    }
    cout << endl;
}

template <typename K, typename V>
void
print_map(unordered_map<K, V> const& m) {
    for (auto const& pair : m) {
        std::cout << "{" << pair.first << ": " << pair.second << "}\n";
    }
}

void
constraint2(SatSolver& s) { // for solver1, j = 1 ~ mO :sum(cij+dij)  <= 1, for
                            // all i = 1 ~ nO
                            // could be improved by pseudo boolean constraint
    // cout <<  "MO " << MO.size() << " " << MO[0].size() << endl;
    for (int i = 0; i < MO[0].size(); i++) {
        /*test*/
        vec<Lit>
            lits_n0; // c_i 1 + d_i 1 + c_i 2 + d_i 2 + ... to prevent all zero
        /*test*/

        for (int j1 = 0; j1 < MO.size(); j1++) {
            for (int j2 = j1 + 1; j2 < MO.size(); j2++) {
                vec<Lit> lits;
                // (cij' + cij'), (cij' + dij') could be omitted by constraint 5
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
constraint3(SatSolver& s) { // for solver1, j = 1 ~ mI+1 :sum(aij+bij)  = 1, for
                            // all i = 1 ~ nI
                            // could be improved by pseudo boolean constraint
    for (int i = 0; i < MI[0].size(); i++) {
        vec<Lit>
            lits_n0; // a_i 1 + b_i 1 + a_i 2 + b_i 2 + ... to prevent all zero
        for (int j1 = 0; j1 < MI.size(); j1++) {
            for (int j2 = j1 + 1; j2 < MI.size(); j2++) {
                vec<Lit> lits;
                // (aij' + aij'), (aij' + bij') could be omitted by constraint 5
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
constraint4_miter(
    SatSolver& s) { // for mitersolver, intentionally make cji -> fi != gj
    vec<Lit> lit_vec;
    vec<Lit> lit_vec2; // let (result + constant 1), this can prevent that if I
                       // assume result = 0 when cij + dij = 0
    vector<Var> temp;
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
            // cout << f[i/2]->getVar2() << " " << g[j]->getVar2() << " " <<
            // MO[i][j]->getVar2() << " " << MO[i+1][j]->getVar2() << endl;
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
constraint5_miter(
    SatSolver& s) { // for mitersolver, aji -> xi == yj;  bji -> xi != yj;  aj
                    // MI+1 -> 0 == yj; bj MI+1 -> 1 == yj;
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

// inputNum_ckt1, outputNum_ckt1, inputNum_ckt2, outputNum_ckt2;
// Construct PHI<0>  (Preprocess not implemented yet.)
void
initCircuit(SatSolver& s, SatSolver& s_miter) { // create 2D array ETs

    // construct x, y
    for (int i = 1; i <= inputNum_ckt1; i++) {
        variable* tmpX = new variable('x', i, -1);
        tmpX->setVar(s.newVar());
        tmpX->setVar2(s_miter.newVar());
        x.push_back(tmpX);
    }
    for (int i = 1; i <= inputNum_ckt2; i++) {
        variable* tmpY = new variable('y', i, -1);
        tmpY->setVar(s.newVar());
        tmpY->setVar2(s_miter.newVar());
        y.push_back(tmpY);
    }

    // construct f, g
    for (int i = 1; i <= outputNum_ckt1; i++) {
        variable* tmpF = new variable('f', i, -1);
        tmpF->setVar(s.newVar());
        tmpF->setVar2(s_miter.newVar());
        f.push_back(tmpF);
    }
    for (int i = 1; i <= outputNum_ckt2; i++) {
        variable* tmpG = new variable('g', i, -1);
        tmpG->setVar(s.newVar());
        tmpG->setVar2(s_miter.newVar());
        g.push_back(tmpG);
    }

    // construct MI matrix
    for (int j = 1; j <= 2 * (inputNum_ckt1 + 1); j++) { // mI
        vector<variable*> col;
        for (int i = 1; i <= inputNum_ckt2; i++) { // nI
            variable* tmp =
                new variable((j % 2 == 1) ? 'a' : 'b', i, (j + 1) / 2);
            tmp->setVar(s.newVar());
            tmp->setVar2(s_miter.newVar());
            col.push_back(tmp);
        }
        MI.push_back(col);
    }

    // construct MO matrix
    vec<Lit> lits;
    for (int j = 1; j <= 2 * outputNum_ckt1; j++) { // mO
        vector<variable*> col;
        for (int i = 1; i <= outputNum_ckt2; i++) { // nO
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
        MO.push_back(col);
    }

    s.addClause(lits);
    lits.clear(); // constraint 1.  sum(sum(c_ij + d_ij)) >0
    constraint2(s);
    constraint3(s);
    // solver(solver1 is used for determining feasible a, b, c, d
    // (permutations), has nothing to do with x, y, f, g)

    constraint4_miter(s_miter);
    constraint5_miter(s_miter);
}
void
readAAG() { // read aag file: top1.aag, top2.aag
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
}
void
readMAP() { // read map     : map1.txt, map2.txt
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
        // inout: 2 -> input, 3 -> output. But portnum has inout info, don't
        // need to store inout. aagVar doesn't matter as well.
        umapNum_ckt1[cnfVar + 1]  = portnum_ckt1[i];
        umapName_ckt1[cnfVar + 1] = portname_ckt1[i];
        i++;
    }
    while (fmap2 >> inout >> aagVar >> cnfVar) {
        // inout: 2 -> input, 3 -> output. But portnum has inout info, don't
        // need to store inout. aagVar doesn't matter as well.
        umapNum_ckt2[cnfVar + 1]  = portnum_ckt2[j];
        umapName_ckt2[cnfVar + 1] = portname_ckt2[j];
        j++;
    }
}
void
readCNF(SatSolver& s_miter) { // read cnf file: top1.cnf, top2.cnf
    // read CNF into miter solver to constraint functionality correctness
    ifstream fcnf1, fcnf2;
    string   fcnf1_name = "", fcnf2_name = "";
    fcnf1_name = fcnf2_name = "./files/cnf/";
    fcnf1_name += "top1.cnf";
    fcnf2_name += "top2.cnf";
    fcnf1.open(fcnf1_name);
    fcnf2.open(fcnf2_name);
    string tmp, cnf;
    int    varnum, clsnum, litTmp;
    while (true) {
        fcnf1 >> tmp;
        if (tmp == "p") {
            fcnf1 >> cnf >> varnum >> clsnum;
            // cout<<"CC: " << cnf << " " << varnum  << " " << clsnum << endl;
            vec<Lit> lits;

            vector<Var> test; // test addClause

            while (fcnf1 >> litTmp) {
                // cout << "LL: " << litTmp << endl;
                if (litTmp != 0) {
                    if (umapNum_ckt1.find(abs(litTmp)) ==
                        umapNum_ckt1.end()) { // doesn't exist
                        umapNum_ckt1[abs(litTmp)] =
                            "n" + to_string(abs(litTmp));
                        Var nV                         = s_miter.newVar();
                        umapInterVar_ckt1[abs(litTmp)] = nV;
                        Lit li;
                        if (litTmp > 0) li = Lit(nV);
                        else li = ~Lit(nV);

                        lits.push(li);
                        test.push_back(nV);
                    } else {
                        string portNumTmp =
                            umapNum_ckt1[abs(litTmp)]; // i0, i1 ..
                        // cout << "PPP: " << portNumTmp << endl;
                        // add clause to s_miter according to i/o and num. ex:
                        // i0 --->  x[0]->getVar2()
                        if (portNumTmp[0] == 'i') {
                            Lit li;
                            if (litTmp > 0)
                                li = Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar2());
                            else
                                li = ~Lit(
                                    x[stoi(portNumTmp.substr(1))]->getVar2());

                            lits.push(li);
                            test.push_back(
                                x[stoi(portNumTmp.substr(1))]->getVar2());
                        } else if (portNumTmp[0] == 'o') { // o
                            Lit li;
                            if (litTmp > 0)
                                li = Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar2());
                            else
                                li = ~Lit(
                                    f[stoi(portNumTmp.substr(1))]->getVar2());

                            lits.push(li);
                            test.push_back(
                                f[stoi(portNumTmp.substr(1))]->getVar2());
                        } else { // n (intervariable)
                            Lit li;
                            if (litTmp > 0)
                                li = Lit(umapInterVar_ckt1[abs(litTmp)]);
                            else li = ~Lit(umapInterVar_ckt1[abs(litTmp)]);

                            lits.push(li);
                            test.push_back(umapInterVar_ckt1[abs(litTmp)]);
                        }
                    }
                } else {
                    s_miter.addClause(lits);
                    lits.clear();
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
            // cout<<"CC: " << cnf << " " << varnum  << " " << clsnum << endl;
            vec<Lit> lits;
            while (fcnf2 >> litTmp) {
                // cout << "LL: " << litTmp << endl;
                if (litTmp != 0) {
                    if (umapNum_ckt2.find(abs(litTmp)) ==
                        umapNum_ckt2.end()) { // doesn't exist
                        umapNum_ckt2[abs(litTmp)] =
                            "n" + to_string(abs(litTmp));
                        Var nV                         = s_miter.newVar();
                        umapInterVar_ckt2[abs(litTmp)] = nV;
                        Lit li;
                        if (litTmp > 0) li = Lit(nV);
                        else li = ~Lit(nV);

                        lits.push(li);
                    } else {
                        string portNumTmp =
                            umapNum_ckt2[abs(litTmp)]; // i0, i1 ..
                        // cout << "PPP: " << portNumTmp << endl;
                        // add clause to s_miter according to i/o and num. ex:
                        // i0 --->  x[0]->getVar2()
                        if (portNumTmp[0] == 'i') {
                            Lit li;
                            if (litTmp > 0)
                                li = Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar2());
                            else
                                li = ~Lit(
                                    y[stoi(portNumTmp.substr(1))]->getVar2());

                            lits.push(li);
                        } else if (portNumTmp[0] == 'o') { // o
                            Lit li;
                            if (litTmp > 0)
                                li = Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar2());
                            else
                                li = ~Lit(
                                    g[stoi(portNumTmp.substr(1))]->getVar2());

                            lits.push(li);
                        } else { // n (intervariable)
                            Lit li;
                            if (litTmp > 0)
                                li = Lit(umapInterVar_ckt2[abs(litTmp)]);
                            else li = ~Lit(umapInterVar_ckt2[abs(litTmp)]);

                            lits.push(li);
                        }
                    }
                } else {
                    s_miter.addClause(lits);
                    lits.clear();
                }
            }

            break;
        }
    }
}
void
AddLearnedClause(SatSolver& s, SatSolver& s_miter) { // probably wrong
    vec<Lit> lits, lits_e; // ex: lits_e:  (not e + not c11)...
    vec<Lit> temp_lits;
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
                // cout << "x" << i << ": " << s_miter.getValue(x[i]->getVar2())
                // << "  y" << j << ": " << s_miter.getValue(y[j]->getVar2()) <<
                // endl;
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
            // cout << "f" << i << ": " << s_miter.getValue(f[i]->getVar2()) <<
            // "  g" << j << ": " << s_miter.getValue(g[j]->getVar2()) << endl;
            if (s_miter.getValue(f[i]->getVar2()) ==
                s_miter.getValue(g[j]->getVar2())) {
                Var dV = MO[2 * i + 1][j]->getVar();
                temp_lits.push(~Lit(dV)); // ex: f2 == g5 -> not d2,5 f2@f[1];
                                          // g5@g[4]; d2,5@MI[9][1]
                // lits_e.push(Ne); lits_e.push(~Lit(dV)); s.addClause(lits_e);
                // lits_e.clear(); for(int i = 0; i < temp_lits.size(); i++){
                //    cout << temp_lits[i] << " ";
                // }
                // cout << endl;
                s.addClause(temp_lits);
                temp_lits.clear();

            } else {
                Var cV = MO[2 * i][j]->getVar();
                temp_lits.push(~Lit(cV)); // ex: f2 != g5 -> not c2,5 f2@x[1];
                                          // g5@y[4]; c2,5@MI[8][1]
                // lits_e.push(Ne); lits_e.push(~Lit(cV)); s.addClause(lits_e);
                // lits_e.clear(); for(int i = 0; i < temp_lits.size(); i++){
                //    cout << temp_lits[i] << " ";
                // }
                // cout << endl;
                s.addClause(temp_lits);
                temp_lits.clear();
            }
        }
    }
    // lits.push(e); s.addClause(lits); lits.clear();
}
void
AddLearnedClause_const(SatSolver& s, SatSolver& s_miter) { // for const 0, 1
    vec<Lit> lits, lits_e; // ex: lits_e:  (not e + not c11)...
    vec<Lit> temp_lits;
    // Var eV = s.newVar();
    // Lit e  =  Lit(eV); // ex: e = c11' · c12' · d11' · d12'
    // Lit Ne = ~Lit(eV); // ex: e = c11' · c12' · d11' · d12'
    // cout << x.size() << " " << y.size() << " " << MI.size() << " " <<
    // MI[0].size() << endl;
    for (int j = 0; j < y.size(); j++) {
        // cout << "xy" << endl;
        if (s_miter.getValue(y[j]->getVar2()) == 1) {
            lits.push(Lit(MI[2 * inputNum_ckt1][j]
                              ->getVar())); // ex: x2 == y5 -> b2,5 x2@x[1];
                                            // y5@y[4]; b2,5@MI[9][1]
        } else {
            lits.push(Lit(MI[2 * inputNum_ckt1 + 1][j]
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
                // lits_e.push(Ne); lits_e.push(~Lit(dV)); s.addClause(lits_e);
                // lits_e.clear();
                s.addClause(temp_lits);
                temp_lits.clear();
            } else {
                Var cV = MO[2 * i][j]->getVar();
                temp_lits.push(~Lit(cV)); // ex: f2 != g5 -> not c2,5 f2@x[1];
                                          // g5@y[4]; c2,5@MI[8][1]
                // lits_e.push(Ne); lits_e.push(~Lit(cV)); s.addClause(lits_e);
                // lits_e.clear();
                s.addClause(temp_lits);
                temp_lits.clear();
            }
        }
    }
    // lits.push(e); s.addClause(lits); lits.clear();
}

void
reportResult(const SatSolver& solver, bool result) {
    solver.printStats();
    cout << (result ? "SAT" : "UNSAT") << endl;
    if (result) {
        printMatrix(solver, MI);
        printMatrix(solver, MO);
        // printArr(solver, x);
        // printArr(solver, y);
        // printArr(solver, f);
        // printArr(solver, g);
    }
}

int
main() {

    SatSolver solver, miterSolver;
    solver.initialize();
    miterSolver.initialize();

    readAAG();
    readMAP();
    initCircuit(solver, miterSolver);
    readCNF(miterSolver);

    /*
    bool result;
    result = solver.solve();
    reportResult(solver, result);
    */

    while (true) {
        // for(int i = 0; i < 10; i++){
        bool SAT1_result, SAT2_result;
        SAT1_result = solver.solve();
        if (!SAT1_result) {
            cout << "No match!!" << endl;
            break;
        } else {
            vec<Lit> assump;
            for (int i = 0; i < MI.size(); i++) {
                for (int j = 0; j < MI[0].size(); j++) {
                    if (solver.getValue(MI[i][j]->getVar()) == 1) {
                        Lit v = Lit(MI[i][j]->getVar2());
                        assump.push(v);
                        cout << "i: " << i << " j: " << j
                             << " val: " << solver.getValue(MI[i][j]->getVar())
                             << endl;
                    } else if (solver.getValue(MI[i][j]->getVar()) == 0) {
                        Lit v = ~Lit(MI[i][j]->getVar2());
                        assump.push(v);
                    }
                }
            }
            cout << "below is MO " << endl;
            for (int i = 0; i < MO.size(); i++) {
                for (int j = 0; j < MO[0].size(); j++) {
                    if (i % 2 == 0) {
                        if ((solver.getValue(MO[i][j]->getVar()) +
                             solver.getValue(MO[i + 1][j]->getVar())) == 0) {
                            assump.push(Lit(big_or[j][i / 2]));
                        }
                    }
                    if (solver.getValue(MO[i][j]->getVar()) == 1) {
                        Lit v = Lit(MO[i][j]->getVar2());
                        assump.push(v);
                        cout << "i: " << i << " j: " << j
                             << " val: " << solver.getValue(MO[i][j]->getVar())
                             << endl;
                    } else if (solver.getValue(MO[i][j]->getVar()) == 0) {
                        Lit v = ~Lit(MO[i][j]->getVar2());
                        assump.push(v);
                    }
                }
            }
            cout << endl;

            string temp;
            // cin >> temp;

            SAT2_result = miterSolver.assumpSolve(assump);
            if (!SAT2_result) {
                cout << "Match found!!" << endl;
                reportResult(solver, SAT1_result);

                break;
            } else {
                // cout << "ELSE" << endl;
                AddLearnedClause(solver, miterSolver);
                // AddLearnedClause_const(solver, miterSolver);
            }
            assump.clear();
        }
    }
}

// cout << '\n' << "===umapNum ckt1" << endl;
// print_map(umapNum_ckt1);
// cout << '\n' << "===umapName ckt1" << endl;
// print_map(umapName_ckt1);
// cout << '\n' << "===umapNum ckt2" << endl;
// print_map(umapNum_ckt2);
// cout << '\n' << "===umapName ckt2" << endl;
// print_map(umapName_ckt2);

// cout << '\n' << "===intervar ckt1" << endl;
// print_map(umapInterVar_ckt1);

// cout << '\n' << "===intervar ckt2" << endl;
// print_map(umapInterVar_ckt2);
// cout << "===" << endl;

// cout << inputNum_ckt1 << " " << outputNum_ckt1 << " " << inputNum_ckt2 << " "
// << outputNum_ckt2 << endl;

// bool result;
// // result = miterSolver.solve();
// // reportResult(miterSolver, result);

// // //test assump
// vec<Lit> assump;
// Lit v;
// v =  Lit(x[0]->getVar());    assump.push(v); // a
// v =  Lit(x[1]->getVar());    assump.push(v); // b
// v =  Lit(x[2]->getVar());    assump.push(v); // c

// v =  Lit(y[0]->getVar());    assump.push(v); // s
// v =  ~Lit(y[1]->getVar());    assump.push(v); // t
// v =  ~Lit(y[2]->getVar());    assump.push(v); // u
// v =  Lit(y[3]->getVar());    assump.push(v); // v
// v =  Lit(y[4]->getVar());    assump.push(v); // w

// result = miterSolver.assumpSolve(assump);
// reportResult(miterSolver, result);
// assump.clear();

// xv  =  Lit(x[2]->getVar());    assump.push(xv);
// xv2 = ~Lit(y[0]->getVar());    assump.push(xv2);
// xv3 =  Lit(y[1]->getVar());    assump.push(xv3);

// result = solver.assumpSolve(assump);
// reportResult(solver, result);
// assump.clear();