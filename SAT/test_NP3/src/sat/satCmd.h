/****************************************************************************
  FileName     [ memCmd.h ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test command classes ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef SAT_CMD_H
#define SAT_CMD_H

#include "cmdParser.h"

CmdClass(NP3SolveCmd);
CmdClass(NP3VerifyCmd);
CmdClass(NP3BindCmd);
// CmdClass(MTNewCmd);
// CmdClass(MTDeleteCmd);
// CmdClass(MTPrintCmd);

#endif // SAT_CMD_H
