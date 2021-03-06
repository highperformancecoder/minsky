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

#ifndef MATRIX_H
#define MATRIX_H

namespace minsky
{
  /// convenience class for accessing matrix elements from a data array
  class Matrix
  {
    std::size_t n;
    double *data;
  public:
    Matrix(std::size_t n, double* data): n(n), data(data) {}
    double& operator()(std::size_t i, std::size_t j) {return data[i*n+j];}
    double operator()(std::size_t i, std::size_t j) const {return data[i*n+j];}
  };
}
#endif
