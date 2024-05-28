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

#ifdef _WIN32
#include <memoryapi.h>
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/resource.h>
#endif

using namespace minsky;
using namespace std;

#include <boost/type_traits.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <boost/pool/pool.hpp>

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
      const char_eq f(e);
      return std::find_if(escape_.begin(),escape_.end(),f)!=escape_.end();
    }
    bool is_c(Char e) {
      const char_eq f(e);
      return std::find_if(c_.begin(),c_.end(),f)!=c_.end();
    }
    bool is_quote(Char e) {
      const char_eq f(e);
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
    bool operator==(const Any& x) const {return static_cast<const any&>(*this)==static_cast<const any&>(x);}
    size_t hash;
  };

  std::string str(const Any& x) {return minsky::str(static_cast<const any&>(x));}

  // slice label token map
  template <class T>
  class Tokens
  {
    unordered_map<string, T> tokens;
    vector<const string*> tokenRefs;
    string empty;
  public:
    T operator[](const string& x) {
      auto i=tokens.find(x);
      if (i==tokens.end())
        {
          i=tokens.emplace(x, tokenRefs.size()).first;
          tokenRefs.push_back(&(i->first));
        }
      return i->second;
    }
    const string& operator[](T i) const {
      if (i<tokenRefs.size()) return *tokenRefs[i];
      return empty;
    }
  };

  // a std::allocator class that tracks the number of bytes allocated
  struct TrackingAllocatorBase
  {
    static size_t allocatedBytes;
    static size_t poolSize;
    static char* pool;
    static char* nextPool;
    static bool destructing;
    static void allocatePool() {
      poolSize=minsky::minsky().physicalMem();
#ifdef _WIN32
      pool=reinterpret_cast<char*>(VirtualAlloc(nullptr,poolSize,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
      if (!pool)
        {
          char message[1024];
          FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,nullptr,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),message,sizeof(message),nullptr);
          cout<<message<<endl;
          throw bad_alloc();
        }
#else
      pool=reinterpret_cast<char*>
        (mmap(nullptr,poolSize,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
      if (pool==MAP_FAILED)
        {
          perror("mmap failed");
          throw bad_alloc();
        }
#endif
      nextPool=pool;
      allocatedBytes=0;
      destructing=false;
    }
    static void deallocatePool() {
#ifdef _WIN32
      VirtualFree(pool,poolSize,MEM_DECOMMIT);
#else
      munmap(pool,poolSize);
#endif
      pool=nextPool=nullptr;
    }
  };

  size_t TrackingAllocatorBase::allocatedBytes=0;
  size_t TrackingAllocatorBase::poolSize=0;
  char* TrackingAllocatorBase::pool=nullptr;
  char* TrackingAllocatorBase::nextPool=nullptr;
  bool TrackingAllocatorBase::destructing=false;
  
  template <class T>
  struct TrackingAllocator: TrackingAllocatorBase
  {
    using value_type=T;
    TrackingAllocator()=default;
    template <class U> TrackingAllocator(const TrackingAllocator<U>&) {}
    unordered_map<size_t, vector<T*>> freeList;
    T* allocate(size_t n) {
      auto i=freeList.find(n);
      if (i!=freeList.end() && !i->second.empty())
        {
          auto r=i->second.back();
          i->second.pop_back();
          return r;
        }
      auto size=n*sizeof(T);
      // align to 8 byte boundary
      auto mod=size & 7;
      if (mod) size=(size-mod)+8;
      allocatedBytes+=size;
      if (!pool || allocatedBytes>poolSize)
        throw bad_alloc();
      auto r=reinterpret_cast<T*>(nextPool);
      nextPool+=size;
      return r;
    }
    void deallocate(T* p, size_t n) {
      if (pool && !destructing)
        freeList[n].push_back(p); // recycle allocation
    }
  };

  
  using SliceLabelToken=uint32_t;
  using Key=vector<SliceLabelToken, TrackingAllocator<SliceLabelToken>>;
  template <class V> using Map=map<Key,V,less<Key>,TrackingAllocator<pair<const Key,V>>>;

  struct NoDataColumns: public std::exception
  {
    const char* what() const noexcept override {return "No data columns specified\nIf dataset has no data, try selecting counter";}
  };
  
  struct DuplicateKey: public std::exception
  {
    std::string msg="Duplicate key";
    Key key;
    DuplicateKey(const Key& x, const Tokens<SliceLabelToken>& tokens): key(x) {
      for (auto& i: x)
        msg+=":"+tokens[i];
      msg+="\nTry selecting a different duplicate key action";
    }
    const char* what() const noexcept override {return msg.c_str();}
  };

  struct InvalidData: public std::exception
  {
    string data; ///< data received in field
    string type; ///< type of data
    string colName; ///< column name
    string msg;
    InvalidData(const string& data, const string& type,const string& colName):
      data(data), type(type), colName(colName)
    {msg="Invalid data: "+data+" for "+type+" dimensioned column: "+colName;}
    const char* what() const noexcept override {return msg.c_str();}
  };

  struct ShortLine: public std::exception
  {
    std::string msg="Short line";
    ShortLine(const Key& x, const Tokens<SliceLabelToken>& tokens) {
      for (auto& i: x)
        msg+=":"+tokens[i];
    }
    const char* what() const noexcept override {return msg.c_str();}
  };
  
  struct MemoryExhausted: public std::exception
  {
    const char* what() const noexcept override {return "exhausted memory - try reducing the rank";}
  };
  
  double quotedStoD(const string& s,size_t& charsProcd)
  {
    //strip possible quote characters
    if (!s.empty() && s[0]==s[s.size()-1] && !isalnum(s[0]))
      {
        const double r=quotedStoD(s.substr(1,s.size()-2),charsProcd);
        charsProcd+=2;
        return r;
      }
    if (s.empty()) return nan(""); // treat empty cell as a missing value
    // first try to read the cell as a number
    try {
      const double r=stod(s,&charsProcd);
      if (charsProcd==s.size())
        return r;
    }
    catch (...) {}
    // if not, then strip any leading non-numerical characters ([^0-9.,+-])
    auto n=s.find_first_of("0123456789,.+-");
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
              size_t c=0;
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
    size_t charsProcd=0;
    const string stripped=stripWSAndDecimalSep(s);
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
  m_nColAxes=std::min(col, maxColumn);
  numCols=std::max(numCols, m_nColAxes);
  if (headerRow>=row)
    headerRow=row>0? row-1: 0;
  if (dimensions.size()<nColAxes()) dimensions.resize(nColAxes());
  if (dimensionNames.size()<nColAxes()) dimensionNames.resize(nColAxes());
  // remove any dimensionCols > nColAxes
  dimensionCols.erase(dimensionCols.lower_bound(nColAxes()), dimensionCols.end());
  // adjust ignored columns
  for (unsigned i=0; i<m_nColAxes; ++i)
    dataCols.erase(i);
  for (unsigned i=m_nColAxes; i<numCols && i<maxColumn; ++i)
    dataCols.insert(i);
}


template <class TokenizerFunction>
void DataSpec::givenTFguessRemainder(std::istream& initialInput, std::istream& remainingInput, const TokenizerFunction& tf)
{
  starts.clear();
  nCols=0;
  row=0;
  firstEmpty=numeric_limits<size_t>::max();
  m_nRowAxes=0;

  const BusyCursor busy(minsky());
  // we don't know how many times we'll be going around the loop here, so pick a largish number for the progress bar
  const ProgressUpdater pu(minsky().progressState,"Guessing CSV format",100);

  try
    {
      if (processChunk(initialInput, tf, CSVDialog::numInitialLines))
        return; // found a Ravel hypercube line.
      ++minsky().progressState;
      while (!processChunk(remainingInput, tf, row+CSVDialog::numInitialLines))
        ++minsky().progressState;
    }
  catch (std::exception&)
    {
      // progressState throws an exception on being cancelled by the user
      throw std::runtime_error("CSV format guess terminated by user, best guess specification used.");
    }
}

void DataSpec::guessRemainder(std::istream& initialInput, std::istream& remainingInput, char sep)
{
  separator=sep;
  if (separator==' ')
    givenTFguessRemainder(initialInput, remainingInput, SpaceSeparatorParser(escape,separator,quote)); //assumes merged whitespace separators
  else
    givenTFguessRemainder(initialInput, remainingInput, Parser(escape,separator,quote));
}

template <class TokenizerFunction>
bool DataSpec::processChunk(std::istream& input, const TokenizerFunction& tf, size_t until)
{
  string buf;
  for (; getline(input, buf) && row<until; ++row)
    {
      if (buf.empty()) continue;
      // remove trailing carriage returns
      if (buf.back()=='\r') buf=buf.substr(0,buf.size()-1);
      if (!buf.empty())
        {
          smatch match;
          static const regex re("\"RavelHypercube=(.*)\"");
          if (regex_match(buf, match, re))
            try
              {
                string metadata=match[1];
                // remove leaning toothpicks
                metadata.erase(remove(metadata.begin(),metadata.end(),'\\'));
                string horizontalName;
                getline(input, buf);
                static const regex re("HorizontalDimension=\"(.*)\"");
                if (regex_match(buf, match, re))
                  {
                    horizontalName=match[1];
                    ++row;
                  }
                populateFromRavelMetadata(metadata, horizontalName, row);
                return true;
              }
            catch (...)
              {
                continue; // in case of error, ignore the RavelHypercube line.
              }
        }
      const boost::tokenizer<TokenizerFunction> tok(buf.begin(),buf.end(), tf);
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
  const double av=sum/(starts.size());
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
  dimensionCols.clear();
  for (; i<nColAxes() && i<maxColumn; ++i) dimensionCols.insert(i);
  dataCols.clear();
  for (; i<nCols && i<maxColumn; ++i) dataCols.insert(i);
  return !input;
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
      guessRemainder(inputCopy,input,',');
    else if (numSemicolons>0.9*row && numSemicolons>numTabs)
      guessRemainder(inputCopy,input,';');
    else if (numTabs>0.9*row)
      guessRemainder(inputCopy,input,'\t');
    else
      guessRemainder(inputCopy,input,' ');
  }
}

void DataSpec::populateFromRavelMetadata(const std::string& metadata, const string& horizontalName, size_t row)
 {
   vector<NamedDimension> ravelMetadata;
   json(ravelMetadata,metadata);
   headerRow=row+2;
   setDataArea(headerRow, ravelMetadata.size());
   dimensionNames.clear();
   dimensions.clear();
   for (auto& i: ravelMetadata)
     if (i.name==horizontalName)
       {
         horizontalDimension=i.dimension;
         horizontalDimName=i.name;
       }
     else
       {
         dimensions.push_back(i.dimension);
         dimensionNames.push_back(i.name);
       }
   for (size_t i=0; i<dimensions.size(); ++i)
     dimensionCols.insert(i);
 }

namespace minsky
{
  // handle DOS files with '\r' '\n' line terminators
  void chomp(string& buf)
  {
    if (!buf.empty() && buf.back()=='\r')
      buf.erase(buf.size()-1);
  }
  
  // gets a line, accounting for quoted newlines
  bool getWholeLine(istream& input, string& line, const DataSpec& spec)
  {
    line.clear();
    bool r=getline(input,line).good();
    chomp(line);
    while (r)
      {
        int quoteCount=0;
        for (auto i: line)
          if (i==spec.quote)
            ++quoteCount;
        if (quoteCount%2==0) break; // data line correctly terminated
        string buf;
        r=getline(input,buf).good(); // read next line and append
        chomp(buf);
        line+=buf;
      }
    escapeDoubledQuotes(line,spec);
    return r || !line.empty();
  }

  void escapeDoubledQuotes(std::string& line,const DataSpec& spec)
  {
    // replace doubled quotes with escape quote
    for (size_t i=1; i<line.size(); ++i)
      if (line[i]==spec.quote && line[i-1]==spec.quote &&
          ((i==1 && (i==line.size()-1|| line[i+1]!=spec.quote)) ||                                       // deal with leading ""
           (i>1 &&
            ((line[i-2]!=spec.quote && line[i-2]!=spec.escape &&
              (line[i-2]!=spec.separator || i==line.size()-1|| line[i+1]!=spec.quote))  // deal with ,''
             ||            // deal with "" middle or end
             (line[i-2]==spec.quote && (i==2 || line[i-3]==spec.separator || line[i-3]==spec.escape)))))) // deal with leading """
        line[i-1]=spec.escape;
  }

  /// handle reporting errors in loadValueFromCSVFileT when loading files
  struct OnError
  {
    /// called on error - \a ex message to pass on, \a row - current row
    template <class E> void operator()(const E& ex, size_t row) {throw ex;}
    /// update a map of keys to first rows for duplicate key processing
    void rowKeyInsert(const Key&, size_t) {}
  };

  template <class P>
  struct ParseCSV
  {
    Map<double> tmpData;  ///< map of data by key
    vector<unordered_map<typename Key::value_type, size_t>> dimLabels;
    Hypercube hc;
    size_t row=0, col=0;
    
    ~ParseCSV() {TrackingAllocatorBase::destructing=true;}
    template <class E>
    ParseCSV(istream& input, const DataSpec& spec, uintmax_t fileSize, E& onError, bool checkValues=false):
      dimLabels(spec.dimensionCols.size())
    {
      const BusyCursor busy(minsky());
      const ProgressUpdater pu(minsky().progressState, "Parsing CSV",2);

      P csvParser(spec.escape,spec.separator,spec.quote);
      string buf;
      Tokens<SliceLabelToken> sliceLabelTokens;
      Map<int> tmpCnt;
      bool tabularFormat=false;
      vector<typename Key::value_type> horizontalLabels;
      vector<AnyVal> anyVal;
      bool memUsageChecked=false;

      try
        {
          for (auto i: spec.dimensionCols)
            {
              hc.xvectors.push_back(i<spec.dimensionNames.size()? spec.dimensionNames[i]: "dim"+str(i));
              hc.xvectors.back().dimension=spec.dimensions[i];
              anyVal.emplace_back(spec.dimensions[i]);
            }
          ++minsky().progressState;
          uintmax_t bytesRead=0;
      
          // skip header lines except for headerRow
          tabularFormat=spec.dataCols.size()>1 || (spec.dataCols.empty() && spec.numCols>spec.nColAxes()+1);
          if (tabularFormat)
            {
              anyVal.emplace_back(spec.horizontalDimension);
              // legacy situation where all data columns are to the right
              if (spec.dataCols.empty())
                for (size_t i=spec.nColAxes(); i<spec.dimensionNames.size(); ++i)
                  {
                    col=i;
                    horizontalLabels.emplace_back(sliceLabelTokens[str(anyVal.back()(spec.dimensionNames[i]),spec.horizontalDimension.units)]);
                  }
              else
                {
                  // explicitly specified data columns
                  for (auto i: spec.dataCols)
                    {
                      col=i;
                      horizontalLabels.emplace_back(sliceLabelTokens[str(anyVal.back()(spec.dimensionNames[i]),spec.horizontalDimension.units)]);
                    }
                  if (spec.headerRow<spec.nRowAxes())
                    {
                      // check whether any further columns exist that are not in
                      // spec.dimensionNames, and add these in as horizontal
                      // data dimension slices
                      for (; row<=spec.headerRow; ++row)
                        getWholeLine(input,buf,spec);
                      const boost::tokenizer<P> tok(buf.begin(), buf.end(), csvParser);
                      auto field=tok.begin();
                      for (size_t i=0; i<spec.dimensionNames.size() && field!=tok.end(); ++i, ++field);
                      for (; field!=tok.end(); ++field)
                        horizontalLabels.emplace_back
                          (sliceLabelTokens[str(anyVal.back()(*field),spec.horizontalDimension.units)]);
                    }
                }
            
              hc.xvectors.emplace_back(spec.horizontalDimName);
              hc.xvectors.back().dimension=spec.horizontalDimension;
              set<typename Key::value_type> uniqueLabels;
              dimLabels.emplace_back();
              for (auto& i: horizontalLabels)
                if (uniqueLabels.insert(i).second)
                  {
                    dimLabels.back()[i]=hc.xvectors.back().size();
                    hc.xvectors.back().emplace_back(sliceLabelTokens[i]);
                  }
            }

             

          for (; row<spec.nRowAxes(); ++row)
            getWholeLine(input,buf,spec);
            
        
          ++minsky().progressState;

          {
            ProgressUpdater pu(minsky().progressState, "Reading data",1);
            for (; getWholeLine(input, buf, spec); ++row)
              {
                if (!memUsageChecked)
                  // fudge this a bit, because we're using a mmap allocator to bypass Electron's heap
                  switch (cminsky().checkMemAllocation((2147483648.0*TrackingAllocatorBase::allocatedBytes)/TrackingAllocatorBase::poolSize))
                    {
                    case Minsky::OK: break;
                    case Minsky::proceed:
                      memUsageChecked=true;
                      break;
                    case Minsky::abort:
                      throw runtime_error("memory threshold exceeded");
                    }
                const boost::tokenizer<P> tok(buf.begin(), buf.end(), csvParser);

                Key key;
                auto field=tok.begin();
                size_t dim=0, dataCols=0;
                col=0;
                for (auto field=tok.begin(); field!=tok.end(); ++col, ++field)
                  if (spec.dimensionCols.contains(col))
                    {
                      // detect blank data lines (favourite Excel artifact)
                      if (spec.dimensions[dim].type!=Dimension::string && field->empty())
                        goto invalidKeyGotoNextLine;
                  
                      if (dim>=hc.xvectors.size())
                        hc.xvectors.emplace_back("?"); // no header present
                      try
                        {
                          auto trimmedField=trimWS(*field);
                          if (trimmedField.empty() && spec.dimensions[col].type!=Dimension::string)
                            onError(InvalidData("<empty>",to_string(spec.dimensions[col].type),spec.dimensionNames[col]),row);
                          auto keyElem=anyVal[dim](trimmedField);
                          auto skeyElem=str(keyElem, spec.dimensions[dim].units);
                          if (dimLabels[dim].emplace(sliceLabelTokens[skeyElem], dimLabels[dim].size()).second)
                            hc.xvectors[dim].emplace_back(keyElem);
                          key.emplace_back(sliceLabelTokens[skeyElem]);
                        }
                      catch (...)
                        {
                          if (spec.dontFail)
                            goto invalidKeyGotoNextLine;
                          onError(InvalidData(*field,to_string(spec.dimensions[col].type),spec.dimensionNames[col]),row);
                        }
                      dim++;
                    }

                if (key.size()<hc.rank()-tabularFormat)
                  {
                    if (spec.dontFail)
                      goto invalidKeyGotoNextLine;
                    onError(ShortLine(key,sliceLabelTokens),row);
                  }

                col=0;
                for (auto field=tok.begin(); field!=tok.end(); ++col,++field)
                  if ((spec.dataCols.empty() && col>=spec.nColAxes()) || spec.dataCols.contains(col) || col>=spec.maxColumn) 
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
                        else if (!checkValues &&
                                 ((s.empty() && (!isdigit(c)&&c!='-'&&c!='+')) ||
                                  ((s=="-"||s=="+") && !isdigit(c))))
                          continue; // skip non-numeric prefix
                        else if (!isspace(c) && c!='.' && c!=',')
                          s+=c;                    
                  
                      // TODO - this disallows special floating point values - is this right?
                      bool valueExists=!s.empty() && s!="\\N" && (isdigit(s[0])||s[0]=='-'||s[0]=='+'||s[0]=='.');
                      if (checkValues && !valueExists && !s.empty() && s!="\\N") // ignore empty cells or explicit nulls
                        onError(InvalidData(s,"value",spec.dimensionNames[col]),row);
                      
                      if (valueExists || !isnan(spec.missingValue))
                        {
                          if (spec.counter)
                            tmpData[key]+=1;
                          else
                            {
                              auto i=tmpData.find(key);
                              double v=spec.missingValue;
                              if (valueExists)
                                try
                                  {
                                    size_t end;
                                    v=stod(s,&end);
                                    if (checkValues && end<s.length())
                                      onError(InvalidData(s,"value",spec.dimensionNames[col]),row);
                                    if (i==tmpData.end())
                                      {
                                        tmpData.emplace(key,v);
                                        onError.rowKeyInsert(key,row);
                                      }
                                  }
                                catch (const std::bad_alloc&)
                                  {throw;}
                                catch (...) // value misunderstood
                                  {
                                    if (checkValues) onError(InvalidData(s,"value",spec.dimensionNames[col]),row);
                                    if (isnan(spec.missingValue)) // if spec.missingValue is NaN, then don't populate the tmpData map
                                      valueExists=false;
                                  }
                              if (valueExists && i!=tmpData.end())
                                switch (spec.duplicateKeyAction)
                                  {
                                  case DataSpec::throwException:
                                    if (!spec.dontFail)
                                      onError(DuplicateKey(key,sliceLabelTokens),row); 
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
                        }
                      dataCols++;
                      if (tabularFormat)
                        key.pop_back();
                      else
                        break; // only one column of data needs to be read
                    }
            
                if (!dataCols)
                  {
                    if (spec.counter || spec.dontFail)
                      tmpData[key]+=1;
                    else
                      onError(ShortLine(key,sliceLabelTokens),row);
                  }
            

                bytesRead+=buf.size();
                pu.setProgress(double(bytesRead)/fileSize);
              invalidKeyGotoNextLine:;
              }
          }
        }
      catch (const std::bad_alloc&)
        { // replace with a more user friendly error message
          throw MemoryExhausted();
        }
      catch (const std::length_error&)
        { // replace with a more user friendly error message
          throw MemoryExhausted();
        }
      catch (const std::exception& ex)
        {
          auto msg=string(ex.what())+" at line:"+to_string(row)+", col:"+to_string(col);
          if (col<spec.dimensionNames.size())
            msg+=" ("+spec.dimensionNames[col]+")";
          throw std::runtime_error(msg);
        }
    }
  };
  
  template <class P,  class E>
  void loadValueFromCSVFileT(VariableValue& vv, istream& input, const DataSpec& spec, uintmax_t fileSize, E& onError)
  {
    const BusyCursor busy(minsky());
    const ProgressUpdater pu(minsky().progressState, "Importing CSV",4);

    // set up off-heap memory allocator, and ensure it is torn down at exit
    TrackingAllocatorBase::allocatePool();
    auto onExit=onStackExit([](){TrackingAllocatorBase::deallocatePool();});

    {
      // check dimension names are all distinct
      set<string> dimNames{spec.horizontalDimName};
      for (auto i: spec.dimensionCols)
        if (!dimNames.insert(spec.dimensionNames[i]).second)
          throw runtime_error("Duplicate dimension: "+spec.dimensionNames[i]);
    }
    
    ParseCSV<P> parseCSV(input,spec,fileSize,onError);
    auto& tmpData=parseCSV.tmpData;
    auto& dimLabels=parseCSV.dimLabels;
    auto& hc=parseCSV.hc;

    try
      {
        // remove zero length dimensions
        {
          auto d=dimLabels.begin();
          assert(hc.xvectors.size()==dimLabels.size());
          for (auto i=hc.xvectors.begin(); i!=hc.xvectors.end();)
            if (i->size()<2)
              {
                hc.xvectors.erase(i);
              }
            else
              {
                ++i;
                ++d;
              }
          assert(hc.xvectors.size()<=dimLabels.size());
        }
        
        for (auto& xv: hc.xvectors)
          xv.imposeDimension();
        ++minsky().progressState;

        if (hc.logNumElements()>log(numeric_limits<size_t>::max()))
          throw runtime_error("Hypercube dimensionality exceeds maximum size, results are likely to be garbage.\n"
                  "Suggest rolling up one or more axes by ignoring them, and setting 'Duplicate Key Action' as appropriate");
            
        
        if (log(tmpData.size())-hc.logNumElements()>=log(0.5)) 
          { // dense case
            vv.index({});
            if (cminsky().checkMemAllocation(2*hc.numElements()*sizeof(double))==Minsky::abort)
              throw MemoryExhausted();            
            vv.hypercube(hc);
            // stash the data into vv tensorInit field
            vv.tensorInit.index({});
            vv.tensorInit.hypercube(hc);
            for (auto& i: vv.tensorInit)
              i=spec.missingValue;
            auto dims=vv.hypercube().dims();
            const ProgressUpdater pu(minsky().progressState,"Loading data",tmpData.size());
            for (auto& i: tmpData)
              {
                size_t idx=0;
                assert (hc.rank()<=i.first.size());
                assert(dimLabels.size()>=hc.rank());
                int j=hc.rank()-1, k=i.first.size()-1; 
                while (j>=0 && k>=0)
                  {
                    while (dimLabels[k].size()<2) --k; // skip over elided dimensions
                    auto dimLabel=dimLabels[k].find(i.first[k]);
                    assert(dimLabel!=dimLabels[k].end());
                    idx = (idx*dims[j]) + dimLabel->second;
                    --j; --k;
                  }
                vv.tensorInit[idx]=i.second;
                ++minsky().progressState;
              }
          }    
        else 
          { // sparse case	
            if (cminsky().checkMemAllocation(6*tmpData.size()*sizeof(double))==Minsky::abort)
              throw MemoryExhausted();	  	  		
            auto dims=hc.dims();
            const ProgressUpdater pu(minsky().progressState,"Indexing and loading",2);
              
            map<size_t,double> indexValue; // intermediate stash to sort index vector
            {
              const ProgressUpdater pu(minsky().progressState,"Building index",tmpData.size());
              for (auto& i: tmpData)
                {
                  size_t idx=0;
                  assert (dims.size()<=i.first.size());
                  assert(dimLabels.size()>=dims.size());
                  int j=dims.size()-1, k=i.first.size()-1;
                  while (j>=0 && k>=0) // changed from for loop to while loop at CodeQL's insistence
                    {
                      while (dimLabels[k].size()<2) --k; // skip over elided dimensions
                      auto dimLabel=dimLabels[k].find(i.first[k]);
                      assert(dimLabel!=dimLabels[k].end());
                      idx = (idx*dims[j]) + dimLabel->second;
                      --j;
                      --k;
                    }
                  if (!isnan(i.second))
                    indexValue.emplace(idx, i.second);
                  ++minsky().progressState;
                }

              vv.tensorInit.index(indexValue);
              vv.tensorInit.hypercube(hc);
            }
            {
              const ProgressUpdater pu(minsky().progressState,"Loading data",indexValue.size());
              size_t j=0;
              for (auto& i: indexValue)
                {
                  vv.tensorInit[j++]=i.second;
                  ++minsky().progressState;
                }
            }
          }
        minsky().progressState.title="Cleaning up";
        minsky().progressState.displayProgress();
      }
    catch (const std::bad_alloc&)
      { // replace with a more user friendly error message
        throw MemoryExhausted();
      }
    catch (const std::length_error&)
      { // replace with a more user friendly error message
        throw MemoryExhausted();
      }
  }
  
  void loadValueFromCSVFile(VariableValue& v, istream& input, const DataSpec& spec, uintmax_t fileSize)
  {
    OnError onError;
    if (spec.separator==' ')
      loadValueFromCSVFileT<SpaceSeparatorParser>(v,input,spec,fileSize,onError);
    else
      loadValueFromCSVFileT<Parser>(v,input,spec,fileSize,onError);
  }

  struct FailedToRewind: public std::exception
  {
    const char* what() const noexcept override {return "Failed to rewind input";}
  };
  
  template <class P>
  void reportFromCSVFileT(istream& input, ostream& output, const DataSpec& spec, uintmax_t fileSize )
  {
    const BusyCursor busy(minsky());
    const ProgressUpdater pu(minsky().progressState, "Generating report",3);
    // set up off-heap memory allocator, and ensure it is torn down at exit
    TrackingAllocatorBase::allocatePool();
    auto onExit=onStackExit([](){TrackingAllocatorBase::deallocatePool();});

    struct ErrorReporter //: public OnError // using duck typing, not dynamic polymorphism
    {
      Map<size_t> firstRow;
      map<size_t,Key> duplicates;
      map<size_t,string> invalidData;
      void operator()(const DuplicateKey& ex, size_t row) {
        duplicates.emplace(firstRow[ex.key],ex.key);
        duplicates.emplace(row,ex.key);
      }
      void operator()(const InvalidData& ex, size_t row) {invalidData.emplace(row, ex.msg);}
      void operator()(const ShortLine& ex, size_t row) {invalidData.emplace(row, ex.msg);}
      /// update a map of keys to first rows for duplicate key processing
      void rowKeyInsert(const Key& key, size_t row) {firstRow.emplace(key,row);}
    } onError;

    // parse file to extract error locations
    ParseCSV<P> parseCSV(input, spec, fileSize, onError, /*checkValues=*/true);

    input.clear();
    input.seekg(0);
    if (!input) throw FailedToRewind();
    string buf;
    size_t row=0;
    string sep{spec.separator};
    multimap<Key,string> duplicateLines;
    vector<string> invalidDataLines;

    {
      // extract all error lines  
      ProgressUpdater pu(minsky().progressState, "Extracting errors",3);
      size_t bytesRead=0;
      for (;  getWholeLine(input, buf, spec); ++row)
        {
          if (onError.duplicates.contains(row))
            duplicateLines.emplace(onError.duplicates[row],"Duplicate key"+sep+buf);
          if (onError.invalidData.contains(row))
            invalidDataLines.push_back(onError.invalidData[row]+sep+buf);
          bytesRead+=buf.size();
          pu.setProgress(double(bytesRead)/fileSize);
        }
    }
    
    // now output report
    input.clear();
    input.seekg(0);
    if (!input) throw FailedToRewind();
    {
      ProgressUpdater pu(minsky().progressState, "Writing report",3);
      size_t bytesRead=0;
      // process header
      for (row=0; row<spec.nRowAxes() && getWholeLine(input, buf, spec); ++row)
        {
          if (row==spec.headerRow)
            output<<"Error"<<sep;
          output<<buf<<endl;
          bytesRead+=buf.size();
          pu.setProgress(double(bytesRead)/fileSize);
        }
      
    // process invalid data
    for (auto& i: invalidDataLines)
      output<<i<<endl;
    // process duplicates
    for (auto& i: duplicateLines)
      output<<i.second<<endl;
    // process remaining good part of the file
    for (; getWholeLine(input, buf, spec); ++row)
      {
        if (!onError.duplicates.contains(row) && !onError.invalidData.contains(row))
          output<<sep+buf<<endl;
        bytesRead+=buf.size();
        pu.setProgress(double(bytesRead)/fileSize);
      }
    }
  }

  void reportFromCSVFile(istream& input, ostream& output, const DataSpec& spec, uintmax_t fileSize)
  {
    if (spec.separator==' ')
      reportFromCSVFileT<SpaceSeparatorParser>(input,output,spec,fileSize);
    else
      reportFromCSVFileT<Parser>(input,output,spec,fileSize);
  }


}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::DataSpec);
