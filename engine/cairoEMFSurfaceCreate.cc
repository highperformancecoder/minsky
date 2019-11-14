/*
  @copyright Steve Keen 2019
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

#ifdef _WIN32
#include "cairoEMFSurfaceCreate.h"
#include <windows.h>
#include <wingdi.h>
#include <cairo/cairo-win32.h>
#include <memory>

#include <iostream>

namespace minsky
{
  namespace {
    struct SurfAndDC
    {
      HDC hdc;
      cairo_surface_t* surf;
    };
    
    cairo_user_data_key_t closeKey;
    void closeFile(void *x)
    {
      std::unique_ptr<SurfAndDC> s(static_cast<SurfAndDC*>(x));
      cairo_surface_flush(s->surf);
      // nb the Delete... function deletes the handle created by Close...
      DeleteEnhMetaFile(CloseEnhMetaFile(s->hdc));
      DeleteDC(s->hdc);
    }
  }
  
  cairo_surface_t* createEMFSurface(const char* filename, double width, double height)
  {
    RECT r{0,0,width,height};
    HDC hdc=CreateEnhMetaFileA(nullptr,filename,nullptr,"Minsky\0");
    //SetBkColor(hdc,0x00ffffff); // set transparent background
    // initialise the image background 
    //Rectangle(hdc,0,0,width,height);
    //SetDCBrushColor(hdc,0xFFFFFF);
    //FillPath(hdc);
    
    auto surf=cairo_win32_surface_create_with_format(hdc,CAIRO_FORMAT_ARGB32);
    SetBkColor(hdc,0x00ffffff);
    std::cout << SetBkMode(hdc, TRANSPARENT) << std::endl;
    // set up a callback to flush and close the EMF file
    cairo_surface_set_user_data(surf,&closeKey,new SurfAndDC{hdc,surf},closeFile);
    
    return surf;
  }
}
#endif
