File.o: File.cpp File.h Global.h
Proof.o: Proof.cpp Proof.h SolverTypes.h Global.h File.h Sort.h
satCmd.o: satCmd.cpp satCmd.h ../../include/cmdParser.h \
 ../../include/cmdCharDef.h sat.h Solver.h SolverTypes.h Global.h \
 VarOrder.h Heap.h Proof.h File.h satMgr.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h
sat.o: sat.cpp sat.h Solver.h SolverTypes.h Global.h VarOrder.h Heap.h \
 Proof.h File.h
satTest_ETold.o: satTest_ETold.cpp sat.h Solver.h SolverTypes.h Global.h \
 VarOrder.h Heap.h Proof.h File.h
satTest_NP3classMIMO.o: satTest_NP3classMIMO.cpp sat.h Solver.h \
 SolverTypes.h Global.h VarOrder.h Heap.h Proof.h File.h
Solver.o: Solver.cpp Solver.h SolverTypes.h Global.h VarOrder.h Heap.h \
 Proof.h File.h Sort.h
