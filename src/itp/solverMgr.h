#ifndef SOLVERMGR_H
#define SOLVERMGR_H

#include "cirMgr.h"
#include "satMgr.h"
#include "sat.h"
class SolverMgr
{
    public:
        SolverMgr(){};
        ~SolverMgr(){};
        void    verification(bool isManualBinded = 0);
        void    solveNP3(string&, string&);
        SatMgr& getSatmgr() { return satmgr; }
    private:
        SatMgr satmgr;
};

#endif // SOLVERMGR_H
