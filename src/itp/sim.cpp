/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include "sim.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
// Simulator *simulator = 0;

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool Simulator::readCircuit(const string &fileName)
{
   ifstream fin(fileName);
   char buf1[4];
   char *buf7 = new char[100]{0};
   string buf5, buf6;
   size_t buf2, buf3, buf4;
   size_t curLineNo = 2;
   buf2 = buf3 = buf4 = -1;
   fin >> buf1;
   for (size_t i = 0; i < 5; ++i)
      fin >> _MILOA[i];
   // no yet deal with error. E.g , input number error, output number error....
   _aagPOs = new size_t[_MILOA[3]];

   // construct the aig gate for all the used gate first
   size_t _secondLine = fin.tellg();
   for (size_t i = 0, n = _MILOA[1] + _MILOA[3] + 3 * _MILOA[4]; i < n; ++i)
   {
      fin >> buf2;
      _totalGates.insert(pair<size_t, CirGate *>(buf2 / 2, new CirGate(buf2 / 2)));
      _totalGateVs.insert(pair<size_t, AigGateV>(buf2, AigGateV((_totalGates.at(buf2 / 2)), buf2 % 2)));
      if (buf2 / 2 == 0) // const 0 & 1
      {
         _totalGates.at(buf2 / 2)->setGateType(CONST_GATE);
         _totalGates.at(buf2 / 2)->setGateTypeI(CONST_GATE);
         (_totalGates.at(buf2 / 2))->faninList().push_back(AigGateV(0, buf2 % 2));
      }
   }

   fin.seekg(_secondLine);
   for (size_t i = 0, n = _MILOA[1]; i < n; ++i) // PIs
   {
      fin >> buf2;
      _PIs.push_back(_totalGates.at(buf2 / 2));
      _PIs.back()->setLineNo(curLineNo++);
      if (_PIs.back()->getGateType() != CONST_GATE)
         _PIs.back()->setGateType(PI_GATE);
      if (_PIs.back()->getGateTypeI() != CONST_GATE)
         _PIs.back()->setGateTypeI(PI_GATE);
      (_PIs.back())->faninList().push_back(AigGateV(0, buf2 % 2));
   }
   for (size_t i = 0, n = _MILOA[3]; i < n; ++i) // POs
   {
      fin >> buf2;
      size_t _index = _MILOA[0] + _POs.size() + 1;
      _POs.push_back(new CirGate(_index));
      _totalGates.insert(pair<size_t, CirGate *>(_index, _POs.back()));
      _POs.back()->setLineNo(curLineNo++);
      _POs.back()->setGateType(PO_GATE);
      _POs.back()->setGateTypeI(PO_GATE);
      (_POs.back())->faninList().push_back(_totalGateVs.at(buf2));
      (_POs.back())->fanoutList().push_back(AigGateV(0, buf2 % 2));
      _totalGateVs.at(buf2).gate()->fanoutList().push_back(AigGateV(_POs.back(), buf2 % 2));
   }

   // size_t aigBegin = fin.tellg();
   for (size_t i = 0, n = _MILOA[4]; i < n; ++i)
   {
      fin >> buf2 >> buf3 >> buf4; // output input1 input2
      _totalGates.at(buf2 / 2)->setLineNo(curLineNo++);
      _totalGates.at(buf2 / 2)->setGateType(AIG_GATE);
      _totalGates.at(buf2 / 2)->setGateTypeI(AIG_GATE);
      (_totalGates.at(buf2 / 2))->faninList().push_back(_totalGateVs.at(buf3));
      (_totalGates.at(buf2 / 2))->faninList().push_back(_totalGateVs.at(buf4));
      (_totalGates.at(buf3 / 2))->fanoutList().push_back(_totalGateVs.at(buf2));
      (_totalGates.at(buf4 / 2))->fanoutList().push_back(_totalGateVs.at(buf2));
   }
   // fin.seekg(aigBegin);
   // for (size_t i = 0, n = _MILOA[4]; i < n; ++i)
   // {
   //    fin >> buf2 >> buf3 >> buf4; // output input1 input2
   //    (_totalGates[buf2 / 2])->faninList().push_back(AigGateV((findGate(buf3 / 2, 1)), buf3 % 2));
   //    (_totalGates[buf2 / 2])->faninList().push_back(AigGateV((findGate(buf4 / 2, 1)), buf4 % 2));
   // }
   // for (size_t i = 0, n = _POs.size(); i < n; ++i)
   //    (_POs[i])->faninList().push_back(AigGateV((findGate(_aagPOs[i] / 2, 0)), _aagPOs[i] % 2));
   // delete[] _aagPOs;
   // for (auto i : _PIs)
   //    cout << ((CirGate *)i)->id() << endl;
   // for (auto i : _POs)
   //    cout << ((CirGate *)i)->id() << endl;
   // for (auto i : _totalGates)
   //    cout << ((CirGate *)i)->id() << endl;
   // fin >> buf2;
   // cout << "buf2 = " << (int)buf2 << endl;
   size_t _indexI = 0, _indexO = 0;
   while (fin.getline(buf7, 100) && buf7[0] != 'c')
   {
      if (buf7[0] == ' ' || buf7[0] == '\n' || buf7[0] == '\r' || (int)buf7[0] == 0)
         continue;
      string _s(buf7);
      cout << _s << endl;
      _s = _s.substr(3, _s.find_last_not_of(' ') + 1);
      if (buf7[0] == 'i')
      {
         (_PIs[(buf7[1] - '0')])->setMnemonic(_s);
         // cout << "PIs mnemonic = " << (_PIs[(buf7[1] - '0')])->mnemonic() << endl;
      }
      else
      {
         (_POs[(buf7[1] - '0')])->setMnemonic(_s);
         // cout << "POs mnemonic = " << (_POs[(buf7[1] - '0')])->mnemonic() << endl;
      }

      // cout << "" << (_PIs[1])->mnemonic() << endl;
      delete[] buf7;
      buf7 = new char[100]{0};
      // while (p != NULL)
      // {
      //    cout << _indexI++ << endl;
      //    printf("%s\n", p);
      //    p = strtok(NULL, d);
      // }
      // if (buf1[0] == 'i')
      //    ((CirGate *)_PIs[_indexI++])->setMnemonic(buf5);
      // else
      //    ((CirGate *)_POs[_indexO++])->setMnemonic(buf5);
   }
   while (fin.getline(buf7, 100))
      setComment(comment() + '\n' + buf7);
   dfsTraversal(_POs, _dfsList_O);
   // for (auto i : _POs)
   // {
   //    for (auto j : ((i))->faninList().front().gate()->faninList())
   //       cout << j.gate()->mnemonic() << " ; ";
   //    cout << endl;
   // }
   cout << "dfsList = \n";
   for(auto i : _dfsList_O){
      cout << i->mnemonic()  << " , " << i->getTypeStr() << " ; " << endl;
   }
   return true;
}

/**********************************************************/
/*   class Simulator member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void Simulator::printSummary() const
{
   cout << "\nCircuit Statistics\n==================\n  PI" << setw(12) << _MILOA[1] << "\n  PO"
        << setw(12) << _MILOA[3] << "\n  AIG" << setw(11) << _MILOA[4] << endl
        << "------------------"
        << "\n  Total" << setw(9) << _MILOA[1] + _MILOA[3] + _MILOA[4] << endl;
}

void Simulator::printNetlist() const
{
   cout << endl;
   for (size_t i = 0, n = _dfsList_O.size(); i < n; ++i)
   {
      CirGate *_it = _dfsList_O[i];
      GateType _type = _it->getGateType();
      if (_type == CONST_GATE)
         cout << "[" << i << "] " << _it->getTypeStr() << '0'; // (_it)->faninList().back().isInv() ? "1" : "0";
      else
      {
         cout << "[" << i << "] " << _it->getTypeStr() << ((_type == AIG_GATE) ? " " : "  ") << (_it)->id() << " ";
         if (_type != PI_GATE)
            for (auto j : (_it)->faninList())
               cout << (j.gate()->getGateType() == UNDEF_GATE ? "*" : "") << (j.isInv() ? "!" : "") << j.gate()->id() << " ";
         if ((_it)->mnemonic() != "")
            cout << '(' << (_it)->mnemonic() << ')';
      }
      cout << endl;
   }
   // for (auto i : _dfsList_O)
   //    cout << (i)->id() << endl;
}

void Simulator::printPIs() const
{
   cout << "PIs of the circuit: ";
   for (size_t i = 0, n = _PIs.size(); i < n; ++i)
      cout << (_PIs[i])->id() << ' ';
   cout << endl;
}

void Simulator::printPOs() const
{
   cout << "POs of the circuit: ";
   for (size_t i = 0, n = _POs.size(); i < n; ++i)
      cout << (_POs[i])->id() << ' ';
   cout << endl;
}

void Simulator::printFloatGates() const
{
   map<size_t, pair<bool, bool>> _floatGates; //(gateID, (reachablefromPO, reachablefromPI))   // reachable = 1, unreachable = 0, i.e., UNDEF
   // (0, 0)   both floating & def_but_not_used
   // (0, 1)   d_b_n_u
   // (1, 0)   floating
   for (auto i : _totalGates)
   {
      bool reachableFromPO = (i.second->getGateType() != INIT_GATE) && (i.second->getGateType() != UNDEF_GATE);
      bool reachableFromPI = !i.second->fanoutList().empty();
      _floatGates.insert(pair<size_t, pair<bool, bool>>(i.second->id(), pair<bool, bool>(reachableFromPO, reachableFromPI)));
   }
   vector<size_t> _floating, _dbnu;
   for (auto i : _floatGates)
   {
      if (i.second.first != 1)
         for (auto j : _totalGates.at(i.first)->fanoutList())
            _floating.push_back(j.gate()->id());
      if (i.second.second != 1)
         _dbnu.push_back(i.first);
   }
   sort(_floating.begin(), _floating.end());
   sort(_dbnu.begin(), _dbnu.end());
   cout << "Gates with floating fanin(s): ";
   for (auto i : _floating)
      cout << i << ' ';
   cout << endl;
   cout << "Gates defined but not used  : ";
   for (auto i : _dbnu)
      cout << i << ' ';
   cout << endl;
}

void Simulator::writeAag(ostream &outfile) const
{
   outfile << "aag ";
   for (size_t i = 0; i < 3; ++i)
      outfile << _MILOA[i] << ' ';
   outfile << _MILOA[3] << ' ';
   size_t _A = 0; // number of aig gate
   for (auto i : _dfsList_O)
      if (i->getGateType() == AIG_GATE)
         ++_A;
   outfile << _A << '\n';
   for (auto i : _PIs)
      outfile << i->id() * 2 << '\n';
   for (auto i : _POs)
      outfile << i->faninList()[0].gate()->id() * 2 + i->faninList()[0].isInv() << '\n';
   for (auto i : _dfsList_O)
      if (i->getGateType() == AIG_GATE)
      {
         outfile << i->id() * 2 << ' ';
         for (size_t j = 0, n = i->faninList().size(); j < n - 1; ++j)
            outfile << i->faninList()[j].gate()->id() * 2 + i->faninList()[j].isInv() << ' ';
         outfile << i->faninList().back().gate()->id() * 2 + i->faninList().back().isInv() << '\n';
      }
   for (size_t i = 0, j = 0, n = _PIs.size(); i < n; ++i)
   {
      if (_PIs[i]->mnemonic() != "")
      {
         outfile << 'i' << j++ << ' ' << _PIs[i]->mnemonic() << '\n';
      }
   }
   for (size_t i = 0, j = 0, n = _POs.size(); i < n; ++i)
   {
      if (_POs[i]->mnemonic() != "")
      {
         outfile << 'o' << j++ << ' ' << _POs[i]->mnemonic() << '\n';
      }
   }
}

void Simulator::simulate(){
   for(size_t i = 0, n = _dfsList_O.size(); i < n; ++i){
      _dfsList_O[i]->simulate();
   }
}
