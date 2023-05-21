/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include "satCmd.h"
#include "cmdParser.h"
#include "sat.h"
#include "satMgr.h"
#include "util.h"
#include <iomanip>
#include <iostream>

using namespace std;

// extern MemTest mtest; // defined in memTest.cpp

SatMgr satmgr;

bool
initSatCmd() {
    //  if (!(cmdMgr->regCmd("NP3Solve", 3, new NP3SolveCmd) &&
    //        cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
    //        cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
    //        cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)))
    if (!(cmdMgr->regCmd("NP3Solve", 4, new NP3SolveCmd))) {
        cerr << "Registering \"mem\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}

//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
NP3SolveCmd::exec(const string& option) {
    satmgr.solveNP3();
    return CMD_EXEC_DONE;
}

void
NP3SolveCmd::usage(ostream& os) const {
    // os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
NP3SolveCmd::help() const {
    // cout << setw(15) << left << "MTReset: "
    //      << "(memory test) reset memory manager" << endl;
}

// //----------------------------------------------------------------------
// //    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
// //----------------------------------------------------------------------
// CmdExecStatus
// MTNewCmd::exec(const string& option) {
//     // TODO

//     // Use try-catch to catch the bad_alloc exception
//     return CMD_EXEC_DONE;
// }

// void
// MTNewCmd::usage(ostream& os) const {
//     // os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t
//     arraySize)]\n";
// }

// void
// MTNewCmd::help() const {
//     // cout << setw(15) << left << "MTNew: "
//     //      << "(memory test) new objects" << endl;
// }

// //----------------------------------------------------------------------
// //    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
// //----------------------------------------------------------------------
// CmdExecStatus
// MTDeleteCmd::exec(const string& option) {
//     // TODO

//     return CMD_EXEC_DONE;
// }

// void
// MTDeleteCmd::usage(ostream& os) const {
//     // os << "Usage: MTDelete <-Index (size_t objId) | "
//     //    << "-Random (size_t numRandId)> [-Array]" << endl;
// }

// void
// MTDeleteCmd::help() const {
//     // cout << setw(15) << left << "MTDelete: "
//     //      << "(memory test) delete objects" << endl;
// }

// //----------------------------------------------------------------------
// //    MTPrint
// //----------------------------------------------------------------------
// CmdExecStatus
// MTPrintCmd::exec(const string& option) {
//     // check option
//     // if (option.size())
//     //    return CmdExec::errorOption(CMD_OPT_EXTRA, option);
//     // mtest.print();

//     return CMD_EXEC_DONE;
// }

// void
// MTPrintCmd::usage(ostream& os) const {
//     // os << "Usage: MTPrint" << endl;
// }

// void
// MTPrintCmd::help() const {
//     // cout << setw(15) << left << "MTPrint: "
//     //      << "(memory test) print memory manager info" << endl;
// }
