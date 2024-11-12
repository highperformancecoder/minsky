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

#ifndef DATAOP_H
#define DATAOP_H

#include "operation.h"

namespace minsky
{
  /// legacy data importer object
  class DataOp: public ItemT<DataOp, Operation<minsky::OperationType::data>>,
                public NamedOp
  {
    CLASSDESC_ACCESS(DataOp);
    friend struct SchemaHelper;
    void updateBB() override {bb.update(*this);}
    void draw(cairo_t* cairo) const override {
      if (description().empty())
        OperationBase::draw(cairo);
      else
        drawUserFunction(cairo);
    }
    
  public:
    ~DataOp() {}
    
    const DataOp& operator=(const DataOp& x); 

    std::map<double, double> data;
    void readData(const std::string& fileName);
    /// initialise with uniform random numbers 
    void initRandom(double xmin, double xmax, unsigned numSamples);
    /// interpolates y data between x values bounding the argument
    double interpolate(double) const;
    /// derivative of the interpolate function. At the data points, the
    /// derivative is defined as the weighted average of the left & right
    /// derivatives, weighted by the respective intervals
    double deriv(double) const;
    Units units(bool check) const override {return m_ports[1]->units(check);}

    void pack(classdesc::pack_t& x, const std::string& d) const override;
    void unpack(classdesc::unpack_t& x, const std::string& d) override;
  };

}

#include "dataOp.cd"
#include "dataOp.xcd"
#endif
