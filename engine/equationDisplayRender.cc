/*
  @copyright Steve Keen 2014
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
#include "dataOp.h"
#include "equations.h"
#include "latexMarkup.h"
#include "selection.h"
#include "lasso.h"
#include "userFunction.h"
#include <pango.h>
#include "minsky_epilogue.h"
using namespace ecolab;
using ecolab::cairo::Surface;
using ecolab::cairo::CairoSave;

namespace MathDAG
{
  namespace
  {

    enum class Anchor {n,e,w,s,ne,nw,se,sw};

    // move pen to the anchor point of \a object
    template <class T>
    void moveToAnchor(cairo_t* cairo, const T& object, Anchor anchor)
    {
      switch (anchor)
        {
        case Anchor::n:
          cairo_rel_move_to(cairo, -0.5*object.width(), 0); break;
        case Anchor::s:
          cairo_rel_move_to(cairo, -0.5*object.width(), -object.height()); break;
        case Anchor::e:
          cairo_rel_move_to(cairo, object.width(), -0.5*object.height()); break;
        case Anchor::w:
          cairo_rel_move_to(cairo, 0, -0.5*object.height()); break;
        case Anchor::ne:
          cairo_rel_move_to(cairo, -object.width(), 0); break;
        case Anchor::nw:
          break;
        case Anchor::se:
          cairo_rel_move_to(cairo, -object.width(), -object.height()); break;
        case Anchor::sw:
          cairo_rel_move_to(cairo, 0, -object.height()); break;
        }
    }     

    // renders latex string to \a cairo, anchored at \a anchor
    // move current point to RHS of text, and returns the height
    double print(cairo_t* cairo, const string& text, Anchor anchor)
    {
      Pango pango(cairo);
      {
        const CairoSave cs(cairo);
        pango.setMarkup(text);
        moveToAnchor(cairo, pango, anchor);
        pango.show();
      }
      cairo_rel_move_to(cairo, pango.width(), 0);
      return pango.height();
    }

    struct RecordingSurface: public Surface
    {
      RecordingSurface(): Surface(cairo_recording_surface_create
                                  (CAIRO_CONTENT_COLOR, nullptr)) 
      {cairo_move_to(cairo(),0,0);}
    };

    // renders \a x a function taking a Surface, with big enough brackets around them
    template <class X>
    double parenthesise(Surface& s, X x, const string& left="(", const string& right=")")
    {
      double xx,yy;
      cairo_get_current_point(s.cairo(),&xx,&yy);
 
      RecordingSurface r;
      x(r);
      Pango pango(s.cairo());
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      pango.setMarkup(left);
      cairo_rel_move_to(s.cairo(),0,-(r.height()-oldFs));
      pango.show();
      cairo_rel_move_to(s.cairo(),0,(r.height()-oldFs));
      cairo_rel_move_to(s.cairo(),pango.width(),0);
      x(s);

      xx+=pango.width()+r.width();
      pango.setMarkup(right);
      cairo_move_to(s.cairo(),xx,yy-r.height()+oldFs);
      pango.show();
      cairo_move_to(s.cairo(),xx+pango.width(),yy);
      return r.height();
    }

    // render list of arguments separated by \a op, or \a empty if arglist is empty
    void naryRender(Surface& dest, const vector<WeakNodePtr>& arglist, int BODMASlevel, const char* op, const char *empty)
    {
      if (arglist.empty())
        print(dest.cairo(),empty,Anchor::nw);
      else
        for (size_t j=0; j<arglist.size(); j++)
          {
            if (arglist[j]->BODMASlevel()>BODMASlevel)
              parenthesise(dest, [&](Surface& dest) {arglist[j]->render(dest);});
            else
              arglist[j]->render(dest);
            if (j!=arglist.size()-1) 
              print(dest.cairo(),op,Anchor::nw);
          }
    }

    void variableRender(Surface& surf, const VariableDAG& v)
    {
      print(surf.cairo(), latexToPango(v.name)+" = ", Anchor::nw);
      if (v.rhs) 
        v.rhs->render(surf);
      else
        print(surf.cairo(), latexToPango(latexInit(v.init)), Anchor::nw);
    }

  }

  void SystemOfEquations::renderEquations(Surface& dest, double height) const
  {
    double x, y; // starting position of current line
    cairo_get_current_point(dest.cairo(),&x,&y);
    const double origin=-y;
    Pango den(dest.cairo());
    den.setMarkup("dt");

    const double fontHeight=30;
    const int baseEqn=origin/fontHeight;
    int eqnNo=0;
    
    for (const VariableDAG* i: variables)
      {
        if (dynamic_cast<const IntegralInputVariableDAG*>(i)) continue;
        if (!i || i->type==VariableType::constant) continue;
        if (eqnNo++ < baseEqn) continue;
        RecordingSurface line;
        variableRender(line,*i);
        cairo_move_to(dest.cairo(), x, y-line.top());
        variableRender(dest,*i);
        y+=line.height()+4;
        cairo_move_to(dest.cairo(), x, y);
        if (y>height) return;
       }

    for (const VariableDAG* i: integrationVariables)
      {
        if (eqnNo++ < baseEqn) continue;
        // initial conditions
        cairo_move_to(dest.cairo(), x, y); // needed to define a current point on the equations tab. for ticket 1256
        y+=print(dest.cairo(), latexToPango(mathrm(i->name))+"(0) = "+
                 latexToPango(latexInit(i->init)),Anchor::nw);   
        
        // differential equation
        Pango num(dest.cairo());
        num.setMarkup("d"+latexToPango(mathrm(i->name)));
        double lineSpacing=num.height()+den.height()+2;

        const VariableDAGPtr input=expressionCache.getIntegralInput(i->valueId);
        if (input && input->rhs)
          { // adjust linespacing to allow enough height for RHS
          RecordingSurface rhs;
          input->rhs->render(rhs);
          lineSpacing = max(rhs.height(), lineSpacing);
        }

        // vertical location of the = sign
        const double eqY=y+max(num.height(), 0.5*lineSpacing);

        cairo_move_to(dest.cairo(), x, eqY-num.height());
        num.show();
        cairo_move_to(dest.cairo(), x, eqY);
        const double solidusLength = max(num.width(),den.width());
        cairo_rel_line_to(dest.cairo(), solidusLength, 0);
        cairo_stroke(dest.cairo());
        cairo_move_to(dest.cairo(), x+solidusLength, eqY);
        // display RHS here
        if (input && input->rhs)
          {
            print(dest.cairo()," = ", Anchor::w);
            input->rhs->render(dest);
          }
        else
          print(dest.cairo()," = 0", Anchor::w);
        cairo_move_to(dest.cairo(), x+0.5*(num.width()-den.width()), eqY);
        den.show();
        cairo_move_to(dest.cairo(), x, y+=lineSpacing);// move to next line

        if (y>height) return;
      }
  } 

  void ConstantDAG::render(ecolab::cairo::Surface& surf) const
  {
    print(surf.cairo(), latexToPango(value),Anchor::nw);
  }

  void VariableDAG::render(ecolab::cairo::Surface& surf) const
  {
    print(surf.cairo(), latexToPango(mathrm(name)), Anchor::nw);
  }

  void LockDAG::render(ecolab::cairo::Surface& surf) const
  {
    print(surf.cairo(), "locked", Anchor::nw);
  }


  template <>
  void OperationDAG<OperationType::constant>::render(Surface& surf) const 
  {
    assert(!"constant deprecated");
  }

  template <>
  void OperationDAG<OperationType::add>::render(Surface& surf) const
  {
    for (size_t i=0; i<arguments.size(); ++i)
      {
        naryRender(surf, arguments[i], BODMASlevel()," + ","0");
        if (i!=arguments.size()-1) print(surf.cairo()," + ",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::subtract>::render(Surface& surf) const 
  {
    if (!arguments.empty())
      naryRender(surf, arguments[0], BODMASlevel(), " + ","0");
    if (arguments.size()>1)
      {
        print(surf.cairo()," - ",Anchor::nw);
        if (arguments[1].size()>1 || 
            (!arguments[1].empty() && BODMASlevel() == arguments[1][0]->BODMASlevel()))
          parenthesise(surf, [&](Surface& s){naryRender(s,arguments[1], BODMASlevel(), " + ","0");});
        else
          naryRender(surf, arguments[1], BODMASlevel(), " + ","0");
      }
  }

  template <>
  void OperationDAG<OperationType::multiply>::render(Surface& surf) const 
  {
    for (size_t i=0; i<arguments.size(); ++i)
      {
        naryRender(surf,arguments[i], BODMASlevel(), " × ","1");
        if (i!=arguments.size()-1) print(surf.cairo()," × ",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::divide>::render(Surface& surf) const 
  {
    RecordingSurface num, den;
    if (!arguments.empty())
      naryRender(num, arguments[0], BODMASlevel()," × ","1");

    if (arguments.size()>1)
      {
        naryRender(den, arguments[1], BODMASlevel()," × ","1");
        if (!arguments[1].empty())
          {
            double x, y; // starting position of current line
            cairo_get_current_point(surf.cairo(),&x,&y);

            const double solidusLength = std::max(num.width(),den.width());

            cairo_move_to(surf.cairo(), x+0.5*(solidusLength-num.width()), y-num.height());
            naryRender(surf, arguments[0], BODMASlevel()," × ","1");
            cairo_move_to(surf.cairo(), x+0.5*(solidusLength-den.width()), y+0.5*num.height()+5);
            naryRender(surf, arguments[1], BODMASlevel()," × ","1");
           
            cairo_move_to(surf.cairo(), x, y+0.5*num.height()+5);
            cairo_rel_line_to(surf.cairo(), solidusLength, 0);
            {
              const CairoSave cs(surf.cairo());
              cairo_set_line_width(surf.cairo(),1);
              cairo_stroke(surf.cairo());
            }
            cairo_move_to(surf.cairo(),x+solidusLength,y);
          }
        else // denominator =1, so just render the numerator
          naryRender(surf, arguments[0], BODMASlevel()," × ","1");
      }
    else // denominator =1, so just render the numerator
      naryRender(surf, arguments[0], BODMASlevel()," × ","1");
  }

  template <>
  void OperationDAG<OperationType::log>::render(Surface& surf) const 
  {
    if (arguments.empty() || arguments[0].empty() || !arguments[0][0])
      print(surf.cairo(),"<i>0</i>",Anchor::nw);
    else
      {
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          {
            const double h=print(surf.cairo(),"log",Anchor::nw);
            cairo_rel_move_to(surf.cairo(), 0, 0.5*h);
            arguments[1][0]->render(surf);
            cairo_rel_move_to(surf.cairo(), 0, -0.5*h);
          }
        else
          print(surf.cairo(),"ln",Anchor::nw);
        arguments[0][0]->render(surf);
      }
  }
  
  template <>
  void OperationDAG<OperationType::userFunction>::render(Surface& surf) const 
  {
    print(surf.cairo(),"<i>"+dynamic_cast<UserFunction&>(*state).description()+"</i>",Anchor::nw);
    if (arguments.empty() || arguments[0].empty() || !arguments[0][0])
      print(surf.cairo(),"(0,0)",Anchor::nw);
    else
      parenthesise(surf, [&](Surface& surf){
        arguments[0][0]->render(surf);
        print(surf.cairo(),",",Anchor::nw);
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          arguments[1][0]->render(surf);
        else
          print(surf.cairo(),"0",Anchor::nw);
        });
  }

  template <>
  void OperationDAG<OperationType::pow>::render(Surface& surf) const 
  {
    if (arguments.empty() || arguments[0].empty() || !arguments[0][0])
      print(surf.cairo(),"<i>0</i>",Anchor::nw);
    else
      {
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          {
            if (arguments[0][0]->BODMASlevel()>BODMASlevel())
              parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
            else
              arguments[0][0]->render(surf);

            RecordingSurface base;
            arguments[0][0]->render(base);
            cairo_rel_move_to(surf.cairo(), 0, -0.5*base.height());
            arguments[1][0]->render(surf);
            cairo_rel_move_to(surf.cairo(), 0, 0.5*base.height());
          }
        else
          print(surf.cairo(),"<i>1</i>",Anchor::nw);
      }
   
  }
  

  template <>
  void OperationDAG<OperationType::time>::render(Surface& surf) const 
  {print(surf.cairo(),"<i>t</i>",Anchor::nw);}  
  template <>
  void OperationDAG<OperationType::euler>::render(Surface& surf) const 
  {print(surf.cairo(),"<i>e</i>",Anchor::nw);}
  template <>
  void OperationDAG<OperationType::pi>::render(Surface& surf) const 
  {print(surf.cairo(),"<i>π</i>",Anchor::nw);}    
  template <>
  void OperationDAG<OperationType::zero>::render(Surface& surf) const 
  {print(surf.cairo(),"<i>0</i>",Anchor::nw);}
  template <>
  void OperationDAG<OperationType::one>::render(Surface& surf) const 
  {print(surf.cairo(),"<i>1</i>",Anchor::nw);}    
  template <>
  void OperationDAG<OperationType::inf>::render(Surface& surf) const 
  {print(surf.cairo(),"<i>∞</i>",Anchor::nw);}   
  template <>
  void OperationDAG<OperationType::percent>::render(Surface& s) const
  {
      double xx,yy;
      cairo_get_current_point(s.cairo(),&xx,&yy);
 
      RecordingSurface r;
      arguments[0][0]->render(r);

      Pango pango(s.cairo());
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());     
      cairo_rel_move_to(s.cairo(),0,-(r.height()-oldFs));
      pango.show();
      cairo_rel_move_to(s.cairo(),0,(r.height()-oldFs));
      cairo_rel_move_to(s.cairo(),pango.width(),0);
      arguments[0][0]->render(s);
      xx+=pango.width()+r.width();
      pango.setMarkup("%");
      cairo_move_to(s.cairo(),xx,yy-r.height()+oldFs);
      pango.show();
      cairo_move_to(s.cairo(),xx+pango.width(),yy);
  }     
  template <>
  void OperationDAG<OperationType::copy>::render(Surface& surf) const
  {print(surf.cairo(),"=",Anchor::nw);} 
  template <>
  void OperationDAG<OperationType::integrate>::render(Surface& surf) const
  {throw error("should not be rendering integration operations");}
    
//    if (IntOp* i=dynamic_cast<IntOp*>(state.get()))
//      if (VariablePtr v=i->intVar)
//        print(surf.cairo(), latexToPango(mathrm(v->name())),Anchor::nw);
//  }
  template <>
  void OperationDAG<OperationType::differentiate>::render(Surface& surf) const 
  {throw error("should not be rendering differentiation operations");}
  template <>
  void OperationDAG<OperationType::sqrt>::render(Surface& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        RecordingSurface arg;
        arguments[0][0]->render(arg);

        // display a scaled surd.
        Pango pango(surf.cairo());
        {
          const CairoSave cs(surf.cairo());
          const double oldFs=pango.getFontSize();
          pango.setFontSize(arg.height());
          pango.setMarkup("√");
          cairo_rel_move_to(surf.cairo(),0,oldFs-arg.height());
          pango.show();
        }
        cairo_rel_move_to(surf.cairo(),pango.width(),0);
        // draw an overbar
        double x,y;
        cairo_get_current_point(surf.cairo(),&x,&y);
        {
          const CairoSave cs(surf.cairo());
          cairo_rel_line_to(surf.cairo(),arg.width(),0);
          cairo_set_line_width(surf.cairo(),1);
          cairo_stroke(surf.cairo());
        }
        
        cairo_move_to(surf.cairo(),x,y-arg.top()+2);
        arguments[0][0]->render(surf);
      }
  } 
  template <>
  void OperationDAG<OperationType::data>::render(Surface& surf) const 
  {
    if (auto d=dynamic_cast<const minsky::DataOp*>(state.get()))
      print(surf.cairo(),latexToPango(mathrm(d->description())),Anchor::nw);
    else
      print(surf.cairo(),latexToPango(mathrm("\\uplus")),Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::ravel>::render(Surface& surf) const 
  {
    if (auto d=dynamic_cast<const minsky::DataOp*>(state.get()))
      print(surf.cairo(),latexToPango(mathrm(d->description())),Anchor::nw);
    else
      print(surf.cairo(),latexToPango(mathrm("\\uplus")),Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::exp>::render(Surface& surf) const 
  {
    print(surf.cairo(),"exp",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::ln>::render(Surface& surf) const 
  {
    print(surf.cairo(),"ln",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::sin>::render(Surface& surf) const
  {
    print(surf.cairo(),"sin",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::cos>::render(Surface& surf) const
  {
    print(surf.cairo(),"cos",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::tan>::render(Surface& surf) const
  {
    print(surf.cairo(),"tan",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::asin>::render(Surface& surf) const
  {
    print(surf.cairo(),"sin<sup>-1</sup>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::acos>::render(Surface& surf) const
  {
    print(surf.cairo(),"cos<small><sup>-1</sup></small>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::atan>::render(Surface& surf) const
  {
    print(surf.cairo(),"tan<small><sup>-1</sup></small>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::sinh>::render(Surface& surf) const
  {
    print(surf.cairo(),"sinh",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::cosh>::render(Surface& surf) const
  {
    print(surf.cairo(),"cosh",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::tanh>::render(Surface& surf) const
  {
    print(surf.cairo(),"tanh",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }

  template <>
  void OperationDAG<OperationType::abs>::render(Surface& s) const
  {
      double xx,yy;
      cairo_get_current_point(s.cairo(),&xx,&yy);
 
      RecordingSurface r;
      arguments[0][0]->render(r);

      Pango pango(s.cairo());
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      pango.setMarkup("|");
      cairo_rel_move_to(s.cairo(),0,-(r.height()-oldFs));
      pango.show();
      cairo_rel_move_to(s.cairo(),0,(r.height()-oldFs));
      cairo_rel_move_to(s.cairo(),pango.width(),0);
      arguments[0][0]->render(s);
      xx+=pango.width()+r.width();
      pango.setMarkup("|");
      cairo_move_to(s.cairo(),xx,yy-r.height()+oldFs);
      pango.show();
      cairo_move_to(s.cairo(),xx+pango.width(),yy);
  }

  template <>
  void OperationDAG<OperationType::floor>::render(Surface& s) const
  {
      double xx,yy;
      cairo_get_current_point(s.cairo(),&xx,&yy);
 
      RecordingSurface r;
      arguments[0][0]->render(r);

      Pango pango(s.cairo());
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      pango.setMarkup("⌊");
      cairo_rel_move_to(s.cairo(),0,-(r.height()-oldFs));
      pango.show();
      cairo_rel_move_to(s.cairo(),0,(r.height()-oldFs));
      cairo_rel_move_to(s.cairo(),pango.width(),0);
      arguments[0][0]->render(s);
      xx+=pango.width()+r.width();
      pango.setMarkup("⌋");
      cairo_move_to(s.cairo(),xx,yy-r.height()+oldFs);
      pango.show();
      cairo_move_to(s.cairo(),xx+pango.width(),yy);
  }

  template <>
  void OperationDAG<OperationType::frac>::render(Surface& surf) const
  {
    print(surf.cairo(),"frac",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  
  template <>
  void OperationDAG<OperationType::Gamma>::render(Surface& surf) const
  {
    print(surf.cairo(),"Γ",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});}
  }
  
  template <>
  void OperationDAG<OperationType::polygamma>::render(Surface& surf) const 
  {
      print(surf.cairo(),"ψ",Anchor::nw);
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])	  
      {
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          {  
            RecordingSurface base;
            arguments[1][0]->render(base);
            cairo_rel_move_to(surf.cairo(), 0, -0.5*base.height());
            parenthesise(surf, [&](Surface& surf){arguments[1][0]->render(surf);});   			  
            arguments[0][0]->render(base);
            cairo_rel_move_to(surf.cairo(), 0, 0.5*base.height());                        
            parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});   			  
          }
        else
          parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
      }
  }      
  
  template <>
  void OperationDAG<OperationType::fact>::render(Surface& s) const
  {
      double xx,yy;
      cairo_get_current_point(s.cairo(),&xx,&yy);
 
      RecordingSurface r;
      arguments[0][0]->render(r);

      Pango pango(s.cairo());
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      cairo_rel_move_to(s.cairo(),0,-(r.height()-oldFs));
      pango.show();
      cairo_rel_move_to(s.cairo(),0,(r.height()-oldFs));
      cairo_rel_move_to(s.cairo(),pango.width(),0);
      arguments[0][0]->render(s);
      xx+=pango.width()+r.width();
      pango.setMarkup("!");
      cairo_move_to(s.cairo(),xx,yy-r.height()+oldFs);
      pango.show();
      cairo_move_to(s.cairo(),xx+pango.width(),yy);
  }   

  template <>
  void OperationDAG<OperationType::sum>::render(Surface& surf) const
  {
    print(surf.cairo(),"∑<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        const double h=parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        cairo_rel_move_to(surf.cairo(), 0, 0.5*h);
        print(surf.cairo(),"<sub>i</sub>",Anchor::nw);
        cairo_rel_move_to(surf.cairo(), 0, -0.5*h);
      }
  }

  template <>
  void OperationDAG<OperationType::product>::render(Surface& surf) const
  {
    print(surf.cairo(),"∏<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::infimum>::render(Surface& surf) const
  {
    print(surf.cairo(),"min<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::supremum>::render(Surface& surf) const
  {
    print(surf.cairo(),"min<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::infIndex>::render(Surface& surf) const
  {
    print(surf.cairo(),"indexOf",Anchor::nw);
    parenthesise(surf, [&](Surface& surf){OperationDAG<OperationType::infimum>().render(surf);});
  }

  template <>
  void OperationDAG<OperationType::supIndex>::render(Surface& surf) const
  {
    print(surf.cairo(),"indexOf",Anchor::nw);
    parenthesise(surf, [&](Surface& surf){OperationDAG<OperationType::supremum>().render(surf);});
  }

  template <>
  void OperationDAG<OperationType::any>::render(Surface& surf) const
  {
    print(surf.cairo(),"Θ(max<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>&gt;0.5)",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::all>::render(Surface& surf) const
  {
    print(surf.cairo(),"∏<sub>i</sub>Θ(",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>&gt;0.5)",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::runningSum>::render(Surface& surf) const
  {
    print(surf.cairo(),"∑<sub>j=0</sub><sup>i</sup>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::runningProduct>::render(Surface& surf) const
  {
    print(surf.cairo(),"∏<sub>j=0</sub><sup>i</sup>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::difference>::render(Surface& surf) const
  {
    print(surf.cairo(),"Δ⁻ᵢ",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>",Anchor::nw);
      }
  }
  
  template <>
  void OperationDAG<OperationType::differencePlus>::render(Surface& surf) const
  {
    print(surf.cairo(),"Δ⁺ᵢ",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
        print(surf.cairo(),"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::index>::render(Surface& surf) const
  {
    print(surf.cairo(),"index",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
      }
  }

  template <>
  void OperationDAG<OperationType::lt>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[1][0]->render(surf);
                  print(surf.cairo()," - ",Anchor::nw);
                  if (arguments[0][0]->BODMASlevel()>1)
                    parenthesise(surf, [&](Surface& surf){
                        arguments[0][0]->render(surf);
                      });
                  else
                    arguments[0][0]->render(surf);
                });
            }
          else
            {
              print(surf.cairo(),"θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){arguments[1][0]->render(surf);});
            }
        else
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  print(surf.cairo()," - ",Anchor::nw);
                  if (arguments[0][0]->BODMASlevel()>1)
                    parenthesise(surf, [&](Surface& surf){
                        arguments[0][0]->render(surf);
                      });
                  else
                    arguments[0][0]->render(surf);
                });
            }
          else
            print(surf.cairo(),"0",Anchor::nw);
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::eq>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"δ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[0][0]->render(surf);
                  print(surf.cairo()," - ",Anchor::nw);
                  arguments[1][0]->render(surf);
                });              
            }
          else
            {
              print(surf.cairo(),"δ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){arguments[1][0]->render(surf);});
            }
        else
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"δ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
            }
          else
            print(surf.cairo(),"1",Anchor::nw);
      }
    else
      print(surf.cairo(),"1",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::le>::render(Surface& surf) const
  {
    if ((!arguments.empty() && !arguments[0].empty() && arguments[0][0]) ||
        (arguments.size()>1 && !arguments[1].empty() && arguments[1][0]))
      {
        OperationDAG<OperationType::lt> lt; lt.arguments=arguments;
        OperationDAG<OperationType::eq> eq; eq.arguments=arguments;
        lt.render(surf);
        print(surf.cairo(),"+",Anchor::nw);
        eq.render(surf);
      }
    else
      print(surf.cairo(),"1",Anchor::nw);
  }


  template <>
  void OperationDAG<OperationType::min>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        print(surf.cairo(),"min",Anchor::nw);
        parenthesise(surf, [&](Surface& surf){
            naryRender(surf,arguments[0],BODMASlevel(),",","∞");
            if (!arguments[1].empty())
              {
                print(surf.cairo(),",",Anchor::nw);
                naryRender(surf,arguments[1],BODMASlevel(),",","∞");
              }
          });
      }
    else
      print(surf.cairo(),"∞",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::max>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        print(surf.cairo(),"max",Anchor::nw);
        parenthesise(surf, [&](Surface& surf){
            naryRender(surf,arguments[0],BODMASlevel(),",","-∞");
            if (!arguments[1].empty())
              {
                print(surf.cairo(),",",Anchor::nw);
                naryRender(surf,arguments[1],BODMASlevel(),",","-∞");
              }
          });
      }
    else
      print(surf.cairo(),"-∞",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::and_>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        naryRender(surf,arguments[0],BODMASlevel(),"∧","1");
        if (!arguments[1].empty())
          {
            print(surf.cairo(),"∧",Anchor::nw);
            naryRender(surf,arguments[0],BODMASlevel(),"∧","1");
          }
      }
    else
      print(surf.cairo(),"1",Anchor::nw);
  }
  

  template <>
  void OperationDAG<OperationType::or_>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        naryRender(surf,arguments[0],BODMASlevel(),"∨","0");
        if (!arguments[1].empty())
          {
            print(surf.cairo(),"∨",Anchor::nw);
            naryRender(surf,arguments[0],BODMASlevel(),"∨","0");
          }
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::not_>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[0].empty() && arguments[0][0])
          {
            print(surf.cairo(),"¬",Anchor::nw);
            parenthesise(surf, [&](Surface& surf){
                arguments[0][0]->render(surf);
              });
          }
        else
          print(surf.cairo(),"0",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::covariance>::render(Surface& surf) const 
  {
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&	  
          arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          print(surf.cairo(),"cov",Anchor::nw);
          parenthesise(surf, [&](Surface& surf){
            arguments[0][0]->render(surf);
            print(surf.cairo(),",",Anchor::nw);
            arguments[0][1]->render(surf);
          });
        }
      else
        print(surf.cairo(),"0",Anchor::nw);
  }
  
  template <>
  void OperationDAG<OperationType::correlation>::render(Surface& surf) const 
  {
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&	  
          arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          print(surf.cairo(),"ρ",Anchor::nw);
          parenthesise(surf, [&](Surface& surf){
            arguments[0][0]->render(surf);
            print(surf.cairo(),",",Anchor::nw);
            arguments[0][1]->render(surf);
          });
        }
      else
        print(surf.cairo(),"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::linearRegression>::render(Surface& surf) const 
  {
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&	  
          arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          print(surf.cairo(),"linReg",Anchor::nw);
          parenthesise(surf, [&](Surface& surf){
            arguments[0][0]->render(surf);
            print(surf.cairo(),",",Anchor::nw);
            arguments[0][1]->render(surf);
          });
        }
      else
        print(surf.cairo(),"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::size>::render(Surface& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf.cairo(),"dim",Anchor::nw);
        parenthesise(surf, [&](Surface& surf){
          arguments[0][0]->render(surf);
          print(surf.cairo(),",i",Anchor::nw); // TODO - can we extract the actual argument here?
        });
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::shape>::render(Surface& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf.cairo(),"shape",Anchor::nw);
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::mean>::render(Surface& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);}, "<",">");
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }
  
  template <>
  void OperationDAG<OperationType::median>::render(Surface& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf.cairo(),"median",Anchor::nw);
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::stdDev>::render(Surface& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf.cairo(),"σ",Anchor::nw);
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::moment>::render(Surface& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](Surface& surf){
          print(surf.cairo(),"Δ",Anchor::nw);
          parenthesise(surf,[&](Surface& surf){arguments[0][0]->render(surf);});
          
          RecordingSurface base;
          arguments[0][0]->render(base);
          cairo_rel_move_to(surf.cairo(), 0, -0.5*base.height());
          print(surf.cairo(),"k",Anchor::nw); // TODO can we extract the actual exponent here?
          cairo_rel_move_to(surf.cairo(), 0, 0.5*base.height());
        },"<",">");
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::histogram>::render(Surface& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf.cairo(),"histogram",Anchor::nw);
        parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
      }
    else
      print(surf.cairo(),"0",Anchor::nw);
  }      

    
  
  template <>
  void OperationDAG<OperationType::innerProduct>::render(Surface& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    parenthesise(surf, [&](Surface& surf){
                         arguments[0][0]->render(surf);
                       });
    print(surf.cairo(),"·",Anchor::nw);
    parenthesise(surf, [&](Surface& surf){
                         arguments[1][0]->render(surf);
                       });
  }

  template <>
  void OperationDAG<OperationType::outerProduct>::render(Surface& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    parenthesise(surf, [&](Surface& surf){
                         arguments[0][0]->render(surf);
                       });
    print(surf.cairo(),"⊗",Anchor::nw);
    parenthesise(surf, [&](Surface& surf){
                         arguments[1][0]->render(surf);
                       });
  }

  template <>
  void OperationDAG<OperationType::gather>::render(Surface& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    parenthesise(surf, [&](Surface& surf){
                         arguments[0][0]->render(surf);});
    parenthesise(surf, [&](Surface& surf){
                         arguments[1][0]->render(surf);},"[","]");
  }

  template <>
  void OperationDAG<OperationType::meld>::render(Surface& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    print(surf.cairo(),"meld",Anchor::nw);
    parenthesise(surf, [&](Surface& surf){
      for (size_t i=0; i<arguments.size(); ++i)
        {
          naryRender(surf, arguments[i], BODMASlevel(),"","");
          if (i!=arguments.size()-1) print(surf.cairo(),",",Anchor::nw);
        }
    });
  }

  template <>
  void OperationDAG<OperationType::merge>::render(Surface& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    print(surf.cairo(),"merge",Anchor::nw);
    parenthesise(surf, [&](Surface& surf){
      for (size_t i=0; i<arguments.size(); ++i)
        {
          naryRender(surf, arguments[i], BODMASlevel(),"","");
          if (i!=arguments.size()-1) print(surf.cairo(),",",Anchor::nw);
        }
    });
  }

  template <>
  void OperationDAG<OperationType::slice>::render(Surface& surf) const
  {
    if (arguments.empty() ||  arguments[0].empty())
      throw error("incorrectly wired");
    print(surf.cairo(),"slice",Anchor::nw);
    parenthesise(surf, [&](Surface& surf){
      arguments[0][0]->render(surf);
      print(surf.cairo(),",",Anchor::nw);
      string slice="0";
      if (state)
        if (auto o=state->operationCast())
          slice=to_string(o->arg);
      print(surf.cairo(),slice,Anchor::nw);
    });
  }



}
