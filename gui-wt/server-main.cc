/*
  @copyright Steve Keen 2012
  @author Michael Roy
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

#include <ios>
#include <boost/exception/all.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/asio/ip/address.hpp>
#include <Wt/WServer>
#include "minskyApp.h"
#include "../str.h"
#include <ecolab_epilogue.h>

#ifndef MINSKY_VERSION
# define MINSKY_VERSION "0.1a"
#endif

using namespace std;
using namespace Wt;
using namespace minsky::gui;
using namespace boost;

namespace po = boost::program_options;
namespace ip = boost::asio::ip;

WT_API po::options_description optDescr("Allowed options");

class CommandLineOptions
{
public:
  unsigned int httpPort;
  string httpAddr;
  string docRoot;
  bool quiet;
  string wtConfig;
  int64_t maxMemoryRequestSize;

public:
  inline CommandLineOptions ()
    : httpPort(8080),
      httpAddr("0.0.0.0"),
      docRoot("."),
      quiet(false),
      wtConfig("./wt_config.xml"),
      maxMemoryRequestSize(1024)
  { }
};

CommandLineOptions cmdLineOpts;

static void initCmdLineOptions()
{
  optDescr.add_options()
    ("help,h", "Display this message")
    ("http-port,p",
      po::value<unsigned int>(&(cmdLineOpts.httpPort))->default_value(8080),
      "Set the server http port, defaults to 8080")
    ("http-address,a",
      po::value<string>(&(cmdLineOpts.httpAddr))->default_value("0.0.0.0"),
      "Set the address to listen to, defaults to 0.0.0.0 (any)")
    ("docroot,r",
      po::value<string>(&(cmdLineOpts.docRoot))->default_value("."),
      "Set the application http root directory, defaults to '.' (root)")
    ("config,c",
      po::value<string>(&(cmdLineOpts.wtConfig))->default_value("./wt_config.xml"),
      "Set the wt config file, defaults to './wt_config.xml'")
    ("max-memory-request-size",
     po::value< ::int64_t >(&(cmdLineOpts.maxMemoryRequestSize))->default_value(1024),
     "threshold for request size (bytes), for spooling the entire request to "
     "disk, to avoid DoS")
    ("quiet,q", "Execute in quiet mode");
}

/**
 *  Processes command line
 */
static int processCmdLine(int argc, char** argv)
{
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, optDescr), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    cout << optDescr << endl;
    return 1;
  }

  //  validate args.
  if (vm.count("http-port"))
  {
    // check port is not in reserved range.
  }

  if (vm.count("http-addr"))
  {
    string str = vm["http-addr"].as<string>();

    //  "any" is valid.
    if (str == "any")
    {
      str = "0.0.0.0";
    }
    else
    {
    //  check dotted format is OK.
      ip::address ipAddr;

      try
      {
        ipAddr = ip::address::from_string(str);
      }
      catch(boost::system::error_code&)
      {
        cout << "** Error: \"" << str << "\" is not a valid ip address" << endl;
        return 2;
      }
      // TODO: refine validation.
      cmdLineOpts.httpAddr = str;
    }

  }

  if (vm.count("docroot"))
  {
    //  pass as is to server
    cmdLineOpts.docRoot = vm["docroot"].as<string>();
  }

  if (vm.count("quiet"))
  {
    // disable std::cout, std::cerr.
    cout.rdbuf(0);
    cerr.rdbuf(0);
  }

  return 0;
}

/**
 *  Application entry point
 */
int main(int argc, char **argv)
{
  int nResult = 3;

  initCmdLineOptions();

  try
  {
    nResult = processCmdLine(argc, argv);
  }
  catch(po::unknown_option &e)
  {
    cout << "Minsky Server version " << MINSKY_VERSION << endl << endl;
    cout << "Error " << e.what() << endl << endl;
    cout << optDescr << endl;
    return 1;
  }

  if(!cmdLineOpts.quiet)
  {
    cout << "Minsky Server version " << MINSKY_VERSION << endl;
  }

  if (!nResult)
  {
  //  create command line arguments for the server.
    vector<string> vArgs;
//    const int SZ_BUFFER_SIZE = 48;
//    char szBuffer[SZ_BUFFER_SIZE];

    vArgs.push_back(argv[0]);
    vArgs.push_back(string("--docroot=")      + cmdLineOpts.docRoot);
    vArgs.push_back(string("--http-addr=")    + cmdLineOpts.httpAddr);
    //    vArgs.push_back(string("--http-port=") + _itoa(cmdLineOpts.httpPort, szBuffer, 10));
    vArgs.push_back(string("--http-port=")    + minsky::str(cmdLineOpts.httpPort));
    vArgs.push_back(string("--config=")       + cmdLineOpts.wtConfig);
    vArgs.push_back(string("--max-memory-request-size=") + minsky::str(cmdLineOpts.maxMemoryRequestSize));

    const char** arguments = new const char*[vArgs.size()];
    for (size_t i = 0; i < vArgs.size(); ++i)
    {
      arguments[i] = vArgs[i].c_str();
    }

    try
    {
    // start server.
      nResult = WRun(vArgs.size(), (char**)arguments, &MinskyApp::createApplication);
    }
    catch(...)
    {
      cout << "Unexpected server error!" << endl;
      cout << boost::current_exception();
      nResult = 3;
    }
    delete[] arguments;
  }
  return nResult;
}
