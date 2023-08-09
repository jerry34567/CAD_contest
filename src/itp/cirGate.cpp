/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
size_t CirGate::_globalRef_s = 0;
void CirGate::reportGate()
{
   cout << "==================================================" << endl;
   string _toprint = "";
   if (_gatetype == UNDEF_GATE)
   {
      _toprint = string("UNDEF(") + to_string(_id) + "), " + "line 0";
   }
   else
   {
      if (!_mnemonic.empty())
         _toprint = getTypeStr() + '(' + to_string(_id) + ")\"" + _mnemonic + "\", line " + to_string(_lineNo); // the later '=' sticks at the end?????????
      else
         _toprint = getTypeStr() + '(' + to_string(_id) + ")" + ", line " + to_string(_lineNo);
   }
   cout << "= " << left << setw(47) << _toprint << "=" << endl;
   cout << "==================================================" << endl;
}

void CirGate::reportFanin(int level)
{
   assert(level >= 0);
   setGlobalRef();
   vector<CirGate *> _dfslist;
   dfsTraversal(_dfslist, 1);                        // isInv(x) = 1 -> !x, = 0 -> x
   unordered_map<size_t, pair<size_t, bool>> _depth; // (gateId, (depth, isInv))
   if (_dfslist.empty())
   {
      cout << getTypeStr() << ' ' << _id << endl;
      return;
   }
   _depth.insert(pair<size_t, pair<size_t, bool>>(_dfslist.front()->id(), pair<size_t, size_t>(0, 0)));
   // for (auto i : _dfslist)
   //    cout << i->id() << endl;
   // cout << "=======\n";
   for (size_t i = 0, n = _dfslist.size(); i < n; ++i)
   {
      size_t _depthI = _depth.at(_dfslist[i]->id()).first;
      CirGate *_dfslistI = _dfslist[i];
      if (_depth.at(_dfslistI->id()).first > (size_t)level)
         continue;

      cout << setw(2 * _depthI + 1) << setfill(' ') << '\0' << (_depth.at(_dfslist[i]->id()).second ? "!" : "") << (_dfslistI->getGateType() == INIT_GATE ? "UNDEF" : _dfslistI->getTypeStr()) << ' ' << _dfslistI->id() << endl;

      if (_dfslistI->getGateType() != PI_GATE && _dfslistI->getGateType() != UNDEF_GATE && _dfslistI->getGateType() != CONST_GATE)
      {
         // cout << "my faninList = ";
         // for (auto k : _dfslistI->faninList())
         //    cout << k.gate()->id() << "; ";
         // cout << "===";
         if ((!(_depth.insert(pair<size_t, pair<size_t, bool>>(_dfslistI->faninList().front().gate()->id(), pair<size_t, size_t>(_depthI + 1, _dfslistI->faninList().front().isInv()))).second) && _dfslistI->getGateType() != PO_GATE) || _dfslistI->faninList().front().gate()->getGateType() == UNDEF_GATE)
         {
            if (_depthI + 1 <= (size_t)level)
            {
               size_t _depthL = _depth.at(_dfslistI->faninList().front().gate()->id()).first;
               cout << setw(2 * _depthL + 1) << setfill(' ') << '\0' << (_depth.at(_dfslistI->faninList().front().gate()->id()).second ? "!" : "")
                    << _dfslistI->faninList().front().gate()->getTypeStr() << ' ' << _dfslistI->faninList().front().gate()->id() << (((_dfslistI->faninList().front().gate()->getGateType() == AIG_GATE) && (_depthL < (size_t)level)) ? " (*)" : "") << endl;
            }
         }
         if ((!(_depth.insert(pair<size_t, pair<size_t, bool>>(_dfslistI->faninList().back().gate()->id(), pair<size_t, size_t>(_depthI + 1, _dfslistI->faninList().back().isInv()))).second) && _dfslistI->getGateType() != PO_GATE) || _dfslistI->faninList().back().gate()->getGateType() == UNDEF_GATE)
         {
            if (_depthI + 1 <= (size_t)level)
            {
               size_t _depthL = _depth.at(_dfslistI->faninList().back().gate()->id()).first;
               cout << setw(2 * _depthL + 1) << setfill(' ') << '\0' << (_depth.at(_dfslistI->faninList().back().gate()->id()).second ? "!" : "")
                    << _dfslistI->faninList().back().gate()->getTypeStr() << ' ' << _dfslistI->faninList().back().gate()->id() << (((_dfslistI->faninList().back().gate()->getGateType() == AIG_GATE) && (_depthL < (size_t)level)) ? " (*)" : "") << endl;
            }
         }
      }
   }
   // for (auto i : _dfslist)
   //    cout << i->_id << endl;
}

void CirGate::reportFanout(int level)
{
   assert(level >= 0);
   setGlobalRef();
   vector<CirGate *> _dfslist;
   dfsTraversal(_dfslist, 1, 1);                     // isInv(x) = 1 -> !x, = 0 -> x
   unordered_map<size_t, pair<size_t, bool>> _depth; // (gateId, (depth, isInv))
   if (_dfslist.empty())
   {
      cout << getTypeStr() << ' ' << _id << endl;
      return;
   }
   _depth.insert(pair<size_t, pair<size_t, bool>>(_dfslist.front()->id(), pair<size_t, size_t>(0, 0)));

   for (size_t i = 0, n = _dfslist.size(); i < n; ++i)
   {
      size_t _depthI = _depth.at(_dfslist[i]->id()).first;
      CirGate *_dfslistI = _dfslist[i];
      if (_depth.at(_dfslistI->id()).first > (size_t)level)
         continue;

      cout << setw(2 * _depthI + 1) << setfill(' ') << '\0' << (_depth.at(_dfslist[i]->id()).second ? "!" : "") << (_dfslistI->getGateTypeI() == INIT_GATE ? "UNDEF" : _dfslistI->getTypeStr()) << ' ' << _dfslistI->id() << endl;

      if (_dfslistI->getGateTypeI() != PO_GATE && _dfslistI->getGateTypeI() != UNDEF_GATE && _dfslistI->getGateTypeI() != CONST_GATE)
      {
         // cout << "my faninList = ";
         // for (auto k : _dfslistI->faninList())
         //    cout << k.gate()->id() << "; ";
         // cout << "===";
         if ((!(_depth.insert(pair<size_t, pair<size_t, bool>>(_dfslistI->fanoutList().front().gate()->id(), pair<size_t, size_t>(_depthI + 1, _dfslistI->fanoutList().front().isInv()))).second) && _dfslistI->getGateTypeI() != PI_GATE && _dfslistI->getGateTypeI() != INIT_GATE) || _dfslistI->fanoutList().front().gate()->getGateTypeI() == UNDEF_GATE)
         {
            if (_depthI + 1 <= (size_t)level)
            {
               size_t _depthL = _depth.at(_dfslistI->fanoutList().front().gate()->id()).first;
               cout << setw(2 * _depthL + 1) << setfill(' ') << '\0' << (_depth.at(_dfslistI->fanoutList().front().gate()->id()).second ? "!" : "")
                    << _dfslistI->fanoutList().front().gate()->getTypeStr() << ' ' << _dfslistI->fanoutList().front().gate()->id() << (((_dfslistI->fanoutList().front().gate()->getGateTypeI() == AIG_GATE) && (_depthL < (size_t)level)) ? " (*)" : "") << endl;
            }
         }
         if ((!(_depth.insert(pair<size_t, pair<size_t, bool>>(_dfslistI->fanoutList().back().gate()->id(), pair<size_t, size_t>(_depthI + 1, _dfslistI->fanoutList().back().isInv()))).second) && _dfslistI->getGateTypeI() != PI_GATE && _dfslistI->getGateTypeI() != INIT_GATE) || _dfslistI->fanoutList().back().gate()->getGateTypeI() == UNDEF_GATE)
         {
            if (_depthI + 1 <= (size_t)level)
            {
               size_t _depthL = _depth.at(_dfslistI->fanoutList().back().gate()->id()).first;
               cout << setw(2 * _depthL + 1) << setfill(' ') << '\0' << (_depth.at(_dfslistI->fanoutList().back().gate()->id()).second ? "!" : "")
                    << _dfslistI->fanoutList().back().gate()->getTypeStr() << ' ' << _dfslistI->fanoutList().back().gate()->id() << (((_dfslistI->fanoutList().back().gate()->getGateTypeI() == AIG_GATE) && (_depthL < (size_t)level)) ? " (*)" : "") << endl;
            }
         }
      }
   }
   // for (auto i : _dfslist)
   //    cout << i->_id << endl;
}
size_t CirGate::simulate(){
   if(_gatetype == AIG_GATE){
      _pattern = _faninList.front().gate()->pattern() & _faninList.back().gate()->pattern();
      int _f1 = _find(_faninList.front().gate()->fanoutList()), _f2 = _find(_faninList.back().gate()->fanoutList());
      assert(_f1 != -1);
      assert(_f2 != -1 );
      assert(_f1 == _f2);
   }
}
