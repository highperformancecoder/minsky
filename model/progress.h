/*
  @copyright Steve Keen 2023
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

#ifndef PROGRESS_H
#define PROGRESS_H
#include <string>

namespace minsky
{
  class Minsky;
  struct BusyCursor
  {
    Minsky& minsky;
    BusyCursor(Minsky& m);
    ~BusyCursor();
  };

  class Progress
  {
    std::string title;
    double delta=100;
    double progress=0;
    bool updaterStack=false; // whether under updater control
    friend class ProgressUpdater;
  public:
    std::shared_ptr<std::atomic<bool>> cancel=std::make_shared<std::atomic<bool>>(false); ///< set to true to cancel process in progreess
    void displayProgress();
    void operator++() {
      if (progress+delta<=100)
        {
          progress+=delta;
          displayProgress();
        }
    }
  };

  class ProgressUpdater
  {
    Progress savedProgress;
    Progress& updatedProgress;
  public:
    ProgressUpdater(Progress& progress, const std::string& title, int numSteps):
      savedProgress(progress), updatedProgress(progress)
    {
      updatedProgress.title=title;
      if (updatedProgress.updaterStack)
        {
          updatedProgress.delta/=numSteps;
        }
      else
        {
          updatedProgress.updaterStack=true;
          updatedProgress.delta=100.0/numSteps;
        }
      updatedProgress.displayProgress();
    }
    /// Sets the progress to a given fraction of this stack's allocation
    void setProgress(double fraction) {
      updatedProgress.progress=savedProgress.progress+savedProgress.delta*fraction;
      updatedProgress.displayProgress();
    }
    ~ProgressUpdater() {
      updatedProgress=savedProgress;
      if (!updatedProgress.updaterStack)
        updatedProgress.progress=0; // reset progress counter once last ProgressUpdater exits
      else
        updatedProgress.displayProgress();
    }
  };
}

#endif
