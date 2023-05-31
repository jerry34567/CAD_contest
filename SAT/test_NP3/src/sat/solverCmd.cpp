/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include "solverCmd.h"
#include "cmdParser.h"
#include "sat.h"
#include "solverMgr.h"
#include "util.h"
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// extern MemTest mtest; // defined in memTest.cpp

SolverMgr solvermgr;

bool
initSolverCmd() {
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
    solvermgr.solveNP3();
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
//    NP3Verify [bool isManualBinded]
//----------------------------------------------------------------------
CmdExecStatus
NP3VerifyCmd::exec(const string& option) {
    string _opt;
    if (!CmdExec::lexSingleOption(option, _opt)) return CMD_EXEC_ERROR;
    solvermgr.verification(!_opt.empty());
    return CMD_EXEC_DONE;
}

void
NP3VerifyCmd::usage(ostream& os) const {
    os << "Usage: NP3Verify [bool isManualBinded]" << endl;
}

void
NP3VerifyCmd::help() const {
    cout << setw(15) << left << "NP3Verify: "
         << "check if the IO mapping is correct." << endl;
}

//----------------------------------------------------------------------
//    NP3Bind <bool isInput> <bool isNegate> <string port1> <string port2>
//----------------------------------------------------------------------
CmdExecStatus
NP3BindCmd::exec(const string& option) {
    vector<string> options; // is port1 == -1(-2) -> bind port2 to constant 0(1)
                            // *** input only
    if (!CmdExec::lexOptions(option, options) || options.size() != 4)
        return CMD_EXEC_ERROR;
    if (options[0] == "1") {
        if (options[2] == "-1" && solvermgr.getSatmgr().addBindClause(
                                      (options[1] == "1"), -1, -100, "",
                                      options[3])) // port2 bind to const 0
            return CMD_EXEC_DONE;
        else if (options[2] == "-2" && solvermgr.getSatmgr().addBindClause(
                                           (options[1] == "1"), -2, -100, "",
                                           options[3])) // port2 bind to const 1
            return CMD_EXEC_DONE;
        else if (solvermgr.getSatmgr().addBindClause(
                     (options[1] == "1"), -100, -100, options[2],
                     options[3])) // input binding
            return CMD_EXEC_DONE;

    } else if (options[0] == "0" &&
               solvermgr.getSatmgr().outputBind(-100, (options[1] == "1"), -100,
                                                options[2],
                                                options[3])) // output binding
    {
        cout << "outputBind fail : NP3BindCmd\n";
        return CMD_EXEC_DONE;
    }
    return CMD_EXEC_ERROR;
}

void
NP3BindCmd::usage(ostream& os) const {
    os << "Usage: NP3Bind <bool isInput> <bool isNegate> <string port1> "
          "<string port2>"
       << endl;
}

void
NP3BindCmd::help() const {
    cout << setw(15) << left << "NP3Bind: "
         << "bind the two given ports together." << endl;
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
