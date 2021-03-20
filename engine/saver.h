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

#ifndef SAVER_H
#define SAVER_H
#include <xml_pack_base.h>
#include <fstream>
#include <string>
#include <thread>

namespace schema3
{
  class Minsky;
}

namespace minsky
{
  extern const char* schemaURL;

  struct Saver
  {
    std::string fileName;
    std::ofstream os;
    classdesc::xml_pack_t packer;
    Saver(const std::string& fileName);
    void save(const schema3::Minsky&);
  };

  struct BackgroundSaver: public Saver, public std::thread
  {
    std::thread thread;
    std::string lastError;
    BackgroundSaver(const std::string& fileName): Saver(fileName) {}
    ~BackgroundSaver() {killThread();}
    void save(const schema3::Minsky&);
    void killThread();
  };
}

#endif
