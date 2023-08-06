#ifndef MYTEST_H
#define MYTEST_H
#include "cadical.hpp"
#include <iostream>
#include <cassert>
using namespace std;
void test123(){
    CaDiCaL::Solver *solver = new CaDiCaL::Solver;

  // ------------------------------------------------------------------
  // Encode Problem and check without assumptions.

  enum { TIE = 1, SHIRT = 2 };

  solver->add (-TIE), solver->add (SHIRT), solver->add (0);
  solver->add (TIE), solver->add (SHIRT), solver->add (0);
  solver->add (-TIE), solver->add (-SHIRT), solver->add (0);

  int res = solver->solve (); // Solve instance.
assert (res == 10);         // Check it is 'SATISFIABLE'.
cout << "hello world!" << endl;
}


#endif // MYTEST_H