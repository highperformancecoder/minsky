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
#include "sheet.h"
#include "str.h"
#include "selection.h"
#include "lasso.h"
#include "minsky.h"
#include "plotWidget.h"
#include <cairo_base.h>
#include <pango.h>

#include "itemT.rcd"
#include "sheet.rcd"
#include "showSlice.xcd"
#include "minskyCairoRenderer.h"
#include "ravelWrap.h"
#include "tensorOp.h"
#include "minsky_epilogue.h"

using namespace minsky;
using namespace ecolab;
using namespace std;

// width of draggable border 
const float border=10;
// proportion of width/height to centre Ravel popout
const double ravelOffset=0.1;

Sheet::Sheet()
{
  m_ports.emplace_back(make_shared<InputPort>(*this));
  iWidth(100);
  iHeight(100);	  
}

double Sheet::ravelSize() const
{
  return 0.25*std::min(m_width,m_height)*zoomFactor();
}

double Sheet::ravelX(double xx) const
{
  return (xx+(0.5+ravelOffset)*zoomFactor()*m_width-x())*inputRavel.radius()/(zoomFactor()*ravelSize());
}

double Sheet::ravelY(double yy) const
{
  return (yy+(0.5+ravelOffset)*zoomFactor()*m_height-y())*inputRavel.radius()/(zoomFactor()*ravelSize());
}      

bool Sheet::onResizeHandle(float xx, float yy) const
{
  float dx=xx-x(), dy=yy-y();
  float w=0.5*m_width*zoomFactor(), h=0.5*m_height*zoomFactor();
  return fabs(dx)>=w-resizeHandleSize() && fabs(dx)<=w+resizeHandleSize() &&
    fabs(dy)>=h-resizeHandleSize() && fabs(dy)<=h+resizeHandleSize() &&
    (!inputRavel || dx>0 || dy>0);
}


void Sheet::drawResizeHandles(cairo_t* cairo) const
{
  auto sf=resizeHandleSize();
  float w=0.5f*m_width*zoomFactor(), h=0.5f*m_height*zoomFactor();
  if (!showRavel)
    drawResizeHandle(cairo,-w,-h,sf,0);
  drawResizeHandle(cairo,w,-h,sf,0.5*M_PI);
  drawResizeHandle(cairo,w,h,sf,0);
  drawResizeHandle(cairo,-w,h,sf,0.5*M_PI);
  cairo_stroke(cairo);
}


bool Sheet::onRavelButton(float xx, float yy) const
{
  float dx=xx-x(), dy=yy-y();
  float w=0.5*m_width*zoomFactor(), h=0.5*m_height*zoomFactor(), b=border*zoomFactor();
  return inputRavel && dx>=-w && dx<=b-w && dy>=-h && dy<=b-h;
}

bool Sheet::inRavel(float xx, float yy) const
{
  auto dx=xx-x(), dy=yy-y();
  float w=0.5*m_width*zoomFactor(), h=0.5*m_height*zoomFactor();
  return showRavel && inputRavel && !(dx>=w && dy>=h) &&
    fabs(ravelX(xx))<1.1*inputRavel.radius() && fabs(ravelY(yy))<1.1*inputRavel.radius();
}

bool Sheet::inItem(float xx, float yy) const
{
  double z=zoomFactor();
  double w=0.5*m_width*z, h=0.5*m_height*z, b=border*z;
  return (abs(xx-x())<w-b && abs(yy-y())<h-b) || onRavelButton(xx,yy) || inRavel(xx,yy);
}

void Sheet::onMouseDown(float x, float y)
{
  if (onRavelButton(x,y))
    {
      showRavel=!showRavel;
      bb.update(*this);
    }
  else if (inRavel(x,y))
    inputRavel.onMouseDown(ravelX(x), ravelY(y));
}

void Sheet::onMouseUp(float x, float y)
{if (inRavel(x,y)) inputRavel.onMouseUp(ravelX(x), ravelY(y));}

bool Sheet::onMouseMotion(float x, float y)
{
  if (inRavel(x,y))
    return inputRavel.onMouseMotion(ravelX(x), ravelY(y));
  else if (inputRavel)
    {
      inputRavel.onMouseLeave();
      return true;
    }
  return false;
}

bool Sheet::onMouseOver(float x, float y)
{
  if (inRavel(x,y))
    return inputRavel.onMouseOver(ravelX(x), ravelY(y));
  else if (inputRavel)
    {
      inputRavel.onMouseLeave();
      return true;
    }
  return false;
}

void Sheet::onMouseLeave()
{if (inputRavel) inputRavel.onMouseLeave();}

ClickType::Type Sheet::clickType(float x, float y) const
{
  if (onResizeHandle(x,y)) return ClickType::onResize;
  if (inItem(x,y)) return ClickType::inItem;                     
  double dx=fabs(x-this->x()), dy=fabs(y-this->y());
  double w=0.5*m_width*zoomFactor(), h=0.5*m_height*zoomFactor();  
  if (dx < w && dy < h)
    return ClickType::onItem;
  return ClickType::outside;  
}

std::vector<Point> Sheet::corners() const
{
  float w=0.5*m_width*zoomFactor(), h=0.5*m_height*zoomFactor();  
  return {{x()-w,y()-h},{x()+w,y()-h},{x()-w,y()+h},{x()+w,y()+h}};
}

bool Sheet::contains(float x, float y) const
{
  return Item::contains(x,y) || inRavel(x,y);
}

bool Sheet::scrollUp()
{
  if (scrollOffset+scrollDelta<scrollMax)
    {
      scrollOffset+=scrollDelta;
      setSliceIndicator();
      return true;
    }
  return false;
}

bool Sheet::scrollDown()
{
  if (scrollOffset>scrollDelta)
    {
      scrollOffset-=scrollDelta;
      setSliceIndicator();
      return true;
    }
  return false;
}

bool Sheet::onKeyPress(int keySym, const std::string& utf8, int state)
{
  switch (keySym)
    {
    case 0xff52: case 0xff53: case 0xff55: //Right, Up
      return scrollUp();
    case 0xff51: case 0xff54: case 0xff56://Left, Down
      return scrollDown();
    default:
        return false;
    }
}

namespace {
  string formattedStr(const XVector& xv, size_t index)
  {
    return str(xv[index], minsky::minsky().dimensions[xv.name].units);
  }
}

void Sheet::setSliceIndicator()
{
  if (!value || value->rank()<=2) return;
  auto idx=value->hypercube().splitIndex(scrollOffset);
  sliceIndicator=formattedStr(value->hypercube().xvectors[2], idx[2]);
  for (size_t i=3; i<idx.size(); ++i)
    sliceIndicator+=" | "+formattedStr(value->hypercube().xvectors[i], idx[i]);
}

namespace
{
  struct ElisionRowChecker
  {
    size_t startRow, numRows, tailStartRow;
    double rowHeight;
    ElisionRowChecker(ShowSlice slice, double height, double rowHeight, size_t size): rowHeight(rowHeight) {
      switch (slice)
        {
        default: // just to shut up the "maybe uninitialised" checker.
        case ShowSlice::head:
          tailStartRow=startRow=0;
          numRows=height/rowHeight+1;
          break;
        case ShowSlice::headAndTail:
          startRow=0;
          numRows=0.5*height/rowHeight;
          tailStartRow=size-numRows+1;
          if (2*numRows*rowHeight>height)
            tailStartRow++;
          break;
        case ShowSlice::tail:
          numRows=height/rowHeight-1;
          tailStartRow=startRow=size-numRows;
          break;
        }
    }
    bool operator()(size_t& row, double& y) {
      if (row==startRow+numRows && row<tailStartRow)
        {
          row=tailStartRow; // for middle elision
          y+=rowHeight;
        }
      return row>tailStartRow+numRows;
    }
  };
}

void Sheet::computeValue()
{
  if (m_ports[0] && (value=m_ports[0]->getVariableValue()) && showRavel && inputRavel )
    {
      bool wasEmpty=inputRavel.numHandles()==0;
      inputRavel.populateFromHypercube(value->hypercube());
      for (size_t i=0; i<inputRavel.numHandles(); ++i)
        inputRavel.displayFilterCaliper(i,true);
      if (wasEmpty)
        switch (value->rank())
          {
          case 0: break;
          case 1:
            inputRavel.setOutputHandleIds({0});
            break;
          default:
            inputRavel.setOutputHandleIds({0,1});
            break;
          }
      
      if (value->rank()>0)
        {
          value=inputRavel.hyperSlice(value);
          if (value->rank()>=2)
            { // swap first and second axes
              auto& xv=value->hypercube().xvectors;
              auto pivot=make_shared<civita::Pivot>();
              pivot->setArgument(value,{});
              pivot->setOrientation(vector<string>{xv[1].name,xv[0].name});
              value=std::move(pivot);
            }
        }
    }
  if (value && value->rank()>2)
    {
      size_t delta=value->hypercube().xvectors[0].size()*value->hypercube().xvectors[1].size();
      if (delta!=scrollDelta)
        {
          scrollDelta=delta;
          scrollOffset=0;
        }
    }
  else
    scrollOffset=scrollDelta=0;
  scrollMax=value? value->size(): 1;
  setSliceIndicator();
}


void Sheet::draw(cairo_t* cairo) const
{
  auto z=zoomFactor();
  m_ports[0]->moveTo(x()-0.5*m_width*z,y());
  if (mouseFocus)
    {
      drawPorts(cairo);
      displayTooltip(cairo,tooltip);
      // Resize handles always visible on mousefocus. For ticket 92.
      drawResizeHandles(cairo);
    }

  cairo_scale(cairo,z,z);

  if (inputRavel)
    {
      if (showRavel)
        {
          cairo::CairoSave cs(cairo);
          cairo_translate(cairo,-(0.5+ravelOffset)*m_width,-(0.5+ravelOffset)*m_height);
          double r=inputRavel.radius();
          double scale=ravelSize()/r;
          cairo_scale(cairo,scale,scale);
          double cornerX=ravelOffset*m_width/scale;
          double cornerY=ravelOffset*m_height/scale;
          // clip out the bottom right quadrant
          r*=1.1; // allow space for arrow heads
          cairo_move_to(cairo,cornerX,cornerY);
          cairo_line_to(cairo,r,cornerY);
          cairo_line_to(cairo,r,-r);
          cairo_line_to(cairo,-r,-r);
          cairo_line_to(cairo,-r,r);
          cairo_line_to(cairo,cornerX,r);
          cairo_stroke_preserve(cairo);
          cairo_clip(cairo);
          CairoRenderer render(cairo);
          inputRavel.render(render);
        }
      // display ravel button
      cairo::CairoSave cs(cairo);
      cairo_translate(cairo,-0.5*m_width,-0.5*m_height);
      cairo_scale(cairo,border/Ravel::svgRenderer.width(),border/Ravel::svgRenderer.height());
      Ravel::svgRenderer.render(cairo);
    }
  
  cairo_rectangle(cairo,-0.5*m_width+border,-0.5*m_height+border,m_width-2*border,m_height-2*border);
  cairo_stroke_preserve(cairo);
  cairo_rectangle(cairo,-0.5*m_width,-0.5*m_height,m_width,m_height);
  cairo_stroke_preserve(cairo);
  // draw border
  if (onBorder)
    { // shadow the border when mouse is over it
      cairo::CairoSave cs(cairo);
      cairo_set_source_rgba(cairo,0.5,0.5,0.5,0.5);
      cairo_set_fill_rule(cairo,CAIRO_FILL_RULE_EVEN_ODD);
      cairo_fill(cairo);
    }
  cairo_new_path(cairo);
  cairo_rectangle(cairo,-0.5*m_width+border,-0.5*m_height+border,m_width-2*border,m_height-2*border);
  cairo_clip(cairo);
  
  if (selected) drawSelected(cairo);

  try
    {
      if (!value || !m_ports[0] || m_ports[0]->numWires()==0) return;
      Pango pango(cairo);
      pango.setFontSize(12.0);
      double x0=-0.5*m_width+border, y0=-0.5*m_height+border;
      if (value->hypercube().rank()==0)
        {
          cairo_move_to(cairo,x0,y0);
          pango.setMarkup(str((*value)[0]));
          pango.show();
        }
      else
        {
          if (value->size()!=scrollMax) minsky().requestReset(); // fix up slice indicator
          
          if (!value->hypercube().xvectors[0].name.empty())
            {
              cairo::CairoSave cs(cairo);
              pango.setMarkup(value->hypercube().xvectors[0].name);
              x0+=pango.height();
              cairo_move_to(cairo,x0, -0.5*pango.width());
              pango.angle=0.5*M_PI;
              pango.show();
              pango.angle=0;
              { // draw vertical grid line
                cairo::CairoSave cs(cairo);
                cairo_set_source_rgba(cairo,0,0,0,0.5);
                cairo_move_to(cairo,x0,-0.5*m_height);
                cairo_line_to(cairo,x0,0.5*m_height);
                cairo_stroke(cairo);
              }                  				
            }

          pango.setMarkup("9999");
          double rowHeight=pango.height();

          double colWidth=0;
          double x=x0, y=y0;   // make sure row labels are aligned with corresponding values. for ticket 1281
          string format=value->hypercube().xvectors[0].dimension.units;
          // calculate label column width
          for (auto& i: value->hypercube().xvectors[0])
            {
              pango.setText(trimWS(str(i,format)));
              colWidth=std::max(colWidth,5+pango.width());
            }                

          if (value->hypercube().rank()>=2)
            {
              y+=2*rowHeight; // allow room for header row
              if (value->hypercube().rank()>2) // allow room for slice indicator
                {
                  y+=rowHeight;
                  cairo::CairoSave cs(cairo);
                  pango.setMarkup(sliceIndicator);
                  cairo_move_to(cairo,0.5*(x0+colWidth+0.5*m_width-pango.width()), y0);
                  y0+=pango.height();
                  pango.show();
                }
              if (!value->hypercube().xvectors[1].name.empty())
                {
                  cairo::CairoSave cs(cairo);
                  pango.setMarkup(value->hypercube().xvectors[1].name);
                  cairo_move_to(cairo,0.5*(x0+colWidth+0.5*m_width-pango.width()), y0);
                  y0+=pango.height();
                  pango.show();
                }
              
              { // draw horizontal grid lines
                cairo::CairoSave cs(cairo);
                cairo_set_source_rgba(cairo,0,0,0,0.5);
                cairo_move_to(cairo,-0.5*m_width,y0-2.5);
                cairo_line_to(cairo,0.5*m_width,y0-2.5);
                cairo_stroke(cairo);
                cairo_move_to(cairo,x,y0+rowHeight-2.5);
                cairo_line_to(cairo,0.5*m_width,y0+rowHeight-2.5);
                cairo_stroke(cairo);
                    
              }
            }

          auto dims=value->hypercube().dims();
          double dataHeight=m_height;
          switch (dims.size())
            {
            case 0: case 1: break;
            case 2: dataHeight-=2*(rowHeight+3); break;
            default: dataHeight-=3*(rowHeight+3); break;
            }
          ElisionRowChecker adjustRowAndFinish(showSlice,dataHeight,rowHeight,dims[0]);
              
          // draw in label column
          auto& xv=value->hypercube().xvectors[0];
          for (size_t i=adjustRowAndFinish.startRow; i<xv.size(); ++i)
            {
              if (adjustRowAndFinish(i,y)) break;
              cairo_move_to(cairo,x,y);
              pango.setText(trimWS(str(xv[i],format)));
              pango.show();
              y+=rowHeight;
            }                
          y=y0;          
          x+=colWidth;            
          if (value->hypercube().rank()==1)
            {
              { // draw vertical grid line
                cairo::CairoSave cs(cairo);
                cairo_set_source_rgba(cairo,0,0,0,0.5);
                cairo_move_to(cairo,x,-0.5*m_height);
                cairo_line_to(cairo,x,0.5*m_height);
                cairo_stroke(cairo);
              }
              for (size_t i=adjustRowAndFinish.startRow; i<value->size(); ++i)
                {
                  if (adjustRowAndFinish(i,y)) break;
                  cairo_move_to(cairo,x,y);
                  auto v=value->atHCIndex(i);
                  if (!std::isnan(v))
                    {
                      pango.setMarkup(str(v));
                      pango.show();
                    }                       
                  y+=rowHeight;
                }
            }
          else
            {
              format=value->hypercube().xvectors[1].dimension.units;
              for (size_t i=0; i<dims[1]; ++i)
                {
                  colWidth=0;
                  y=y0;
                  cairo_move_to(cairo,x,y);
                  pango.setText(trimWS(str(value->hypercube().xvectors[1][i],format)));
                  pango.show();
                  { // draw vertical grid line
                    cairo::CairoSave cs(cairo);
                    cairo_set_source_rgba(cairo,0,0,0,0.5);
                    cairo_move_to(cairo,x-2.5,y0);
                    cairo_line_to(cairo,x-2.5,0.5*m_height);
                    cairo_stroke(cairo);
                  }
                  colWidth=std::max(colWidth, 5+pango.width());
                  for (size_t j=adjustRowAndFinish.startRow; j<dims[0]; ++j)
                    {
                      if (adjustRowAndFinish(j,y)) break;
                      y+=rowHeight;
                      cairo_move_to(cairo,x,y);
                      auto v=value->atHCIndex(j+i*dims[0]+scrollOffset);
                      if (!std::isnan(v))
                        {
                          pango.setText(str(v));
                          pango.show();
                        }
                      colWidth=std::max(colWidth, pango.width());
                    }
                  x+=colWidth;
                  if (x>0.5*m_width) break;
                }
            }
          // draw grid
          {
            cairo::CairoSave cs(cairo);
            cairo_set_source_rgba(cairo,0,0,0,0.2);
            for (y=y0+0.8*rowHeight; y<0.5*m_height; y+=2*rowHeight)
              {
                cairo_rectangle(cairo,x0,y,m_width,rowHeight);
                cairo_fill(cairo);
              }
          }
                
        }
    }
  catch (...) {/* exception most likely invalid variable value */}
  cairo_reset_clip(cairo);
  cairo_rectangle(cairo,-0.5*m_width,-0.5*m_height,m_width,m_height);
  cairo_clip(cairo);
}

