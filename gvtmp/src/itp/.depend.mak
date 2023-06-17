cirMgr.o: cirMgr.cpp cirMgr.h sat.h Solver.h SolverTypes.h Global.h \
 VarOrder.h Heap.h Proof.h File.h
File.o: File.cpp File.h Global.h
gvBitVec.o: gvBitVec.cpp gvBitVec.h ../../include/gvIntType.h \
 ../../include/gmpxx.h ../../include/gmp.h
gvSatCmd.o: gvSatCmd.cpp gvSatCmd.h ../../include/gvCmdMgr.h sat.h \
 Solver.h SolverTypes.h Global.h VarOrder.h Heap.h Proof.h File.h \
 solverMgr.h cirMgr.h satMgr.h ../../include/util.h ../../include/rnGen.h \
 ../../include/myUsage.h
Proof.o: Proof.cpp Proof.h SolverTypes.h Global.h File.h Sort.h
reader.o: reader.cpp reader.h
sat.o: sat.cpp sat.h Solver.h SolverTypes.h Global.h VarOrder.h Heap.h \
 Proof.h File.h
satMgr.o: satMgr.cpp satMgr.h cirMgr.h sat.h Solver.h SolverTypes.h \
 Global.h VarOrder.h Heap.h Proof.h File.h
Solver.o: Solver.cpp Solver.h SolverTypes.h Global.h VarOrder.h Heap.h \
 Proof.h File.h Sort.h
solverMgr.o: solverMgr.cpp solverMgr.h cirMgr.h sat.h Solver.h \
 SolverTypes.h Global.h VarOrder.h Heap.h Proof.h File.h satMgr.h
