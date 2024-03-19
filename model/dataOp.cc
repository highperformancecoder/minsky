
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
#include "dataOp.h"
#include "selection.h"
#include "lasso.h"
#include "variableValue.h"
#include "classdesc.h"
#include "plotWidget.h"
#include "nobble.h"
#include "minsky_epilogue.h"

using namespace std;
namespace minsky
{
  void DataOp::readData(const string& fileName)
  {
    ifstream f(fileName.c_str());
    data.clear();
    // for now, we just read pairs of numbers, separated by
    // whitespace. Later, we need to add in the smarts to handle a
    // variety of CSV formats
    double x, y;
    while (f>>x>>y)
      data[x]=y; // TODO: throw if more than one equal value of x provided?

    // trim any leading directory
    const size_t p=fileName.rfind('/');
    // '/' is guaranteed not to be in fileName, so we can use that as
    // a delimiter
    description("\\verb/"+
                ((p!=string::npos)? fileName.substr(p+1): fileName) + "/");
  }

  void DataOp::initRandom(double xmin, double xmax, unsigned numSamples)
  {
    data.clear();
    const double dx=(xmax-xmin)/numSamples;
    for (double x=xmin; x<xmax; x+=dx) //NOLINT
      data[x]=double(rand())/RAND_MAX;
  }

  double DataOp::interpolate(double x) const
  {
    // not terribly sensible, but need to return something
    if (data.empty()) return 0;

    const map<double, double>::const_iterator v=data.lower_bound(x);
    if (v==data.end())
      return data.rbegin()->second;
    if (v==data.begin())
      return v->second;
    if (v->first > x)
      {
        map<double, double>::const_iterator v0=v;
        --v0;
        return (x-v0->first)*(v->second-v0->second)/
          (v->first-v0->first)+v0->second;
      }
    assert(v->first==x);
    return v->second;
  }

  double DataOp::deriv(double x) const
  {
    const map<double, double>::const_iterator v=data.lower_bound(x);
    if (v==data.end() || v==data.begin())
      return 0;
    map<double, double>::const_iterator v1=v, v2=v; 
    --v1;
    if (v->first==x)
      {
        ++v2;
        if (v2==data.end()) v2=v;
        return (v2->second-v1->second)/(v2->first-v1->first);
      }
    return (v->second-v1->second)/(v->first-v1->first);
  }

  void DataOp::pack(classdesc::pack_t& x, const string& d) const
  {::pack(x,d,*this);}
      
  void DataOp::unpack(classdesc::unpack_t& x, const string& d)
  {::unpack(x,d,*this);}

}

//CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::DataOp);
