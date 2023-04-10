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

#include "minsky.h"
#include "mdlReader.h"
#include "operation.h"
#include "userFunction.h"
#include "selection.h" //TODO why is this needed?
#include "minsky_epilogue.h"

#include <boost/locale.hpp>

using boost::locale::conv::utf_to_utf;
using namespace std;
namespace minsky
{
  namespace
  {
    string readToken(istream& mdlFile, char delim, bool appendDelim=false)
    {
      string r;
      string c;
      while (mdlFile>>GetUtf8Char(c))
        if (c[0]==delim || c[0]=='~' || c[0]=='|')
          {
            if (appendDelim)
              r+=c[0];
            break;
          }
        else if (c[0]=='{') /* inline comment - read up to close brace */
          {
            while (c[0]!='}' && mdlFile>>GetUtf8Char(c));
            continue;
          }
        else if (c[0]=='\\')
          // quoted end of line processing
          if (mdlFile>>GetUtf8Char(c) &&  (c[0]=='\n'||c[0]=='\r'))
            r+=c;
          else
            r+="\\"+c;
        else if (c[0]>=' ') // strip out control characters
          r+=c;
      
      return r;
    }

    bool identifierChar(int c)
    {return c>0x7f || isalnum(c) || c=='\'' || c=='$';}
    
    // collapse multiple whitespace characters, and if in the middle
    // of an identifier (alphanum <space> alphanum, replace with
    // underscore, and tolower the characters (case insensitive)
    string collapseWS(const string& x)
    {
      // work in UTF-32 to simplify string analysis
      auto xx=utf_to_utf<uint32_t>(x);
      basic_string<uint32_t> result;
      uint32_t lastNonWS=0;
      bool quoted=false, lastWS=false, inIdentifier=false;
      for (auto& i: xx)
        {
          if (i=='"' &&lastNonWS!='\\')
            {
              quoted=!quoted;
              inIdentifier=quoted;
              // identifiers not allowed to end in .
              if (!inIdentifier && result.back()=='.')
                result.erase(result.end()-1);
              continue; // strip quotes
            }
          
          if (!quoted && !inIdentifier && isalpha(i))
            inIdentifier=true;
          if (!quoted && inIdentifier && !identifierChar(i) && !isspace(i) && i!='_')
            {
              // identifiers not allowed to end in .
              if (result.back()=='.')
                result.erase(result.end()-1);
              inIdentifier=false;
            }
          
          if (!isspace(i) && i!='_')
            {
              // convert verboten characters into ones more friendly to exprtk
              // we use . as an escape into a numerical unicode code uXXXX, terminated by another .
              basic_string<uint32_t> exprTkGoodChar;
              if (inIdentifier)
                exprTkGoodChar=utf_to_utf<uint32_t>(".u"+to_string(i)+".");
              else
                exprTkGoodChar+=i;
              // camelcase if collapsing whitespace in an identifier, ' ' otherwise
              if (lastWS)
                if (quoted || (identifierChar(i) && identifierChar(lastNonWS)))
                  if (isalnum(i))
                    result+=toupper(i);
                  else
                    result+=exprTkGoodChar;
                else
                  {
                    result+=' ';
                    if (isalnum(i))
                      result+=tolower(i);
                    else
                      result+=exprTkGoodChar;
                  }
              else
                if (isalnum(i))
                  result+=tolower(i);
                else
                  result+=exprTkGoodChar;
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
      std::string args;
      std::string expression;
    };
  
    
    map<string, FunctionDef> venSimFunctions={
      {"arccos",{"(x)","acos(x)"}},
      {"arcsin",{"(x)","asin(x)"}},
      {"arctan",{"(x)","atan(x)"}},
      {"gammaLn",{"(x)","gammaLn(x)"}},
      {"integer",{"(x)","floor(x)"}},
      {"ifThenElse",{"(x,y,z)","x? y: z"}},
      {"ln",{"(x)","log(x)"}},
      {"log",{"(x,y)","log(x)/log(y)"}},
      {"modulo",{"(x,y)","x%y"}},
      {"power",{"(x,y)","x^y"}},
      {"pulse",{"(x,y)","(time>=x)*(time<x+y)"}},
      {"pulseTrain",{"(s,b,r,e)","tm:=time%r; (time<e)*(time>=s)*(tm<((s+b)%r))*(tm>=(s%r))"}},
      {"quantum",{"(x,y)","floor(x/y)"}},
      {"ramp",{"(s,a,b)","var t1:=min(a,b); var t2:=max(a,b); (clamp(t1,time,t2)-t1)*s"}},
      {"step",{"(x,y)","y*(time>=x)"}},
      {"xidz",{"(x,y,z)","var r:=y/z; isfinite(r)? r: x"}},
      {"zidz",{"(x,y)","var r:=y/z; isfinite(r)? r:0"}}
    };

    set<string> functionsAdded; // track user functions added to group
    regex identifier(R"([A-Za-z][A-Za-z0-9._]*[A-Za-z0-9_])");
  
    void addDefinitionToPort(Group& group, const shared_ptr<Port>& port, const string& name, const string& definition)
    {
      smatch match;
      if (regex_match(definition,match,identifier))
        {
          VariablePtr rhs(VariableBase::flow, definition);
          group.addItem(rhs);
          if (port)
            group.addWire(rhs->ports(0).lock(), port);
          return;
        }
      
      auto function=new UserFunction(name,definition);
      group.addItem(function); //ownership passed
      for (auto& i: function->symbolNames())
        {
          auto f=venSimFunctions.find(i);
          if (f!=venSimFunctions.end())
            {
              if (functionsAdded.insert(f->first).second)
                addDefinitionToPort(group,nullptr,f->first+f->second.args,f->second.expression);
            }
       }
      if (port)
        group.addWire(function->ports(0), port);
    }
    
    void defineLookupFunction(Group& group, const std::string& name, const std::string& data)
    {
      regex lookupPairsPattern(R"((\[[^\]]*\],)?(\(.*\)))");
      smatch match; 
      map<double,double> xData;
      if (regex_match(data,match,lookupPairsPattern))
        {
          regex extractHead(R"(\(([^,]*),([^)]*)\)(,(\(.*\)))*)");
          // note match[3] is the trailing data, match[4] strips the leading ,
          for (auto d=match[2].str(); regex_match(d, match, extractHead); d=match[4])
            xData[stod(match[1])]=stod(match[2]);
        }
      else
        {
          vector<double> xyData;
          for (size_t offs=0; offs<data.size(); ++offs)
            xyData.push_back(stod(data.substr(offs),&offs));
          if (xyData.size()%2!=0)
            throw runtime_error("Odd amount of data specified");
          for (size_t i=0; i<xyData.size()/2; ++i)
            xData[xyData[i]]=xyData[i+xyData.size()/2];
        }
      auto f=make_shared<Group>();
      f->self=f;
      f->title=name;
      group.addItem(f);
      VariablePtr dataVar(VariableType::flow,"data");
      f->addItem(dataVar);
      dataVar->moveTo(f->x()-50,f->y()-20);
      OperationPtr gather(OperationType::gather);
      f->addItem(gather);
      gather->moveTo(f->x()+30,f->y()-10);
      VariablePtr inVar(VariableType::flow,"in"), outVar(VariableType::flow,"out");
      f->addItem(inVar);
      f->addItem(outVar);
      f->inVariables.push_back(inVar);
      f->outVariables.push_back(outVar);
      f->addWire(*dataVar, *gather, 1);
      f->addWire(*f->inVariables[0], *gather, 2);
      f->addWire(*gather, *f->outVariables[0], 1);
      
      XVector xVals("0",{Dimension::value,""});
      auto& tensorInit=dataVar->vValue()->tensorInit;
      for (auto& i: xData)
        xVals.push_back(i.first);
      Hypercube hc; hc.xvectors.push_back(std::move(xVals));
      tensorInit.hypercube(std::move(hc));

      assert(tensorInit.size()==xData.size());
      auto j=tensorInit.begin();
      for (auto& i: xData)
        *j++=i.second;

      *dataVar->vValue()=tensorInit;
    }
  }

  void readMdl(Group& group, Simulation& simParms, istream& mdlFile)
  {
    set<string> integrationVariables;
    regex integ(R"(\s*integ\s*\(([^,]*),([^,]*)\))");
    regex number(R"(\d*\.?\d+[Ee]?\d*)");
    regex unitFieldPattern(R"(([^\[\]]*)(\[.*\])?)");
    regex sliderSpecPattern(R"(\[([^,]*),?([^,]*),?([^,]*)\])");
    regex lookupPattern(R"(([^(]*)\((.*)\))");
    smatch match;
    functionsAdded.clear();

    string c;
    string currentMDLGroup;
    while (mdlFile>>GetUtf8Char(c))
      {
        if (isspace(c[0])) continue;
        switch (c[0])
          {
          case '{':
              // check this is a UTF-8 file
            if (trimWS(readToken(mdlFile,'}'))!="UTF-8")
              throw runtime_error("only UTF-8 file encoding is supported");
            continue;
        
          case '*':
            // group leadin
            for (; c[0]=='*'; mdlFile>>GetUtf8Char(c));
            currentMDLGroup=trimWS(readToken(mdlFile,'*'));
              
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
        string nameStr=readToken(mdlFile,'=',true /* append delimiter */);
        string name=collapseWS(trimWS(c+nameStr.substr(0,nameStr.length()-1)));
        if (name.substr(0,9)==R"(\\\---///)")
          break; // we don't parse the sketch information - not used in Minsky
        string definition;
        if (nameStr.back()=='=')
          // only read definition if this was a variable definition
          definition=collapseWS(trimWS(readToken(mdlFile,'~')));
        switch (definition[0])
          {
          case '=': case ':': // for now, treat constant assignment and data assignment equally to numeric assignment
            definition.erase(definition.begin());
            break;
          default:
            break;
          }
        auto unitField=trimWS(readToken(mdlFile,'~'));
        regex_match(unitField,match,unitFieldPattern);
        string units=trimWS(match[1]);
        string sliderSpec;
        if (match.size()>1)
          sliderSpec=match[2];
        
        string comments=trimWS(readToken(mdlFile,'|'));


        if (currentMDLGroup==".Control")
          {
            if (name=="timeStep")
              {
                simParms.timeUnit=units;
                if (regex_match(definition,match,number))
                  simParms.stepMax=stod(definition);
              }
            if (!regex_match(definition,match,number)) continue;
            if (name=="initialTime") simParms.t0=stod(definition);
            if (name=="finalTime") simParms.tmax=stod(definition);
            continue;
          }
        if (regex_match(name,match,lookupPattern))
          defineLookupFunction(group, match[1], match[2]);
        else if (regex_match(definition,match,integ))
          {
            auto intOp=new IntOp;
            group.addItem(intOp);
            intOp->description(name);
            intOp->detailedText=comments;
            auto& v=intOp->intVar;
            integrationVariables.insert(name);
            auto integrand=match[1].str();
            addDefinitionToPort(group, intOp->ports(1).lock(), "Integrand of "+name,integrand);

            auto init=match[2].str();
            if (regex_match(init,match,identifier))
              v->init(init);
            else
              {
                // we need to add another variable, and attach it to a function block
                addDefinitionToPort(group, intOp->ports(2).lock(), "Initial value of "+name, init);
              }
            try  // absorb any errors in units - we have a chance to fix these later
              {
                v->setUnits(units);
              }
            catch (...) {}
            v->detailedText=comments;

          }
        else if (regex_match(definition,match,number))
          {
            VariablePtr v(VariableBase::parameter, name);
            group.addItem(v);
            v->init(definition);
            try  // absorb any errors in units - we have a chance to fix these later
              {
                v->setUnits(units);
              }
            catch (...) {}
            v->detailedText=comments;
            if (regex_match(sliderSpec,match,sliderSpecPattern))
              {
                vector<string> spec;
                for (size_t i=1; i<=match.size(); ++i) spec.push_back(match[i]);
                if (!spec.empty() && regex_match(spec[0],match,number))
                  v->sliderMin=stod(spec[0]);
                else
                  v->sliderMin=0.1*stod(definition);
                if (spec.size()>1 && regex_match(spec[1],match,number))
                  v->sliderMax=stod(spec[1]);
                else
                  v->sliderMax=10*stod(definition);
                if (spec.size()>2 && regex_match(spec[2],match,number))
                  v->sliderStep=stod(spec[2]);
                v->adjustSliderBounds();
              }
            else
              v->initSliderBounds();
          }
        else
          {
            VariablePtr v(VariableBase::flow, name);
            group.addItem(v);
            addDefinitionToPort(group, v->ports(1).lock(), "Def: "+name, definition);
            try  // absorb any errors in units - we have a chance to fix these later
              {
                v->setUnits(units);
              }
            catch (...) {}
            v->detailedText=comments;
          }
      }

  }
}
