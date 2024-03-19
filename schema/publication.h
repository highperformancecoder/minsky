/*
  @copyright Steve Keen 2024
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

#ifndef PUBLICATION_H
#define PUBLICATION_H
#include <string>
#include <vector>
namespace schema3
{
  struct PublicationItem
  {
    int item=-1;
    float x=100,y=100;
    float zoomX=1, zoomY=1;
    double rotation=0;
    bool editorMode=false;
    PublicationItem()=default;
    PublicationItem(int item, const PublicationItem& x) {*this=x; this->item=item;}
  };
  
  struct PublicationTab
  {
    std::string name;
    std::vector<PublicationItem> items;
    float x=100,y=100,zoomFactor=1;
  };
}
using classdesc::xsd_generate;
using classdesc::xml_pack;
#include "publication.cd" 
#include "publication.rcd" 
#include "publication.xcd" 
#endif
