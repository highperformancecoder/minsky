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
#include "minsky.h"
#include "godleyTable.h"
#include "port.h"
#include "flowCoef.h"
#include "godleyExport.h"
#include "assetClass.rcd"
#include "godleyTable.rcd"
#include "minsky_epilogue.h"
using namespace minsky;

const char* GodleyTable::initialConditions="Initial Conditions";

void GodleyTable::markEdited()
{
  minsky::minsky().markEdited();
}

bool GodleyTable::initialConditionRow(unsigned row) const
{
  if (row>=rows()) return false;
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

bool GodleyTable::singularRow(unsigned row, unsigned col)
{
  for (size_t c=0; c<cols(); ++c)
    if (c!=col && !cell(row, c).empty())
      return false;
  return true;
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
  // insert extra empty column if an asset class gets emptied out of this
  orderAssetClasses();
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
  auto targetAssetClass=_assetClass(col+n);
  for (size_t row=0; row<rows(); ++row)
    {
      string cellToMove;
      cellToMove.swap(data[row][col]);
      for (int i=n; abs(i)>0; i=i>0? i-1:i+1)
        cellToMove.swap(data[row][col+i]);
      cellToMove.swap(data[row][col]);
    }
  auto ac=_assetClass(col);
  for (int i=n; abs(i)>0; i=i>0? i-1:i+1)
    swap(ac, m_assetClass[col+i]);
  swap(ac, m_assetClass[col]);
    
  _assetClass(col+n, targetAssetClass);
  // insert extra empty column if an asset class gets emptied out of this
  orderAssetClasses();
  // save text in currently highlighted column heading.  For tickets 1058/1094/1122/1127.
  savedText=data[0][col];
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
          // disable duplicate column test on equity columns (feature #174)
          if (_assetClass(c)!=AssetClass::equity && !uvars.insert(var).second)
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
  if (col==0) return noAssetClass;
  return col<m_assetClass.size()? m_assetClass[col]: noAssetClass;
}

GodleyTable::AssetClass GodleyTable::_assetClass
(size_t col, GodleyTable::AssetClass cls) 
{
  if (col==0) return noAssetClass; // don't set column 0 asset class
  if (col>=m_assetClass.size())
    m_assetClass.resize(cols(), noAssetClass);
  assert(cols()>col);
  m_assetClass[col]=cls;
  return _assetClass(col);
}

bool GodleyTable::singleEquity() const {
  assert(cols()>=3);
  return m_assetClass[cols()-2]!=GodleyAssetClass::equity;
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
  if (row==0)
    throw runtime_error("rowSum not valid for stock var names");
  
  // accumulate the total for each variable
  map<string,double> sum;
  set<string> colNamesSeen;

  for (size_t c=1; c<cols(); ++c)
    {
      FlowCoef fc(cell(row,c));
      if (!fc.name.empty()||initialConditionRow(row))
        {
          // apply accounting relation to the initial condition row
          if (signConventionReversed(c))
            sum[fc.name]-=fc.coef;
          else
            sum[fc.name]+=fc.coef;
        }
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
          abs(i->second)>5*std::numeric_limits<double>::epsilon()? ret<<i->second : ret<<0; // only display decimals if sum of row is larger than 5*2.22045e-16. for ticket 1244 
        abs(i->second)>5*std::numeric_limits<double>::epsilon()? ret<<i->first : ret<<"";  
      }    

  //if completely empty, substitute a zero
  if (ret.str().empty()) 
    return "0";
  return ret.str();

}

std::vector<std::string> GodleyTable::getColumn(unsigned col) const
{
  std::vector<std::string> r;
  for (unsigned row=0; row<rows(); ++row)
    r.push_back(cell(row,col));
  return r;
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
}

void GodleyTable::nameUnique()
{
  for (int i=1;;++i)
    {
      string trialName="Godley"+to_string(i);
      
      if (!cminsky().model->findAny
          (&Group::items,
           [&](const ItemPtr& i)
           {
             auto g=dynamic_cast<GodleyIcon*>(i.get());
             return g && g->table.title == trialName;
           }))
        {
          title = trialName;
          break;
        }
    }
}

void GodleyTable::exportToLaTeX(const string& filename) const
{
  ofstream f(filename);
  minsky::exportToLaTeX(f, *this);
  if (!f) throw runtime_error("cannot save to "+filename);
}

void GodleyTable::exportToCSV(const string& filename) const
{
  ofstream f(filename);
  minsky::exportToCSV(f, *this);
  if (!f) throw runtime_error("cannot save to "+filename);
}

void GodleyTable::orderAssetClasses()
{
  unsigned numRows=rows()>1? rows(): 1;
  map<AssetClass,Data> tmpCols;
  for (unsigned c=1; c<cols(); ++c)
    if (_assetClass(c)==noAssetClass)
      tmpCols[asset].push_back(getColumn(c));
    else
      tmpCols[_assetClass(c)].push_back(getColumn(c));

  // add empty column if asset class not present, and count number of cols
  unsigned numCols=1;
  for (int ac=asset; ac<=equity; ++ac)
    {
      auto& tc=tmpCols[AssetClass(ac)];
      // strip out any blank columns
      tc.erase(remove_if(tc.begin(), tc.end(), [](const vector<string>& x)
                         {return x.empty() || x[0].empty();}), tc.end());
      // ensure at least one column is present in an asset class
      if (tc.empty())
        tc.emplace_back(numRows);
      
      numCols+=tc.size();
    }

  resize(numRows, numCols);
  unsigned col=1;
  for (int ac=asset; ac<=equity; ++ac)
    for (auto& colData: tmpCols[AssetClass(ac)])
      {
        for (unsigned row=0; row<rows(); ++row)
          cell(row,col)=colData[row];
        _assetClass(col,AssetClass(ac));
        col++;
      }
}

void GodleyTable::rename(const std::string& from, const std::string& to)
{
  // if no stock vars found, check flow var cells.
  for (size_t r=0; r<rows(); ++r)
    for (size_t c=1; c<cols(); ++c)
      {
        FlowCoef fc(cell(r,c));
        if (!fc.name.empty() && fc.name==from)
          {
            fc.name=to;
            cell(r,c)=fc.str();
          }
      }
}

void GodleyTable::renameFlows(const std::string& from, const std::string& to)
{
  for (size_t r=1; r<rows(); ++r) 	
    for (size_t c=1; c<cols(); ++c)
      {
        FlowCoef fc(cell(r,c));
        if (!fc.name.empty() && fc.name==from)
          {
            fc.name=to;
            cell(r,c)=fc.str();
          }
      }
}

void GodleyTable::renameStock(const std::string& from, const std::string& to)
{
    for (size_t c=1; c<cols(); ++c)
      {
        FlowCoef fc(cell(0,c));
        if (!fc.name.empty() && fc.name==from)
          {
            fc.name=to;
            cell(0,c)=fc.str();
          }
      }
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::GodleyTable);
