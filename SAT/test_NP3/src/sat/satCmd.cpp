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
#include <string>

using namespace std;

// extern MemTest mtest; // defined in memTest.cpp

SatMgr satmgr;

bool
initSatCmd() {
    //  if (!(cmdMgr->regCmd("NP3Solve", 3, new NP3SolveCmd) &&
    //        cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
    //        cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
    //        cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)))
    if (!(cmdMgr->regCmd("NP3Solve", 4, new NP3SolveCmd) &&
          cmdMgr->regCmd("NP3Verify", 4, new NP3VerifyCmd) &&
          cmdMgr->regCmd("NP3Bind", 4, new NP3BindCmd))) {
        cerr << "Registering \"NP3\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}

//----------------------------------------------------------------------
//    NP3Solve
//----------------------------------------------------------------------
CmdExecStatus
NP3SolveCmd::exec(const string& option) {
    satmgr.solveNP3();
    return CMD_EXEC_DONE;
}

void
NP3SolveCmd::usage(ostream& os) const {
    os << "Usage: NP3Solve" << endl;
}

void
NP3SolveCmd::help() const {
    cout << setw(15) << left << "NP3Solve: "
         << "check if top1.cir is NP3 equilivance with top2.cir. If true, show "
            "the IO mapping."
         << endl;
}

//----------------------------------------------------------------------
//    NP3Verify
//----------------------------------------------------------------------
CmdExecStatus
NP3VerifyCmd::exec(const string& option) {
    satmgr.verification();
    return CMD_EXEC_DONE;
}

void
NP3VerifyCmd::usage(ostream& os) const {
    os << "Usage: NP3Verify" << endl;
}

void
NP3VerifyCmd::help() const {
    cout << setw(15) << left << "NP3Verify: "
         << "check if the IO mapping is correct." << endl;
}

//----------------------------------------------------------------------
//    NP3Bind <string port1> <string port2>
//----------------------------------------------------------------------
CmdExecStatus
NP3BindCmd::exec(const string& option) {
    // vector<string> options;
    // if (!CmdExec::lexOptions(option, options)) return CMD_EXEC_ERROR;
    // satmgr.addBindClause(0,-1,-1,)
    return CMD_EXEC_DONE;
}

void
NP3BindCmd::usage(ostream& os) const {
    os << "Usage: NP3Verify" << endl;
}

void
NP3BindCmd::help() const {
    cout << setw(15) << left << "NP3Verify: "
         << "check if the IO mapping is correct." << endl;
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
