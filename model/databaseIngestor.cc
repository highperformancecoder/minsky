/*
  @copyright Steve Keen 2025
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

#include "databaseIngestor.h"
#include "databaseIngestor.rcd"
#include "minsky_epilogue.h"

using civita::Dimension;
using soci::use;
using soci::transaction;
using namespace std;

namespace minsky
{
  vector<string> parseRow(const string& line, char separator)
  {
    if (separator==' ')
      {
        SpaceSeparatorParser csvParser;
        const boost::tokenizer<SpaceSeparatorParser> tok(line.begin(),line.end(), csvParser);
        return {tok.begin(), tok.end()};
      }
    Parser csvParser;
    const boost::tokenizer<Parser> tok(line.begin(),line.end(), csvParser);
    return {tok.begin(), tok.end()};
  }
  
  void DatabaseIngestor::connect(const string& dbType, const string& connection)
  {session=make_shared<soci::session>(dbType,connection);}

  void DatabaseIngestor::createTable
  (const std::vector<std::string>& filenames, const DataSpec& spec)
  {
    if (!session.get()) return;
    *session<<"drop table if exists "+table;
    // for now, load time data as strings - TODO handle date-time parsing
    string def="create table "+table+" (";
    for (size_t i=0; i<spec.dimensionNames.size() &&
           i<spec.dimensions.size() && i<spec.maxColumn; ++i)
      if (spec.dimensionCols.contains(i)||spec.dataCols.contains(i))
        {
          def+=(i? ", '":"'")+spec.dimensionNames[i]+"'";
          if (spec.dimensionCols.contains(i))
            {
              if (spec.dimensions[i].type==Dimension::value)
                def+=" double";
              else
                def+=" char(255)"; // TODO - better string type?
            }
          else
            def+=" double";
        }
    if (!filenames.empty())
      {
        ifstream input(filenames.front());
        string line;
        for (size_t row=0; getWholeLine(input,line,spec) && row<spec.nRowAxes(); ++row); // skip header rows
        if (getWholeLine(input,line,spec))
          {
            auto parsedRow=parseRow(line, spec.separator);
            for (size_t i=spec.maxColumn; i<parsedRow.size(); ++i) // remaining columns are data
              def+=parsedRow[i]+" double ";
          }
      }
    def+=")";
    *session<<def;
  }
  
  
  template <class Tokeniser> void DatabaseIngestor::load
  (const std::vector<std::string>& filenames, const DataSpec& spec)
  {
    Tokeniser csvParser;
    set<unsigned> insertCols;
    // compute complement of dimensionCols union dataCols
    for (unsigned i=0; i<spec.numCols; ++i)
      if (spec.dimensionCols.contains(i) || spec.dataCols.contains(i))
        insertCols.insert(i);
    vector<string> cells(insertCols.size());
    
    // prepare the insertion string based on spec
    string insertStatement="insert into "+table+"(";
    for (size_t i=0; i<spec.numCols; ++i)
      if (insertCols.contains(i))
        insertStatement+=(i?",'":"'")+spec.dimensionNames[i]+"'";
    insertStatement+=") values(";
    for (size_t i=0; i<spec.numCols; ++i)
      if (insertCols.contains(i))
        insertStatement+=(i?",:a":":a")+to_string(i);
    insertStatement+=")";
    cout<<insertStatement<<endl;

    auto temp=session->prepare<<insertStatement;
    for (auto& c: cells) temp=(temp,use(c));
    soci::statement statement(temp);
    for (auto f: filenames)
      {
        ifstream input(f);
        size_t row=0;
        size_t bytesRead=0;
        string line;
        for (; getWholeLine(input,line,spec) && row<spec.nRowAxes(); ++row); // skip header rows
        transaction tr(*session); // does implicit session.begin(). RAII to cleanup in case of exception
        for (; getWholeLine(input,line,spec); ++row)
          {
            bytesRead+=line.size();
            const boost::tokenizer<Tokeniser> tok(line.begin(),line.end(), csvParser);
            unsigned col=0, cell=0;
            for (auto t: tok)
              if (insertCols.contains(col++))
                cells[cell++]=t;
            statement.execute(true);
            if (row%1000==0)
              {
                session->commit();
                session->begin();
                cout<<f<<": "<<(double(bytesRead)/filesystem::file_size(f))<<"%\r"<<flush;
              }
          }
        tr.commit();
      }
  }
  
  void DatabaseIngestor::importFromCSV
  (const std::vector<std::string>& filenames, const DataSpec& spec)
  {
    if (!session.get()) return;
    if (!session->is_connected()) session->reconnect();
    if (!session->is_connected()) return;

    // TODO check if table exists, and call createTable if not
    // select * from table limit 1 - and check for exception thrown?
    

    if (spec.separator==' ')
      load<SpaceSeparatorParser>(filenames,spec);
    else
      load<Parser>(filenames,spec);
  }

}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::DatabaseIngestor);
