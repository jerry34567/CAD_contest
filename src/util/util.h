/****************************************************************************
  FileName     [ util.h ]
  PackageName  [ util ]
  Synopsis     [ Define the prototypes of the exported utility functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef UTIL_H
#define UTIL_H

#include "myUsage.h"
#include "rnGen.h"
#include <istream>
#include <vector>

using namespace std;

// Extern global variable defined in util.cpp
extern RandomNumGen rnGen;
extern MyUsage      myUsage;

// In myString.cpp
extern int    myStrNCmp(const string& s1, const string& s2, unsigned n);
extern size_t myStrGetTok(const string& str, string& tok, size_t pos = 0,
                          const char del = ' ');
extern bool   myStr2Int(const string& str, int& num);
extern bool   isValidVarName(const string& str);

// In myGetChar.cpp
extern char myGetChar(istream&);
extern char myGetChar();

// In util.cpp
extern int    listDir(vector<string>&, const string&, const string&);
extern size_t getHashSize(size_t s);

// In aiger.h
#ifdef __cplusplus
extern "C"
{
#endif

    int aig2aag(char* _src_name, char* _dst_name);

#ifdef __cplusplus
}
#endif

// In input_formatter.cpp
extern bool input_formatter(string src_name, string dst_name);

// Other utility template functions
template <class T>
void
clearList(T& l) {
    T tmp;
    l.swap(tmp);
}

template <class T, class D>
void
removeData(T& l, const D& d) {
    size_t des = 0;
    for (size_t i = 0, n = l.size(); i < n; ++i) {
        if (l[i] != d) { // l[i] will be kept, so des should ++
            if (i != des) l[des] = l[i];
            ++des;
        }
        // else l[i] == d; to be removed, so des won't ++
    }
    l.resize(des);
}
unsigned long long 
getRandomULL(unsigned long long const& min, unsigned long long const& max);
#endif // UTIL_H
