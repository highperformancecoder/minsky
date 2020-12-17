/*
  @copyright Steve Keen 2020
  @author Russell Standish
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mdlReader.h"
#include "operation.h"
#include "userFunction.h"
#include "selection.h" //TODO why is this needed?
#include "minsky.h"
#include "minsky_epilogue.h"

#include <boost/locale.hpp>

using boost::locale::conv::utf_to_utf;
using namespace std;
namespace minsky
{
  namespace
  {
    string readToken(istream& mdlFile, char delim)
    {
      string r;
      string c;
      while (mdlFile>>GetUtf8Char(c))
        if (c[0]==delim)
          break;
        else if (c[0]>=' ') // strip out control characters
          r+=c;
      
      return trimWS(r);
    }

    bool identifierChar(int c)
    {return isalnum(c) || c=='\'' || c=='$';}
    
    // collapse multiple whitespace characters, and if in the middle
    // of an identifier (alphanum <space> alphanum, replace with
    // underscore, and tolower the characters (case insensitive)
    string collapseWS(const string& x)
    {
      // work in UTF-32 to simplify string analysis
      auto xx=utf_to_utf<uint32_t>(x);
      basic_string<uint32_t> result;
      uint32_t lastNonWS=0;
      bool quoted=false, lastWS=false;;
      for (auto& i: xx)
        {
          if (i=='"' &&lastNonWS!='\\')
            quoted=!quoted;
          
          if (!isspace(i) && i!='_')
            {
              // camelcase if collapsing whitespace in an identifier, ' ' otherwise
              if (lastWS)
                if (quoted || (identifierChar(i) && identifierChar(lastNonWS)))
                  if (isascii(i))
                    result+=toupper(i);
                  else
                    result+=i;
                else
                  result+=utf_to_utf<uint32_t>(" ")+i;
              else
                if (isascii(i))
                  result+=tolower(i);
                else
                  result+=i;
              lastNonWS=i;
              lastWS=false;
            }
          else
            lastWS=true;
        }
      return utf_to_utf<char>(result);
    }

    struct FunctionDef
    {
      std::string expression;
      std::function<void(const std::string&,exprtk::symbol_table<double>&)> addToTable;
    };

#define FUNCTION(def) [](const std::string& name, exprtk::symbol_table<double>& table){table.add_function(name,def);}       
    
    map<string, FunctionDef> venSimFunctions={
      {"step",{"y*(x>time)",FUNCTION([](double x,double y){return y*(x>minsky().t);})}}
    };

    void addDefinitionToPort(Group& group, const shared_ptr<Port>& port, const string& name, const string& definition)
    {
      static regex identifier(R"([A-Za-z]\w*)");
      smatch match;
      if (regex_match(definition,match,identifier))
        {
          VariablePtr rhs(VariableBase::flow, definition);
          group.addItem(rhs);
          if (port)
            group.addWire(rhs->ports[0], port);
          return;
        }
      
      auto function=new UserFunction;
      group.addItem(function);
      function->description(name);
      function->expression=definition;
      function->addVariable("time",minsky().t);
      for (auto i: function->externalSymbolNames())
        {
          auto f=venSimFunctions.find(i);
          if (f!=venSimFunctions.end())
            {
              addDefinitionToPort(group,nullptr,f->first,f->second.expression);
              f->second.addToTable(f->first, UserFunction::globalSymbols());
            }
       }
      if (port)
        group.addWire(function->ports[0], port);
    }
    
  }

  void readMdl(Group& group, istream& mdlFile)
  {
    set<string> integrationVariables;
    regex integ(R"(\s*integ\s*\(([^,]*),([^,]*)\))");
    regex number(R"(\d*\.?\d+[Ee]?\d*)");

    UserFunction::globalSymbols().clear();
    
    string c;
    string currentMDLGroup;
    while (mdlFile>>GetUtf8Char(c))
      {
        if (isspace(c[0])) continue;
        switch (c[0])
          {
          case '{':
              // check this is a UTF-8 file
            if (readToken(mdlFile,'}')!="UTF-8")
              throw runtime_error("only UTF-8 file encoding is supported");
            continue;
        
          case '*':
            // group leadin
            for (; c[0]=='*'; mdlFile>>GetUtf8Char(c));
            currentMDLGroup=readToken(mdlFile,'*');
            if (currentMDLGroup==".Control")
              return; // TODO Add processing of simulation parameters
            while (mdlFile>>GetUtf8Char(c))
              if (c[0]=='|')
                {
                  c.clear();
                  break; // end of group leadin
                }
            break;
          }
        
        // macros?
        // read variable name
        string name=collapseWS(c+readToken(mdlFile,'='));
        if (name.substr(0,9)==R"(\\\---///)")
          break; // we don't parse the sketch information - not used in Minsky
        string definition=collapseWS(readToken(mdlFile,'~'));
        string units=readToken(mdlFile,'~');
        string comments=readToken(mdlFile,'|');
        smatch match;
        if (regex_match(definition,match,integ))
          {
            auto intOp=new IntOp;
            group.addItem(intOp);
            intOp->description(name);
            intOp->detailedText=comments;
            auto& v=intOp->intVar;
            integrationVariables.insert(name);
            v->init(match[2].str());
            v->setUnits(units);
            v->detailedText=comments;

            auto integrand=match[1].str();
            addDefinitionToPort(group, intOp->ports[1], "Integrand of "+name, integrand);
          }
        else if (regex_match(definition,match,number))
          {
            VariablePtr v(VariableBase::parameter, name);
            group.addItem(v);
            v->init(definition);
            v->initSliderBounds();
            v->setUnits(units);
            v->detailedText=comments;
          }
        else
          {
            VariablePtr v(VariableBase::flow, name);
            group.addItem(v);
            addDefinitionToPort(group, v->ports[1], "Def: "+name, definition);
            v->setUnits(units);
            v->detailedText=comments;
          }
      }

  }
}
