#include <iostream>
#include <vector>
#include "sat.h"

using namespace std;
/* global variable, modify to different situation */
// Var eventNum = 3; Var timePeriod = 10;
Var eventNum = 8; Var timePeriod = 60;

class EventTime
{
public:
   EventTime(Var event, Var time): _event(event),  _time(time) {}
   ~EventTime() {}

   Var getEvent() const {return _event;}
   Var getTime() const {return _time;}
   Var getVar() const { return _var; }
   void setEvent(const Var& v){_event = v;}
   void setTime(const Var& v){_time = v;}
   void setVar(const Var& v) { _var = v; }

private:
   Var   _event; //event
   Var   _time;  //timegrid
   Var   _var;   //unique, for solver.

};
vector<vector<EventTime *>> ETs;  
// ETs[0][0]: event 0 time -1   ~   ETs[2][10]: event 2 time 9


void initCircuit(SatSolver& s){ //create 2D array ETs 
   // ETs.push_back(new EventTime(1,-1));
   for(Var events = 0; events < eventNum; events++){ //3 events(nodes) : 0 ~ 2
      vector<EventTime *>ET;
      for(Var time = 0; time <= timePeriod; time++){  //if t = 10, time(timegrid) : -1 ~ 9
         EventTime* tmp = new EventTime(events, time-1);
         Var v = s.newVar();
         tmp->setVar(v);
         ET.push_back(tmp);
      }
      ETs.push_back(ET);
   }

}

void addEncClauses(SatSolver& solver){
   for(Var e = 0; e < eventNum; e++){ //3 events(nodes) : 0 ~ 2
      solver.addUnit(ETs[e][0]->getVar(),false);
      solver.addUnit(ETs[e][timePeriod]->getVar(),true);
   
      for(Var t = 0; t < timePeriod; t++){  //if t = 10, time(timegrid) : -1 ~ 9
         solver.addBinary(ETs[e][t]->getVar(),false,ETs[e][t+1]->getVar(),true);
      } 
   }
}

void addRectClause(SatSolver& solver, Var eA, Var eB, Var Alow, Var Aup, Var Blow, Var Bup){
   //(A2 + A6' + B1 + B5') == [3,6]x[2,5]  Alow:3, Aup:6, Blow:2, Bup:5 == ETs[eA][3] + ~E`Ts[eA][7] + ETs[eA][2] + ~ETs[eA][6]
   // cout << " eA: " << eA << " eB: " << eB << " Alow: " << Alow << " Aup: " << Aup << " Blow: " << Blow << " Bup: " << Bup << endl; //correct
   vec<Lit> lits;
   if(Alow <= timePeriod && Alow >= 0){
      Lit lalow   = Lit(ETs[eA][Alow]->getVar());   
      lits.push(lalow);
   }
   if(Aup+1 <= timePeriod && Aup+1 >= 0){
      Lit laup    = ~Lit(ETs[eA][Aup+1]->getVar());   
      lits.push(laup);
   }
   if(Blow <= timePeriod && Blow >= 0){
      Lit lblow  = Lit(ETs[eB][Blow]->getVar());
      lits.push(lblow);
   }
   if(Bup+1 <= timePeriod && Bup+1 >= 0){
      Lit lbup    = ~Lit(ETs[eB][Bup+1]->getVar());   
      lits.push(lbup);
   }
   if(lits.size() != 0){
      solver.addClause(lits);   lits.clear();   
   }
}

void addRectClausesPair(SatSolver& solver, Var eA, Var eB, Var l, Var u){ 
   // time(eB) - time(eA) in [tlow,tup] (period: timePeriod)
   // rectangles between two parallel line pair: (l, u) l=3, u=-5     u<l
   Var height = (l-u-1)/2;
   Var width  = ((l-u-1)%2 == 1)? (l-u-1)/2: (l-u-1)/2-1;
   for(Var x2 = -height; x2 < timePeriod; x2++){
      Var x1 = x2-u-1-width;
      
      if(((x1+width) >= 0) && (x1 <= (timePeriod -1))){
         addRectClause(solver, eA, eB, x1, (x1+width), x2, (x2+height));
      }
   }
}

void addConstraint(SatSolver& solver, Var eA, Var eB, Var l, Var u){
   // call this function for each constaint, l or u one must in  -(timePeriod-1) ~ (timePeriod -1)
   // l=3, u=5  l<u
   // A(ans:1) --- [3,5]10 ---> B(ans:4)   
   addRectClausesPair(solver, eA, eB, l-timePeriod, u-2*timePeriod);
   addRectClausesPair(solver, eA, eB, l           , u-  timePeriod);
   addRectClausesPair(solver, eA, eB, l+timePeriod, u             );
}

/*modify for different situation*/
void genMyProof(SatSolver& s){
   // remember to change event number and time period global variables at line 8.
   addEncClauses(s);

   // addConstraint(s, 0, 1, 3, 5);
   // addConstraint(s, 1, 2, 2, 2);
   // addConstraint(s, 2, 0, 1, 2);
   
   addConstraint(s,1,2,16,18);
   addConstraint(s,1,3,16,18);
   addConstraint(s,1,4,16,18);
   addConstraint(s,2,0,7,7);
   addConstraint(s,3,0,6,6);
   addConstraint(s,4,0,8,8);
   addConstraint(s,2,6,3,57);
   addConstraint(s,4,6,5,55);
   addConstraint(s,5,6,12,12);
   addConstraint(s,6,7,4,6);

   // addConstraint(s, 0, 1, 3, 5);
   // addConstraint(s, 1, 2, 2, 2);
   // addConstraint(s, 2, 0, 2, 4);


   // addConstraint(s,0,1,3,3);
   // addConstraint(s,1,2,5,8);
   // addConstraint(s,2,0,3,10);

   
/*
   addConstraint(s,0,3,10,10);
   addConstraint(s,0,4,10,15);
   addConstraint(s,1,3,4,8);
   addConstraint(s,1,0,53,56);
   addConstraint(s,2,1,0,3);
   addConstraint(s,2,5,37,37);
   addConstraint(s,4,3,56,59);
   addConstraint(s,4,5,24,28);
*/




   // addConstraint(s,0,1,8,38);
   // addConstraint(s,0,2,2,58);
   // addConstraint(s,1,3,2,58);
   // addConstraint(s,2,3,8,38);
   // addConstraint(s,0,4,2,55);
   // addConstraint(s,4,5,20,24);
   // addConstraint(s,4,2,2,55);
   // addConstraint(s,6,3,3,54);
   // addConstraint(s,6,1,3,54);
   // addConstraint(s,5,6,17,18);





   /*
   addConstraint(s, 0, 1, 3, 5);
   addConstraint(s, 1, 2, 2, 2);
   addConstraint(s, 2, 0, 2, 4);
   */
}



void reportResult(const SatSolver& solver, bool result){
   solver.printStats();
   cout << (result? "SAT" : "UNSAT") << endl;
   if (result) {
      cout << "gatesize: " << ETs.size() << endl;
      for (int i = 0; i < eventNum; i++){ // expect something like: 111100000 ?
         cout << "Event " << i << ": ";
         int consecutiveZero = 0;
         for (int j = 0; j < timePeriod; j++){
            cout << solver.getValue(ETs[i][j]->getVar()) << " ";
            if(solver.getValue(ETs[i][j]->getVar()) == 0){
               consecutiveZero++;
            }
         }
         cout << " ==> Event "<< i << " time: " << consecutiveZero -1 << endl;
      }
   }
}

int main(){

   SatSolver solver;
   solver.initialize();

   initCircuit(solver);
   genMyProof(solver);
   bool result;
   result = solver.solve();
   reportResult(solver, result);

   cout << endl << endl << "======================" << endl;


}
