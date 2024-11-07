/*
  @copyright Steve Keen 2016
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

#include "pango.h"
#include "cairoItems.h"
#include "godleyExport.h"
#include "flowCoef.h"
#include "latexMarkup.h"
#include "group.h"
#include "selection.h"
#include "lasso.h"
#include "variableValue.h"
#include "minsky_epilogue.h"

using namespace std;
using classdesc::enum_keysData;

namespace minsky
{
  namespace
  {
    string fcStr(const FlowCoef& fc)
    {
      auto nm=uqName(fc.name);
      if (fc.coef==1)
        return nm;
      if (fc.coef==-1)
        return "-"+nm;
      if (fc.coef==0)
        return "";
      return str(fc.coef)+nm;
    }

  // trim enclosing <i> tags
    string trim(string x)
    {
      if (x.substr(0,3)=="<i>")
        x=x.substr(3);
      if (x.length()>=4 && x.substr(x.length()-4)=="</i>")
        return x.substr(0,x.length()-4);
      return x;
    }
}

  void exportToCSV(std::ostream& s, const GodleyTable& g)
  {
    s<<'"'<<g.getCell(0,0)<<'"';
    for (unsigned i=1; i<g.cols(); ++i)
      s<<",\""<<trim(latexToPango(uqName(g.getCell(0,i))))<<'"';
    s<<'\n';
    if (g.doubleEntryCompliant)
      {
        s<<"Asset Class";
        for (unsigned i=1; i<g.cols(); ++i)
          s<<","<<enum_keysData<GodleyAssetClass::AssetClass>::
            keysData[g._assetClass(i)].name;
        s<<"\n";
      }
    for (unsigned r=1; r<g.rows(); ++r)
      {
        s<<g.getCell(r,0);
        for (unsigned c=1; c<g.cols(); ++c)
          s<<",\""<<trim(latexToPango(fcStr(FlowCoef(g.getCell(r,c)))))<<'"';
        s<<'\n';
      }
  }

  void exportToLaTeX(std::ostream& f, const GodleyTable& g)
  {
    f<<"\\documentclass{article}\n\\begin{document}\n";
    f<<"\\begin{tabular}{|c|";
    for (unsigned i=1; i<g.cols(); ++i)
      f<<'c';
    f<<"|}\n\\hline\n";
    f<<"Flows $\\downarrow$ / Stock Variables $\\rightarrow$";
    for (unsigned i=1; i<g.cols(); ++i)
      f<<"&\\multicolumn{1}{|c|}{$"<<uqName(g.getCell(0,i))<<"$}";

    // asset class descriptors
    if (g.doubleEntryCompliant)
      {
        f<<"\\\\\\cline{2-"<<g.cols()<<"}Asset Class";
        unsigned repeat=0;
        GodleyAssetClass::AssetClass asset=GodleyAssetClass::noAssetClass;

        auto outputAC=[&]() {
          if (repeat>0)
            f<<"&\\multicolumn{"<<repeat<<"}{|c|}{"<<
              enum_keysData<GodleyAssetClass::AssetClass>::
              keysData[asset].name<<'}';
        };
          
        for (unsigned i=1; i<g.cols(); ++i)
          if (g._assetClass(i)!=asset)
            {
              outputAC();
              asset=g._assetClass(i);
              repeat=1;
            }
          else
            repeat++;

        // now output last column
        outputAC();
      }

    f<<"\\\\\\hline\n";
    for (unsigned r=1; r<g.rows(); ++r)
      {
        f<<g.getCell(r,0);
        for (unsigned c=1; c<g.cols(); ++c)
          f<<"&$"<<fcStr(FlowCoef(g.getCell(r,c)))<<'$';
        f<<"\\\\\n";
      }
    f<<"\\hline\n\\end{tabular}\n";
    f<<"\\end{document}\n";
  }
}
