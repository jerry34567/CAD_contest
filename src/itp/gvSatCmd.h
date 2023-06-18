/****************************************************************************
  FileName     [ satCmd.h ]
  PackageName  [ sat ]
  Synopsis     [ Define basic sat prove package commands ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_SAT_CMD_H_
#define GV_SAT_CMD_H_

#include "gvCmdMgr.h"
#include "sat.h"
#include "solverMgr.h"
#include "util.h"

// ============================================================================
// Classes for Prove package commands
// ============================================================================
GV_COMMAND(SATVerifyItpCmd, GV_CMD_TYPE_COMMON);
GV_COMMAND(SATVerifyBmcCmd, GV_CMD_TYPE_COMMON);
GV_COMMAND(NP3SolveCmd, GV_CMD_TYPE_COMMON);
GV_COMMAND(NP3VerifyCmd, GV_CMD_TYPE_COMMON);
GV_COMMAND(NP3BindCmd, GV_CMD_TYPE_COMMON);

#endif
