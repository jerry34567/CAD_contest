/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SIM_H
#define SIM_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include "cirGate.h"
#include "../util/util.h"
#include "cirDef.h"

using namespace std;


// TODO: Define your own data members and member functions
class Simulator
{
public:
  Simulator() : _comment("")
  {
    _MILOA = new size_t[5]{0};
  }
  ~Simulator() { delete[] _MILOA;}

  // Access functions
  // return '0' if "gid" corresponds to an undefined gate.
  CirGate *getGate(unsigned gid) const
  {
    try
    {
      return _totalGates.at(gid);
    }
    catch (std::exception &e)
    {
      return 0;
    }
  }

  // Member functions about circuit construction
  bool readCircuit(const string &);

  // Member functions about circuit reporting
  void printSummary() const;
  void printNetlist() const;
  void printPIs() const;
  void printPOs() const;
  void printFloatGates() const;
  void writeAag(ostream &) const;
  void dfsTraversal(const vector<CirGate *> &sinkList, vector<CirGate *> &_dfsList)
  {
    CirGate::setGlobalRef();
    // for (auto i : _POs)
    //   cout << dynamic_cast<CirGate *>(i)->faninList()[0].gate()->id() << endl;
    // cout << endl;
    for (size_t i = 0, n = sinkList.size(); i < n; ++i)
      dynamic_cast<CirGate *>(sinkList[i])->dfsTraversal(_dfsList);
  }
  CirGate *findGate(size_t _id, bool _IO) // _IO = 0 -> _POs, _IO = 1 -> _PIs
  {
    if (!_IO && _totalGates[_id] != 0)
      return _totalGates[_id];
    for (auto i : _PIs)
      if (dynamic_cast<CirGate *>(i)->id() == _id)
        return i;
    return 0;
  }
  void setComment(string _s) { _comment = _s; }
  string comment() { return _comment; }
  vector<CirGate *> &dfsList() { return _dfsList_O; }
  vector<CirGate *> &dfsList_I() { return _dfsList_I; }
  void simulate();

private:
  size_t *_MILOA;
  vector<CirGate *> _PIs;
  vector<CirGate *> _POs;
  map<size_t, CirGate *> _totalGates; // total gates
  map<size_t, AigGateV> _totalGateVs; // total gateVs
  vector<CirGate *> _dfsList_O;                 // dfs from PO
  vector<CirGate *> _dfsList_I;                 // dfs from PI
  size_t *_aagPOs;
  string _comment;
};

#endif // CIR_MGR_H
