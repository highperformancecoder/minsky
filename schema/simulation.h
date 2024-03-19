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
#ifndef SIMULATION_H
#define SIMULATION_H

#include <cmath>
#include <string>

namespace minsky
{
 struct Simulation
  {
    double stepMin{0}, stepMax{0.01};
    int nSteps{1};
    double epsRel{1e-2}, epsAbs{1e-3};
    int order{4};
    bool implicit{false};
    int simulationDelay{0};
    std::string timeUnit;
    double tmax{INFINITY}, t0{0};
  };
}

#include "simulation.cd" 
#include "simulation.xcd" 
#include "simulation.rcd" 
#endif
