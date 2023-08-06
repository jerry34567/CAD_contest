#include "sat.h"

using namespace std;

void SatSolver::_recordCurrentVarVal(int _state){
    _currentValOfEachVar.clear();
    if(_state == 10)
        for(int i = 1; i < _curVar; ++i){
            _currentValOfEachVar.insert(pair<Var, bool>(i, (_solver->val(i) > 0)));
        }
}
