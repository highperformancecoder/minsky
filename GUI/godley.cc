/*
  @copyright Steve Keen 2012
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
#include "godley.h"
#include "port.h"
#include "variableManager.h"
#include "minsky.h"
#include <ecolab_epilogue.h>
using namespace minsky;

const char* GodleyTable::initialConditions="Initial Conditions";

void GodleyTable::markEdited()
{
  minsky::minsky().markEdited();
}

bool GodleyTable::initialConditionRow(unsigned row) const
{
  const string& label=cell(row,0);
  static size_t initialConditionsSz=strlen(initialConditions);
  size_t i, j;
  // trim any leading whitespaces
  for (i=0; isspace(label[i]); ++i);
  // compare case insensitively
  for (j=0; j<initialConditionsSz && i<label.size() && 
         toupper(label[i])==toupper(initialConditions[j]); ++i, ++j);
  return j==initialConditionsSz;
}

void GodleyTable::insertRow(unsigned row)
{
  if (row<=data.size())
    {
      data.insert(data.begin()+row, vector<string>(cols()));
      markEdited();
    }
}

void GodleyTable::insertCol(unsigned col)
{
  if (col>=m_assetClass.size())
    m_assetClass.resize(cols(), noAssetClass);
  m_assetClass.insert(m_assetClass.begin()+col, 
                      col==0? noAssetClass: m_assetClass[col-1]);
  for (unsigned row=0; row<data.size(); ++row)
    {
      for (size_t i=data[row].size(); i<col; ++i)
        data[row].insert(data[row].end(), "");
      data[row].insert(data[row].begin()+col, "");
    }
  markEdited();
}

void GodleyTable::deleteCol(unsigned col)
{
  if (col>=m_assetClass.size())
    m_assetClass.resize(cols(), noAssetClass);
  m_assetClass.erase(m_assetClass.begin()+col-1);
  if (col>0 && col<=data[0].size())
    {
      for (unsigned row=0; row<rows(); ++row)
        data[row].erase(data[row].begin()+col-1);
      markEdited();
    }
}

void GodleyTable::moveRow(int row, int n)
{
  if (n==0 || row<0 || row>=int(rows()) || row+n<0 || row+n>=int(rows())) 
    return;
  vector<string> rowToMove;
  rowToMove.swap(data[row]);
  for ( ; abs(n)>0; n=n>0? n-1:n+1)
    rowToMove.swap(data[row+n]);
  rowToMove.swap(data[row]);
}

void GodleyTable::moveCol(int col, int n)
{
  if (n==0 || col<0 || col>=int(cols()) || col+n<0 || col+n>=int(cols())) 
    return;
  for (size_t row=0; row<rows(); ++row)
    {
      string cellToMove;
      cellToMove.swap(data[row][col]);
      for (int i=n; abs(i)>0; i=i>0? i-1:i+1)
        cellToMove.swap(data[row][col+i]);
      cellToMove.swap(data[row][col]);
    }
}


vector<string> GodleyTable::getColumnVariables() const
{
  set<string> uvars; //set for uniqueness checking
  vector<string> vars;
  for (size_t c=1; c<cols(); ++c)
    {
      string var=trimWS(cell(0,c));
      if (!var.empty())
        {
          if (!uvars.insert(var).second)
            throw error("Duplicate column label detected");
          vars.push_back(var);
        }
    }
  return vars;
}

vector<string> GodleyTable::getVariables() const
{
  vector<string> vars; 
  set<string> uvars; //set for uniqueness checking
  for (size_t r=1; r<rows(); ++r)
    if (!initialConditionRow(r))
      for (size_t c=1; c<cols(); ++c)
        {
          FlowCoef fc(cell(r,c));
          if (!fc.name.empty() && uvars.insert(fc.name).second)
            vars.push_back(fc.name);
        }
  return vars;
}

GodleyTable::AssetClass GodleyTable::_assetClass(size_t col) const 
{
  return col<m_assetClass.size()? m_assetClass[col]: noAssetClass;
}

GodleyTable::AssetClass GodleyTable::_assetClass
(size_t col, GodleyTable::AssetClass cls) 
{
  if (col>=m_assetClass.size())
    m_assetClass.resize(cols(), noAssetClass);
  assert(cols()>col);
  m_assetClass[col]=cls;
  return _assetClass(col);
}


string GodleyTable::assetClass(TCL_args args)
{
  int col=args;
  if (args.count) 
    {
      _assetClass
        (col, AssetClass(classdesc::enumKey<AssetClass>((char*)args)));
      markEdited();  
    }
  return classdesc::enumKey<AssetClass>(_assetClass(col));
}

string GodleyTable::rowSum(int row) const
{
  // accumulate the total for each variable
  map<string,double> sum;
  for (size_t c=1; c<cols(); ++c)
    {
      FlowCoef fc(cell(row,c));
      if (!fc.name.empty()||initialConditionRow(row))
        sum[fc.name]+=fc.coef;
    }

  // create symbolic representation of each term
  ostringstream ret;
  for (map<string,double>::iterator i=sum.begin(); i!=sum.end(); ++i)
    if (i->second!=0)
      {
        if (!ret.str().empty() &&i->second>0)
          ret<<"+";
        if (i->second==-1)
          ret<<"-";
        else if (i->second!=1)
          ret<<i->second;
        ret<<i->first;
      }

  //if completely empty, substitute a zero
  if (ret.str().empty()) 
    return "0";
  else 
    return ret.str();

}

void GodleyTable::setDEmode(bool mode)
{
  if (mode==doubleEntryCompliant) return;
  doubleEntryCompliant=true; // to allow signConventionReversed to work
  for (size_t r=1; r<rows(); ++r)
    if (!initialConditionRow(r))
      for (size_t c=1; c<cols(); ++c)
        if (signConventionReversed(c))
          {
            string& formula=cell(r,c);
            unsigned start=0;
            while (start<formula.length() && isspace(formula[start])) start++;
            if (start==formula.length()) continue; // empty cell
            if (formula[start]=='-')
              formula.erase(start,1); // turns a negative into a positive
            else
              formula.insert(start,"-");
          }
  doubleEntryCompliant=mode;
  markEdited();  
}

void GodleyTable::nameUnique()
{
  for (int i=1;;++i)
    {
      string trialName="Godley"+str(i);
      GodleyIcons::iterator it=minsky().godleyItems.begin();
      for (; it!=minsky().godleyItems.end(); ++it)
        if (it->table.title == trialName)
          break;
      if (it==minsky().godleyItems.end())
        {
          title = trialName;
          break;
        }
    }
}
