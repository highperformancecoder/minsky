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
#include "minsky_epilogue.h"

#include "exprtk/exprtk.hpp"
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
  }

  void readMdl(Group& group, istream& mdlFile)
  {
    exprtk::parser<double> parser;
    parser.enable_unknown_symbol_resolver();
    exprtk::symbol_table<double> symbols;
    exprtk::expression<double> compiled;
    compiled.register_symbol_table(symbols);
    set<string> integrationVariables;
    
    string c;
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
        regex integ(R"(\s*integ\s*\(([^,]*),([^,]*)\))");
        smatch match;
        if (regex_match(definition,match,integ))
          {
            auto intOp=new IntOp;
            group.addItem(intOp);
            intOp->description(name);
            intOp->detailedText=comments;
            auto& v=intOp->intVar;
            integrationVariables.insert(name);
            auto function=new UserFunction;
            group.addItem(function);
            function->description("Integrand of "+name);
            function->expression=match[1].str();
            parser.compile(function->expression, compiled);
            group.addWire(function->ports[0], intOp->ports[1]);
            v->init(match[2].str());
            v->setUnits(units);
            v->detailedText=comments;
          }
        else
          {
            VariablePtr v(VariableBase::flow, name);
            group.addItem(v);
            auto function=new UserFunction;
            group.addItem(function);
            function->description("Def: "+name);
            function->expression=definition;
            parser.compile(function->expression, compiled);
            group.addWire(function->ports[0], v->ports[1]);
            v->setUnits(units);
            v->detailedText=comments;
          }
      }

//    // add in the variables
//    std::vector<std::string> variableNames;
//    symbols.get_variable_list(variableNames);
//    for (auto& name: variableNames)
//      if (!integrationVariables.count(name))
//        group.addItem(new Variable<VariableType::flow>(name));

  }
}
