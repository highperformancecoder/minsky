/*
  @copyright Steve Keen 2019
  @author Russell Standish
  @author Wynand Dednam
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
#include "parameterSheet.h"
#include "latexMarkup.h"
#include "group.h"
#include <pango.h>
#include "minsky_epilogue.h"
#include "minsky.h"
using namespace std;
using ecolab::cairo::Surface;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky
{
	
  namespace
  {    
    struct CroppedPango: public Pango
    {
      cairo_t* cairo;
      double w, x=0, y=0;
      CroppedPango(cairo_t* cairo, double width): Pango(cairo), cairo(cairo), w(width) {}
      void setxy(double xx, double yy) {x=xx; y=yy;}
      void show() {
        CairoSave cs(cairo);
        cairo_rectangle(cairo,x,y,w,height());
        cairo_clip(cairo);
        cairo_move_to(cairo,x,y);
        Pango::show();
      }
    };
  }

  void ParameterSheet::redraw(int, int, int width, int height)
  {
    if (surface.get()) {
        cairo_t* cairo=surface->cairo();  
        CroppedPango pango(cairo, colWidth);
        rowHeight=15;
        pango.setFontSize(5.0*rowHeight);
	    
        if (!minsky().canvas.model->empty()) {	  
          populateItemVector();			               
          cairo_translate(cairo,offsx,offsy); 
          colLeftMargin.clear();         
          draw(cairo); 
          ecolab::cairo::Surface surf
            (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));            
          draw(surf.cairo());      
          m_width=surf.width();
          m_height=surf.height();
          //cout << m_width << " " << m_height << " " << offsx << " " << offsy << " " << rows() << " " << cols() << " " << hoverRow << " " << hoverCol << endl;
          //for (size_t i=0; i< colLeftMargin.size(); i++) cout << " " << colLeftMargin[i];
          //cout << endl;
          //if (((fabs(offsx)>1.5*m_width) || (offsx<0 && fabs(offsx)>600)) || ((fabs(offsy)>1.5*m_height) || (offsy<0 && fabs(offsy)>800))) cairo_translate(surf.cairo(),0.0,0.0); 
        }     
      }
    }

}
