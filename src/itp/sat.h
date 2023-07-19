/****************************************************************************
  FileName     [ sat.h ]
  PackageName  [ sat ]
  Synopsis     [ Define miniSat solver interface functions ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu ]
  Copyright    [ Copyleft(c) 2010-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SAT_H
#define SAT_H

#include <cassert>
#include <iostream>
#include "Solver.h"
#include <vector>
#include <unordered_map>

using namespace std;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
      //   auto h1 = std::hash<T1>{}(p.first);
      //   auto h2 = std::hash<T2>{}(p.second);

        std::size_t seed = 0;
        // Hash the first element
        seed ^= std::hash<T1>()(p.first);
        // Combine with the hash of the second element
        seed ^= std::hash<T2>()(p.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
      //   return h1 ^ h2;  
    }
};

/********** MiniSAT_Solver **********/
class SatSolver
{
   public : 
      SatSolver():_solver(0) { }
      ~SatSolver() { if (_solver) delete _solver; }

      // Solver initialization and reset
      void initialize() {
         reset();
         if (_curVar == 0) { _solver->newVar(); ++_curVar; }
      }
      void reset() {
         if (_solver) delete _solver;
         _solver = new Solver();
         _assump.clear(); _curVar = 0;
      }

      // Constructing proof model
      // Return the Var ID of the new Var
      inline Var newVar() { _solver->newVar(); return _curVar++; }

      // fa/fb = true if it is inverted
      void addAigCNF(Var vf, Var va, bool fa, Var vb, bool fb) {
         vec<Lit> lits;
         Lit lf = Lit(vf);
         Lit la = fa? ~Lit(va): Lit(va);
         Lit lb = fb? ~Lit(vb): Lit(vb);
         lits.push(la); lits.push(~lf);
         _solver->addClause(lits); lits.clear();
         lits.push(lb); lits.push(~lf);
         _solver->addClause(lits); lits.clear();
         lits.push(~la); lits.push(~lb); lits.push(lf);
         _solver->addClause(lits); lits.clear();
      }
      // fa/fb = true if it is inverted
      void addXorCNF(Var vf, Var va, bool fa, Var vb, bool fb) {
         vec<Lit> lits;
         Lit lf = Lit(vf);
         Lit la = fa? ~Lit(va): Lit(va);
         Lit lb = fb? ~Lit(vb): Lit(vb);
         lits.push(~la); lits.push( lb); lits.push( lf);
         _solver->addClause(lits); lits.clear();
         lits.push( la); lits.push(~lb); lits.push( lf);
         _solver->addClause(lits); lits.clear();
         lits.push( la); lits.push( lb); lits.push(~lf);
         _solver->addClause(lits); lits.clear();
         lits.push(~la); lits.push(~lb); lits.push(~lf);
         _solver->addClause(lits); lits.clear();
      }

      void addMuxCNF(Var x, Var s, Var t, Var f){
         vec<Lit> lits;
         Lit lx = Lit(x);
         Lit ls = Lit(s);
         Lit lt = Lit(t);
         Lit lf = Lit(f);
         lits.push(~ls); lits.push(~lt); lits.push( lx);
         _solver->addClause(lits); lits.clear();
         lits.push(~ls); lits.push( lt); lits.push(~lx);
         _solver->addClause(lits); lits.clear();
         lits.push( ls); lits.push(~lf); lits.push( lx);
         _solver->addClause(lits); lits.clear();
         lits.push( ls); lits.push( lf); lits.push(~lx);
         _solver->addClause(lits); lits.clear();
         lits.push(~lt); lits.push(~lf); lits.push( lx);
         _solver->addClause(lits); lits.clear();
         lits.push( lt); lits.push( lf); lits.push(~lx);
         _solver->addClause(lits); lits.clear();
      }

      // For incremental proof, use "assumeSolve()"
      void assumeRelease() { _assump.clear(); }
      void assumeProperty(Var prop, bool val) {
         _assump.push(val? Lit(prop): ~Lit(prop));
      }
      bool assumpSolve() { return _solver->solve(_assump); }
      
      // my version assumpSolve 2023 5 13
      bool assumpSolve(vec<Lit>& _as) { return _solver->solve(_as); }

      // For one time proof, use "solve"
      void assertProperty(Var prop, bool val) {
         _solver->addUnit(val? Lit(prop): ~Lit(prop));
      }
      bool solve() { _solver->solve(); return _solver->okay(); }

      // Functions about Reporting
      // Return 1/0/-1; -1 means unknown value
      int getValue(Var v) const {   
         return (_solver->modelValue(v)==l_True?1:
                (_solver->modelValue(v)==l_False?0:-1)); }
      void printStats() const { const_cast<Solver*>(_solver)->printStats(); }

      //addclause self version
      void addClause(const vec<Lit>& lits){
         _solver->addClause(lits);
      }

      void addUnit(Var prop, bool val) {
         _solver->addUnit(val? Lit(prop): ~Lit(prop));
      }

      void addBinary(Var va, bool fa, Var vb, bool fb){ //false: va' (inverted)
         Lit la = fa? Lit(va): ~Lit(va);
         Lit lb = fb? Lit(vb): ~Lit(vb);
         _solver->addBinary(la, lb);
      }

      void addAtLeast(vector<Var>& vec_var, int at_least_num, int sum, int index){
         Var V = newVar();
         map[make_pair(0,0)] = V;
         _solver->addUnit(Lit(V));
         recursiveAtLeast(vec_var,at_least_num,sum,index);
         // vector<Var> temp_var;
         // cout << "temp size: " << vec_var.size() << endl;
         // tempCondition5(temp_var, vec_var, 0, vec_var.size()-1, at_least_num+1);
      }

      void recursiveAtLeast(vector<Var>& vec_var, int at_least_num, int sum, int index){
         // cout << "sum: " << sum << " index: " << index << endl;
         if(index == vec_var.size()-1){
            // cout << "temp1 " << endl;
            if (!map.count({sum,index})){
               vec<Lit> lits;
               Lit lx = Lit(map[{sum,index}]);
               Lit ls = Lit(vec_var[index]);
               lits.push( lx); lits.push(~ls);
               _solver->addClause(lits); lits.clear();
               lits.push(~lx); lits.push( ls);
               _solver->addClause(lits); lits.clear();
            }
         }
         else if(sum == at_least_num){
            // cout << "temp2 " << endl;
            if(!map.count({sum,index+1})){
               Var V = newVar();
               map[make_pair(sum,index+1)] = V;               
               recursiveAtLeast(vec_var,at_least_num,sum,index+1);
            }
            vec<Lit> lits;
            Lit lx = Lit(map[{sum,index}]);
            Lit ls = Lit(vec_var[index]);
            Lit lf = Lit(map[{sum,index+1}]);
            lits.push(~lx); lits.push( ls); lits.push( lf);
            _solver->addClause(lits); lits.clear();
            lits.push( lx); lits.push(~ls);
            _solver->addClause(lits); lits.clear();
            lits.push( lx); lits.push(~lf);
            _solver->addClause(lits); lits.clear();
         }
         else if(vec_var.size() - (index - sum + 1) == at_least_num){
            // cout << "temp3 " << endl;
            if(!map.count({sum+1,index+1})){
               Var V = newVar();
               map[make_pair(sum+1,index+1)] = V;               
               recursiveAtLeast(vec_var,at_least_num,sum+1,index+1);
            }
            vec<Lit> lits;
            Lit lx = Lit(map[{sum,index}]);
            Lit ls = Lit(vec_var[index]);
            Lit lt = Lit(map[{sum+1,index+1}]);
            lits.push( lx); lits.push(~ls); lits.push(~lt);
            _solver->addClause(lits); lits.clear();
            lits.push(~lx); lits.push( ls);
            _solver->addClause(lits); lits.clear();
            lits.push(~lx); lits.push( lt);
            _solver->addClause(lits); lits.clear();
         }
         else{
            // cout << "temp4 " << endl;
            if(!map.count({sum,index+1})){
               Var V = newVar();
               map[make_pair(sum,index+1)] = V;               
               recursiveAtLeast(vec_var,at_least_num,sum,index+1);
            }
            if(!map.count({sum+1,index+1})){
               Var V = newVar();
               map[make_pair(sum+1,index+1)] = V;               
               recursiveAtLeast(vec_var,at_least_num,sum+1,index+1);
            }
            addMuxCNF(map[{sum,index}], vec_var[index], map[{sum+1,index+1}], map[{sum,index+1}]);
         }
      }

      void MapClear(){
         map.clear();
      }

   private : 
      Solver           *_solver;    // Pointer to a Minisat solver
      Var               _curVar;    // Variable currently
      vec<Lit>          _assump;    // Assumption List for assumption solve
      unordered_map<pair<int,int>, Var, pair_hash> map;
};

#endif  // SAT_H

