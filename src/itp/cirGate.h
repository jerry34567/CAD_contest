/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"

using namespace std;

class CirGate;
// class AigGate;
class AigGateV;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class AigGateV
{
public:
#define NEG 0x1
  AigGateV(CirGate *g, size_t phase) : _gateV(size_t(g) + phase){};
  CirGate *gate() const { return (CirGate *)(_gateV & ~size_t(NEG)); };
  bool isInv() const { return (_gateV & NEG); }

private:
  size_t _gateV;
};
class CirGate
{
public:
  CirGate(size_t id) : _ref(0), _id(id), _mnemonic(""), _gatetype(INIT_GATE), _gatetype_I(INIT_GATE), _pattern(0) {}
  virtual ~CirGate()
  {
    _faninList.clear();
    _fanoutList.clear();
  }

  // Basic access methods
  string _getTypeStr(GateType _g) const
  {
    switch (_g)
    {
    case UNDEF_GATE:
      return "UNDEF";
      break;
    case PI_GATE:
      return "PI";
      break;
    case PO_GATE:
      return "PO";
      break;
    case AIG_GATE:
      return "AIG";
      break;
    case CONST_GATE:
      return "CONST";
      break;
    case INIT_GATE:
      return "INIT";
      break;
    default:
      return "getTypeStr Error";
      break;
    }
  }
  string getTypeStr() const { return _getTypeStr(_gatetype); };
  string getTypeStrI() const { return _getTypeStr(_gatetype_I); };

  void setGateType(GateType _t) { _gatetype = _t; }
  void setGateTypeI(GateType _t) { _gatetype_I = _t; }
  unsigned getLineNo() const { return _lineNo; }
  void setLineNo(size_t _l) { _lineNo = _l; }
  GateType getGateType() { return _gatetype; }
  GateType getGateTypeI() { return _gatetype_I; }

  // Printing functions
  void printGate() const { return; };
  void reportGate();
  void reportFanin(int level);
  void reportFanout(int level);

  bool isGlobalRef() { return (_ref == _globalRef_s); }
  void setToGlobalRef() { _ref = _globalRef_s; }
  static void setGlobalRef() { ++_globalRef_s; }

  vector<AigGateV> &faninList() { return _faninList; }
  vector<AigGateV> &fanoutList() { return _fanoutList; }
  size_t id() { return _id; }
  void setMnemonic(string _s) { _mnemonic = _s; }
  string mnemonic() { return _mnemonic; }
  size_t pattern() const { return _pattern; }
  void   setPattern(const size_t _p) { _pattern = _p; }
  size_t simulate();

  void dfsTraversal(vector<CirGate *> &dfsList, bool _postPre = 0, bool _inOut = 0) // _postPre : 0 -> postOrder, 1 -> preOrder
  // _inOut : 0 -> from PO, 1 -> from PI
  {
    vector<AigGateV> &_list = !_inOut ? _faninList : _fanoutList;
    if (_list.empty())
    {
      if (!_inOut)
        setGateType(UNDEF_GATE);
      else
        setGateTypeI(UNDEF_GATE);
      return;
    }
    if (_postPre)
      dfsList.push_back(this);
    for (vector<AigGateV>::iterator it = _list.begin(), n = _list.end(); it != n; ++it)
    {
      if (it->gate() != 0 && !it->gate()->isGlobalRef())
      {
        it->gate()->setToGlobalRef();
        it->gate()->dfsTraversal(dfsList, _postPre, _inOut);
      }
    }
    if (!_postPre)
      dfsList.push_back(this);
  }

private:
  size_t _lineNo;
  GateType _gatetype;
  GateType _gatetype_I; // gatetype look from PI

  vector<AigGateV> _faninList;
  vector<AigGateV> _fanoutList;
  size_t _ref;
  size_t _id;
  static size_t _globalRef_s;
  string _mnemonic;
  size_t _pattern;
  int _find(vector<AigGateV>& _f){
    for(size_t i = 0, n = _f.size(); i < n; ++i)
      if(_f[i].gate() == this)
        return (_f[i].isInv()? 1 : 0);
    return -1;
  }
protected:
};

// class AigGate : public CirGate
// {
// public:
//   AigGate() {}
//   ~AigGate()
//   { //_faninList.clear(); }
//   }
//   // bool isGlobalRef() { return (_ref == _globalRef_s); }
//   // void setToGlobalRef() { _ref = _globalRef_s; }
//   // static void setGlobalRef() { ++_globalRef_s; }
//   // void printGate() const { return; };
//   // vector<AigGateV> &faninList() { return _faninList; }
//   // size_t id() { return _id; }
//   // void setMnemonic(string _s) { _mnemonic = _s; }
//   // string mnemonic() { return _mnemonic; }

//   // void dfsTraversal(vector<CirGate *> &dfsList)
//   // {
//   //   if (_faninList.empty())
//   //   {
//   //     setGateType(UNDEF_GATE);
//   //     return;
//   //   }
//   //   for (vector<AigGateV>::iterator it = _faninList.begin(), n = _faninList.end(); it != n; ++it)
//   //   {
//   //     // AigGateV &it = _faninList[i];
//   //     if (it->gate() != 0 && !it->gate()->isGlobalRef())
//   //     {
//   //       it->gate()->setToGlobalRef();
//   //       it->gate()->dfsTraversal(dfsList);
//   //     }
//   //   }
//   //   dfsList.push_back(this);
//   // }

// private:
//   // vector<AigGateV> _faninList;
//   // size_t _ref;
//   // size_t _id;
//   // static size_t _globalRef_s;
//   // string _mnemonic;
// };

#endif // CIR_GATE_H
