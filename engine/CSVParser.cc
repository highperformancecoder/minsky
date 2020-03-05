/*
  @copyright Steve Keen 2018
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

#include "CSVParser.h"
#include "minsky.h"
#include "minsky_epilogue.h"
using namespace minsky;
using namespace std;

#include <boost/type_traits.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>

typedef boost::escaped_list_separator<char> Parser;
typedef boost::tokenizer<Parser> Tokenizer;

struct SpaceSeparatorParser
{
  char escape, quote;
  SpaceSeparatorParser(char escape='\\', char sep=' ', char quote='"'):
    escape(escape), quote(quote) {}
  template <class I>
  bool operator()(I& next, I end, std::string& tok)
  {
    tok.clear();
    bool quoted=false;
    for (; next!=end; ++next)
      if (*next==escape)
        tok+=*(++next);
      else if (*next==quote)
        quoted=!quoted;
      else if (!quoted && isspace(*next))
        {
          while (isspace(*next)) ++next;
          return true;
        }
      else
        tok+=*next;
    return !tok.empty();
  }
  void reset() {}
};

struct NoDataColumns: public std::exception
{
  const char* what() const noexcept override {return "No data columns";}
};
struct DuplicateKey: public std::exception
{
  std::string msg="Duplicate key";
  DuplicateKey(const vector<string>& x) {
    for (auto& i: x)
      msg+=":"+i;
  }
  const char* what() const noexcept override {return msg.c_str();}
};

namespace
{
  const size_t maxRowsToAnalyse=100;
  
  double quotedStoD(const string& s,size_t& charsProcd)
  {
    double r=stod(s,&charsProcd);
    if (charsProcd==s.size()) return r;
    //strip possible quote characters
    if (!s.empty() && s[0]==s[s.size()-1])
      {
        r=stod(s.substr(1),&charsProcd);
        charsProcd+=2;
      }
    return r;
  }

  string stripWSAndDecimalSep(const string& s)
  {
    string r;
    for (auto c: s)
      if (!isspace(c) && c!=',' && c!='.')
        r+=c;
    return r;
  }

  bool isNumerical(const string& s)
  {
    size_t charsProcd;
    string stripped=stripWSAndDecimalSep(s);
    try
      {
        quotedStoD(stripped, charsProcd);
      }
    catch (...) {return false;}
    return charsProcd==stripped.size();
  }
  
  // returns first position of v such that all elements in that or later
  // positions are numerical or null
  size_t firstNumerical(const vector<string>& v)
  {
    size_t r=0;
    for (size_t i=0; i<v.size(); ++i)
      try
        {
          if (!v[i].empty())
            {
              size_t c;
              auto s=stripWSAndDecimalSep(v[i]);
              quotedStoD(s,c);
              if (c!=s.size())
                r=i+1;
            }
        }
      catch (...)
        {
          r=i+1;
        }
    return r;
  }

  // returns true if all elements of v after start are empty
  bool emptyTail(const vector<string>& v, size_t start)
  {
    for (size_t i=start; i<v.size(); ++i)
      if (!v[i].empty()) return false;
    return true;
  }
}

void DataSpec::setDataArea(size_t row, size_t col)
{
  m_nRowAxes=row;
  m_nColAxes=col;
  if (headerRow>=row)
    headerRow=row>0? row-1: 0;
  if (dimensions.size()<nColAxes()) dimensions.resize(nColAxes());
  if (dimensionNames.size()<nColAxes()) dimensionNames.resize(nColAxes());
  // remove any dimensionCols > nColAxes
  dimensionCols.erase(dimensionCols.lower_bound(nColAxes()), dimensionCols.end());
}


template <class TokenizerFunction>
void DataSpec::givenTFguessRemainder(std::istream& input, const TokenizerFunction& tf)
{
    vector<size_t> starts;
    size_t nCols=0;
    string buf;
    size_t row=0;
    size_t firstEmpty=numeric_limits<size_t>::max();
    dimensionCols.clear();

    m_nRowAxes=0;
    for (; getline(input, buf) && row<maxRowsToAnalyse; ++row)
      {
        // remove trailing carriage returns
        if (buf.back()=='\r') buf=buf.substr(0,buf.size()-1);
        boost::tokenizer<TokenizerFunction> tok(buf.begin(),buf.end(), tf);
        vector<string> line(tok.begin(), tok.end());
        starts.push_back(firstNumerical(line));
        nCols=std::max(nCols, line.size());
        if (starts.back()==line.size())
          m_nRowAxes=row;
        if (starts.size()-1 < firstEmpty && starts.back()<nCols && emptyTail(line, starts.back()))
          firstEmpty=starts.size()-1;
      }
    // compute average of starts, then look for first row that drops below average
    double sum=0;
    for (unsigned long i=0; i<starts.size(); ++i) 
      sum+=starts[i];
    double av=sum/(starts.size());
    for (; starts.size()>m_nRowAxes && (starts[m_nRowAxes]>av); 
         ++m_nRowAxes);
    m_nColAxes=0;
    for (size_t i=nRowAxes(); i<starts.size(); ++i)
      m_nColAxes=std::max(m_nColAxes,starts[i]);
    // if more than 1 data column, treat the first row as an axis row
    if (m_nRowAxes==0 && nCols-m_nColAxes>1)
      m_nRowAxes=1;
    
    if (firstEmpty==m_nRowAxes) ++m_nRowAxes; // allow for possible colAxes header line
    headerRow=nRowAxes()>0? nRowAxes()-1: 0;
    for (size_t i=0; i<nColAxes(); ++i) dimensionCols.insert(i);
}

void DataSpec::guessRemainder(std::istream& input, char sep)
{
  separator=sep;
  if (separator==' ')
    givenTFguessRemainder(input,SpaceSeparatorParser(escape,separator,quote)); //asumes merged whitespace separators
  else
    givenTFguessRemainder(input,Parser(escape,separator,quote));
}


void DataSpec::guessFromStream(std::istream& input)
{
  size_t numCommas=0, numSemicolons=0, numTabs=0;
  size_t row=0;
  string buf;
  ostringstream streamBuf;
  for (; getline(input, buf) && row<maxRowsToAnalyse; ++row, streamBuf<<buf<<endl)
    for (auto c:buf)
      switch (c)
        {
        case ',':
          numCommas++;
          break;
        case ';':
          numSemicolons++;
          break;
        case '\t':
          numTabs++;
          break;
        }

  {
    istringstream inputCopy(streamBuf.str());
    if (numCommas>0.9*row && numCommas>numSemicolons && numCommas>numTabs)
      guessRemainder(inputCopy,',');
    else if (numSemicolons>0.9*row && numSemicolons>numTabs)
      guessRemainder(inputCopy,';');
    else if (numTabs>0.9*row)
      guessRemainder(inputCopy,'\t');
    else
      guessRemainder(inputCopy,' ');
  }

  {
    //fill in guessed dimension names
    istringstream inputCopy(streamBuf.str());
    guessDimensionsFromStream(inputCopy);
  }
}

void DataSpec::guessDimensionsFromStream(std::istream& i)
{
  if (separator==' ')
    guessDimensionsFromStream(i,SpaceSeparatorParser(escape,quote));
  else
    guessDimensionsFromStream(i,Parser(escape,separator,quote));
}
    
template <class T>
void DataSpec::guessDimensionsFromStream(std::istream& input, const T& tf)
{
  string buf;
  size_t row=0;
  for (; row<=headerRow; ++row) getline(input, buf);
  boost::tokenizer<T> tok(buf.begin(),buf.end(), tf);
  dimensionNames.assign(tok.begin(), tok.end());
  for (;row<=nRowAxes(); ++row) getline(input, buf);
  vector<string> data(tok.begin(),tok.end());
  for (size_t col=0; col<data.size() && col<nColAxes(); ++col)
    try
      {
        // only select value type if the datafield is a pure double
        size_t c;
        string s=stripWSAndDecimalSep(data[col]);
        double v=quotedStoD(s, c);
        if (c!=s.size()) throw 0; // try parsing as time
        dimensions.emplace_back(Dimension::value,"");
      }
    catch (...)
      {
        try
          {
            Dimension dim(Dimension::time,"%Y-Q%Q");
            anyVal(dim, data[col]);
            dimensions.push_back(dim);
          }
        catch (...)
          {
            try
              {
                Dimension dim(Dimension::time,"");
                anyVal(dim, data[col]);
                dimensions.push_back(dim);
              }
            catch (...)
              {
                dimensions.emplace_back(Dimension::string,"");
              }
          }
      }
}




namespace minsky
{
  template <class P>
  void reportFromCSVFileT(istream& input, ostream& output, const DataSpec& spec)
  {
    typedef vector<string> Key;
    map<Key,string> lines;
    multimap<Key,string> duplicateLines;
    string buf;
    P csvParser(spec.escape,spec.separator,spec.quote);
    for (size_t row=0; getline(input, buf); ++row)
      {
        if (row==spec.headerRow)
          {
            output<<"error"<<spec.separator<<buf<<endl;
            continue;
          }
        if (row>=spec.nRowAxes())
          {
            boost::tokenizer<P> tok(buf.begin(), buf.end(), csvParser);
            Key key;
            auto field=tok.begin();
            for (size_t i=0, dim=0; i<spec.nColAxes() && field!=tok.end(); ++i, ++field)
              if (spec.dimensionCols.count(i))
                key.push_back(*field);
            if (field==tok.end())
              {
                output<<"missing numerical data"<<spec.separator<<buf<<endl;
                continue;
              }

            for (; field!=tok.end(); ++field)
              {
                string x=*field;
                if (x.back()=='\r') x=x.substr(0,x.size()-1); //deal with MS nonsense
                if (!x.empty() && !isNumerical(x))
                  {
                    output<<"invalid numerical data"<<spec.separator<<buf<<endl;
                    continue;
                  }
                if (spec.columnar) break; // only one column to check
              }

            auto rec=lines.find(key);
            if (rec!=lines.end())
              {
                duplicateLines.insert(*rec);
                lines.erase(rec);
              }
            if (duplicateLines.count(key))
              duplicateLines.emplace(key, buf);
            else
              lines.emplace(key, buf);
          }
      }    
    for (auto& i: duplicateLines)
      output<<"duplicate key"<<spec.separator<<i.second<<endl;
    for (auto& i: lines)
      output<<spec.separator<<i.second<<endl;
  }

  void reportFromCSVFile(istream& input, ostream& output, const DataSpec& spec)
  {
    if (spec.separator==' ')
      reportFromCSVFileT<SpaceSeparatorParser>(input,output,spec);
    else
      reportFromCSVFileT<Parser>(input,output,spec);
  }

  template <class P>
  void loadValueFromCSVFileT(VariableValue& v, istream& input, const DataSpec& spec)
  {
    P csvParser(spec.escape,spec.separator,spec.quote);
    string buf;
    typedef vector<string> Key;
    map<Key,double> tmpData;
    multimap<Key,double> tmpAll; 
    map<Key,int> tmpCnt;
    vector<map<string,size_t>> dimLabels(spec.dimensionCols.size());
    bool tabularFormat=false;
    Hypercube hc;
    vector<string> horizontalLabels;
	       
    for (size_t i=0; i<spec.nColAxes(); ++i)
      if (spec.dimensionCols.count(i))
        hc.xvectors.push_back(i<spec.dimensionNames.size()? spec.dimensionNames[i]: "dim"+str(i));

    try
      {
        for (size_t row=0; getline(input, buf); ++row)
          {
            boost::tokenizer<P> tok(buf.begin(), buf.end(), csvParser);

            assert(spec.headerRow<=spec.nRowAxes());
            if (row==spec.headerRow && !spec.columnar) // in header section
              {
                vector<string> parsedRow(tok.begin(), tok.end());
                if (parsedRow.size()>spec.nColAxes()+1)
                  {
                    tabularFormat=true;
                    horizontalLabels.assign(parsedRow.begin()+spec.nColAxes(), parsedRow.end());
                    hc.xvectors.emplace_back(spec.horizontalDimName);
                    for (auto& i: horizontalLabels) hc.xvectors.back().push_back(i);
                    dimLabels.emplace_back();
                    for (size_t i=0; i<horizontalLabels.size(); ++i)
                      dimLabels.back()[horizontalLabels[i]]=i;
                  }
              }
            else if (row>=spec.nRowAxes())// in data section
              {
                Key key;
                auto field=tok.begin();
                for (size_t i=0, dim=0; i<spec.nColAxes() && field!=tok.end(); ++i, ++field)
                  if (spec.dimensionCols.count(i))
                    {
                      if (dim>=hc.xvectors.size())
                        hc.xvectors.emplace_back("?"); // no header present
                      key.push_back(*field);
                      if (dimLabels[dim].emplace(*field, dimLabels[dim].size()).second)
                        hc.xvectors[dim].push_back(*field);
                      dim++;
                    }
                    
                if (field==tok.end())
                  throw NoDataColumns();
          
                for (size_t col=0; field != tok.end(); ++field, ++col)
                  {
                    if (tabularFormat)
                      key.push_back(horizontalLabels[col]);

                    // remove thousands separators, and set decimal separator to '.' ("C" locale)
                    string s;
                    for (auto c: *field)
                      if (c==spec.decSeparator)
                        s+='.';
                      else if (!isspace(c) && c!='.' && c!=',')
                        s+=c;                    

                    auto i=tmpData.find(key);
                    bool valueExists=true;
                    double v=spec.missingValue;
                    try
                      {
                        v=stod(s);
                      }
                    catch (...) // value misunderstood
                      {
                        if (isnan(spec.missingValue)) // if spec.missingValue is NaN, then don't populate the tmpData map
                          valueExists=false;
                      }
                    if (valueExists)
                      {
                        if (i==tmpData.end())
                          tmpData.emplace(key,v);
                        else	
                          switch (spec.duplicateKeyAction)
                            {
                            case DataSpec::throwException:
                              throw DuplicateKey(key); 
                            case DataSpec::sum:
                              i->second+=v;
                              break;
                            case DataSpec::product:
                              i->second*=v;
                              break;
                            case DataSpec::min:
                              if (v<i->second)
                                i->second=v;
                              break;
                            case DataSpec::max:
                              if (v>i->second)
                                i->second=v;
                              break;
                            case DataSpec::av:
                              {
                                int& c=tmpCnt[key]; // c initialised to 0
                                i->second=((c+1)*i->second + v)/(c+2);
                                c++;
                              }
                              break;
                            }
                      }
                    if (tabularFormat)
                      key.pop_back();
                  }
              }
          }
                  
        size_t numHyperCubeElems=1;
        for (auto& i : hc.xvectors) numHyperCubeElems*=i.size();
                           
        double sparsityRatio =  static_cast<double>(1.0-static_cast<double>(tmpData.size())/numHyperCubeElems); 
		
        if (sparsityRatio <= 0.5) 
          { // dense case
            v.index({});
            if (!cminsky().checkMemAllocation(hc.numElements()*sizeof(double)))
              throw runtime_error("memory threshold exceeded");            
            v.hypercube(hc);
            // stash the data into vv tensorInit field
            v.tensorInit.hypercube(hc);
            for (auto& i: v.tensorInit)
              i=spec.missingValue;
            auto dims=v.hypercube().dims();
            for (auto& i: tmpData)
              {
                size_t idx=0;
                assert (hc.rank()==i.first.size());
                assert(dimLabels.size()==hc.rank());
                for (int j=hc.rank()-1; j>=0; --j)
                  {
                    assert(dimLabels[j].count(i.first[j]));
                    idx = (idx*dims[j]) + dimLabels[j][i.first[j]];
                  }
                v.tensorInit[idx]=i.second;  
              }
          }    
        else 
          { // sparse case	
            if (!cminsky().checkMemAllocation(tmpData.size()*sizeof(double)))
              throw runtime_error("memory threshold exceeded");	  	  		
            auto dims=hc.dims();
              
            map<size_t,double> indexValue; // intermediate stash to sort index vector
            for (auto& i: tmpData)
              {
                size_t idx=0;
                assert (dims.size()==i.first.size());
                assert(dimLabels.size()==dims.size());
                for (int j=dims.size()-1; j>=0; --j)
                  {
                    assert(dimLabels[j].count(i.first[j]));
                    idx = (idx*dims[j]) + dimLabels[j][i.first[j]];
                  }
                if (!isnan(i.second))
                  indexValue.emplace(idx, i.second);
              }
            
            vector<size_t> index;
            for (auto& i: indexValue)
              {
                index.push_back(i.first);
                v.tensorInit.push_back(i.first, i.second);
              }
            v.index(index);
            v.hypercube(hc);
            v.tensorInit.hypercube(hc);
          }                 

      }
    catch (const std::bad_alloc&)
      { // replace with a more user friendly error message
        throw std::runtime_error("exhausted memory - try reducing the rank");
      }
    catch (const std::length_error&)
      { // replace with a more user friendly error message
        throw std::runtime_error("exhausted memory - try reducing the rank");
      }
  }
  
  void loadValueFromCSVFile(VariableValue& v, istream& input, const DataSpec& spec)
  {
    if (spec.separator==' ')
      loadValueFromCSVFileT<SpaceSeparatorParser>(v,input,spec);
    else
      loadValueFromCSVFileT<Parser>(v,input,spec);
  }
}
