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
using soci::use;
using soci::transaction;

namespace minsky
{
  vector<string> parseRow(const string& line, char separator)
  {
    if (separator==' ')
      {
        SpaceSeparatorParser csvParser;
        const boost::tokenizer<SpaceSeparatorParser> tok(line.begin(),line.end(), csvParser);
        return {tok.begin(), tok.end();}
      }
    Parser csvParser;
    const boost::tokenizer<Parser> tok(line.begin(),line.end(), csvParser);
    return {tok.begin(), tok.end();}
  }
  
  void DatabaseIngestor::createTable
  (const std::vector<std::string>& filenames, const DataSpecSchema& spec)
  {
    session<<"drop "+table+" if exists";
    // for now, load time data as strings - TODO handle date-time parsing
    string def="create table "+table+" (";
    for (size_t i=0; i<spec.maxColumn; ++i)
      {
        def+=spec.dimensionNames[i];
        if (spec.dimensionCols.contains(i))
          {
            if (spec.dimensions[i].type==Dimension::value)
              def+=" double ";
            else
              def+=" char(255) "; // TODO - better string type?
          }
        else if (spec.dataCols.contains(i))
          def+=" double ";
      }
    if (!filenames.empty())
      {
        ifstream input(filenames.begin());
        for (; getWholeLine(input,line,spec) && row<spec.nRowAxes; ++row); // skip header rows
        auto parsedRow=parseRow(input, spec.separator);
        for (size_t i=spec.maxColumn; i<parsedRow.size(); ++i) // remaining columns are data
          def+=parsedRow[i]+" double ";
      }
    def+=")";
    session<<def;
  }
  
  template <class Tokeniser> DatabaseIngestor::load
  (soci::statement& stmt, const std::vector<std::string>& filenames,
   const DataSpecSchema& spec)
  {
    Tokeniser csvParser;
    vector<string> cells;
    // prepare the insertion string based on spec
    auto statement=(session.prepare<<"upsert into "+table+" ...",use(cells));
    for (auto f: filenames)
      {
        ifstream input(f);
        size_t row=0;
        string line;
        for (; getWholeLine(input,line,spec) && row<spec.nRowAxes; ++row); // skip header rows
        transaction(session); // does implicit session.begin(). RAII to cleanup in case of exception
        for (; getWholeLine(input,line,spec); ++row)
          {
            const boost::tokenizer<Tokeniser> tok(line.begin(),line.end(), csvParser);
            cells.assign(tok.begin(), tok.end());
            stmt.execute(true);
            if (row%100==0)
              {
                session.commit();
                session.begin();
              }
          }
        transaction.commit();
      }
  }
  
  void DatabaseIngestor::importFromCSV
  (const std::vector<std::string>& filenames, const DataSpecSchema& spec)
  {
    if (!session.is_connected()) session.reconnect();
    if (!session.is_connected()) return;

    // TODO check if table exists, and call createTable if not
    // select * from table limit 1 - and check for exception thrown?
    

    if (spec.separator==' ')
      load<SpaceSeparatorParser>(statement,filenames,spec);
    else
      load<Parser>(statement,filenames,spec);
  }

}
