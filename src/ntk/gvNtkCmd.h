#ifndef GV_NTK_CMD_H
#define GV_NTK_CMD_H

#include "gvCmdMgr.h"

GV_COMMAND(GVSetEngineCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(GVReadDesignCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(GVPrintInfoCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(GVFile2AigCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(GVYosysOriginalCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(GVFile2BlifCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(GVWriteAigCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(GVBlastNtkCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(GVPrintAigCmd, GV_CMD_TYPE_NETWORK);

#endif