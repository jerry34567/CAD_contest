/****************************************************************************
  FileName     [ main.cpp ]
  PackageName  [ main ]
  Synopsis     [ Define main() ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_MAIN_CC
#define GV_MAIN_CC

#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "gvUsage.h"
#include "kernel/yosys.h"
#include "util.h"
using namespace std;

string   GVMsg::_allName = "";
ofstream GVMsg::_allout;
GVMsgMgr gvMsg;

GVUsage gvUsage;

//----------------------------------------------------------------------
//    Global cmd Manager
//----------------------------------------------------------------------
GVCmdMgr* gvCmdMgr = new GVCmdMgr("gv");

extern bool GVinitCommonCmd();
extern bool GVinitNtkCmd();
extern bool GVinitSimCmd();
extern bool GVinitVrfCmd();
extern bool GVinitAbcCmd();
extern bool GVinitModCmd();
extern bool GVinitBddCmd();
extern bool GVinitProveCmd();

static void
usage() {
    cout << "Usage: cirTest [ -File < doFile > ]" << endl;
}

static void
myexit() {
    usage();
    exit(-1);
}

int
main(int argc, char** argv) {
    myStrNCmp("-File", argv[1], 2); //?????
    myUsage.reset();
    // ifstream dof;

    // if (argc == 3) { // -file <doFile>
    //     if (myStrNCmp("-File", argv[1], 2) == 0) {
    //         if (!gvCmdMgr->openDofile(argv[2])) {
    //             cout << "Error: cannot open file \"" << argv[2] << "\"!!\n";
    //             myexit();
            // }
    //     } else {
    //         cout << "Error: unknown argument \"" << argv[1] << "\"!!\n";
    //         myexit();
    //     }
    // } else if (argc != 1) {
    //     cout << "Error: illegal number of argument (" << argc << ")!!\n";
    //     myexit();
    // }
    
    Yosys::yosys_setup();                     // initial yosys command
    Yosys::log_streams.push_back(&std::cout); // log yosys message
    if (!(GVinitCommonCmd() && GVinitNtkCmd() && GVinitSimCmd() &&
          GVinitVrfCmd() && GVinitAbcCmd() && GVinitModCmd() &&
          GVinitBddCmd() && GVinitProveCmd()))
        return 1;

    // GVCmdExecStatus status = GV_CMD_EXEC_DONE;
    // while (status != GV_CMD_EXEC_QUIT) {
    //     gvCmdMgr->setPrompt();
    //     status = gvCmdMgr->execOneCmd();
    //     cout << endl;
    // }

/******************************************/
    ifstream inputfile(argv[1]);
    string cmd;
    string verilog_1;
    string verilog_2;
    int num_bus;
    int num_port;
    vector<vector<string>> bus_list_1;
    vector<vector<string>> bus_list_2;
    string port;

    inputfile >> verilog_1 >> num_bus;
    bus_list_1.reserve(num_bus);

    for(int i = 0; i < num_bus; ++i){
        vector<string> temp;
        temp.reserve(num_port);
        inputfile >> num_port;
        for(int j = 0; j < num_port; ++j){
            inputfile >> port;
            temp.push_back(port);
        }
        bus_list_1.push_back(temp);
    }
    
    inputfile >> verilog_2 >> num_bus;
    bus_list_2.reserve(num_bus);
    for(int i = 0; i < num_bus; ++i){
        vector<string> temp;
        temp.reserve(num_port);
        inputfile >> num_port;
        for(int j = 0; j < num_port; ++j){
            inputfile >> port;
            temp.push_back(port);
        }
        bus_list_2.push_back(temp);
    }

    // cmd = "read design -v " + verilog_1;
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "file2 aig -v -o top1.aig";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "reset system y";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "read design -v " + verilog_2;
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "file2 aig -v -o top2.aig";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "reset system y";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "set engine abc";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "abccmd read top1.aig";
    // // cmd = "read design -aig top1.aig";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "abccmd write_cnf top1.cnf";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "abccmd read top2.aig";
    // // cmd = "read design -aig top2.aig";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "abccmd write_cnf top2.cnf";
    // // cmd = "abccmd write_cnf top2.cnf";
    // gvCmdMgr->execOneCmd(cmd);
    // cmd = "q -f";
    // gvCmdMgr->execOneCmd(cmd);

    cmd = "set engine abc";
    gvCmdMgr->execOneCmd(cmd);
    cmd = "abccmd read_verilog " + verilog_1;
    gvCmdMgr->execOneCmd(cmd);
    cmd = "abccmd fraig";
    gvCmdMgr->execOneCmd(cmd);
    cmd = "abccmd write_aiger -s top1.aig";
    gvCmdMgr->execOneCmd(cmd);
    cmd = "abccmd write_cnf top1.cnf";
    gvCmdMgr->execOneCmd(cmd);
    cmd = "abccmd read_verilog " + verilog_2;
    gvCmdMgr->execOneCmd(cmd);
    cmd = "abccmd fraig";
    gvCmdMgr->execOneCmd(cmd);
    cmd = "abccmd write_aiger -s top2.aig";
    gvCmdMgr->execOneCmd(cmd);
    cmd = "abccmd write_cnf top2.cnf";
    gvCmdMgr->execOneCmd(cmd);

/*******************************************/
    return 0;
}

#endif