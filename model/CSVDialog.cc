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

#include "CSVDialog.h"                                                           
#include "group.h"                                                               
#include "selection.h"
#include "lasso.h"
#include <pango.h>

#include "CSVDialog.rcd"
#include "minsky_epilogue.h"                                                     
#include "zStream.h"
#include "dimension.h"
                                                                                 
#include <boost/asio/ssl/error.hpp>                                              
#include <boost/asio/ssl/stream.hpp>                                             
#include <boost/asio.hpp>                                                        
#include <boost/beast/core.hpp>                                                  
#include <boost/beast/http.hpp>                                                  
#include <boost/beast/version.hpp>  
                                                                                        
#include <boost/filesystem.hpp>                                                  
                                                                                 
#include "certify/include/boost/certify/extensions.hpp"                          
#include "certify/include/boost/certify/https_verification.hpp"                  
                                                                                 
#include <cstdlib>
#include <chrono>
#include <iostream>                                                              
#include <string>                                                                
#include <stdexcept>                                                                                                                         
#include <sstream>      
#include <regex>    

using namespace std;
using namespace minsky;
using namespace civita;
using ecolab::Pango;
using ecolab::cairo::CairoSave;
using tcp = boost::asio::ip::tcp;       
namespace ssl = boost::asio::ssl;       
namespace http = boost::beast::http;    

const unsigned CSVDialog::numInitialLines;

void CSVDialog::reportFromFile(const std::string& input, const std::string& output) const
{
  ifstream is(input);
  stripByteOrderingMarker(is);
  ofstream of(output);
  reportFromCSVFile(is,of,spec);
}

namespace
{
  // manage temporary files
  struct CacheEntry
  {
    chrono::time_point<chrono::system_clock> timestamp;
    string url, filename;
    CacheEntry(const string& url): timestamp(chrono::system_clock::now()), url(url),
                            filename(boost::filesystem::unique_path().string()) {}
    ~CacheEntry() {boost::filesystem::remove(filename);}
    bool operator<(const CacheEntry& x) const {return url<x.url;}
  };

  // note: this cache will leak disk storage if Minsky is killed, not shut down cleanly
  struct Cache: private set<CacheEntry> 
  {
    using set<CacheEntry>::find;
    using set<CacheEntry>::end;
    using set<CacheEntry>::erase;
    iterator emplace(const string& url)
    {
      if (size()>=10)
        {
          // find oldest element and erase
          auto entryToErase=begin();
          auto ts=entryToErase->timestamp;
          for (auto i=begin(); i!=end(); ++i)
            if (i->timestamp<ts)
              {
                ts=i->timestamp;
                entryToErase=i;
              }
          erase(entryToErase);
        }
      return set<CacheEntry>::emplace(url).first;
    }
  };
}

// Return file name after downloading a CSV file from the web.
std::string CSVDialog::loadWebFile(const std::string& url)
{
  static Cache cache;
  auto cacheEntry = cache.find(url);
  if (cacheEntry!=cache.end())
    {
      if (chrono::system_clock::now() < cacheEntry->timestamp + chrono::minutes(5))
        return cacheEntry->filename;
      // expire the entry
      cache.erase(cacheEntry);
    }
  
  // Parse input URL. Also handles URLs of the type username:password@example.com/pathname#section. See https://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform
  regex ex(R"((http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\??([^ #]*)#?([^ ]*)|^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)");
  cmatch what;
  if (regex_match(url.c_str(), what, ex)) {
   // what[0] contains the whole string 	 
   // what[1] is the protocol
   // what[2] is the domain  
   // what[3] is the port    
   // what[4] is the path    
   // what[5] is the query   
   // what[6] is the fragment		  
  } else throw runtime_error("Failure to match URL: "+url);
         
  auto const protocol =what[1];
  auto const host = what[2];
  auto const target = what[4];
  
  // The io_context is required for all I/O
  boost::asio::io_context ioc;
  
  // The SSL context is required, and holds certificates
  ssl::context ctx{ssl::context::tls_client};
  
  // Verify the remote server's certificate. See https://github.com/djarek/certify/blob/master/examples/get_page.cpp
  ctx.set_verify_mode(ssl::verify_peer | ssl::context::verify_fail_if_no_peer_cert);    
  ctx.set_default_verify_paths();
  
  // tag::ctx_setup_source[]
  boost::certify::enable_native_https_server_verification(ctx);
  // end::ctx_setup_source[]        
  		        
  // These objects perform our I/O
  tcp::resolver resolver{ioc};
  ssl::stream<tcp::socket> stream{ioc, ctx};        
  
  // tag::stream_setup_source[]. See https://github.com/djarek/certify/blob/master/examples/get_page.cpp
  boost::certify::set_server_hostname(stream, host.str());
  boost::certify::sni_hostname(stream, host);
  // end::stream_setup_source[]         

  // Look up the domain name
  auto const results = resolver.resolve(host.str(), protocol.str());
          
  // Make the connection on the IP address we get from a lookup
  boost::asio::connect(stream.next_layer(), results.begin(), results.end());                   
       
  // Perform the SSL handshake
  stream.handshake(ssl::stream_base::client);             

  // Set up an HTTP GET request message
  http::request<http::dynamic_body> req;
  req.method(http::verb::get);     
  req.target(target.str());     
  req.version(10);
  req.set(http::field::host, host.str());
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Send the HTTP request to the remote host
  http::write(stream, req);

  // This buffer is used for reading and must be persisted
  boost::beast::flat_buffer buffer;

  // Declare a container to hold the response
  http::response<http::dynamic_body> res;

  // Receive the HTTP response
  http::read(stream, buffer, res);

  // Check response status and throw error all values 400 and above. See https://www.boost.org/doc/libs/master/boost/beast/http/status.hpp for status codes
  if (res.result_int() >= 400) throw runtime_error("Invalid HTTP response. Response code: " + std::to_string(res.result_int()));
                          
  // Dump the outstream into a temporary file for loading it into Minsky' CSV parser 
  std::string tempStr = cache.emplace(url)->filename;

  std::ofstream outFile(tempStr, std::ofstream::binary);  
  
  outFile << boost::beast::buffers_to_string(res.body().data());
       
  // Gracefully close the socket
  boost::system::error_code ec;
  stream.shutdown(ec);
  if (ec == boost::asio::error::eof)
  {
      // Rationale:
      // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
      ec.assign(0, ec.category());
  }
  if (ec)
      throw boost::system::system_error{ec}; 
      
  // If we get here then the connection is closed gracefully         
      
  // Return the file name for loading the in csvimport.tcl 
  return tempStr;
}

void CSVDialog::loadFile()
{
  loadFileFromName(url.find("://")==string::npos? url: loadWebFile(url));
}

void CSVDialog::guessSpecAndLoadFile()
{
  spec=DataSpec();
  string fname = url.find("://")==string::npos? url: loadWebFile(url);
  spec.guessFromFile(fname);
  loadFileFromName(fname);
  classifyColumns();
}

void CSVDialog::loadFileFromName(const std::string& fname)
{  
  ifstream is(fname);
  stripByteOrderingMarker(is);
  initialLines.clear();
  for (size_t i=0; i<numInitialLines && is; ++i)
    {
      initialLines.emplace_back();
      getline(is, initialLines.back());
    }
  // Ensure dimensions.size() is the same as nColAxes() upon first load of a CSV file. For ticket 974.
  if (spec.dimensions.size()<spec.nColAxes()) spec.setDataArea(spec.nRowAxes(),spec.nColAxes());    
}

template <class Parser>
vector<vector<string>> parseLines(const Parser& parser, const vector<string>& lines)
{
  vector<vector<string>> r;
  for (const auto& line: lines)
    {
      r.emplace_back();
      try
        {
          boost::tokenizer<Parser> tok(line.begin(), line.end(), parser);
          for (auto& t: tok)
            r.back().push_back(t);
        }
      catch (...) // if not parseable, place entire line in first cell
        {
          r.back().push_back(line);
        }
    }
  return r;
}

namespace
{
  struct CroppedPango: public Pango
  {
    cairo_t* cairo;
    double w, x=0, y=0;
    CroppedPango(cairo_t* cairo, double width): Pango(cairo), cairo(cairo), w(width) {}
    void setxy(double xx, double yy) {x=xx; y=yy;}
    void show() {
      CairoSave cs(cairo);
      cairo_rectangle(cairo,x,y,w,height());
      cairo_clip(cairo);
      cairo_move_to(cairo,x,y);
      Pango::show();
    }
  };
}

bool CSVDialog::redraw(int, int, int, int)
{
  cairo_t* cairo=surface->cairo();
  CroppedPango pango(cairo, colWidth);
  rowHeight=15;
  pango.setFontSize(0.8*rowHeight);
  
  vector<vector<string>> parsedLines=parseLines();

  // LHS row labels
  {
    Pango pango(cairo);
    pango.setText("Dimension");
    cairo_move_to(cairo,xoffs-pango.width()-5,0);
    pango.show();
    pango.setText("Type");
    cairo_move_to(cairo,xoffs-pango.width()-5,rowHeight);
    pango.show();
    pango.setText("Format");
    cairo_move_to(cairo,xoffs-pango.width()-5,2*rowHeight);
    pango.show();
    if (flashNameRow)
      pango.setMarkup("<b>Name</b>");
    else
      pango.setText("Name");
    cairo_move_to(cairo,xoffs-pango.width()-5,3*rowHeight);
    pango.show();
    pango.setText("Header");
    cairo_move_to(cairo,xoffs-pango.width()-5,(4+spec.headerRow)*rowHeight);
    pango.show();
    
  }	
  
  set<size_t> done;
  double x=xoffs, y=0;
  size_t col=0;
  for (; done.size()<parsedLines.size(); ++col)
    {
      if (col<spec.nColAxes())
        {// dimension check boxes
          CairoSave cs(cairo);
          double cbsz=5;
          cairo_set_line_width(cairo,1);
          cairo_translate(cairo,x+0.5*colWidth,y+0.5*rowHeight);
          cairo_rectangle(cairo,-cbsz,-cbsz,2*cbsz,2*cbsz);
          if (spec.dimensionCols.count(col))
            {
              cairo_move_to(cairo,-cbsz,-cbsz);
              cairo_line_to(cairo,cbsz,cbsz);
              cairo_move_to(cairo,cbsz,-cbsz);
              cairo_line_to(cairo,-cbsz,cbsz);
            }
          cairo_stroke(cairo);
        }
      y+=rowHeight;
      // type
      if (spec.dimensionCols.count(col) && col<spec.dimensions.size() && col<spec.nColAxes())
        {
          pango.setText(classdesc::enumKey<Dimension::Type>(spec.dimensions[col].type));
          pango.setxy(x,y);
          pango.show();
        }
      y+=rowHeight;
      if (spec.dimensionCols.count(col) && col<spec.dimensions.size() && col<spec.nColAxes())
        {
          pango.setText(spec.dimensions[col].units);
          pango.setxy(x,y);
          pango.show();
        }
      y+=rowHeight;
      if (spec.dimensionCols.count(col) && col<spec.dimensionNames.size() && col<spec.nColAxes())
        {
          pango.setText(spec.dimensionNames[col]);
          pango.setxy(x,y);
          pango.show();
        }
      y+=rowHeight;
      for (size_t row=0; row<parsedLines.size(); ++row)
        {
          auto& line=parsedLines[row];
          if (col<line.size())
            {
              CairoSave cs(cairo);
              pango.setText(line[col]);
              pango.setxy(x, y);
              if (row==spec.headerRow && !(spec.columnar && col>spec.nColAxes()))
                if (col<spec.nColAxes())
                  cairo_set_source_rgb(surface->cairo(),0,0.7,0);
                else
                  cairo_set_source_rgb(surface->cairo(),0,0,1);
              else if (row<spec.nRowAxes() || (col<spec.nColAxes() && !spec.dimensionCols.count(col)) ||
                       (spec.columnar && col>spec.nColAxes()))
                cairo_set_source_rgb(surface->cairo(),1,0,0);
              else if (col<spec.nColAxes())
                cairo_set_source_rgb(surface->cairo(),0,0,1);
              pango.show();
            }
          else
            done.insert(row);
          y+=rowHeight;
        }
      {
        CairoSave cs(cairo);
        cairo_set_source_rgb(cairo,.5,.5,.5);
        cairo_move_to(cairo,x-2.5,0);
        cairo_rel_line_to(cairo,0,(parsedLines.size()+4)*rowHeight);
        cairo_stroke(cairo);
      }
      x+=colWidth+5;
      y=0;
    }
  m_tableWidth=(col-1)*(colWidth+5);
  for (size_t row=0; row<parsedLines.size()+5; ++row)
    {
      CairoSave cs(cairo);
      cairo_set_source_rgb(cairo,.5,.5,.5);
      cairo_move_to(cairo,xoffs-2.5,row*rowHeight);
      cairo_rel_line_to(cairo,m_tableWidth,0);
      cairo_stroke(cairo);
    }
  return true;
}

size_t CSVDialog::columnOver(double x) const
{
  return size_t((x-xoffs)/(colWidth+5));
}

size_t CSVDialog::rowOver(double y) const
{
  return size_t(y/rowHeight);
}

std::vector<std::vector<std::string>> CSVDialog::parseLines()
{
  vector<vector<string>> parsedLines;
  if (spec.mergeDelimiters)
    if (spec.separator==' ')
      parsedLines=::parseLines(boost::char_separator<char>(), initialLines);
    else
      {
        char separators[]={spec.separator,'\0'};
        parsedLines=::parseLines
          (boost::char_separator<char>(separators,""),initialLines);
      }
  else
    parsedLines=::parseLines
      (boost::escaped_list_separator<char>(spec.escape,spec.separator,spec.quote),
       initialLines);
  if (spec.headerRow<parsedLines.size())
    spec.numCols=parsedLines[spec.headerRow].size();
  else if (parsedLines.empty())
    spec.numCols=0;
  else
    spec.numCols=parsedLines.front().size();
  return parsedLines;
}

void CSVDialog::populateHeaders()
{
  auto parsedLines=parseLines();
  if (spec.headerRow>parsedLines.size()) return;
  spec.dimensionNames=parsedLines[spec.headerRow];
}

void CSVDialog::populateHeader(size_t col)
{
  auto parsedLines=parseLines();
  if (spec.headerRow>parsedLines.size()) return;
  auto& headers=parsedLines[spec.headerRow];
  if (col<headers.size())
    spec.dimensionNames[col]=headers[col];
}

void CSVDialog::classifyColumns()
{
  auto parsedLines=parseLines();
  spec.dimensionCols.clear();
  spec.dataCols.clear();
  spec.dimensions.resize(spec.numCols);
  for (auto col=0; col<spec.numCols; ++col)
    {
      bool entryFound=false, timeFound=true, numberFound=true;
      for (size_t row=spec.dataRowOffset; row<parsedLines.size(); ++row)
        if (col<parsedLines[row].size() && !parsedLines[row][col].empty())
          {
            entryFound=true;
            if (numberFound && !isNumerical(parsedLines[row][col]))
              numberFound=false;
            static AnyVal any(Dimension(Dimension::time,""));
            if (timeFound)
              try
                {any(parsedLines[row][col]);}
              catch (...)
                {timeFound=false;}
          }
      if (entryFound)
        {
          if (numberFound)
            spec.dataCols.insert(col);
          else
            {
              spec.dimensionCols.insert(col);
              if (timeFound)
                spec.dimensions[col].type=Dimension::time;
              else
                spec.dimensions[col].type=Dimension::string;
              spec.dimensions[col].units.clear();
            }
        }
    }
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::CSVDialog);
