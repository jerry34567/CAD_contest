REFPKGS  = cmd
# SRCPKGS  = mem util
SRCPKGS  = sat util 
LIBPKGS  = $(REFPKGS) $(SRCPKGS)
# MAIN     = main
MAIN     = main

LIBS     = $(addprefix -l, $(LIBPKGS))
SRCLIBS  = $(addsuffix .a, $(addprefix lib, $(SRCPKGS)))

EXEC     = satTest

.PHONY : all

all:   EXEC  = satTest
# debug: EXEC  = memTest.debug

all:  DEBUG_FLAG =
# debug:DEBUG_FLAG = -DMEM_DEBUG

all: libs main

libs:
	@for pkg in $(SRCPKGS); \
	do \
		echo "Checking $$pkg..."; \
		cd src/$$pkg; make -f make.$$pkg --no-print-directory \
                                   DEBUG_FLAG=$(DEBUG_FLAG) PKGNAME=$$pkg; \
		cd ../..; \
	done

main:
	@echo "Checking $(MAIN)..."
	@cd src/$(MAIN);  \
            make -f make.$(MAIN) --no-print-directory INCLIB="$(LIBS)" \
                 EXEC=$(EXEC);
	@ln -fs bin/$(EXEC) .
#	@strip bin/$(EXEC)


# satTest: clean File.o Proof.o Solver.o satTest.o
# 	g++ -o $@ -std=c++11 -g File.o Proof.o Solver.o satTest.o

# File.o: File.cpp
# 	g++ -c -std=c++11 -g File.cpp

# Proof.o: Proof.cpp
# 	g++ -c -std=c++11 -g Proof.cpp

# Solve.o: Solver.cpp
# 	g++ -c -std=c++11 -g Solver.cpp

# satTest.o: satTest.cpp
# 	g++ -c -std=c++11 -g satTest.cpp

clean:
	rm -f *.o satTest tags
	@for pkg in $(SRCPKGS); \
	do \
		echo "Cleaning $$pkg..."; \
		cd src/$$pkg; make -f make.$$pkg --no-print-directory \
                   PKGNAME=$$pkg clean; \
                cd ../..; \
	done
	@echo "Cleaning $(MAIN)..."
	@cd src/$(MAIN); make -f make.$(MAIN) --no-print-directory clean
	@echo "Removing $(SRCLIBS)..."
	@cd lib; rm -f $(SRCLIBS)
	@echo "Removing $(EXEC)..."
	@rm -rf bin/$(EXEC)* 

linux18 linux16 mac:
	@for pkg in $(REFPKGS); \
	do \
		cd lib; ln -sf lib$$pkg-$@.a lib$$pkg.a; cd ..; \
        done
# @cd ref; ln -sf memTest-$@ memTest; \
	# 	 ln -sf memTest-$@.debug memTest.debug; cd ..;