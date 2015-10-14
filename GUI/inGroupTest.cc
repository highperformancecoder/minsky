/*
  @copyright Steve Keen 2012
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

#include "inGroupTest.h"
#include <ecolab_epilogue.h>

using namespace minsky;
using namespace std;

InGroup::Cell::Cell(int id, const GroupIcon& g): 
  id(id), 
  y0(g.y()-0.5*g.height*g.zoomFactor), 
  y1(g.y()+0.5*g.height*g.zoomFactor), 
  area(g.width*g.height*g.zoomFactor* g.zoomFactor) 
{
  x0=g.x()-(0.5*g.width)*g.zoomFactor;
  x1=g.x()+(0.5*g.width)*g.zoomFactor;
}


namespace
{
  void excludeSelfAndChildren(set<int>& excludeIds, 
                              const IntrusiveMap<int, GroupIcon>& g, int id)
  {
    excludeIds.insert(id);
    auto excludeGroup=g.find(id);
    if (excludeGroup!=g.end())
      for (int i: excludeGroup->groups())
        excludeSelfAndChildren(excludeIds, g, i);
  }
     
}

void InGroup::initGroupList(const IntrusiveMap<int, GroupIcon>& g, int exclude)
{
  cells.clear();
  // construct all the Cells
  vector<Cell> rects;
  set<int> excludeIds;
  excludeSelfAndChildren(excludeIds, g, exclude);

  for (auto i=g.begin(); i!=g.end(); ++i)
    if (excludeIds.count(i->id())==0)
      rects.push_back(Cell(i->id(), *i));

  // compute total bounds
  ymin=xmin=numeric_limits<float>::max(); ymax=xmax=-xmin;
  for (vector<Cell>::const_iterator i=rects.begin(); i!=rects.end(); ++i)
    {
      if (xmin>i->x0) xmin=i->x0;
      if (xmax<i->x1) xmax=i->x1;
      if (ymin>i->y0) ymin=i->y0;
      if (ymax<i->y1) ymax=i->y1;
    }
  if (g.empty()) return;

  // start by assuming rectangles are even distributed, so divide
  // total area into g.size() cells. If all the rectangles are stacked
  // on top of each other, this algorithm will be unbalanced, but that
  // is fairly pathological, as well designed layouts will spread out
  // the groups
  float sqrtNoGroups=sqrt(g.size());
  xBinSz=(xmax-xmin)/sqrtNoGroups;
  yBinSz=(ymax-ymin)/sqrtNoGroups;
  cells.resize(int(sqrtNoGroups+1), 
               std::vector<std::set<Cell> >(int(sqrtNoGroups+1)));
  size_t ix,iy;
  float x,y;
  for (vector<Cell>::const_iterator i=rects.begin(); i!=rects.end(); ++i)
    for (x=i->x0, ix=(x-xmin)/xBinSz; ix<cells.size() && x<i->x1+xBinSz; x+=xBinSz, ix=(x-xmin)/xBinSz)
      for (y=i->y0, iy=(y-ymin)/yBinSz; iy<cells[ix].size() && y<i->y1+yBinSz; y+=yBinSz, iy=(y-ymin)/yBinSz)
        cells[ix][iy].insert(*i);
}




int InGroup::containingGroup(float x, float y) const
{
  if (x<xmin||x>xmax||y<ymin||y>ymax) return -1;

  /// rectangles are binned into cells of size (xBinSz, yBinSz),
  /// lookup searches linearly over cell contents, sorted by rectangle
  /// size
  int xi=(x-xmin)/xBinSz, yi=(y-ymin)/yBinSz;
  for (set<Cell>::const_iterator i=cells[xi][yi].begin(); i!=cells[xi][yi].end(); ++i)
    if (i->inRect(x,y))
      return i->id;
  return -1;
}

int InGroup::groupContainingBoth(float x0, float y0, float x1, float y1) const
{
  if (x0<xmin||x0>xmax||y0<ymin||y0>ymax||
      x1<xmin||x1>xmax||y1<ymin||y1>ymax) return -1;

  /// rectangles are binned into cells of size (xBinSz, yBinSz),
  /// lookup searches linearly over cell contents, sorted by rectangle
  /// size
  int xi=(x0-xmin)/xBinSz, yi=(y0-ymin)/yBinSz;
  for (set<Cell>::const_iterator i=cells[xi][yi].begin(); i!=cells[xi][yi].end(); ++i)
    if (i->inRect(x0,y0) && i->inRect(x1,y1))
      return i->id;
  return -1;
}
