/*
  @copyright Steve Keen 2012
  @author Michael Roy
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

#ifndef _GUIDEFS_H
#define _GUIDEFS_H

#include <boost/exception/all.hpp>  //  needed for GUI_CATCH_ALL()
#include <exception>

/**
 *  @def GUI_CATCH_ALL()
 *        Standard GUI catch clauses that print exception information.
 *  @note Use only for debugging or to log minor errors.
 */
#define GUI_CATCH_ALL()                                                             \
  catch(boost::exception& e)                                                        \
  {                                                                                 \
    std::cerr << "Minksy: boost exception caught in " << BOOST_CURRENT_FUNCTION << endl;\
    std::cerr << boost::diagnostic_information(e) << endl;                          \
  }                                                                                 \
  catch(std::exception& e)                                                          \
  {                                                                                 \
    std::cerr << "Minksy: std exception caught in " << BOOST_CURRENT_FUNCTION << endl;\
    std::cerr << e.what() << endl;                                                  \
  }                                                                                 \
  catch(...)                                                                        \
  {                                                                                 \
    std::cerr << "Minksy: exception caught in " << BOOST_CURRENT_FUNCTION << endl;  \
    std::cerr << "Unknown exception" << endl;                                       \
  }

#endif
