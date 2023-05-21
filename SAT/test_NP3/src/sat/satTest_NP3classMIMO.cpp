#include <iostream>
#include <vector>
#include "sat.h"

using namespace std;
class variable{
public:
   variable(char name, Var sub1,Var sub2): _name(name),_sub1(sub1), _sub2(sub2){}
   ~variable(){}
   Var getName() const {return _name;}
   Var getSub1() const {return _sub1;}
   Var getSub2() const {return _sub2;}
   Var getVar () const {return _var ;}
   void setName(const char& v){_name = v;} 
   void setSub1(const Var&  v){_sub1 = v;} 
   void setSub2(const Var&  v){_sub2 = v;} 
   void setVar (const Var&  v){_var  = v;} 
private:
   char _name; 
   Var _sub1;
   Var _sub2;
   Var _var;
   // Var _aigVar; do we need this??
};

class MatchI
{
public:
   MatchI(Var x, bool x_neg, Var y, bool mat): _x(x),_x_neg(x_neg), _y(y), _mat(mat) {}
   ~MatchI() {}
   Var  getX()     const {return _x;}
   bool getXneg()  const {return _x_neg;}
   Var  getY()     const {return _y;}
   bool getMat()   const {return _mat;}
   Var  getVar()   const {return _var;}
   
   void setX   (const Var&  v){_x      = v;}
   void setXneg(const bool& v){_x_neg  = v;}
   void setY   (const Var&  v){_y      = v;}
   void setMat (const bool& v){_mat    = v;}
   void setVar (const Var&  v){_var    = v;}
private:
   Var   _x;      // x sub num. 
   bool  _x_neg;  // x negation, if _x_neg: true -> x'
   Var   _y;      // y sub num
   bool  _mat;    // if _mat: true -> match
   Var   _var;    //unique, for solver.
   // ex: b2,1 is true     ->  _x = 1, _x_neg: true, _y = 2, _mat: true 
   //                      ->   x1' connect to y2

   // if constant 0        ->  _x = 0, _x_neg: false. if constant 1 -> _x = 0, _x_neg: true
   // ex: a3, mI+1 is true ->  _x = 0, _x_neg: false, _y = 3, _mat: true
};

class MatchO
{
public:
   MatchO(Var f, bool f_neg, Var g, bool mat): _f(f),_f_neg(f_neg), _g(g), _mat(mat) {}
   ~MatchO() {}
   Var  getF()     const {return _f;}
   bool getFneg()  const {return _f_neg;}
   Var  getG()     const {return _g;}
   bool getMat()   const {return _mat;}
   Var getVar()    const {return _var;}
   
   void setF   (const Var&  v){_f      = v;}
   void setFneg(const bool& v){_f_neg  = v;}
   void setG   (const Var&  v){_g      = v;}
   void setMat (const bool& v){_mat    = v;}
   void setVar (const Var&  v){_var    = v;}
private:
   Var   _f;      // f sub num. 
   bool  _f_neg;  // f negation, if _f_neg: true -> f'
   Var   _g;      // g sub num
   bool  _mat;    // if _mat: true -> match
   Var   _var;    //unique, for solver.
   // ex: b2,1 is true     ->  _f = 1, _f_neg: true, _g = 2, _mat: true 
   //                      ->   f1' connect to g2
};
// how to check if this MIMO is feasible solution fast? 

void printMatrix(vector<vector<variable*>> M){
   for (int j = 0; j < M.size(); j++){
      for (int i=0; i < M[0].size(); i++){
         cout << M[j][i]->getName() << " " << M[j][i]->getSub1() << " " << M[j][i]->getSub2() << " " << M[j][i]->getVar() << " " << endl;
      }
   }
}

void initCircuit(SatSolver& s, int numIn, int numOut){ //create 2D array ETs 
   vector<vector<variable*>> MI, MO;

   // construct MI matrix
   for (int j = 1; j <= 2*(numIn + 1); j++){ // mI
      vector<variable*> col;
      for(int i = 1; i <= numIn; i++){       // nI
         variable* tmp = new variable((j%2 == 1)?'a':'b', i, (j+1)/2);
         tmp->setVar(s.newVar());
         col.push_back(tmp);
      }
      MI.push_back(col);
   }

   // construct MO matrix
   for (int j = 1; j <= 2*numOut; j++){      // mO
      vector<variable*> col;
      for(int i = 1; i <= numOut; i++){      // nO
         variable* tmp = new variable((j%2 == 1)?'c':'d', i, (j+1)/2);
         tmp->setVar(s.newVar());
         col.push_back(tmp);
      }
      MO.push_back(col);
   }

   // test print 
   printMatrix(MI);
   printMatrix(MO);
   
}

/*modify for different situation*/
void genMyProof(SatSolver& s){
 
}


void reportResult(const SatSolver& solver, bool result){
   solver.printStats();
   cout << (result? "SAT" : "UNSAT") << endl;
   if (result) {
      
   }
}

int main(){

   SatSolver solver;
   solver.initialize();

   initCircuit(solver, 5, 3);
   genMyProof(solver);
   bool result;
   result = solver.solve();
   reportResult(solver, result);

   cout << endl << endl << "======================" << endl;


}
