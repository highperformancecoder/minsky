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

#include "minsky.h"
#include "CSVParser.h"

#include "CSVParser.rcd"
#include "dataSpecSchema.rcd"
#include "dimension.rcd"
#include "nobble.h"
#include "minsky_epilogue.h"

#if defined(__linux__)
#include <sys/sysinfo.h>
#endif

using namespace minsky;
using namespace std;

#include <boost/type_traits.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>

namespace escapedListSeparator
{
  // pinched from boost::escape_list_separator, and modified to not throw
  template <class Char,
            class Traits = BOOST_DEDUCED_TYPENAME std::basic_string<Char>::traits_type >
  class EscapedListSeparator {

  private:
    typedef std::basic_string<Char,Traits> string_type;
    struct char_eq {
      Char e_;
      char_eq(Char e):e_(e) { }
      bool operator()(Char c) {
        return Traits::eq(e_,c);
      }
    };
    string_type  escape_;
    string_type  c_;
    string_type  quote_;
    bool last_;

    bool is_escape(Char e) {
      char_eq f(e);
      return std::find_if(escape_.begin(),escape_.end(),f)!=escape_.end();
    }
    bool is_c(Char e) {
      char_eq f(e);
      return std::find_if(c_.begin(),c_.end(),f)!=c_.end();
    }
    bool is_quote(Char e) {
      char_eq f(e);
      return std::find_if(quote_.begin(),quote_.end(),f)!=quote_.end();
    }
    template <typename iterator, typename Token>
    void do_escape(iterator& next,iterator end,Token& tok) {
      if (++next == end)
        // don't throw, but pass on verbatim
        tok+=escape_.front();
      if (Traits::eq(*next,'n')) {
        tok+='\n';
        return;
      }
      if (is_quote(*next)) {
        tok+=*next;
        return;
      }
      if (is_c(*next)) {
        tok+=*next;
        return;
      }
      if (is_escape(*next)) {
        tok+=*next;
        return;
      }
      // don't throw, but pass on verbatim
      tok+=escape_.front()+*next;
    }

  public:

    explicit EscapedListSeparator(Char  e = '\\',
                                  Char c = ',',Char  q = '\"')
      : escape_(1,e), c_(1,c), quote_(1,q), last_(false) { }

    EscapedListSeparator(string_type e, string_type c, string_type q)
      : escape_(e), c_(c), quote_(q), last_(false) { }

    void reset() {last_=false;}

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next,InputIterator end,Token& tok) {
      bool bInQuote = false;
      tok = Token();

      if (next == end) {
        if (last_) {
          last_ = false;
          return true;
        }
        return false;
      }
      last_ = false;
      for (;next != end;++next) {
        if (is_escape(*next)) {
          do_escape(next,end,tok);
        }
        else if (is_c(*next)) {
          if (!bInQuote) {
            // If we are not in quote, then we are done
            ++next;
            // The last character was a c, that means there is
            // 1 more blank field
            last_ = true;
            return true;
          }
          tok+=*next;
        }
        else if (is_quote(*next)) {
          bInQuote=!bInQuote;
        }
        else {
          tok += *next;
        }
      }
      return true;
    }
  };
}
using Parser=escapedListSeparator::EscapedListSeparator<char>;

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

namespace
{
  /// An any with cached hash
  struct Any: public any
  {
    Any()=default;
    Any(const any& x): any(x), hash(x.hash()) {}
    bool operator<(const Any& x) const {return static_cast<const any&>(*this)<x;}
    bool operator==(const Any& x) const {return static_cast<const any&>(*this)==x;}
    size_t hash;
  };

  std::string str(const Any& x) {return minsky::str(static_cast<const any&>(x));}
  
  struct NoDataColumns: public std::exception
  {
    const char* what() const noexcept override {return "No data columns";}
  };
  struct DuplicateKey: public std::exception
  {
    std::string msg="Duplicate key";
    DuplicateKey(const vector<Any>& x) {
      for (auto& i: x)
        msg+=":"+str(i);
    }
    DuplicateKey(const vector<string>& x) {
      for (auto& i: x)
        msg+=":"+i;
    }
    const char* what() const noexcept override {return msg.c_str();}
  };

  double quotedStoD(const string& s,size_t& charsProcd)
  {
    //strip possible quote characters
    if (!s.empty() && s[0]==s[s.size()-1] && !isalnum(s[0]))
      {
        double r=quotedStoD(s.substr(1,s.size()-2),charsProcd);
        charsProcd+=2;
        return r;
      }
    // strip any leading non-numerical characters ([^0-9.,])
    auto n=s.find_first_of("0123456789,.");
    return stod(s.substr(n),&charsProcd);
  }

  string stripWSAndDecimalSep(const string& s)
  {
    string r;
    for (auto c: s)
      if (!isspace(c) && c!=',' && c!='.')
        r+=c;
    return r;
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

namespace minsky
{
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
}

namespace std
{
  template <>
  struct hash<Any>
  {
    size_t operator()(const Any& x) const {return x.hash;}
  };
  template <class T>
  struct hash<vector<T>>
  {
    size_t operator()(const vector<T>& x) const {
      size_t r=0;
      for (auto& i: x) r^=std::hash<T>()(i);
      return r;
    }
  };

}

void DataSpec::setDataArea(size_t row, size_t col)
{
  m_nRowAxes=row;
  const size_t maxCols=16384; // Excel's limit
  m_nColAxes=std::min(col, maxCols);
  numCols=std::max(numCols, m_nColAxes);
  if (headerRow>=row)
    headerRow=row>0? row-1: 0;
  if (row==headerRow) row++; //TODO handle no header properly
  if (dimensions.size()<nColAxes()) dimensions.resize(nColAxes());
  if (dimensionNames.size()<nColAxes()) dimensionNames.resize(nColAxes());
  // remove any dimensionCols > nColAxes
  dimensionCols.erase(dimensionCols.lower_bound(nColAxes()), dimensionCols.end());
  // adjust ignored columns
  for (unsigned i=0; i<m_nColAxes; ++i)
    dataCols.erase(i);
  for (unsigned i=m_nColAxes; i<numCols; ++i)
    dataCols.insert(i);
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
    for (; getline(input, buf) && row<CSVDialog::numInitialLines; ++row)
      {
        // remove trailing carriage returns
        if (buf.back()=='\r') buf=buf.substr(0,buf.size()-1);
        boost::tokenizer<TokenizerFunction> tok(buf.begin(),buf.end(), tf);
        vector<string> line(tok.begin(), tok.end());
        if (!line.empty())
          {
            smatch match;
            static const regex re("RavelHypercube=(.*)");
            if (regex_match(line[0], match, re))
              try
                {
                    populateFromRavelMetadata(match[1], row);
                    return;
                }
              catch (...)
                {
                  continue; // in case of error, ignore the RavelHypercube line.
                }
          }
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
    // if nRowAxes exceeds numInitialLines, assume first row is a header row, and that that is all there is.
    if (m_nRowAxes>=row-1) m_nRowAxes=1;
    m_nColAxes=0;
    for (size_t i=nRowAxes(); i<starts.size(); ++i)
      m_nColAxes=std::max(m_nColAxes,starts[i]);
    // if more than 1 data column, treat the first row as an axis row
    if (m_nRowAxes==0 && nCols-m_nColAxes>1)
      m_nRowAxes=1;
    
    if (firstEmpty==m_nRowAxes) ++m_nRowAxes; // allow for possible colAxes header line
    headerRow=nRowAxes()>0? nRowAxes()-1: 0;
    size_t i=0;
    for (; i<nColAxes(); ++i) dimensionCols.insert(i);
    for (; i<nCols; ++i) dataCols.insert(i);
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
  for (; getline(input, buf) && row<CSVDialog::numInitialLines; ++row, streamBuf<<buf<<endl)
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

  if (dimensionNames.empty())
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
        quotedStoD(s, c);
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

 void DataSpec::populateFromRavelMetadata(const std::string& metadata, size_t row)
 {
   vector<NamedDimension> ravelMetadata;
   json(ravelMetadata,metadata);
   columnar=true;
   headerRow=row+2;
   setDataArea(headerRow, ravelMetadata.size());
   dimensionNames.clear();
   dimensions.clear();
   for (auto& i: ravelMetadata)
     {
       dimensions.push_back(i.dimension);
       dimensionNames.push_back(i.name);
     }
   for (size_t i=0; i<dimensions.size(); ++i)
     dimensionCols.insert(i);
 }

namespace minsky
{
  template <class P>
  void reportFromCSVFileT(istream& input, ostream& output, const DataSpec& spec)
  {
    typedef std::vector<std::string> Key;
    map<Key,string> lines;
    multimap<Key,string> duplicateLines;
    string buf;
    P csvParser(spec.escape,spec.separator,spec.quote);
    for (size_t row=0; getline(input, buf); ++row)
      {
        // remove trailing carriage returns
        if (!buf.empty() && buf.back()=='\r') buf=buf.substr(0,buf.size()-1);
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
            size_t i=0;
            for (; field!=tok.end(); ++i, ++field)
              if (spec.dimensionCols.count(i))
                key.push_back(*field);

            for (i=0, field=tok.begin(); field!=tok.end(); ++i, ++field)
              if ((spec.dataCols.empty() && i>=spec.nColAxes()) || spec.dataCols.count(i))
                {
                  string x=*field;
                  if (!x.empty())
                    {
                      if (x.back()=='\r') x=x.substr(0,x.size()-1); //deal with MS nonsense
                      if (!isNumerical(x))
                        {
                          output<<"invalid numerical data"<<spec.separator<<buf<<endl;
                          continue;
                        }
                    }
                  if (spec.columnar) break; // only one column to check
                }

            if ((spec.dataCols.empty() && i<=spec.nColAxes()) || i<=*spec.dataCols.end())
              output<<"missing numerical data"<<spec.separator<<buf<<endl;
            
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

  // handle DOS files with '\r' '\n' line terminators
  void chomp(string& buf)
  {
    if (!buf.empty() && buf.back()=='\r')
      buf.erase(buf.size()-1);
  }
  
  // gets a line, accounting for quoted newlines
  bool getWholeLine(istream& input, string& line, char quote, char separator)
  {
    bool r=getline(input,line).good();
    chomp(line);
    while (r)
      {
        // count the number of quote characters after last separator. If odd, then line is not terminated correctly
        auto n=line.rfind(separator);
        if (n==string::npos)
          n=0;
        else
          ++n;
        int quoteCount=0;
        for (; n<line.size(); ++n)
          if (line[n]==quote)
            ++quoteCount;
        if (quoteCount%2==0) break; // data line correctly terminated
        string buf;
        r=getline(input,buf).good(); // read next line and append
        chomp(buf);
        line+=buf;
      }
    return r;
  }
  
  template <class P>
  void loadValueFromCSVFileT(VariableValue& vv, istream& input, const DataSpec& spec, uintmax_t fileSize)
  {
    BusyCursor busy(minsky());
    P csvParser(spec.escape,spec.separator,spec.quote);
    string buf;
    typedef vector<string> Key;
    unordered_map<Key,double> tmpData;
    unordered_map<Key,int> tmpCnt;
    vector<unordered_map<typename Key::value_type, size_t>> dimLabels(spec.dimensionCols.size());
    bool tabularFormat=false;
    Hypercube hc;
    vector<typename Key::value_type> horizontalLabels;
    vector<AnyVal> anyVal;

    ProgressUpdater pu(minsky().progressState, "Importing CSV",3);
    for (auto i: spec.dimensionCols)
      {
        hc.xvectors.push_back(i<spec.dimensionNames.size()? spec.dimensionNames[i]: "dim"+str(i));
        hc.xvectors.back().dimension=spec.dimensions[i];
        anyVal.emplace_back(spec.dimensions[i]);
      }
    ++minsky().progressState;
    size_t row=0, col=0;
    uintmax_t bytesRead=0;
    try
      {
        ProgressUpdater pu(minsky().progressState, "Parsing file",1);
        // skip header lines except for headerRow
        if (spec.headerRow<spec.nRowAxes() && !spec.columnar)
          {
            for (; row<spec.headerRow; ++row)
              getline(input,buf);
            getWholeLine(input, buf, spec.quote, spec.separator);
            ++row;
            boost::tokenizer<P> tok(buf.begin(), buf.end(), csvParser);
            vector<string> parsedRow(tok.begin(), tok.end());
            tabularFormat=spec.dataCols.size()>1 || (spec.dataCols.empty() && parsedRow.size()>spec.nColAxes()+1);
            if (tabularFormat)
              {
                anyVal.emplace_back(spec.horizontalDimension);
                // legacy situation where all data columns are to the right
                if (spec.dataCols.empty() && parsedRow.size()>spec.nColAxes()+1)
                  for (auto i=parsedRow.begin()+spec.nColAxes(); i!=parsedRow.end(); ++i)
                    horizontalLabels.emplace_back(str(anyVal.back()(*i),spec.horizontalDimension.units));
                else
                  // explicitly specified data columns
                  for (auto i: spec.dataCols)
                    if (i<parsedRow.size())
                      horizontalLabels.emplace_back(str(anyVal.back()(parsedRow[i]),spec.horizontalDimension.units));
                hc.xvectors.emplace_back(spec.horizontalDimName);
                hc.xvectors.back().dimension=spec.horizontalDimension;
                set<typename Key::value_type> uniqueLabels;
                dimLabels.emplace_back();
                for (auto& i: horizontalLabels)
                  if (uniqueLabels.insert(i).second)
                    {
                      dimLabels.back()[i]=hc.xvectors.back().size();
                      hc.xvectors.back().emplace_back(i);
                    }
              }
          }
        for (; row<spec.nRowAxes(); ++row)
          getline(input,buf);
            
        
        for (; getWholeLine(input, buf, spec.quote, spec.separator); ++row)
          {
#if defined(__linux__) // TODO remove or generalise
            {
              struct sysinfo s;
              sysinfo(&s);
              if (s.freeram<1000000)
                throw runtime_error("exhausted memory");
            }
#endif
            boost::tokenizer<P> tok(buf.begin(), buf.end(), csvParser);

            assert(spec.headerRow<=spec.nRowAxes());
            Key key;
            auto field=tok.begin();
            size_t dim=0, dataCols=0;
            col=0;
            for (auto field=tok.begin(); field!=tok.end(); ++col, ++field)
              if (spec.dimensionCols.count(col))
                {
                  // detect blank data lines (favourite Excel artifact)
                  if (spec.dimensions[dim].type!=Dimension::string && field->empty())
                    goto keyEmptyGotoNextLine;
                  
                  if (dim>=hc.xvectors.size())
                    hc.xvectors.emplace_back("?"); // no header present
                  try
                    {
                      auto keyElem=anyVal[dim](*field);
                      auto skeyElem=str(keyElem, spec.dimensions[dim].units);
                      if (dimLabels[dim].emplace(skeyElem, dimLabels[dim].size()).second)
                        hc.xvectors[dim].emplace_back(keyElem);
                      key.emplace_back(skeyElem);
                    }
                  catch (...)
                    {
                      throw std::runtime_error("Invalid data: "+*field+" for "+
                                               to_string(spec.dimensions[dim].type)+
                                               " dimensioned column: "+spec.dimensionNames[dim]);
                    }
                  dim++;
                }

            col=0;
            for (auto field=tok.begin(); field!=tok.end(); ++col,++field)
              if ((spec.dataCols.empty() && col>=spec.nColAxes()) || spec.dataCols.count(col)) 
                {                    
                  if (tabularFormat)
                    key.emplace_back(horizontalLabels[dataCols]);
                  else if (dataCols)
                    break; // only 1 value column, everything to right ignored
                  
                  // remove thousands separators, and set decimal separator to '.' ("C" locale)
                  string s;
                  for (auto c: *field)
                    if (c==spec.decSeparator)
                      s+='.';
                    else if (s.empty() && !isdigit(c))
                      continue; // skip non-numeric prefix
                    else if (!isspace(c) && c!='.' && c!=',')
                      s+=c;                    
                  
                  // TODO - this disallows special floating point values - is this right?
                  bool valueExists=!s.empty() && (isdigit(s[0])||s[0]=='-'||s[0]=='+'||s[0]=='.');
                  if (valueExists || !isnan(spec.missingValue))
                    {
                      auto i=tmpData.find(key);
                      double v=spec.missingValue;
                      if (valueExists)
                        try
                          {
                            v=stod(s);
                            if (i==tmpData.end())
                              tmpData.emplace(key,v);
                          }
                        catch (...) // value misunderstood
                          {
                            if (isnan(spec.missingValue)) // if spec.missingValue is NaN, then don't populate the tmpData map
                              valueExists=false;
                          }
                      if (valueExists && i!=tmpData.end())
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
                  dataCols++;
                  if (tabularFormat)
                    key.pop_back();
                  else
                    break; // only one column of data needs to be read
                }
            
            if (!dataCols)
              throw NoDataColumns();
            

            bytesRead+=buf.size();
            pu.setProgress(double(bytesRead)/fileSize);
          keyEmptyGotoNextLine:;
          }
        ++minsky().progressState;

        // remove zero length dimensions
        {
          auto d=dimLabels.begin();
          assert(hc.xvectors.size()==dimLabels.size());
          for (auto i=hc.xvectors.begin(); i!=hc.xvectors.end();)
              if (i->empty())
                {
                  hc.xvectors.erase(i);
                  dimLabels.erase(d);
                }
              else
                {
                  ++i;
                  ++d;
                }
          assert(hc.xvectors.size()==dimLabels.size());
        }
        
        for (auto& xv: hc.xvectors)
          xv.imposeDimension();
        ++minsky().progressState;

        if (log(tmpData.size())-hc.logNumElements()>=log(0.5)) 
          { // dense case
            vv.index({});
            if (!cminsky().checkMemAllocation(hc.numElements()*sizeof(double)))
              throw runtime_error("memory threshold exceeded");            
            vv.hypercube(hc);
            // stash the data into vv tensorInit field
            vv.tensorInit.index({});
            vv.tensorInit.hypercube(hc);
            for (auto& i: vv.tensorInit)
              i=spec.missingValue;
            auto dims=vv.hypercube().dims();
            ProgressUpdater pu(minsky().progressState,"Loading data",tmpData.size());
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
                vv.tensorInit[idx]=i.second;
                ++minsky().progressState;
              }
          }    
        else 
          { // sparse case	
            if (!cminsky().checkMemAllocation(tmpData.size()*sizeof(double)))
              throw runtime_error("memory threshold exceeded");	  	  		
            auto dims=hc.dims();
            ProgressUpdater pu(minsky().progressState,"Indexing and loading",2);
              
            map<size_t,double> indexValue; // intermediate stash to sort index vector
            {
              ProgressUpdater pu(minsky().progressState,"Building index",tmpData.size());
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
                  ++minsky().progressState;
                }

              vv.tensorInit.index(indexValue);
              vv.tensorInit.hypercube(hc);
            }
            {
              ProgressUpdater pu(minsky().progressState,"Loading data",indexValue.size());
              size_t j=0;
              for (auto& i: indexValue)
                {
                  vv.tensorInit[j++]=i.second;
                  ++minsky().progressState;
                }
              vv=vv.tensorInit;
            }
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
    catch (const std::exception& ex)
      {
        throw std::runtime_error(string(ex.what())+" at line:"+to_string(row)+", col:"+to_string(col));
      }
  }
  
  void loadValueFromCSVFile(VariableValue& v, istream& input, const DataSpec& spec, uintmax_t fileSize)
  {
    if (spec.separator==' ')
      loadValueFromCSVFileT<SpaceSeparatorParser>(v,input,spec,fileSize);
    else
      loadValueFromCSVFileT<Parser>(v,input,spec,fileSize);
  }
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::DataSpec);
