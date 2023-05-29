#ifndef SOLVERMGR_H
#define SOLVERMGR_H

#include "cirMgr.h"
#include "satMgr.h"

class SolverMgr
{
    public:
        SolverMgr(){};
        ~SolverMgr(){};
        void    verification(bool isManualBinded = 0);
        void    solveNP3();
        SatMgr& getSatmgr() { return satmgr; }

    private:
        SatMgr satmgr;
};

#endif // SOLVERMGR_H
