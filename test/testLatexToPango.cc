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
#include <gtest/gtest.h>
#include <iostream>
using namespace minsky;
using namespace std;

TEST(LaTeXToPango, LaTeXToPango)
{
  struct {const char* latex, *pango;} 
  qr[]= {
    {"x^yz", "<i>x<span size=\"smaller\" rise=\"5000\">y</span>z</i>"},
    {"x^{yy}z","<i>x<span size=\"smaller\" rise=\"5000\">yy</span>z</i>"},
    {"x^\\mathtt{yy}z","<i>x<span size=\"smaller\" rise=\"5000\"><tt>yy</tt></span>z</i>"},
    {"x^{\\mathbf{yy}}z","<i>x<span size=\"smaller\" rise=\"5000\"><b>yy</b></span>z</i>"},
    {"\\mathrm{xxx}", "<i>xxx</i>"},
    {"\\mathit{xxx}", "<i><i>xxx</i></i>"},
    {"\\mathcal{xxx}", "<i><i>xxx</i></i>"},
    {"x_yz", "<i>x<span size=\"smaller\" rise=\"-2000\">y</span>z</i>"},
    {"x_{yy}z","<i>x<span size=\"smaller\" rise=\"-2000\">yy</span>z</i>"},
    {"x_\\mathtt{yy}z","<i>x<span size=\"smaller\" rise=\"-2000\"><tt>yy</tt></span>z</i>"},
    {"x_{\\mathbf{yy}}z","<i>x<span size=\"smaller\" rise=\"-2000\"><b>yy</b></span>z</i>"},
    {"x_{\\mathbf{yy}z}","<i>x<span size=\"smaller\" rise=\"-2000\"><b>yy</b>z</span></i>"},
    {"\\sqrt2","<i>√2</i>"},
    {"\\sqrt{2}","<i>√2</i>"},
    {"\\sqrt[3]2","<i><span size=\"smaller\" rise=\"5000\">3</span>√2</i>"},
    {"\\sqrt[3]{2}","<i><span size=\"smaller\" rise=\"5000\">3</span>√2</i>"},
    {"\\verb+a_b^y\\mathrm <>&\'\"+",
     "<i><tt>a_b^y\\mathrm &lt;&gt;&amp;&apos;&quot;</tt></i>"},
    {"\\\"u","<i>ü</i>"},
    {"\\\"{u}","<i>ü</i>"},
    {"a_\\mathrm1","<i>a<span size=\"smaller\" rise=\"-2000\">1</span></i>"},
    {"a{bcd}","<i>abcd</i>"},
    {"a_1","<i>a<span size=\"smaller\" rise=\"-2000\">1</span></i>"},
  };
    
  for (size_t i=0; i<sizeof(qr)/sizeof(qr[0]); ++i)
    {
      EXPECT_EQ(qr[i].pango, latexToPango(qr[i].latex));
      if (qr[i].pango != latexToPango(qr[i].latex))
        cout << qr[i].latex << " failed."<<endl;
    }
    
}

