itp.d: ../../include/gvSatCmd.h ../../include/sat.h ../../include/VarOrder.h ../../include/cirMgr.h ../../include/satMgr.h ../../include/solverMgr.h 
../../include/gvSatCmd.h: gvSatCmd.h
	@rm -f ../../include/gvSatCmd.h
	@ln -fs ../src/itp/gvSatCmd.h ../../include/gvSatCmd.h
../../include/sat.h: sat.h
	@rm -f ../../include/sat.h
	@ln -fs ../src/itp/sat.h ../../include/sat.h
../../include/VarOrder.h: VarOrder.h
	@rm -f ../../include/VarOrder.h
	@ln -fs ../src/itp/VarOrder.h ../../include/VarOrder.h
../../include/cirMgr.h: cirMgr.h
	@rm -f ../../include/cirMgr.h
	@ln -fs ../src/itp/cirMgr.h ../../include/cirMgr.h
../../include/satMgr.h: satMgr.h
	@rm -f ../../include/satMgr.h
	@ln -fs ../src/itp/satMgr.h ../../include/satMgr.h
../../include/solverMgr.h: solverMgr.h
	@rm -f ../../include/solverMgr.h
	@ln -fs ../src/itp/solverMgr.h ../../include/solverMgr.h
