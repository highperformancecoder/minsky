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
#ifndef CSVPARSER_H
#define CSVPARSER_H

#include "dimension.h"
#include "dataSpecSchema.h"
#include "classdesc_access.h"
#include "str.h"

#include <cstddef>
#include <string>
#include <set>
#include <fstream>
#include <filesystem>

namespace minsky
{
  class VariableValue;

  class DataSpec: public DataSpecSchema
  {
    std::size_t m_nRowAxes=0, m_nColAxes=0;
    CLASSDESC_ACCESS(DataSpec);
  public:
    /// maximum number of columns that can be configured independently. Columns after this limit are treated as "data"
    std::size_t maxColumn=1000;

    /// start row of the data area
    std::size_t nRowAxes() const {return m_nRowAxes;}
    /// start column of the data area
    std::size_t nColAxes() const {return m_nColAxes;}

    // handle extra initialisation on conversion
    DataSpecSchema toSchema() {
      dataRowOffset=nRowAxes();
      dataColOffset=nColAxes();
      return *this;
    }

    DataSpec& operator=(const DataSpecSchema& x) {
      DataSpecSchema::operator=(x);
      setDataArea(dataRowOffset, dataColOffset);
      dimensionCols=x.dimensionCols; // revert clobber by setDataArea
      dataCols=x.dataCols; // revert clobber by setDataArea
      return *this;
    }
    
    void toggleDimension(std::size_t c) {
      auto i=dimensionCols.find(c);
      if (i==dimensionCols.end())
        dimensionCols.insert(c);
      else
        dimensionCols.erase(i);
    }

    /// set top left cell of the data area
    void setDataArea(std::size_t row, std::size_t col);
    
    /// initial stab at dataspec from examining stream
    void guessFromStream(std::istream& file, uintmax_t fileSize=uintmax_t(-1));

    /// initial stab at dataspec from examining file
    void guessFromFile(const std::string& fileName) {
      std::ifstream is(fileName);
      stripByteOrderingMarker(is);
      guessFromStream(is, std::filesystem::file_size(fileName));
    }

    /// populates this spec from a "RavelHypercube" entry, \a row is the row being read, used to set the headerRow attribute
    /// \a If horizontalName is one of the dimensions, data is written in a tabular format
    void populateFromRavelMetadata(const std::string& metadata, const std::string& horizontalName, std::size_t row);

    /// number of unique values in each column
    /// corrected for header row, so may be slightly inaccurate if header row contains one of the values
    const std::vector<size_t>& uniqueValues() const {return m_uniqueValues;}
    
  private:
    /// try to fill in remainder of spec, given a tokenizer function tf
    /// eg boost::escaped_list_separator<char> tf(escape,separator,quote)
    template <class T>
    void givenTFguessRemainder(std::istream& initialInput, std::istream& remainingInput, const T& tf, uintmax_t fileSize);

    /// figure out the tokenizer function and call givenTFguessRemainder
    void guessRemainder(std::istream& initialInput, std::istream& remainingInput, char separator, uintmax_t fileSize);

    std::vector<size_t> starts;
    size_t nCols=0;
    size_t row=0;
    size_t firstEmpty=std::numeric_limits<size_t>::max();

    /// number of unique values in each column
    std::vector<size_t> m_uniqueValues;

    /// process chunk of input, updating guessed spec
    /// @return true if there's no more work to be done
    template <class T, class U>
    bool processChunk(std::istream& input, const T& tf, size_t until, U&);
  };

  /// creates a report CSV file from input, with errors sorted at
  /// begining of file, with a column for error messages
  void reportFromCSVFile(std::istream& input, std::ostream& output, const DataSpec& spec, uintmax_t fileSize);

  /// load a variableValue from a list of files according to data spec
  void loadValueFromCSVFile(VariableValue&,const std::vector<std::string>& filenames,const DataSpec&);
  /// load a variableValue from a stream according to data spec
  void loadValueFromCSVFile(VariableValue&, std::istream& input, const DataSpec&);

  /// replace doubled quotes with escaped quotes
  void escapeDoubledQuotes(std::string&,const DataSpec&);

  /// get complete line from input, allowing for quoted linefeed
  bool getWholeLine(std::istream& input, std::string& line, const DataSpec& spec);

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
      if (++next >= end)
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

    explicit EscapedListSeparator(Char  e = '\\')
      : escape_(1,e), c_(1,','), quote_(1,'\"'), last_(false) { }
    EscapedListSeparator(Char  e, Char c,Char  q = '\"')
      : escape_(1,'\\'), c_(1,','), quote_(1,q), last_(false) { }
    EscapedListSeparator(EscapedListSeparator::string_type e, EscapedListSeparator::string_type c,
                         EscapedListSeparator::string_type q)
      : escape_(e), c_(c), quote_(q), last_(false) { }

    void reset() {last_=false;}

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next,InputIterator end,Token& tok) {
      bool bInQuote = false;
      tok = Token();

      if (next >= end) {
        next=end; // reset next in case it has adavanced beyond
        if (last_) {
          last_ = false;
          return true;
        }
        return false;
      }
      last_ = false;
      while (next < end) {
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
        ++next;
      }
      return true;
    }
  };
}
using Parser=escapedListSeparator::EscapedListSeparator<char>;

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
    while (next!=end)
      {
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
        ++next;
      }
    return !tok.empty();
  }
  void reset() {}
};


  
}

#include "CSVParser.cd"
#include "CSVParser.xcd"
#endif
