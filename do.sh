#!/bin/bash
read -p " > Please enter the case you want to solve: " case
./gv ./release/$case/input
./aigtoaig top1.aig $case"_1.aag"
./aigtoaig top2.aig $case"_2.aag"

# move *.aag to SAT/test_NP3/files/aag
mv $case"_1.aag" SAT/test_NP3/files/aag
mv $case"_2.aag" SAT/test_NP3/files/aag

#rename .cnf and move them to SAT/test_NP3/files/cnf
mv top1.cnf $case"_1.cnf"
mv top2.cnf $case"_2.cnf"
mv $case"_1.cnf" SAT/test_NP3/files/cnf
mv $case"_2.cnf" SAT/test_NP3/files/cnf

#rename map files and move them to SAT/test_NP3/files/map
mv map1 $case"map1"
mv map2 $case"map2"
mv $case"map1" SAT/test_NP3/files/map
mv $case"map2" SAT/test_NP3/files/map

#remove those generated files in ./CAD_contest
rm top1.aig top2.aig

cd SAT/test_NP3/
./satTest -f do/do1
 