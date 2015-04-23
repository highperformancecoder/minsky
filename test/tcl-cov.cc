/*
  Temporarily in Minsky for development purposes, but will be moved to EcoLab
*/
// Utility programme for outputting coverage analysis of TCL code

#include "coverage.h"
#include <ecolab_epilogue.h>
#include <vector>
#include <fstream>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
using boost::regex;
using boost::smatch;
using boost::filesystem::current_path;

using namespace minsky;
using namespace std;

int count_matched_braces(const string& x)
{
  int r=0;
  for (size_t i=0; i<x.size(); ++i)
    switch (x[i])
      {
      case '{': r++; break;
      case '}': r--; break;
      case '\\':  // skip quoted braces
        if (x[i+1]=='{' || x[i+1]=='}') i++;
        break;
      }
  return r;
}

struct Element
{
  enum Type {namesp, proc};
  Type type;
  string name;
  unsigned braceCnt; //brace level of outermost scope of this element
  unsigned lineCnt=0;
  Element(Type type=namesp, string name="",  unsigned braceCnt=0):
    type(type), name(name), braceCnt(braceCnt) {}
};

struct ElementStack: public vector<Element>
{
  // true if at leat one proc element is present
  bool isProc() const {
    for (const Element& e: *this)
      if (e.type==Element::proc) return true;
    return false;
  }
  // return the fully qualified name corresponding to this stack
  string fqName() const {
    if (!isProc()) return "";
    // first find the topmost proc name
    auto proc=rbegin();
    for (; proc!=rend(); ++proc)
      if (proc->type==Element::proc)
        break;
    
    // now build name
    string r;
    for (const Element& e: *this)
      {
        if (e.braceCnt==proc->braceCnt) break;
        if (e.type==Element::namesp)
          r+="::"+e.name;
      }
    r+="::"+proc->name;
    return r;
  }
};

int main(int argc, char** argv)
{
  if (argc<3)
    {
      printf("usage %s <coverage>.cov <file>.tcl\n",argv[0]);
      return 1;
    }

  Coverage cov;
  cov.init(argv[1],'r');
  ifstream inp(argv[2]);
  string line;
  ElementStack elemStack;
  regex namespace_def(R"(^\s*namespace\s+eval\s+(\w+).*)"), 
    proc_def(R"(^\s*proc\s+(\w+).*)");
  smatch m;
  unsigned braceCnt=0;

  // append current working directory if path is relative
  string fname;
  if (argv[2][2]=='/')
    fname=argv[2];
  else
    fname=(current_path()/argv[2]).string();

  cout << fname << endl;

  int linec=0;
  while (getline(inp,line))
    {
      linec++;
      if (regex_match(line,m,namespace_def))
        elemStack.push_back(Element(Element::namesp,m[1],braceCnt));
      else  if (regex_match(line,m,proc_def))
        elemStack.push_back(Element(Element::proc, m[1], braceCnt));

      unsigned c=cov[cov.key(fname, linec)];
      //      cout << c << " " << cov.key(fname, linec) << endl;
      if (c || elemStack.isProc())
        {
          if (c || (c=cov[cov.key(elemStack.fqName(), elemStack.back().lineCnt)]))
            cout << c << "\t" << line << endl;
          else
            cout << "###\t" << line << endl;
        }
      else
        cout << "\t"<<line<<endl;

      braceCnt+=count_matched_braces(line);
      if (!elemStack.empty())
        {
          elemStack.back().lineCnt++;
          if (braceCnt==elemStack.back().braceCnt)
            {
              // remove top level element from stack, transferring linecount from inner element
              unsigned l=elemStack.back().lineCnt;
              elemStack.pop_back();
              if (!elemStack.empty())
                elemStack.back().lineCnt+=l;
            }
        }

    }
}
