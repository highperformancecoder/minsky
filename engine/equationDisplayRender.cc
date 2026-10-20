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
#include "cairoItems.h"
#include "dataOp.h"
#include "equations.h"
#include "latexMarkup.h"
#include "selection.h"
#include "lasso.h"
#include "userFunction.h"
#include "mansouraCairo.h"
#include "minsky_epilogue.h"
using namespace ecolab;
using mansoura::IMansoura;
using mansoura::PreserveContext;

namespace MathDAG
{
  namespace
  {

    enum class Anchor {n,e,w,s,ne,nw,se,sw};

    // move pen to the anchor point of \a object
    template <class T>
    void moveToAnchor(const IMansoura& cairo, const T& object, Anchor anchor)
    {
      switch (anchor)
        {
        case Anchor::n:
          cairo.relMoveTo(-0.5*object.width(), 0); break;
        case Anchor::s:
          cairo.relMoveTo(-0.5*object.width(), -object.height()); break;
        case Anchor::e:
          cairo.relMoveTo(object.width(), -0.5*object.height()); break;
        case Anchor::w:
          cairo.relMoveTo(0, -0.5*object.height()); break;
        case Anchor::ne:
          cairo.relMoveTo(-object.width(), 0); break;
        case Anchor::nw:
          break;
        case Anchor::se:
          cairo.relMoveTo(-object.width(), -object.height()); break;
        case Anchor::sw:
          cairo.relMoveTo(0, -object.height()); break;
        }
    }     

    /// text laid out with pango markup, drawn at the current point of a context
    class Text
    {
      std::unique_ptr<mansoura::ICacheRender> r;
    public:
      Text(const IMansoura& ctx): r(ctx.cachedRender(mansoura::TextProperties())) {}
      void setMarkup(const string& m) {r->setMarkup(m);}
      void setFontSize(double sz) {r->setFontSize(sz);}
      double getFontSize() const {return r->fontSize();}
      void show() {r->show();}
      double width() const {return r->extents().width;}
      double height() const {return r->extents().height;}
    };

    // renders latex string to \a cairo, anchored at \a anchor
    // move current point to RHS of text, and returns the height
    double print(const IMansoura& cairo, const string& text, Anchor anchor)
    {
      Text pango(cairo);
      {
        const PreserveContext cs(cairo);
        pango.setMarkup(text);
        moveToAnchor(cairo, pango, anchor);
        pango.show();
      }
      cairo.relMoveTo(pango.width(), 0);
      return pango.height();
    }

    /// a context that records what is drawn to it, for measuring purposes
    struct RecordingSurface
    {
      std::unique_ptr<IMansoura> ctx=mansoura::makeRecordingContext();
      RecordingSurface() {ctx->moveTo(0,0);}
      operator const IMansoura&() const {return *ctx;}
      double left() const {return mansoura::recordedInkExtents(*ctx).left;}
      double top() const {return mansoura::recordedInkExtents(*ctx).top;}
      double width() const {return mansoura::recordedInkExtents(*ctx).width;}
      double height() const {return mansoura::recordedInkExtents(*ctx).height;}
    };

    // renders \a x a function taking a Surface, with big enough brackets around them
    template <class X>
    double parenthesise(const IMansoura& s, X x, const string& left="(", const string& right=")")
    {
      double xx,yy;
      s.getCurrentPoint(xx,yy);
 
      RecordingSurface r;
      x(r);
      Text pango(s);
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      pango.setMarkup(left);
      s.relMoveTo(0,-(r.height()-oldFs));
      pango.show();
      s.relMoveTo(0,(r.height()-oldFs));
      s.relMoveTo(pango.width(),0);
      x(s);

      xx+=pango.width()+r.width();
      pango.setMarkup(right);
      s.moveTo(xx,yy-r.height()+oldFs);
      pango.show();
      s.moveTo(xx+pango.width(),yy);
      return r.height();
    }

    // render list of arguments separated by \a op, or \a empty if arglist is empty
    void naryRender(const IMansoura& dest, const vector<WeakNodePtr>& arglist, int BODMASlevel, const char* op, const char *empty)
    {
      if (arglist.empty())
        print(dest,empty,Anchor::nw);
      else
        for (size_t j=0; j<arglist.size(); j++)
          {
            if (arglist[j]->BODMASlevel()>BODMASlevel)
              parenthesise(dest, [&](const IMansoura& dest) {arglist[j]->render(dest);});
            else
              arglist[j]->render(dest);
            if (j!=arglist.size()-1) 
              print(dest,op,Anchor::nw);
          }
    }

    void variableRender(const IMansoura& surf, const VariableDAG& v)
    {
      print(surf, latexToPango(v.name)+" = ", Anchor::nw);
      if (v.rhs) 
        v.rhs->render(surf);
      else
        print(surf, latexToPango(latexInit(v.init)), Anchor::nw);
    }

  }

  void SystemOfEquations::renderEquations(const IMansoura& dest, double height) const
  {
    double x, y; // starting position of current line
    dest.getCurrentPoint(x,y);
    const double origin=-y;
    Text den(dest);
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
        dest.moveTo(x, y-line.top());
        variableRender(dest,*i);
        y+=line.height()+4;
        dest.moveTo(x, y);
        if (y>height) return;
       }

    for (const VariableDAG* i: integrationVariables)
      {
        if (eqnNo++ < baseEqn) continue;
        // initial conditions
        dest.moveTo(x, y); // needed to define a current point on the equations tab. for ticket 1256
        y+=print(dest, latexToPango(mathrm(i->name))+"(0) = "+
                 latexToPango(latexInit(i->init)),Anchor::nw);   
        
        // differential equation
        Text num(dest);
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

        dest.moveTo(x, eqY-num.height());
        num.show();
        dest.moveTo(x, eqY);
        const double solidusLength = max(num.width(),den.width());
        dest.relLineTo(solidusLength, 0);
        dest.stroke();
        dest.moveTo(x+solidusLength, eqY);
        // display RHS here
        if (input && input->rhs)
          {
            print(dest," = ", Anchor::w);
            input->rhs->render(dest);
          }
        else
          print(dest," = 0", Anchor::w);
        dest.moveTo(x+0.5*(num.width()-den.width()), eqY);
        den.show();
        dest.moveTo(x, y+=lineSpacing);// move to next line

        if (y>height) return;
      }
  } 

  void ConstantDAG::render(const IMansoura& surf) const
  {
    print(surf, latexToPango(value),Anchor::nw);
  }

  void VariableDAG::render(const IMansoura& surf) const
  {
    print(surf, latexToPango(mathrm(name)), Anchor::nw);
  }

  void LockDAG::render(const IMansoura& surf) const
  {
    print(surf, "locked", Anchor::nw);
  }


  template <>
  void OperationDAG<OperationType::constant>::render(const IMansoura& surf) const 
  {
    assert(!"constant deprecated");
  }

  template <>
  void OperationDAG<OperationType::add>::render(const IMansoura& surf) const
  {
    for (size_t i=0; i<arguments.size(); ++i)
      {
        naryRender(surf, arguments[i], BODMASlevel()," + ","0");
        if (i!=arguments.size()-1) print(surf," + ",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::subtract>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty())
      naryRender(surf, arguments[0], BODMASlevel(), " + ","0");
    if (arguments.size()>1)
      {
        print(surf," - ",Anchor::nw);
        if (arguments[1].size()>1 || 
            (!arguments[1].empty() && BODMASlevel() == arguments[1][0]->BODMASlevel()))
          parenthesise(surf, [&](const IMansoura& s){naryRender(s,arguments[1], BODMASlevel(), " + ","0");});
        else
          naryRender(surf, arguments[1], BODMASlevel(), " + ","0");
      }
  }

  template <>
  void OperationDAG<OperationType::multiply>::render(const IMansoura& surf) const 
  {
    for (size_t i=0; i<arguments.size(); ++i)
      {
        naryRender(surf,arguments[i], BODMASlevel(), " × ","1");
        if (i!=arguments.size()-1) print(surf," × ",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::divide>::render(const IMansoura& surf) const 
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
            surf.getCurrentPoint(x,y);

            const double solidusLength = std::max(num.width(),den.width());

            surf.moveTo(x+0.5*(solidusLength-num.width()), y-num.height());
            naryRender(surf, arguments[0], BODMASlevel()," × ","1");
            surf.moveTo(x+0.5*(solidusLength-den.width()), y+0.5*num.height()+5);
            naryRender(surf, arguments[1], BODMASlevel()," × ","1");
           
            surf.moveTo(x, y+0.5*num.height()+5);
            surf.relLineTo(solidusLength, 0);
            {
              const PreserveContext cs(surf);
              surf.setLineWidth(1);
              surf.stroke();
            }
            surf.moveTo(x+solidusLength,y);
          }
        else // denominator =1, so just render the numerator
          naryRender(surf, arguments[0], BODMASlevel()," × ","1");
      }
    else // denominator =1, so just render the numerator
      naryRender(surf, arguments[0], BODMASlevel()," × ","1");
  }

  template <>
  void OperationDAG<OperationType::log>::render(const IMansoura& surf) const 
  {
    if (arguments.empty() || arguments[0].empty() || !arguments[0][0])
      print(surf,"<i>0</i>",Anchor::nw);
    else
      {
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          {
            const double h=print(surf,"log",Anchor::nw);
            surf.relMoveTo(0, 0.5*h);
            arguments[1][0]->render(surf);
            surf.relMoveTo(0, -0.5*h);
          }
        else
          print(surf,"ln",Anchor::nw);
        arguments[0][0]->render(surf);
      }
  }
  
  template <>
  void OperationDAG<OperationType::userFunction>::render(const IMansoura& surf) const 
  {
    print(surf,"<i>"+dynamic_cast<UserFunction&>(*state).description()+"</i>",Anchor::nw);
    if (arguments.empty() || arguments[0].empty() || !arguments[0][0])
      print(surf,"(0,0)",Anchor::nw);
    else
      parenthesise(surf, [&](const IMansoura& surf){
        arguments[0][0]->render(surf);
        print(surf,",",Anchor::nw);
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          arguments[1][0]->render(surf);
        else
          print(surf,"0",Anchor::nw);
        });
  }

  template <>
  void OperationDAG<OperationType::pow>::render(const IMansoura& surf) const 
  {
    if (arguments.empty() || arguments[0].empty() || !arguments[0][0])
      print(surf,"<i>0</i>",Anchor::nw);
    else
      {
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          {
            if (arguments[0][0]->BODMASlevel()>BODMASlevel())
              parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
            else
              arguments[0][0]->render(surf);

            RecordingSurface base;
            arguments[0][0]->render(base);
            surf.relMoveTo(0, -0.5*base.height());
            arguments[1][0]->render(surf);
            surf.relMoveTo(0, 0.5*base.height());
          }
        else
          print(surf,"<i>1</i>",Anchor::nw);
      }
   
  }
  

  template <>
  void OperationDAG<OperationType::time>::render(const IMansoura& surf) const 
  {print(surf,"<i>t</i>",Anchor::nw);}  
  template <>
  void OperationDAG<OperationType::euler>::render(const IMansoura& surf) const 
  {print(surf,"<i>e</i>",Anchor::nw);}
  template <>
  void OperationDAG<OperationType::pi>::render(const IMansoura& surf) const 
  {print(surf,"<i>π</i>",Anchor::nw);}    
  template <>
  void OperationDAG<OperationType::zero>::render(const IMansoura& surf) const 
  {print(surf,"<i>0</i>",Anchor::nw);}
  template <>
  void OperationDAG<OperationType::one>::render(const IMansoura& surf) const 
  {print(surf,"<i>1</i>",Anchor::nw);}    
  template <>
  void OperationDAG<OperationType::inf>::render(const IMansoura& surf) const 
  {print(surf,"<i>∞</i>",Anchor::nw);}   
  template <>
  void OperationDAG<OperationType::percent>::render(const IMansoura& s) const
  {
      double xx,yy;
      s.getCurrentPoint(xx,yy);
 
      RecordingSurface r;
      arguments[0][0]->render(r);

      Text pango(s);
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());     
      s.relMoveTo(0,-(r.height()-oldFs));
      pango.show();
      s.relMoveTo(0,(r.height()-oldFs));
      s.relMoveTo(pango.width(),0);
      arguments[0][0]->render(s);
      xx+=pango.width()+r.width();
      pango.setMarkup("%");
      s.moveTo(xx,yy-r.height()+oldFs);
      pango.show();
      s.moveTo(xx+pango.width(),yy);
  }     
  template <>
  void OperationDAG<OperationType::copy>::render(const IMansoura& surf) const
  {print(surf,"=",Anchor::nw);} 
  template <>
  void OperationDAG<OperationType::integrate>::render(const IMansoura& surf) const
  {throw error("should not be rendering integration operations");}
    
//    if (IntOp* i=dynamic_cast<IntOp*>(state.get()))
//      if (VariablePtr v=i->intVar)
//        print(surf, latexToPango(mathrm(v->name())),Anchor::nw);
//  }
  template <>
  void OperationDAG<OperationType::differentiate>::render(const IMansoura& surf) const 
  {throw error("should not be rendering differentiation operations");}
  template <>
  void OperationDAG<OperationType::sqrt>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        RecordingSurface arg;
        arguments[0][0]->render(arg);

        // display a scaled surd.
        Text pango(surf);
        {
          const PreserveContext cs(surf);
          const double oldFs=pango.getFontSize();
          pango.setFontSize(arg.height());
          pango.setMarkup("√");
          surf.relMoveTo(0,oldFs-arg.height());
          pango.show();
        }
        surf.relMoveTo(pango.width(),0);
        // draw an overbar
        double x,y;
        surf.getCurrentPoint(x,y);
        {
          const PreserveContext cs(surf);
          surf.relLineTo(arg.width(),0);
          surf.setLineWidth(1);
          surf.stroke();
        }
        
        surf.moveTo(x,y-arg.top()+2);
        arguments[0][0]->render(surf);
      }
  } 
  template <>
  void OperationDAG<OperationType::data>::render(const IMansoura& surf) const 
  {
    if (auto d=dynamic_cast<const minsky::DataOp*>(state.get()))
      print(surf,latexToPango(mathrm(d->description())),Anchor::nw);
    else
      print(surf,latexToPango(mathrm("\\uplus")),Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::ravel>::render(const IMansoura& surf) const 
  {
    if (auto d=dynamic_cast<const minsky::DataOp*>(state.get()))
      print(surf,latexToPango(mathrm(d->description())),Anchor::nw);
    else
      print(surf,latexToPango(mathrm("\\uplus")),Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::exp>::render(const IMansoura& surf) const 
  {
    print(surf,"exp",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::ln>::render(const IMansoura& surf) const 
  {
    print(surf,"ln",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::sin>::render(const IMansoura& surf) const
  {
    print(surf,"sin",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::cos>::render(const IMansoura& surf) const
  {
    print(surf,"cos",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::tan>::render(const IMansoura& surf) const
  {
    print(surf,"tan",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::asin>::render(const IMansoura& surf) const
  {
    print(surf,"sin<sup>-1</sup>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::acos>::render(const IMansoura& surf) const
  {
    print(surf,"cos<small><sup>-1</sup></small>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::atan>::render(const IMansoura& surf) const
  {
    print(surf,"tan<small><sup>-1</sup></small>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::sinh>::render(const IMansoura& surf) const
  {
    print(surf,"sinh",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::cosh>::render(const IMansoura& surf) const
  {
    print(surf,"cosh",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  template <>
  void OperationDAG<OperationType::tanh>::render(const IMansoura& surf) const
  {
    print(surf,"tanh",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }

  template <>
  void OperationDAG<OperationType::abs>::render(const IMansoura& s) const
  {
      double xx,yy;
      s.getCurrentPoint(xx,yy);
 
      RecordingSurface r;
      arguments[0][0]->render(r);

      Text pango(s);
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      pango.setMarkup("|");
      s.relMoveTo(0,-(r.height()-oldFs));
      pango.show();
      s.relMoveTo(0,(r.height()-oldFs));
      s.relMoveTo(pango.width(),0);
      arguments[0][0]->render(s);
      xx+=pango.width()+r.width();
      pango.setMarkup("|");
      s.moveTo(xx,yy-r.height()+oldFs);
      pango.show();
      s.moveTo(xx+pango.width(),yy);
  }

  template <>
  void OperationDAG<OperationType::floor>::render(const IMansoura& s) const
  {
      double xx,yy;
      s.getCurrentPoint(xx,yy);
 
      RecordingSurface r;
      arguments[0][0]->render(r);

      Text pango(s);
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      pango.setMarkup("⌊");
      s.relMoveTo(0,-(r.height()-oldFs));
      pango.show();
      s.relMoveTo(0,(r.height()-oldFs));
      s.relMoveTo(pango.width(),0);
      arguments[0][0]->render(s);
      xx+=pango.width()+r.width();
      pango.setMarkup("⌋");
      s.moveTo(xx,yy-r.height()+oldFs);
      pango.show();
      s.moveTo(xx+pango.width(),yy);
  }

  template <>
  void OperationDAG<OperationType::frac>::render(const IMansoura& surf) const
  {
    print(surf,"frac",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  
  template <>
  void OperationDAG<OperationType::Gamma>::render(const IMansoura& surf) const
  {
    print(surf,"Γ",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});}
  }
  
  template <>
  void OperationDAG<OperationType::polygamma>::render(const IMansoura& surf) const 
  {
      print(surf,"ψ",Anchor::nw);
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])	  
      {
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          {  
            RecordingSurface base;
            arguments[1][0]->render(base);
            surf.relMoveTo(0, -0.5*base.height());
            parenthesise(surf, [&](const IMansoura& surf){arguments[1][0]->render(surf);});   			  
            arguments[0][0]->render(base);
            surf.relMoveTo(0, 0.5*base.height());                        
            parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});   			  
          }
        else
          parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
      }
  }      
  
  template <>
  void OperationDAG<OperationType::fact>::render(const IMansoura& s) const
  {
      double xx,yy;
      s.getCurrentPoint(xx,yy);
 
      RecordingSurface r;
      arguments[0][0]->render(r);

      Text pango(s);
      const double oldFs=pango.getFontSize();
      pango.setFontSize(r.height());
      s.relMoveTo(0,-(r.height()-oldFs));
      pango.show();
      s.relMoveTo(0,(r.height()-oldFs));
      s.relMoveTo(pango.width(),0);
      arguments[0][0]->render(s);
      xx+=pango.width()+r.width();
      pango.setMarkup("!");
      s.moveTo(xx,yy-r.height()+oldFs);
      pango.show();
      s.moveTo(xx+pango.width(),yy);
  }   

  template <>
  void OperationDAG<OperationType::sum>::render(const IMansoura& surf) const
  {
    print(surf,"∑<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        const double h=parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        surf.relMoveTo(0, 0.5*h);
        print(surf,"<sub>i</sub>",Anchor::nw);
        surf.relMoveTo(0, -0.5*h);
      }
  }

  template <>
  void OperationDAG<OperationType::product>::render(const IMansoura& surf) const
  {
    print(surf,"∏<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::infimum>::render(const IMansoura& surf) const
  {
    print(surf,"min<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::supremum>::render(const IMansoura& surf) const
  {
    print(surf,"min<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::infIndex>::render(const IMansoura& surf) const
  {
    print(surf,"indexOf",Anchor::nw);
    parenthesise(surf, [&](const IMansoura& surf){OperationDAG<OperationType::infimum>().render(surf);});
  }

  template <>
  void OperationDAG<OperationType::supIndex>::render(const IMansoura& surf) const
  {
    print(surf,"indexOf",Anchor::nw);
    parenthesise(surf, [&](const IMansoura& surf){OperationDAG<OperationType::supremum>().render(surf);});
  }

  template <>
  void OperationDAG<OperationType::any>::render(const IMansoura& surf) const
  {
    print(surf,"Θ(max<sub>i</sub>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>i</sub>&gt;0.5)",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::all>::render(const IMansoura& surf) const
  {
    print(surf,"∏<sub>i</sub>Θ(",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>i</sub>&gt;0.5)",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::runningSum>::render(const IMansoura& surf) const
  {
    print(surf,"∑<sub>j=0</sub><sup>i</sup>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>j</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::runningAv>::render(const IMansoura& surf) const
  {
    print(surf,"(1/i)∑<sub>j=0</sub><sup>i</sup>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>j</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::runningProduct>::render(const IMansoura& surf) const
  {
    print(surf,"∏<sub>j=0</sub><sup>i</sup>",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>j</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::difference>::render(const IMansoura& surf) const
  {
    print(surf,"Δ⁻ᵢ",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>i</sub>",Anchor::nw);
      }
  }
  
  template <>
  void OperationDAG<OperationType::differencePlus>::render(const IMansoura& surf) const
  {
    print(surf,"Δ⁺ᵢ",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
        print(surf,"<sub>i</sub>",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::index>::render(const IMansoura& surf) const
  {
    print(surf,"index",Anchor::nw);
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
      }
  }

  template <>
  void OperationDAG<OperationType::lt>::render(const IMansoura& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf,"θ",Anchor::nw);
              parenthesise(surf, [&](const IMansoura& surf){
                  arguments[1][0]->render(surf);
                  print(surf," - ",Anchor::nw);
                  if (arguments[0][0]->BODMASlevel()>1)
                    parenthesise(surf, [&](const IMansoura& surf){
                        arguments[0][0]->render(surf);
                      });
                  else
                    arguments[0][0]->render(surf);
                });
            }
          else
            {
              print(surf,"θ",Anchor::nw);
              parenthesise(surf, [&](const IMansoura& surf){arguments[1][0]->render(surf);});
            }
        else
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf,"θ",Anchor::nw);
              parenthesise(surf, [&](const IMansoura& surf){
                  print(surf," - ",Anchor::nw);
                  if (arguments[0][0]->BODMASlevel()>1)
                    parenthesise(surf, [&](const IMansoura& surf){
                        arguments[0][0]->render(surf);
                      });
                  else
                    arguments[0][0]->render(surf);
                });
            }
          else
            print(surf,"0",Anchor::nw);
      }
    else
      print(surf,"0",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::eq>::render(const IMansoura& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[1].empty() && arguments[1][0])
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf,"δ",Anchor::nw);
              parenthesise(surf, [&](const IMansoura& surf){
                  arguments[0][0]->render(surf);
                  print(surf," - ",Anchor::nw);
                  arguments[1][0]->render(surf);
                });              
            }
          else
            {
              print(surf,"δ",Anchor::nw);
              parenthesise(surf, [&](const IMansoura& surf){arguments[1][0]->render(surf);});
            }
        else
          if (!arguments[0].empty() && arguments[0][0])
            {
              print(surf,"δ",Anchor::nw);
              parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
            }
          else
            print(surf,"1",Anchor::nw);
      }
    else
      print(surf,"1",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::le>::render(const IMansoura& surf) const
  {
    if ((!arguments.empty() && !arguments[0].empty() && arguments[0][0]) ||
        (arguments.size()>1 && !arguments[1].empty() && arguments[1][0]))
      {
        OperationDAG<OperationType::lt> lt; lt.arguments=arguments;
        OperationDAG<OperationType::eq> eq; eq.arguments=arguments;
        lt.render(surf);
        print(surf,"+",Anchor::nw);
        eq.render(surf);
      }
    else
      print(surf,"1",Anchor::nw);
  }


  template <>
  void OperationDAG<OperationType::min>::render(const IMansoura& surf) const
  {
    if (!arguments.empty())
      {
        print(surf,"min",Anchor::nw);
        parenthesise(surf, [&](const IMansoura& surf){
            naryRender(surf,arguments[0],BODMASlevel(),",","∞");
            if (!arguments[1].empty())
              {
                print(surf,",",Anchor::nw);
                naryRender(surf,arguments[1],BODMASlevel(),",","∞");
              }
          });
      }
    else
      print(surf,"∞",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::max>::render(const IMansoura& surf) const
  {
    if (!arguments.empty())
      {
        print(surf,"max",Anchor::nw);
        parenthesise(surf, [&](const IMansoura& surf){
            naryRender(surf,arguments[0],BODMASlevel(),",","-∞");
            if (!arguments[1].empty())
              {
                print(surf,",",Anchor::nw);
                naryRender(surf,arguments[1],BODMASlevel(),",","-∞");
              }
          });
      }
    else
      print(surf,"-∞",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::and_>::render(const IMansoura& surf) const
  {
    if (!arguments.empty())
      {
        naryRender(surf,arguments[0],BODMASlevel(),"∧","1");
        if (!arguments[1].empty())
          {
            print(surf,"∧",Anchor::nw);
            naryRender(surf,arguments[0],BODMASlevel(),"∧","1");
          }
      }
    else
      print(surf,"1",Anchor::nw);
  }
  

  template <>
  void OperationDAG<OperationType::or_>::render(const IMansoura& surf) const
  {
    if (!arguments.empty())
      {
        naryRender(surf,arguments[0],BODMASlevel(),"∨","0");
        if (!arguments[1].empty())
          {
            print(surf,"∨",Anchor::nw);
            naryRender(surf,arguments[0],BODMASlevel(),"∨","0");
          }
      }
    else
      print(surf,"0",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::not_>::render(const IMansoura& surf) const
  {
    if (!arguments.empty())
      {
        if (!arguments[0].empty() && arguments[0][0])
          {
            print(surf,"¬",Anchor::nw);
            parenthesise(surf, [&](const IMansoura& surf){
                arguments[0][0]->render(surf);
              });
          }
        else
          print(surf,"0",Anchor::nw);
      }
  }

  template <>
  void OperationDAG<OperationType::covariance>::render(const IMansoura& surf) const 
  {
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&	  
          arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          print(surf,"cov",Anchor::nw);
          parenthesise(surf, [&](const IMansoura& surf){
            arguments[0][0]->render(surf);
            print(surf,",",Anchor::nw);
            arguments[1][0]->render(surf);
          });
        }
      else
        print(surf,"0",Anchor::nw);
  }
  
  template <>
  void OperationDAG<OperationType::correlation>::render(const IMansoura& surf) const 
  {
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&	  
          arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          print(surf,"ρ",Anchor::nw);
          parenthesise(surf, [&](const IMansoura& surf){
            arguments[0][0]->render(surf);
            print(surf,",",Anchor::nw);
            arguments[1][0]->render(surf);
          });
        }
      else
        print(surf,"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::linearRegression>::render(const IMansoura& surf) const 
  {
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&	  
          arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          print(surf,"linReg",Anchor::nw);
          parenthesise(surf, [&](const IMansoura& surf){
            arguments[0][0]->render(surf);
            print(surf,",",Anchor::nw);
            arguments[1][0]->render(surf);
          });
        }
      else
        print(surf,"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::bulkLinearRegression>::render(const IMansoura& surf) const 
  {
      if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&	  
          arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          print(surf,"bLinReg",Anchor::nw);
          parenthesise(surf, [&](const IMansoura& surf){
            arguments[0][0]->render(surf);
            print(surf,",",Anchor::nw);
            arguments[1][0]->render(surf);
          });
        }
      else
        print(surf,"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::size>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf,"dim",Anchor::nw);
        parenthesise(surf, [&](const IMansoura& surf){
          arguments[0][0]->render(surf);
          print(surf,",i",Anchor::nw); // TODO - can we extract the actual argument here?
        });
      }
    else
      print(surf,"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::shape>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf,"shape",Anchor::nw);
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
      }
    else
      print(surf,"0",Anchor::nw);
  }

  template <>
  void OperationDAG<OperationType::mean>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);}, "&lt;","&gt;");
      }
    else
      print(surf,"0",Anchor::nw);
  }
  
  template <>
  void OperationDAG<OperationType::median>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf,"median",Anchor::nw);
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
      }
    else
      print(surf,"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::stdDev>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf,"σ",Anchor::nw);
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
      }
    else
      print(surf,"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::moment>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        parenthesise(surf, [&](const IMansoura& surf){
          print(surf,"Δ",Anchor::nw);
          parenthesise(surf,[&](const IMansoura& surf){arguments[0][0]->render(surf);});
          
          RecordingSurface base;
          arguments[0][0]->render(base);
          surf.relMoveTo(0, -0.5*base.height());
          print(surf,"k",Anchor::nw); // TODO can we extract the actual exponent here?
          surf.relMoveTo(0, 0.5*base.height());
        },"&lt;","&gt;");
      }
    else
      print(surf,"0",Anchor::nw);
  }      

  template <>
  void OperationDAG<OperationType::histogram>::render(const IMansoura& surf) const 
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        print(surf,"histogram",Anchor::nw);
        parenthesise(surf, [&](const IMansoura& surf){arguments[0][0]->render(surf);});
      }
    else
      print(surf,"0",Anchor::nw);
  }      

    
  
  template <>
  void OperationDAG<OperationType::innerProduct>::render(const IMansoura& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    parenthesise(surf, [&](const IMansoura& surf){
                         arguments[0][0]->render(surf);
                       });
    print(surf,"·",Anchor::nw);
    parenthesise(surf, [&](const IMansoura& surf){
                         arguments[1][0]->render(surf);
                       });
  }

  template <>
  void OperationDAG<OperationType::outerProduct>::render(const IMansoura& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    parenthesise(surf, [&](const IMansoura& surf){
                         arguments[0][0]->render(surf);
                       });
    print(surf,"⊗",Anchor::nw);
    parenthesise(surf, [&](const IMansoura& surf){
                         arguments[1][0]->render(surf);
                       });
  }

  template <>
  void OperationDAG<OperationType::gather>::render(const IMansoura& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    parenthesise(surf, [&](const IMansoura& surf){
                         arguments[0][0]->render(surf);});
    parenthesise(surf, [&](const IMansoura& surf){
                         arguments[1][0]->render(surf);},"[","]");
  }

  template <>
  void OperationDAG<OperationType::meld>::render(const IMansoura& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    print(surf,"meld",Anchor::nw);
    parenthesise(surf, [&](const IMansoura& surf){
      for (size_t i=0; i<arguments.size(); ++i)
        {
          naryRender(surf, arguments[i], BODMASlevel(),"","");
          if (i!=arguments.size()-1) print(surf,",",Anchor::nw);
        }
    });
  }

  template <>
  void OperationDAG<OperationType::merge>::render(const IMansoura& surf) const
  {
    if (arguments.size()<2 ||  arguments[0].empty() || arguments[1].empty())
      throw error("incorrectly wired");
    print(surf,"merge",Anchor::nw);
    parenthesise(surf, [&](const IMansoura& surf){
      for (size_t i=0; i<arguments.size(); ++i)
        {
          naryRender(surf, arguments[i], BODMASlevel(),"","");
          if (i!=arguments.size()-1) print(surf,",",Anchor::nw);
        }
    });
  }

  template <>
  void OperationDAG<OperationType::slice>::render(const IMansoura& surf) const
  {
    if (arguments.empty() ||  arguments[0].empty())
      throw error("incorrectly wired");
    print(surf,"slice",Anchor::nw);
    parenthesise(surf, [&](const IMansoura& surf){
      arguments[0][0]->render(surf);
      print(surf,",",Anchor::nw);
      string slice="0";
      if (state)
        if (auto o=state->operationCast())
          slice=to_string(o->arg);
      print(surf,slice,Anchor::nw);
    });
  }



}
