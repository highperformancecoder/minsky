/*
  @copyright Steve Keen 2021
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

#include "saver.h"
#include "schema3.h"
#include "minsky_epilogue.h"

namespace minsky
{
  const char* schemaURL="http://minsky.sf.net/minsky";
  
  Saver::Saver(const string& fileName): fileName(fileName), packer(os, schemaURL) {}

  namespace
  {
    struct OnExit
    {
      std::function<void()> f;
      OnExit(const std::function<void()>& f): f(f) {}
      ~OnExit() {f();}
    };
  }
  
  void Saver::save(const schema3::Minsky& m)
  {
    os.open(fileName);
    const OnExit closeOnExit([&](){os.close();});

    try
      {
        packer.abort=false; // reset abort flag
        xml_pack(packer, "Minsky", m);
      }
    catch (const xml_pack_t::PackAborted&) {}
    catch (...) {
      // if exception is due to file error, provide a more useful message
      if (!os)
        throw runtime_error("cannot save to "+fileName);
      throw;
    }
  }

  void BackgroundSaver::killThread()
  {
    if (thread.joinable())
      {
        packer.abort=true;
        thread.join();
      }
  }
  
  void BackgroundSaver::save(const schema3::Minsky& m)
  {
    killThread();
    if (!lastError.empty())
      {
        lastError.clear();
        throw std::runtime_error(lastError);
      }
    thread=std::thread([this,m](){
      try
        {
          Saver::save(m);
        }
      catch (const std::exception& ex) {lastError=ex.what();}
      catch (...) {} // we don't want any error to propagate
    });
  }

  
}
