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
#include "equations.h"
#include "latexMarkup.h"
#include <pango.h>
#include <ecolab_epilogue.h>
using namespace ecolab;
using ecolab::cairo::Surface;

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
      cairo_save(cairo);
      Pango pango(cairo);
      //      pango.setFontSize(10);
      pango.setMarkup(text);
      moveToAnchor(cairo, pango, anchor);
      pango.show();
      cairo_restore(cairo);
      cairo_rel_move_to(cairo, pango.width(), 0);
      return pango.height();
    }

    // renders \a source (which is a recording surface) at current point
    // note: doesn't work with nested recording surfaces, ie the destination surface cannot be a recording surface
    void displaySurface(cairo_t* cairo, const Surface& source, Anchor anchor)
    {
      cairo_save(cairo);
      double x,y;
      moveToAnchor(cairo, source, anchor);
      cairo_get_current_point(cairo, &x, &y);
      cairo_rectangle(cairo, x,y,source.width(), source.height());
      cairo_set_source_rgb(cairo,0,1,0);
      cairo_stroke_preserve(cairo);
      cairo_set_source_surface(cairo, source.surface(), x-source.left(),y-source.top());
      cairo_fill(cairo);
      cairo_restore(cairo);
    }

    struct RecordingSurface: public Surface
    {
      RecordingSurface(): Surface(cairo_recording_surface_create
                                  (CAIRO_CONTENT_COLOR, nullptr)) 
      {cairo_move_to(cairo(),0,0);}
    };

    // renders \a x a function taking a Surface, with big enough brackets around them
    template <class X>
    double parenthesise(Surface& s, X x)
    {
      double xx,yy;
      cairo_get_current_point(s.cairo(),&xx,&yy);
 
      RecordingSurface r;
      x(r);
      Pango pango(s.cairo());
      double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      pango.setMarkup("(");
      cairo_rel_move_to(s.cairo(),0,-(r.height()-oldFs));
      pango.show();
      cairo_rel_move_to(s.cairo(),0,(r.height()-oldFs));
      cairo_rel_move_to(s.cairo(),pango.width(),0);
      x(s);
      xx+=pango.width()+r.width();
      pango.setMarkup(")");
      cairo_move_to(s.cairo(),xx,yy-r.height()+oldFs);
      //      cairo_rel_move_to(s.cairo(),0,-(r.height()-oldFs));
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
        print(surf.cairo(), latexToPango(MathDAG::latex(v.init)), Anchor::nw);
    }

  }

  void SystemOfEquations::renderEquations(Surface& dest) const
  {
    double x, y; // starting position of current line
    cairo_get_current_point(dest.cairo(),&x,&y);
    Pango den(dest.cairo());
    den.setMarkup("dt");

    for (const VariableDAG* i: variables)
      {
        if (dynamic_cast<const IntegralInputVariableDAG*>(i)) continue;
        if (i->type==VariableType::constant) continue;
        RecordingSurface line;
        variableRender(line,*i);
        cairo_move_to(dest.cairo(), x, y-line.top());
        variableRender(dest,*i);
        y+=line.height()+4;
        cairo_move_to(dest.cairo(), x, y);
       }

    for (const VariableDAG* i: integrationVariables)
      {
        // initial conditions
        y+=print(dest.cairo(), latexToPango(mathrm(i->name))+"(0) = "+
                 latexToPango(MathDAG::latex(i->init)),Anchor::nw);
        
        // differential equation
        Pango num(dest.cairo());
        num.setMarkup("d"+latexToPango(mathrm(i->name)));
        double lineSpacing=num.height()+den.height()+2;

        VariableDAGPtr input=expressionCache.getIntegralInput
          (VariableManager::valueId(i->scope,i->name));
        if (input && input->rhs)
          { // adjust linespacing to allow enough height for RHS
          RecordingSurface rhs;
          input->rhs->render(rhs);
          lineSpacing = max(rhs.height(), lineSpacing);
        }

        // vertical location of the = sign
        double eqY=y+max(num.height(), 0.5*lineSpacing);

        cairo_move_to(dest.cairo(), x, eqY-num.height());
        num.show();
        cairo_move_to(dest.cairo(), x, eqY);
        double solidusLength = max(num.width(),den.width());
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
        

      }
  } 

  void ConstantDAG::render(ecolab::cairo::Surface& surf) const
  {
    print(surf.cairo(), latexToPango(MathDAG::latex(value)),Anchor::nw);
  }

  void VariableDAG::render(ecolab::cairo::Surface& surf) const
  {
    print(surf.cairo(), latexToPango(mathrm(name)), Anchor::nw);
  }


  template <>
  void OperationDAG<OperationType::constant>::render(Surface& surf) const 
  {
    print(surf.cairo(), latexToPango(MathDAG::latex(init)),Anchor::nw);
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
    if (arguments.size()>0)
      naryRender(num, arguments[0], BODMASlevel()," × ","1");

    if (arguments.size()>1)
      {
        naryRender(den, arguments[1], BODMASlevel()," × ","1");
        if (!arguments[1].empty())
          {
            double x, y; // starting position of current line
            cairo_get_current_point(surf.cairo(),&x,&y);

            double solidusLength = std::max(num.width(),den.width());

            cairo_move_to(surf.cairo(), x+0.5*(solidusLength-num.width()), y-num.height());
            naryRender(surf, arguments[0], BODMASlevel()," × ","1");
            cairo_move_to(surf.cairo(), x+0.5*(solidusLength-den.width()), y+0.5*num.height()+5);
            naryRender(surf, arguments[1], BODMASlevel()," × ","1");
           
            cairo_move_to(surf.cairo(), x, y+0.5*num.height()+5);
            cairo_rel_line_to(surf.cairo(), solidusLength, 0);
            cairo_save(surf.cairo());
            cairo_set_line_width(surf.cairo(),1);
            cairo_stroke(surf.cairo());
            cairo_restore(surf.cairo());
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
            double h=print(surf.cairo(),"log",Anchor::nw);
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
  void OperationDAG<OperationType::copy>::render(Surface& surf) const
  {print(surf.cairo(),"=",Anchor::nw);}
  template <>
  void OperationDAG<OperationType::integrate>::render(Surface& surf) const
  {
    if (IntOp* i=dynamic_cast<IntOp*>(state.get()))
      if (VariablePtr v=i->getIntVar())
        print(surf.cairo(), latexToPango(mathrm(v->name())),Anchor::nw);
  }
  template <>
  void OperationDAG<OperationType::differentiate>::render(Surface& surf) const 
  {throw error("should not be rendering differentiation operations");}
  template <>
  void OperationDAG<OperationType::sqrt>::render(Surface& surf) const 
  {
    //print(surf.cairo(),"√",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        RecordingSurface arg;
        arguments[0][0]->render(arg);

        // display a scaled surd.
        cairo_save(surf.cairo());
        Pango pango(surf.cairo());
        double oldFs=pango.getFontSize();
        pango.setFontSize(arg.height());
        pango.setMarkup("√");
        cairo_rel_move_to(surf.cairo(),0,oldFs-arg.height());
        pango.show();
        cairo_restore(surf.cairo());
        cairo_rel_move_to(surf.cairo(),pango.width(),0);
        // draw an overbar
        double x,y;
        cairo_get_current_point(surf.cairo(),&x,&y);
        cairo_save(surf.cairo());
        cairo_rel_line_to(surf.cairo(),arg.width(),0);
        cairo_set_line_width(surf.cairo(),1);
        cairo_stroke(surf.cairo());
        cairo_restore(surf.cairo());
        
        cairo_move_to(surf.cairo(),x,y-arg.top()+2);
        //displaySurface(surf.cairo(),arg,Anchor::nw);
        arguments[0][0]->render(surf);
      }
  } 
  template <>
  void OperationDAG<OperationType::data>::render(Surface& surf) const 
  {
    if (auto d=dynamic_cast<const minsky::NamedOp*>(state.get()))
      print(surf.cairo(),latexToPango(mathrm(d->description)),Anchor::nw);
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
      double oldFs=pango.getFontSize();
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
      //      cairo_rel_move_to(s.cairo(),0,-(r.height()-oldFs));
      pango.show();
      cairo_move_to(s.cairo(),xx+pango.width(),yy);
  }

  template <>
  void OperationDAG<OperationType::le>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[1][0]->render(surf);
                  print(surf.cairo()," - ",Anchor::nw);
                  arguments[0][0]->render(surf);
                });
              print(surf.cairo(),"+δ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[1][0]->render(surf);
                  print(surf.cairo()," - ",Anchor::nw);
                  arguments[0][0]->render(surf);
                });              
            }
          else
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){arguments[1][0]->render(surf);});
              print(surf.cairo(),"+δ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){arguments[1][0]->render(surf);});
            }
        else
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  print(surf.cairo()," - ",Anchor::nw);
                  arguments[0][0]->render(surf);
                });
              print(surf.cairo(),"+δ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){arguments[0][0]->render(surf);});
            }
          else
            print(surf.cairo(),"1",Anchor::nw);
      }
    else
      print(surf.cairo(),"1",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::lt>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[1][0]->render(surf);
                  print(surf.cairo()," - ",Anchor::nw);
                  arguments[0][0]->render(surf);
                });
            }
          else
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){arguments[1][0]->render(surf);});
            }
        else
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  print(surf.cairo()," - ",Anchor::nw);
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
  void OperationDAG<OperationType::min>::render(Surface& surf) const
  {
    print(surf.cairo(),"min",Anchor::nw);
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            parenthesise(surf, [&](Surface& surf){
                arguments[0][0]->render(surf);
                print(surf.cairo(),",",Anchor::nw);
                arguments[1][0]->render(surf);
              });
          else
            parenthesise(surf, [&](Surface& surf){
                arguments[1][0]->render(surf);
                print(surf.cairo(),",0",Anchor::nw);
              });
        else
          if (!arguments[0].empty() && arguments[0][0])
            parenthesise(surf, [&](Surface& surf){
                arguments[0][0]->render(surf);
                print(surf.cairo(),",0",Anchor::nw);
              });
        else
          print(surf.cairo(),"(0,0)",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::max>::render(Surface& surf) const
  {
    print(surf.cairo(),"max",Anchor::nw);
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            parenthesise(surf, [&](Surface& surf){
                arguments[0][0]->render(surf);
                print(surf.cairo(),",",Anchor::nw);
                arguments[1][0]->render(surf);
              });
          else
            parenthesise(surf, [&](Surface& surf){
                arguments[1][0]->render(surf);
                print(surf.cairo(),",0",Anchor::nw);
              });
        else
          if (!arguments[0].empty() && arguments[0][0])
            parenthesise(surf, [&](Surface& surf){
                arguments[0][0]->render(surf);
                print(surf.cairo(),",0",Anchor::nw);
              });
        else
          print(surf.cairo(),"(0,0)",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::and_>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[0][0]->render(surf);
                  print(surf.cairo()," - 0.5",Anchor::nw);
                });
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[1][0]->render(surf);
                  print(surf.cairo()," - 0.5",Anchor::nw);
                });
            }
          else
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[0][0]->render(surf);
                  print(surf.cairo()," - 0.5",Anchor::nw);
                });
            }
        else if (!arguments[0].empty() && arguments[0][0])
          {
            print(surf.cairo(),"Θ",Anchor::nw);
            parenthesise(surf, [&](Surface& surf){
                arguments[1][0]->render(surf);
                print(surf.cairo()," - 0.5",Anchor::nw);
              });
          }
       else
         print(surf.cairo(),"0",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::or_>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf.cairo(),"max",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  print(surf.cairo(),"Θ",Anchor::nw);
                  parenthesise(surf, [&](Surface& surf){
                      arguments[0][0]->render(surf);
                      print(surf.cairo()," - 0.5",Anchor::nw);
                    });
                  print(surf.cairo(),",",Anchor::nw);
                  print(surf.cairo(),"Θ",Anchor::nw);
                  parenthesise(surf, [&](Surface& surf){
                      arguments[1][0]->render(surf);
                      print(surf.cairo()," - 0.5",Anchor::nw);
                    });
                });
            }
          else
            {
              print(surf.cairo(),"Θ",Anchor::nw);
              parenthesise(surf, [&](Surface& surf){
                  arguments[0][0]->render(surf);
                  print(surf.cairo()," - 0.5",Anchor::nw);
                });
            }
        else if (!arguments[0].empty() && arguments[0][0])
          {
            print(surf.cairo(),"Θ",Anchor::nw);
            parenthesise(surf, [&](Surface& surf){
                arguments[1][0]->render(surf);
                print(surf.cairo()," - 0.5",Anchor::nw);
              });
          }
        else
          print(surf.cairo(),"0",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::not_>::render(Surface& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[0].empty() && arguments[0][0])
          {
            print(surf.cairo(),"Θ",Anchor::nw);
            parenthesise(surf, [&](Surface& surf){
                print(surf.cairo(),"0.5 - ",Anchor::nw);
                arguments[0][0]->render(surf);
              });
          }
        else
          print(surf.cairo(),"0",Anchor::nw);
      }
  }



}
