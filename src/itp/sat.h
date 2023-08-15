/****************************************************************************
  FileName     [ sat.h ]
  PackageName  [ sat ]
  Synopsis     [ Define CaDiCaL solver interface functions ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu ]
  Copyright    [ Copyleft(c) 2010-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SAT_H
#define SAT_H

#include <cassert>
#include <iostream>
#include "cadical.hpp"
#include "myGlobal.h"
#include <vector>
#include <unordered_map>
#include <map>

using namespace std;

typedef int Var;

class Lit{
public:
   friend Lit operator ~ (Lit p);
   Lit(): _litVar(0){};
   explicit Lit(Var const _v): _litVar(_v){};
   ~Lit(){};
   // void invert(){_litVar = -_litVar;}
   int litVar(){ return _litVar; }
private:
      Var _litVar;
};
inline Lit operator ~ (Lit p) { Lit q(-p._litVar); return q;}

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

/********** Cadical_Solver **********/
class SatSolver
{
   public : 
      SatSolver():_solver(0), _isConstrained(0) { }
      ~SatSolver() { if (_solver) delete _solver; }

      // Solver initialization and reset
      void initialize() {
         reset();
         // if (_curVar == 1) { ++_curVar; }
      }
      void reset() {
         if (_solver) delete _solver;
         _solver = new CaDiCaL::Solver();
         _assump.clear(); _curVar = 1;_isConstrained = 0;
      }

      // Constructing proof model
      // Return the Var ID of the new Var
      inline Var newVar() { return _curVar++; }

      // fa/fb = true if it is inverted
      void addAigCNF(Var vf, Var va, bool fa, Var vb, bool fb) {
         vec<Lit> lits;
         Lit lf = Lit(vf);
         Lit la = fa? ~Lit(va): Lit(va);
         Lit lb = fb? ~Lit(vb): Lit(vb);
         lits.push(la); lits.push(~lf);
         addClause(lits); lits.clear();
         lits.push(lb); lits.push(~lf);
         addClause(lits); lits.clear();
         lits.push(~la); lits.push(~lb); lits.push(lf);
         addClause(lits); lits.clear();
      }
      // fa/fb = true if it is inverted
      void addXorCNF(Var vf, Var va, bool fa, Var vb, bool fb) {
         vec<Lit> lits;
         Lit lf = Lit(vf);
         Lit la = fa? ~Lit(va): Lit(va);
         Lit lb = fb? ~Lit(vb): Lit(vb);
         lits.push(~la); lits.push( lb); lits.push( lf);
         addClause(lits); lits.clear();
         lits.push( la); lits.push(~lb); lits.push( lf);
         addClause(lits); lits.clear();
         lits.push( la); lits.push( lb); lits.push(~lf);
         addClause(lits); lits.clear();
         lits.push(~la); lits.push(~lb); lits.push(~lf);
         addClause(lits); lits.clear();
      }

      void addMuxCNF(Var x, Var s, Var t, Var f){
         vec<Lit> lits;
         Lit lx = Lit(x);
         Lit ls = Lit(s);
         Lit lt = Lit(t);
         Lit lf = Lit(f);
         lits.push(~ls); lits.push(~lt); lits.push( lx);
         addClause(lits); lits.clear();
         lits.push(~ls); lits.push( lt); lits.push(~lx);
         addClause(lits); lits.clear();
         lits.push( ls); lits.push(~lf); lits.push( lx);
         addClause(lits); lits.clear();
         lits.push( ls); lits.push( lf); lits.push(~lx);
         addClause(lits); lits.clear();
         lits.push(~lt); lits.push(~lf); lits.push( lx);
         addClause(lits); lits.clear();
         lits.push( lt); lits.push( lf); lits.push(~lx);
         addClause(lits); lits.clear();
      }

      // For incremental proof, use "assumeSolve()"
      void assumeRelease() { _assump.clear(); }
      void assumeConstrainRelease(){ _isConstrained = 0;}
      void assumeProperty(Var prop, bool val) {
         _assump.push(val? Lit(prop): ~Lit(prop));
      }
      void assumeClause(vec<Lit>& ps){
         assert(_isConstrained == 0);
         for(int i = 0, n = ps.size(); i < n; ++i){
            _solver->constrain(ps[i].litVar());
         }
         _solver->constrain(0);
         _isConstrained = 1;
      }
      bool assumpSolve() {
         for(size_t i = 0, n = _assump.size(); i < n; ++i){
            _solver->assume(_assump[i].litVar());
         }
         int ret = _solver->solve(); 
         _recordCurrentVarVal(ret);
         assert(ret != 0); // if Unsolved -> abort
         // for(size_t i = 0, n = _assump.size(); i < n; ++i){
         //    _solver->assume(_assump[i].litVar());
         // }
         return ret == 10;
      }
      
      // my version assumpSolve 2023 5 13
      bool assumpSolve(vec<Lit>& _as) { 
         for(size_t i = 0, n = _as.size(); i < n; ++i){
            _solver->assume(_as[i].litVar());
         }
         int ret = _solver->solve(); 
         assert(ret != 0); // if Unsolved -> abort
         _recordCurrentVarVal(ret);
         // for(size_t i = 0, n = _as.size(); i < n; ++i){
         //    _solver->assume(_as[i].litVar());
         // }
         return ret == 10;
      }

      // For one time proof, use "solve"
      void assertProperty(Var prop, bool val) {
         _solver->add(val ? prop: -prop);
         // _solver->addUnit(val? Lit(prop): ~Lit(prop));
      }
      bool solve(){
         int ret = _solver->solve();
         assert(ret != 0);
         _recordCurrentVarVal(ret);
         // cout << "solve : " << _solver->status() << endl;
         return ret == 10;
      }
      // bool solve() { return _solver->solve(); return _solver->okay(); }

      // Functions about Reporting
      // Return 1/0/-1; -1 means unknown value
      int getValue(Var v) const {
         map<Var, bool>::const_iterator ret;
         ret = _currentValOfEachVar.find(v);
         assert(ret != _currentValOfEachVar.end());
         return ret->second;

         // return (_solver->val(v) < 0 ? 0 : 1);   // not sure what will return ???????
      }
      bool isInCore(Lit l){
         return _solver->failed(l.litVar());
      }
      // void printStats() const { const_cast<Solver*>(_solver)->printStats(); } // not yet done
      void printStats() const { cout << "hello world" << endl; } // not yet done


      void addUnit(Var prop, bool val) {
         _solver->add(val ? prop : -prop);
         _solver->add(0);
         // _solver->addUnit(val? Lit(prop): ~Lit(prop));
      }

      void addBinary(Var va, bool fa, Var vb, bool fb){ //false: va' (inverted)
         _solver->add(fa ? va : -va);
         _solver->add(fb ? vb : -vb);
         _solver->add(0);
      }

      void addAtLeast(vector<Var>& vec_var, int at_least_num, int sum, int index){
         Var V = newVar();
         _map[make_pair(0,0)] = V;
         addUnit(V, 1);
         recursiveAtLeast(vec_var,at_least_num,sum,index);
         // vector<Var> temp_var;
         // cout << "temp size: " << vec_var.size() << endl;
         // tempCondition5(temp_var, vec_var, 0, vec_var.size()-1, at_least_num+1);
      }

      void recursiveAtLeast(vector<Var>& vec_var, int at_least_num, int sum, int index){
         // cout << "sum: " << sum << " index: " << index << endl;
         if(index == vec_var.size()-1){
            // cout << "temp1 " << endl;
            if (!_map.count({sum,index})){
               vec<Lit> lits;
               Lit lx = Lit(_map[{sum,index}]);
               Lit ls = Lit(vec_var[index]);
               lits.push( lx); lits.push(~ls);
               addClause(lits); lits.clear();
               lits.push(~lx); lits.push( ls);
               addClause(lits); lits.clear();
            }
         }
         else if(sum == at_least_num){
            // cout << "temp2 " << endl;
            if(!_map.count({sum,index+1})){
               Var V = newVar();
               _map[make_pair(sum,index+1)] = V;               
               recursiveAtLeast(vec_var,at_least_num,sum,index+1);
            }
            vec<Lit> lits;
            Lit lx = Lit(_map[{sum,index}]);
            Lit ls = Lit(vec_var[index]);
            Lit lf = Lit(_map[{sum,index+1}]);
            lits.push(~lx); lits.push( ls); lits.push( lf);
            addClause(lits); lits.clear();
            lits.push( lx); lits.push(~ls);
            addClause(lits); lits.clear();
            lits.push( lx); lits.push(~lf);
            addClause(lits); lits.clear();
         }
         else if(vec_var.size() - (index - sum + 1) == at_least_num){
            // cout << "temp3 " << endl;
            if(!_map.count({sum+1,index+1})){
               Var V = newVar();
               _map[make_pair(sum+1,index+1)] = V;               
               recursiveAtLeast(vec_var,at_least_num,sum+1,index+1);
            }
            vec<Lit> lits;
            Lit lx = Lit(_map[{sum,index}]);
            Lit ls = Lit(vec_var[index]);
            Lit lt = Lit(_map[{sum+1,index+1}]);
            lits.push( lx); lits.push(~ls); lits.push(~lt);
            addClause(lits); lits.clear();
            lits.push(~lx); lits.push( ls);
            addClause(lits); lits.clear();
            lits.push(~lx); lits.push( lt);
            addClause(lits); lits.clear();
         }
         else{
            // cout << "temp4 " << endl;
            if(!_map.count({sum,index+1})){
               Var V = newVar();
               _map[make_pair(sum,index+1)] = V;               
               recursiveAtLeast(vec_var,at_least_num,sum,index+1);
            }
            if(!_map.count({sum+1,index+1})){
               Var V = newVar();
               _map[make_pair(sum+1,index+1)] = V;               
               recursiveAtLeast(vec_var,at_least_num,sum+1,index+1);
            }
            addMuxCNF(_map[{sum,index}], vec_var[index], _map[{sum+1,index+1}], _map[{sum,index+1}]);
         }
      }

      void MapClear(){
         _map.clear();
      }
      void addClause(vec<Lit>& ps){
         for(size_t i = 0, n = ps.size(); i < n; ++i)
            _solver->add(ps[i].litVar());
         _solver->add(0);
      }
      //addclause self version
      // void addClause(const vec<Lit>& lits){
      //    addClause(lits);
      // }
      int status() const {return _solver->status();};
      CaDiCaL::Solver*  _solver;    // Pointer to a CaDiCaL solver
   private : 
      bool              _isConstrained; // make sure to release after every assumeSolve
      Var               _curVar;    // Variable currently
      vec<Lit>          _assump;    // Assumption List for assumption solve
      unordered_map<pair<int,int>, Var, pair_hash> _map;
      map<Var, bool> _currentValOfEachVar;
      void _recordCurrentVarVal(int _state);
};

#endif
