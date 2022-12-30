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

#include "minsky.h"
#include "rungeKutta.h"
#include "variableValue.h"
#include "error.h"
#include "matrix.h"
#include "rungeKutta.rcd"
#include "rungeKutta.xcd"
#include "simulation.rcd"
#include "minsky_epilogue.h"

#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>
#include <vector>

//#include <thread>
// std::thread apparently not supported on MXE for now...
#include <boost/thread.hpp>

using namespace std;

namespace minsky
{
  /// checks if any GUI events are waiting, and proces an event if so
  /// TODO - make this a virtual method of RungeKutta?
  //  void doOneEvent(bool idleTasksOnly);

  /*
    For using GSL Runge-Kutta routines
  */

  int RungeKutta::RKfunction(double t, const double y[], double f[], void *params)
  {
    if (params==NULL) return GSL_EBADFUNC;
    try
      {
        ((RungeKutta*)params)->evalEquations(f,t,y);
      }
    catch (std::exception& e)
      {
        ((RungeKutta*)params)->threadErrMsg=e.what();
        return GSL_EBADFUNC;
      }
    return GSL_SUCCESS;
  }

  int RungeKutta::jacobian(double t, const double y[], double * dfdy, double dfdt[], void * params)
  {
    if (params==NULL) return GSL_EBADFUNC;
    Matrix jac(stockVars.size(), dfdy);
    try
      {
        ((RungeKutta*)params)->evalJacobian(jac,t,y);
      }
    catch (std::exception& e)
      {
        ((RungeKutta*)params)->threadErrMsg=e.what();
        return GSL_EBADFUNC;
      }   
    return GSL_SUCCESS;
  }

  struct RKdata
  {
    gsl_odeiv2_system sys;
    gsl_odeiv2_driver* driver;

    static void errHandler(const char* reason, const char* file, int line, int gsl_errno) {
      throw ecolab::error("gsl: %s:%d: %s",file,line,reason);
    }

    RKdata(RungeKutta* minsky) {
      gsl_set_error_handler(errHandler);
      sys.function=RungeKutta::RKfunction;
      sys.jacobian=RungeKutta::jacobian;
      sys.dimension=minsky->stockVars.size();
      sys.params=minsky;
      const gsl_odeiv2_step_type* stepper;
      switch (minsky->order)
        {
        case 1: 
          if (!minsky->implicit)
            throw ecolab::error("First order explicit solver not available");
          stepper=gsl_odeiv2_step_rk1imp;
          break;
        case 2: 
          stepper=minsky->implicit? gsl_odeiv2_step_rk2imp: gsl_odeiv2_step_rk2;
          break;
        case 4:
          stepper=minsky->implicit? gsl_odeiv2_step_rk4imp: gsl_odeiv2_step_rkf45;
          break;
        default:
          throw ecolab::error("order %d solver not supported",minsky->order);
        }
      driver = gsl_odeiv2_driver_alloc_y_new
        (&sys, stepper, minsky->stepMax, minsky->epsAbs, 
         minsky->epsRel);
      gsl_odeiv2_driver_set_hmax(driver, minsky->stepMax);
      gsl_odeiv2_driver_set_hmin(driver, minsky->stepMin);
    }
    ~RKdata() {gsl_odeiv2_driver_free(driver);}
  };

  void RungeKutta::rkreset()
  {
    if (order==1 && !implicit)
      ode.reset(); // do explicit Euler
    else
      ode.reset(new RKdata(this)); // set up GSL ODE routines
  }

  void RungeKutta::rkstep()
  {
    if (nSteps<1) return;
    resetIfFlagged();
    running=true;
    
    // create a private copy for worker thread use
    vector<double> stockVarsCopy(stockVars);
    RKThreadRunning=true;
    int err=GSL_SUCCESS;
    // run RK algorithm on a separate worker thread so as to not block UI. See ticket #6
    boost::thread rkThread([&]() {
      try
        { 
          double tp=reverse? -t: t;
          if (ode)
            {
              gsl_odeiv2_driver_set_nmax(ode->driver, nSteps);
              // we need to update Minsky's t synchronously to support the t operator
              // potentially means t and stockVars out of sync on GUI, but should still be thread safe
              err=gsl_odeiv2_driver_apply(ode->driver, &tp, numeric_limits<double>::max(), 
                                          stockVarsCopy.data());
            }
          else // do explicit Euler method
            {
              vector<double> d(stockVarsCopy.size());
              for (int i=0; i<nSteps; ++i, tp+=stepMax)
                {
                  evalEquations(d.data(), tp, stockVarsCopy.data());
                  for (size_t j=0; j<d.size(); ++j)
                    stockVarsCopy[j]+=d[j];
                }
            }
          t=reverse? -tp:tp;
        }
      catch (const std::exception& ex)
        {
          // catch any thrown exception, and report back to GUI thread
          threadErrMsg=ex.what();
        }
      catch (...)
        {
          threadErrMsg="Unknown exception thrown on ODE solver thread";
        }
      RKThreadRunning=false;
    });

    while (RKThreadRunning)
      {
        // while waiting for thread to finish, check and process any UI events
        usleep(1000);
        doOneEvent(false);
      }
    rkThread.join();

    if (!threadErrMsg.empty())
      {
        auto msg=std::move(threadErrMsg);
        threadErrMsg.clear(); // to be sure, to be sure
        // rethrow exception so message gets displayed to user
        throw runtime_error(msg);
      }

    if (resetIfFlagged())
      return; // in case reset() was called during the step evaluation

    switch (err)
      {
      case GSL_SUCCESS: case GSL_EMAXITER: break;
      case GSL_FAILURE:
        throw error("unspecified error GSL_FAILURE returned");
      case GSL_EBADFUNC: 
        gsl_odeiv2_driver_reset(ode->driver);
        throw error("Invalid arithmetic operation detected");
      default:
        throw error("gsl error: %s",gsl_strerror(err));
      }

    stockVars.swap(stockVarsCopy);

    // update flow variables
    evalEquations();

  }

  void RungeKutta::evalJacobian(Matrix& jac, double t, const double sv[])
  {
    EvalOpBase::t=reverse? -t: t;
    double reverseFactor=reverse? -1: 1;
    // firstly evaluate the flow variables. Initialise to flowVars so
    // that no input vars are correctly initialised
    vector<double> flow=flowVars;
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(flow.data(), flow.size(), sv);

    // then determine the derivatives with respect to variable j
    for (size_t j=0; j<stockVars.size(); ++j)
      {
        vector<double> ds(stockVars.size()), df(flowVars.size());
        ds[j]=1;
        for (size_t i=0; i<equations.size(); ++i)
          equations[i]->deriv(df.data(), df.size(), ds.data(), sv, flow.data());
        vector<double> d(stockVars.size());
        evalGodley.eval(d.data(), df.data());
        for (vector<Integral>::iterator i=integrals.begin(); 
             i!=integrals.end(); ++i)
          {
            assert(i->stock->idx()>=0 && i->input().idx()>=0);
            d[i->stock->idx()] = 
              i->input().isFlowVar()? df[i->input().idx()]: ds[i->input().idx()];
          }
        for (size_t i=0; i<stockVars.size(); i++)
          jac(i,j)=reverseFactor*d[i];
      }
  
  }
  
  void RungeKutta::evalEquations(double result[], double t, const double vars[])
  {
    EvalOpBase::t=reverse? -t: t;
    double reverseFactor=reverse? -1: 1;
    // firstly evaluate the flow variables. Initialise to flowVars so
    // that no input vars are correctly initialised
    vector<double> flow(flowVars);
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(flow.data(), flow.size(), vars);

    // then create the result using the Godley table
    for (size_t i=0; i<stockVars.size(); ++i) result[i]=0;
    evalGodley.eval(result, flow.data());

    // integrations are kind of a copy
    for (vector<Integral>::iterator i=integrals.begin(); i<integrals.end(); ++i)
      {
        if (i->input().idx()<0)
          {
            if (i->operation)
              minsky().displayErrorItem(*i->operation);
            throw error("integral not wired");
          }
        // enable element-wise integration of tensor variables. for feature 147
        assert(i->input().size()==i->stock->size());
	for (size_t j=0; j<i->input().size(); ++j)
          result[i->stock->idx()+j] = reverseFactor *
            (i->input().isFlowVar()? flow[i->input().idx()+j] : vars[i->input().idx()+j]);
      } 
  }


}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::RungeKutta);
