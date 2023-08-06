cirMgr.o: cirMgr.cpp cirMgr.h sat.h cadical.hpp myGlobal.h
gvBitVec.o: gvBitVec.cpp gvBitVec.h ../../include/gvIntType.h \
 ../../include/gmpxx.h ../../include/gmp.h
gvSatCmd.o: gvSatCmd.cpp gvSatCmd.h ../../include/gvCmdMgr.h sat.h \
 cadical.hpp myGlobal.h solverMgr.h cirMgr.h satMgr.h \
 ../../include/util.h ../../include/myUsage.h ../../include/rnGen.h
sat.o: sat.cpp sat.h cadical.hpp myGlobal.h
satMgr.o: satMgr.cpp satMgr.h cirMgr.h sat.h cadical.hpp myGlobal.h
solverMgr.o: solverMgr.cpp solverMgr.h cirMgr.h sat.h cadical.hpp \
 myGlobal.h satMgr.h
