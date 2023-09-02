cirGate.o: cirGate.cpp cirGate.h cirDef.h ../../include/util.h \
 ../../include/myUsage.h ../../include/rnGen.h
cirMgr.o: cirMgr.cpp cirMgr.h sat.h cadical.hpp myGlobal.h ntkMgr.h \
 ../../include/util.h ../../include/myUsage.h ../../include/rnGen.h \
 cirDef.h cirGate.h
gvBitVec.o: gvBitVec.cpp gvBitVec.h ../../include/gvIntType.h \
 ../../include/gmpxx.h ../../include/gmp.h
gvSatCmd.o: gvSatCmd.cpp gvSatCmd.h ../../include/gvCmdMgr.h sat.h \
 cadical.hpp myGlobal.h solverMgr.h cirMgr.h ntkMgr.h \
 ../../include/util.h ../../include/myUsage.h ../../include/rnGen.h \
 cirDef.h cirGate.h satMgr.h
ntkMgr.o: ntkMgr.cpp ntkMgr.h cirMgr.h sat.h cadical.hpp myGlobal.h \
 cirGate.h cirDef.h ../../include/util.h ../../include/myUsage.h \
 ../../include/rnGen.h
sat.o: sat.cpp sat.h cadical.hpp myGlobal.h
satMgr.o: satMgr.cpp satMgr.h cirMgr.h sat.h cadical.hpp myGlobal.h \
 ntkMgr.h ../../include/util.h ../../include/myUsage.h \
 ../../include/rnGen.h cirDef.h cirGate.h
solverMgr.o: solverMgr.cpp solverMgr.h cirMgr.h sat.h cadical.hpp \
 myGlobal.h ntkMgr.h ../../include/util.h ../../include/myUsage.h \
 ../../include/rnGen.h cirDef.h cirGate.h satMgr.h
