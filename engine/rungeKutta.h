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

#ifndef RUNGEKUTTA_H
#define RUNGEKUTTA_H
#include "simulation.h"
#include "evalOp.h"
#include "evalGodley.h"
#include "integral.h"

namespace minsky
{
  struct RKdata; // an internal structure for holding Runge-Kutta data
  class Matrix; // convenience class for accessing matrix elements from a data array

  /// components excluded from reflection
  struct RungeKuttaExclude
  {
  protected:
    std::shared_ptr<RKdata> ode;
    EvalOpVector equations;
    std::vector<Integral> integrals;
    /// used to report a thrown exception on the simulation thread
    std::string threadErrMsg;
    /// flag indicates that RK engine is computing a step
    volatile bool RKThreadRunning=false;
  };
  
  class RungeKutta: public Simulation, public classdesc::Exclude<RungeKuttaExclude>, public ValueVector
  {
    CLASSDESC_ACCESS(RungeKutta);
  protected:
    void evalEquations(double result[], double, const double vars[]);
    void evalJacobian(Matrix&, double, const double vars[]);
    /// function to be integrated (internal use)
    static int RKfunction(double, const double y[], double f[], void*);
    /// compute jacobian (internal use)
    static int jacobian(double, const double y[], double*, double dfdt[], void*);
    friend struct RKdata;
  public:
    double t{0}; ///< time
    bool running=false; ///< controls whether simulation is running
    bool reverse=false; ///< reverse direction of simulation
    EvalGodley evalGodley;

    virtual ~RungeKutta()=default;
    /// checks whether a reset is required, and resets the simulation if so
    /// @return whether simulation was reset
    virtual bool resetIfFlagged() {return false;}
    void rkreset(); ///< reset the simulation
    void rkstep();  ///< step the equations (by n steps, default 1)
    /// evaluate the flow equations without stepping.
    /// @throw ecolab::error if equations are illdefined
    void evalEquations() {
      for (auto& eq: equations)
        eq->eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
    }

  };
}
#include "rungeKutta.cd"
#endif
