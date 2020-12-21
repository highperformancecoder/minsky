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

#include "group.h"

namespace minsky
{
  string readToken(istream& mdlFile, char delim)
  {
    string r;
    char c;
    while (mdlFile>>c)
      if (c==delim)
        break;
      else
        r+=c;   // TODO - handle UTF8 encoding. Modern Vensims always generate UTF-8 - but check initial {UTF-8} sequence
    return r;
  }
  
  void readMdl(Group& group, istream& mdlFile)
  {
    char c;
    string name;
    while (mdlFile>>c)
      {
        // check this is a UTF-8 file
        if (c=='{' && readToken(mdlFile,'}')!="UTF-8")
          throw runtime_error("file encoding not supported");
        
        if (c=='*')
          // group leadin
          while (mdlFile>>c)
            if (c=='|')
              break; // end of group leadin

        // macros?
        // read variable name
        string name=readToken(mdlFile,'=');
        string definition=readToken(mdlFile,'~');
        string units=readToken(mdlFile,'~');
        string comments=readToken(mdlFile,'|');
        regex integ(R'\s*INTEG\s*(([^,]*),([^,]*))');
        smatch match;
        if (regex_match(definition,match,integ))
          {
            auto intOp=new IntOp;
            group.addItem(intOp);
            intOp->description(name);
            intOp->detailedText=comments;
            auto& v=intOp->intVar();
            auto function=new Function(match[1].str());
            group.addItem(function);
            group.addWire(function->ports[0], intOp->ports[1]);
            v->init=match[2].str();
            v->setUnits(units);
            v->detailedText=comments;
          }
        else
          {
            VariablePtr v(VariableBase::flow, name);
            group.addItem(v);
            auto function=new Function(definition);
            group.addItem(function);
            group.addWire(function->ports[0], v->ports[1]);
            v->setUnits(units);
            v->detailedText=comments;
          }
      }
  }
}
