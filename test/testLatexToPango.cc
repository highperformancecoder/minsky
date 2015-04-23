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
#include "latexMarkup.h"
#include <UnitTest++/UnitTest++.h>
#include <iostream>
using namespace minsky;
using namespace std;

TEST(LaTeXToPango)
{
  struct {const char* latex, *pango;} 
  qr[]= {
    {"x^yz", "<i>x<sup>y</sup>z</i>"},
    {"x^{yy}z","<i>x<sup>yy</sup>z</i>"},
    {"x^\\mathtt{yy}z","<i>x<sup><tt>yy</tt></sup>z</i>"},
    {"x^{\\mathbf{yy}}z","<i>x<sup><b>yy</b></sup>z</i>"},
    {"\\mathrm{xxx}", "<i><span style=\"normal\">xxx</span></i>"},
    {"\\mathit{xxx}", "<i><i>xxx</i></i>"},
    {"\\mathcal{xxx}", "<i><i>xxx</i></i>"},
    {"x_yz", "<i>x<sub>y</sub>z</i>"},
    {"x_{yy}z","<i>x<sub>yy</sub>z</i>"},
    {"x_\\mathtt{yy}z","<i>x<sub><tt>yy</tt></sub>z</i>"},
    {"x_{\\mathbf{yy}}z","<i>x<sub><b>yy</b></sub>z</i>"},
    {"x_{\\mathbf{yy}z}","<i>x<sub><b>yy</b>z</sub></i>"},
    {"\\sqrt2","<i>√2</i>"},
    {"\\sqrt{2}","<i>√2</i>"},
    {"\\sqrt[3]2","<i><small><sup>3</sup></small>√2</i>"},
    {"\\sqrt[3]{2}","<i><small><sup>3</sup></small>√2</i>"},
    {"\\verb+a_b^y\\mathrm <>&\'\"+",
     "<i><tt>a_b^y\\mathrm &lt;&gt;&amp;&apos;&quot;</tt></i>"},
    {"\\\"u","<i>ü</i>"},
    {"\\\"{u}","<i>ü</i>"},
    {"a_\\mathrm1","<i>a<sub><span style=\"normal\">1</span></sub></i>"},
    {"a{bcd}","<i>abcd</i>"},
    {"a_1","<i>a<sub>1</sub></i>"},
  };
    
  for (size_t i=0; i<sizeof(qr)/sizeof(qr[0]); ++i)
    {
      CHECK_EQUAL(qr[i].pango, latexToPango(qr[i].latex));
      if (qr[i].pango != latexToPango(qr[i].latex))
        cout << qr[i].latex << " failed."<<endl;
    }
    
}

