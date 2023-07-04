/****************************************************************************
  FileName     [ satCmd.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Define basic sat prove package commands ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/

#include "gvSatCmd.h"
// #include "gvMsg.h"
// #include "gvSatMgr.h"
#include "sat.h"
#include "solverMgr.h"
#include "util.h"
#include <cstring>
#include <iomanip>
#include <string>
using namespace std;

// static SATMgr* satMgr = new SATMgr();
SolverMgr solvermgr;

bool
GVinitItpCmd() {
    return (gvCmdMgr->regCmd("NP3 S", 3, 1, new NP3SolveCmd) &&
            gvCmdMgr->regCmd("NP3 V", 3, 1, new NP3VerifyCmd) &&
            gvCmdMgr->regCmd("NP3 B", 3, 1, new NP3BindCmd));
}

//----------------------------------------------------------------------
//    SATVerify ITP < -Netid <netId> | -Output <outputIndex> > >
//----------------------------------------------------------------------
// GVCmdExecStatus
// SATVerifyItpCmd::exec(const string& option) {
/*    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() < 2)
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (options.size() > 2)
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);

    bool isNet = false;

    if (myStrNCmp("-Netid", options[0], 2) == 0) isNet = true;
    else if (myStrNCmp("-Output", options[0], 2) == 0) isNet = false;
    else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    int     num = 0;
    GVNetId netId;
    if (!myStr2Int(options[1], num) || (num < 0))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (isNet) {
        if ((unsigned)num >= gvNtkMgr->getNetSize()) {
            gvMsg(GV_MSG_ERR) << "Net with Id " << num
                              << " does NOT Exist in Current Ntk !!" <<
   endl; return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        netId = GVNetId::makeNetId(num);
    } else {
        if ((unsigned)num >= gvNtkMgr->getOutputSize()) {
            gvMsg(GV_MSG_ERR) << "Output with Index " << num
                              << " does NOT Exist in Current Ntk !!" <<
   endl; return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        netId = gvNtkMgr->getOutput(num);
    }
    // get PO's input, since the PO is actually a redundant node and should
   be
    // removed
    GVNetId redundantNode = gvNtkMgr->getGVNetId(netId.id);
    GVNetId monitor       = gvNtkMgr->getInputNetId(redundantNode, 0);
    satMgr->verifyPropertyItp(gvNtkMgr->getNetNameFromId(redundantNode.id),
                              monitor);
*/
//     return GV_CMD_EXEC_DONE;
// }

// void
// SATVerifyItpCmd::usage(const bool& verbose) const {
//     // gvMsg(GV_MSG_IFO)
//     //     << "Usage: SATVerify ITP < -Netid <netId> | -Output <outputIndex>
//     >"
//     //     << endl;
// }

// void
// SATVerifyItpCmd::help() const {
//     cout << setw(20) << left << "SATVerify ITP:"
//          << "check the monitor by interpolation-based technique" << endl;
// }

// //----------------------------------------------------------------------
// //    SATVerify BMC < -Netid <netId> | -Output <outputIndex> > >
// //----------------------------------------------------------------------
/*
GVCmdExecStatus
SATVerifyBmcCmd::exec(const string& option) {
    /*vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() < 2)
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (options.size() > 2)
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);

    bool isNet = false;

    if (myStrNCmp("-Netid", options[0], 2) == 0) isNet = true;
    else if (myStrNCmp("-Output", options[0], 2) == 0) isNet = false;
    else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    int     num = 0;
    GVNetId netId;
    if (!myStr2Int(options[1], num) || (num < 0))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (isNet) {
        if ((unsigned)num >= gvNtkMgr->getNetSize()) {
            gvMsg(GV_MSG_ERR) << "Net with Id " << num
                              << " does NOT Exist in Current Ntk !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        netId = GVNetId::makeNetId(num);
    } else {
        if ((unsigned)num >= gvNtkMgr->getOutputSize()) {
            gvMsg(GV_MSG_ERR) << "Output with Index " << num
                              << " does NOT Exist in Current Ntk !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        netId = gvNtkMgr->getOutput(num);
    }
    // get PO's input, since the PO is actually a redundant node and should be
    // removed
    GVNetId redundantNode = gvNtkMgr->getGVNetId(netId.id);
    GVNetId monitor       = gvNtkMgr->getInputNetId(redundantNode, 0);
    satMgr->verifyPropertyBmc(gvNtkMgr->getNetNameFromId(redundantNode.id),
                              monitor);

return GV_CMD_EXEC_DONE;
}

void
SATVerifyBmcCmd::usage(const bool& verbose) const {
    // gvMsg(GV_MSG_IFO)
    //     << "Usage: SATVerify BMC < -Netid <netId> | -Output < outputIndex >>
    //     "
    //     << endl;
}

void
SATVerifyBmcCmd::help() const {
    cout << setw(20) << left << "SATVerify BMC:"
         << "check the monitor by bounded model checking" << endl;
}
*/
//----------------------------------------------------------------------
//    NP3 Solve <string input_file_name>
//----------------------------------------------------------------------
GVCmdExecStatus
NP3SolveCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    solvermgr.solveNP3(options[0]);
    return GV_CMD_EXEC_DONE;
}

void
NP3SolveCmd::usage(const bool& verbose) const {
    // gvMsg(GV_MSG_IFO) << "Usage: NP3Solve" << endl;
}

void
NP3SolveCmd::help() const {
    cout << setw(15) << left << "NP3Solve: "
         << "check if top1.cir is NP3 equilivance with top2.cir. If true, show "
            "the IO mapping."
         << endl;
}

//----------------------------------------------------------------------
//    NP3 V [bool isManualBinded]
//----------------------------------------------------------------------
GVCmdExecStatus
NP3VerifyCmd::exec(const string& option) {
    // string _opt;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    // if (!) return GV_CMD_EXEC_DONE;
    solvermgr.verification(!options.empty());
    return GV_CMD_EXEC_DONE;
}

void
NP3VerifyCmd::usage(const bool& verbose) const {
    // gvMsg(GV_MSG_IFO) << "Usage: NP3Solve" << endl;
}

void
NP3VerifyCmd::help() const {
    cout << setw(15) << left << "NP3Verify: "
         << "check if the IO mapping is correct." << endl;
}

//----------------------------------------------------------------------
//    NP3Bind <bool isInput> <bool isNegate> <string port1> <string port2>
//----------------------------------------------------------------------
GVCmdExecStatus
NP3BindCmd::exec(const string& option) {
    vector<string> options; // is port1 == -1(-2) -> bind port2 to constant 0(1)
    // *** input only
    GVCmdExec::lexOptions(option, options);
    if (options.size() != 4) return GV_CMD_EXEC_ERROR;
    if (options[0] == "1") {
        if (options[2] == "-1" && solvermgr.getSatmgr().addBindClause(
                                      (options[1] == "1"), -1, -100, "",
                                      options[3])) // port2 bind to const 0
            return GV_CMD_EXEC_DONE;
        else if (options[2] == "-2" && solvermgr.getSatmgr().addBindClause(
                                           (options[1] == "1"), -2, -100, "",
                                           options[3])) // port2 bind to const 1
            return GV_CMD_EXEC_DONE;
        else if (solvermgr.getSatmgr().addBindClause(
                     (options[1] == "1"), -100, -100, options[2],
                     options[3])) // input binding
            return GV_CMD_EXEC_DONE;

    } else if (options[0] == "0" &&
               !solvermgr.getSatmgr().outputBind(-100, (options[1] == "1"),
                                                 -100, options[2],
                                                 options[3])) // output binding
    {
        cout << "outputBind fail : NP3BindCmd\n";
        return GV_CMD_EXEC_DONE;
    }
    return GV_CMD_EXEC_DONE;
}

void
NP3BindCmd::usage(const bool& verbose) const {
    // gvMsg(GV_MSG_IFO) << "Usage: NP3Solve" << endl;
}

void
NP3BindCmd::help() const {
    cout << setw(15) << left << "NP3Bind: "
         << "bind the two given ports together." << endl;
}