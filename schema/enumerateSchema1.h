/*
  @copyright Steve Keen 2013
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

#ifndef ENEUMERATE_SCHEMA1_H
#define ENEUMERATE_SCHEMA1_H

// These are declared in a separate header file, as classdesc core
// dumps on template member function invocation

namespace schema1
{
  /// utilities to call registerType for these class heirarchies, for
  /// different factory types
  template<class F>
  void enumerateRegisterItems(F& f)
  {
    f.template registerType<Item>();
    f.template registerType<Port>();
    f.template registerType<Wire>();
    f.template registerType<Operation>();
    f.template registerType<Variable>();
    f.template registerType<Plot>();
    f.template registerType<Group>();
    f.template registerType<Switch>();
    f.template registerType<Godley>();
    f.template registerType<Group>();
  }

  template<class Factory>
  void enumerateRegisterLayout(Factory& f)
  {
    f.template registerType<Layout>();
    f.template registerType<PositionLayout>();
    f.template registerType<WireLayout>();
    f.template registerType<ItemLayout>();
    f.template registerType<GroupLayout>();
    f.template registerType<SliderLayout>();
    f.template registerType<PlotLayout>();
    f.template registerType<UnionLayout>();
  }

}

#endif
