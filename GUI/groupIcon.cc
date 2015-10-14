/*
  @copyright Steve Keen 2012
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
#include "groupIcon.h"
#include "minsky.h"
#include "init.h"
#include "cairoItems.h"
#include "str.h"
#include <cairo_base.h>
#include <plot.h>
#include <ecolab_epilogue.h>

using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;
using namespace minsky;

#include <boost/regex.hpp>
#include <boost/geometry/geometry.hpp>
using namespace boost::geometry;

namespace minsky
{
  using std::set;
  namespace
  {
    // allow room for port icons on vertical sides 
    const int portOffset=10;

    // for debugging
    void printPortLoc(int i)
    {
      const Port& p=cminsky().ports[i];
      printf("port[%d] @ (%f,%f)\n",i,p.x(),p.y());
    }

    // returns true if x==y, within a certain relative constant
    bool near_eq(double x, double y)
    {
      const double epsilon=1e-10;
      return (abs(x)<epsilon && abs(y)<epsilon) || 
                     (abs(x)>=epsilon && abs((x-y)/x)<epsilon);
    }

    // return true if \a rect overlaps partly, or wholly \a poly
    bool includesPart(const Rectangle& rect, const Polygon& poly)
    {
      return covered_by(return_envelope<Rectangle>(poly), rect) || 
        intersects(rect, poly);
    }

    bool boundariesIntersect(const Rectangle& rect, const Polygon& poly)
    {
      return intersects(rect, poly) && 
        !covered_by(return_envelope<Rectangle>(poly), rect) && 
        !covered_by(rect, return_envelope<Rectangle>(poly));
    }

    /// translate a wire coordinate list by (\a dx,\a dy)
    array<float> translateWireCoords(array<float> c, float dx, float dy)
    {
      assert(c.size()%2==0);
      c[2*pcoord(c.size()/2)]+=dx;
      c[2*pcoord(c.size()/2)+1]+=dy;
      return c;
    }

    bool itemExists(const string& item, int id)
    {
      tclcmd cmd;
      cmd<<"info commands .wiring.canvas\n";
      if (cmd.result.empty())
        return false;
      cmd << ".wiring.canvas find withtag "|item|id|"\n";
      return !cmd.result.empty();
    }
  
    struct GroupIconItem: public CairoImage
    {
      static Tk_ConfigSpec configSpecs[];
      
      GroupIconItem(): displayContents(false) {}
      bool displayContents;
      int id;
      void draw()
      {
        if (cairoSurface && id>=0)
          {
            const GroupIcon& g=cminsky().groupItems[id];
            g.draw(cairoSurface->cairo());
            if (displayContents!=g.displayContents())
              {
                createOrDeleteContentItems(g.displayContents());
                displayContents=g.displayContents();
              }
          }
      }

      // if \a display is true, ensure content items are visible on
      // canvas, if false, then delete content itemse
      void createOrDeleteContentItems(bool display)
      {
        DisableEventProcessing e;
        tclcmd cmd;
        GroupIcon& g=minsky::minsky().groupItems[id];
        g.updatePortLocation();
        if (display)
          {
            set<int> coupledVars;
            vector<int>::const_iterator i=g.operations().begin(); 
            for (; i!=g.operations().end(); ++i)
              if (!itemExists("op",*i))
                {
                  OperationBase& op=*minsky::minsky().operations[*i];
                  op.visible=true;
                  op.zoom(g.x(), g.y(), g.localZoom()/op.zoomFactor);
                  cmd<<"drawOperation"<<*i<<"\n";
                  cmd|".wiring.canvas addtag groupitems"|id|" withtag op"|*i|"\n";
                  if (op.type()==OperationType::constant)
                    {
                      cmd<<"drawSlider"<<*i<<op.x()<<op.y()<<"\n";
                      cmd|".wiring.canvas addtag groupitems"|id|
                        " withtag slider"|*i|"\n";
                    }
                  // if this is a coupled integral, save the integral
                  // variable for later use
                  if (IntOp* ivar=dynamic_cast<IntOp*>(&op))
                    if (ivar->coupled())
                      coupledVars.insert(ivar->intVarID());
                }

            set<int> edgeVars=g.edgeSet();
            for (i=g.variables().begin(); i!=g.variables().end(); ++i)
              if (edgeVars.count(*i)==0 && coupledVars.count(*i)==0 &&
                  !itemExists("var",*i))
                {
                  VariableBase& v=*minsky::minsky().variables[*i];
                  v.visible=true;
                  v.zoom(g.x(), g.y(), g.localZoom()/v.zoomFactor);
                  cmd<<"newVar"<<*i<<"\n";
                  cmd|".wiring.canvas addtag groupitems"|id|" withtag var"|*i|"\n";
                }
            for (i=g.groups().begin(); i!=g.groups().end(); ++i)
              if (!itemExists("groupItem",*i))
                {
                  GroupIcon& gg=minsky::minsky().groupItems[*i];
                  gg.visible=true;
                  gg.zoom(g.x(), g.y(), g.localZoom()/gg.zoomFactor);
                  cmd<<"newGroupItem"<<*i<<"\n";
                  cmd|".wiring.canvas addtag groupitems"|id|" withtag groupItem"|*i|"\n";
                }
            for (i=g.wires().begin(); i!=g.wires().end(); ++i)
              if (!itemExists("wire",*i))
                {
                  Wire& w=minsky::minsky().wires[*i];
                  w.visible=true;
                  cmd << "adjustWire"<<w.to<<"\n";
                  cmd|".wiring.canvas addtag groupitems"|id|" withtag wire"|*i|"\n";
                }
          }
        else
          {
            cmd|".wiring.canvas delete groupitems"|id|"\n";
            vector<int>::const_iterator i=g.operations().begin(); 
            for (; i!=g.operations().end(); ++i)
              {
                OperationBase& op=*minsky::minsky().operations[*i];
                op.m_x/=op.zoomFactor;
                op.m_y/=op.zoomFactor;
                op.zoomFactor=1;
                op.visible=false;
                cmd|".wiring.canvas delete slider"|*i|"\n";
              }
            set<int> eVars=g.edgeSet();
            for (i=g.variables().begin(); i!=g.variables().end(); ++i)
              if (!eVars.count(*i))
                {
                  VariableBase& v=*minsky::minsky().variables[*i];
                  v.m_x/=v.zoomFactor;
                  v.m_y/=v.zoomFactor;
                  v.zoomFactor=1;
                  v.visible=false;              
                }
            for (i=g.wires().begin(); i!=g.wires().end(); ++i)
              {
                minsky::minsky().wires[*i].visible=false;
              }
            for (i=g.groups().begin(); i!=g.groups().end(); ++i)
              {
                GroupIcon& g=minsky::minsky().groupItems[*i];
                g.zoom(g.x(), g.y(), 1/g.zoomFactor);
                g.visible=false;
              }
          }
        // lower this group icon below everybody else, and it parents, and so on
        for (int toLower=id; toLower!=-1; )
          {
            cmd|".wiring.canvas lower groupItem"|toLower|";";
            toLower=minsky::minsky().groupItems[toLower].parent();
          }          
        cmd|"\n";
      }
    };

    // we need some extra fields to handle the additional options
    struct TkXGLItem: public ImageItem
    {
      int id; // C++ object identifier
    };

    static Tk_CustomOption tagsOption = {
      (Tk_OptionParseProc *) Tk_CanvasTagsParseProc,
      Tk_CanvasTagsPrintProc, (ClientData) NULL
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    Tk_ConfigSpec GroupIconItem::configSpecs[] =
    {
      {TK_CONFIG_INT, "-id", NULL, NULL,
       NULL, Tk_Offset(TkXGLItem, id), 0},
      {TK_CONFIG_CUSTOM, "-tags", NULL, NULL,
       NULL, 0, TK_CONFIG_NULL_OK, &tagsOption},
      {TK_CONFIG_END}
    };
#pragma GCC diagnostic pop

    int creatProc(Tcl_Interp *interp, Tk_Canvas canvas, 
                  Tk_Item *itemPtr, int objc,Tcl_Obj *CONST objv[])
    {
      TkXGLItem* tkXGLItem=(TkXGLItem*)(itemPtr);
      tkXGLItem->id=-1;
      int r=createImage<GroupIconItem>(interp,canvas,itemPtr,objc,objv);
      if (r==TCL_OK)
        {
          GroupIconItem* xglItem=(GroupIconItem*)(tkXGLItem->cairoItem);
          if (xglItem) 
            {
              xglItem->id = tkXGLItem->id;
              TkImageCode::ComputeImageBbox(canvas, tkXGLItem);
            }
        }
      return r;
    }

  // overrride cairoItem's configureProc to process the extra config options
  int configureProc(Tcl_Interp *interp,Tk_Canvas canvas,Tk_Item *itemPtr,
                    int objc,Tcl_Obj *CONST objv[],int flags)
  {
    return TkImageCode::configureCairoItem
      (interp,canvas,itemPtr,objc,objv,flags, GroupIconItem::configSpecs);
  }

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
    // register GodleyItem with Tk for use in canvases.
    int registerItem()
    {
      static Tk_ItemType iconType = cairoItemType();
      iconType.name="group";
      iconType.itemSize=sizeof(TkXGLItem);
      iconType.createProc=creatProc;
      iconType.configProc=configureProc;
      iconType.configSpecs=GroupIconItem::configSpecs;
      Tk_CreateItemType(&iconType);
      return 0;
    }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    Minsky* l_minsky=NULL;
  }

  static int dum=(initVec().push_back(registerItem), 0);

  void GroupIcon::deleteContents()
  {
    //remove any displayed items
    tclcmd()|"if [winfo exists .wiring.canvas] {"
      ".wiring.canvas delete groupitems"|id()|"}\n"; 
    // delete all contained objects. Make a copy because erase edits the source vector
    vector<int> ids=m_operations;
    for (int id: ids)
      minsky().operations.erase(id);
    ids=m_variables;
    for (int id: ids)
      minsky().variables.erase(id);
#ifdef NDEBUG
    // wires should be removed as a result of removing above objects
    for (int i:m_wires)
      assert(cminsky().wires.count(i)==0);
#endif
    m_operations.clear();
    m_variables.clear();
    m_wires.clear();
  }


  std::vector<int> GroupIcon::ports() const
  {
    vector<int> r;
    set<int>::const_iterator i=inVariables.begin();
    for (int i: inVariables)
      r.push_back(minsky().variables[i]->inPort());
    for (int i: outVariables)
      r.push_back(minsky().variables[i]->outPort());
    return r;
  }

  float GroupIcon::x() const
  {
    if (parent()==-1)
      return m_x;
    else
      return m_x+minsky().groupItems[parent()].x();
  }

  float GroupIcon::y() const
  {
    if (parent()==-1)
      return m_y;
    else
      return m_y+minsky().groupItems[parent()].y();
  }

  float GroupIcon::rotFactor() const
  {
    float rotFactor;
    float ac=abs(cos(rotation*M_PI/180));
    static const float invSqrt2=1/sqrt(2);
    if (ac>=invSqrt2) 
      rotFactor=1.15/ac; //use |1/cos(angle)| as rotation factor
    else
      rotFactor=1.15/sqrt(1-ac*ac);//use |1/sin(angle)| as rotation factor
    return rotFactor;
  }

  vector<VariablePtr> GroupIcon::edgeVariables() const
  {
    vector<VariablePtr> r;

    // nb various methods below assume that the input variables come
    // before the output ones.
    for (set<int>::iterator i=inVariables.begin(); i!=inVariables.end(); ++i)
      {
        r.push_back(minsky().variables[*i]);
        assert(r.back()->type()!=VariableType::undefined);
      }
    for (set<int>::iterator i=outVariables.begin(); i!=outVariables.end(); ++i)
      {
        r.push_back(minsky().variables[*i]);
        assert(r.back()->type()!=VariableType::undefined);
      }
    return r;
  }

  void GroupIcon::addEdgeVariable
  (set<int>& varVector, set<string>& varNames, 
   vector<Wire>& additionalWires, int port)
  {
    if (!cminsky().ports.count(port))
      return;
    const VariablePtr& v=cminsky().variables.getVariableFromPort(port);
    if (v->type()!=VariableBase::undefined)
      {
        //int id=cminsky().variables.getIDFromVariable(v);
        if (v->group==id()) {//variable already in group, ensure it is an edgevar
          if (varNames.insert(v->name()).second)
            varVector.insert(v.id());
          return;
        } 
        if ((v->group<0 || 
             // don't insert another group's edge variables
             cminsky().groupItems[v->group].edgeSet().count(v.id())==0) &&
            varNames.insert(v->name()).second) // only insert if name is unique
          {
            varVector.insert(v.id());
            VariableManager::iterator vi=minsky().variables.find(v.id());
            if (parent()>-1) minsky().groupItems[parent()].removeVariable(*vi);
            addVariable(*vi);
            return;
          }
      }
    // create a new edge variable
    static int nextVarName=0;
    string newName=str(id())+":"+str(nextVarName);
    while (minsky().variables.values.count(newName)>0)
      {
        ++nextVarName;
        newName=str(id())+":"+str(nextVarName);
      }
    assert(varNames.count(newName)==0);
    varNames.insert(newName);
    int newId=minsky().variables.newVariable(newName, VariableType::flow);
    varVector.insert(newId);
    VariablePtr newV=minsky().variables[newId];
    m_variables.push_back(newId);
    createdVars.push_back(newId);
    newV->group=id();
    newV->visible=false;
    const Port& p=cminsky().ports[port];
    // insert variable into wire
    array<int> wires=minsky().wiresAttachedToPort(port);
    for (size_t w=0; w<wires.size(); ++w)
      {
        Wire& wire=minsky().wires[wires[w]];
        if (p.input())
          minsky().wires[wires[w]].to=newV->inPort();
        else
          minsky().wires[wires[w]].from=newV->outPort();
      }
    if (p.input())
      additionalWires.push_back(Wire(newV->outPort(),port));
    else
      additionalWires.push_back(Wire(port,newV->inPort()));
    additionalWires.back().visible=false;
    additionalWires.back().group=id();
  }

  void GroupIcon::eliminateIOduplicates()
  {
    set<int> vars;
    inVarNames.clear();
    for (set<int>::iterator i=inVariables.begin(); i!=inVariables.end(); ++i)
      if (inVarNames.insert(minsky::minsky().variables[*i]->name()).second)
        vars.insert(*i);
    inVariables.swap(vars);

    outVarNames.clear();
    vars.clear();
    for (set<int>::iterator i=outVariables.begin(); i!=outVariables.end(); ++i)
      if (outVarNames.insert(minsky::minsky().variables[*i]->name()).second)
        vars.insert(*i);
    outVariables.swap(vars);

    assert(inVariables.size()==inVarNames.size());
    assert(outVariables.size()==outVarNames.size());
  }

  void GroupIcon::ungroup()
  {
    DisableEventProcessing e;
    if (parent()>-1)
      {
        GroupIcons::iterator parentGroup=minsky().groupItems.find(parent());
        GroupIcons::iterator thisGroup=minsky().groupItems.find(id());
        while (!m_operations.empty())
          {
            Operations::iterator o=minsky().operations.find(m_operations.front());
            if (o!=minsky().operations.end())
              {
                parentGroup->addOperation(*o);
                parentGroup->addAnyWires((*o)->ports());
                removeOperation(*o);
                removeAnyWires((*o)->ports());
              }
          }
      
        set<int> removedVars(createdVars.begin(), createdVars.end());
        while (!m_variables.empty())
          {
            VariableManager::iterator v=minsky().variables.find(m_variables.front());
            if (v!=minsky().variables.end())
              {
                if (removedVars.count(m_variables.front())==0)
                  {
                    parentGroup->addVariable(*v);
                    parentGroup->addAnyWires((*v)->ports());
                  }
                removeVariable(*v);
                removeAnyWires((*v)->ports());
              }
          }
      
        parentGroup->removeGroup(*thisGroup);
      }
    else
      { //ungrouping into global scope

        // we must apply visibility to the wires first, as the call to
        // toggleCoupled in the operations section potentially deletes a
        // wire.
        for (size_t i=0; i<m_wires.size(); ++i)
          {
            Wire& w=minsky().wires[m_wires[i]];
            w.visible=true;
            array<float> coords=w.coords();
            w.group=-1;
            w.coords(coords);
          }

        set<int> intVars;
        for (size_t i=0; i<m_operations.size(); ++i)
          {
            OperationBase& o=*minsky().operations[m_operations[i]];
            float x=o.x(), y=o.y();
            o.group=-1;
            o.moveTo(x, y);
            o.visible=true;
            o.setZoom(minsky().zoomFactor());
            if (IntOp* i=dynamic_cast<IntOp*>(&o))
              {
                if (!i->coupled()) i->toggleCoupled();
                intVars.insert(i->intVarID()); // save for use in next loop
              }
          }
        VariableManager& vars=minsky().variables;
        for (size_t i=0; i<m_variables.size(); ++i)
          {
            VariableBase& v=*vars[m_variables[i]];
            // restore variable coordinates to their absolute values
            float x=v.x(), y=v.y();
            v.group=-1; 
            v.moveTo(x, y);
            v.setZoom(minsky().zoomFactor());
            if (intVars.count(m_variables[i])==0) 
              v.visible=true;
          }

        for (size_t i=0; i<m_groups.size(); ++i)
          {
            GroupIcon& g=minsky().groupItems[m_groups[i]];
            float x=g.x(), y=g.y();
            g.m_parent=parent();
            g.moveTo(x,y);
            g.visible = true;
            g.setZoom(minsky().zoomFactor());
          }
      }


    assert(minsky().groupItems.uniqueGroupMembership());

    m_operations.clear();
    m_variables.clear();
    m_wires.clear();
    m_groups.clear();
    inVariables.clear();
    outVariables.clear();
    inVarNames.clear();
    outVarNames.clear();

    // remove variables created during the createGroup() operation
    for (vector<int>::const_iterator i=createdVars.begin(); 
         i!=createdVars.end(); ++i)
      {
        // before deleting the variables, we need to join the wires
        VariablePtr v=minsky().variables[*i];
        array<int> incomingWires=minsky().wiresAttachedToPort(v->inPort());
        assert(incomingWires.size()<=1);
        if (incomingWires.size()>0)
          {
            array<int> outgoingWires=minsky().wiresAttachedToPort(v->outPort());
            Wire& inWire=minsky().wires[incomingWires[0]];
            for (size_t j=0; j<outgoingWires.size(); ++j)
              {
                Wire& w=minsky().wires[outgoingWires[j]];
                w.from=inWire.from;
                // remove coordinates of connecting variable
                w.coords(inWire.coords()[pcoord(inWire.coords().size()-2)]<<
                         w.coords()[pcoord(w.coords().size()-2)+2]);
                w.visible=inWire.visible;
                w.group=inWire.group;
                assert(w.coords().size()>=4);
              }
          }
        minsky().variables.erase(*i);
      }

    assert(minsky().groupItems.uniqueGroupMembership());

  }

  void GroupIcon::moveTo(float x1, float y1)
  {
    float dx=x1-x(), dy=y1-y();
    m_x+=dx; m_y+=dy;
  
    /*
      TODO, callbacks to TCL interpreter is way to slow - figure out how
      notify Tk of coordinate changes
    */
    if (displayContents())
      {
        DisableEventProcessing e;
        tclcmd cmd;
        cmd<<"llength [info commands .wiring.canvas]\n";
        if (cmd.result=="0") return; // not in GUI environment
        for (vector<int>::const_iterator i=m_operations.begin(); 
             i!=m_operations.end(); ++i)
          {
            OperationBase& op=*minsky().operations[*i];
            (cmd|".wiring.canvas coords op"|*i)<<op.x()<<op.y()<<"\n";
          }
        for (vector<int>::const_iterator i=m_variables.begin(); 
             i!=m_variables.end(); ++i)
          {
            VariableBase& v=*minsky().variables[*i];
            (cmd|".wiring.canvas coords var"|*i)<<v.x()<<v.y()<<"\n";        
          }
        for (vector<int>::const_iterator i=m_wires.begin(); 
             i!=m_wires.end(); ++i)
          (cmd|".wiring.canvas coords wire"|*i)<<minsky().wires[*i].coords()<<"\n";       for (vector<int>::const_iterator i=m_groups.begin(); 
                                                                                               i!=m_groups.end(); ++i)
                                                                                            {
                                                                                              GroupIcon& g=minsky().groupItems[*i];
                                                                                              // force movement of canvas items contained within
                                                                                              g.moveTo(g.x(),g.y());
                                                                                              (cmd|".wiring.canvas coords groupItem"|*i)<<g.x()<<g.y()<<"\n";        
                                                                                            }
 
      }      
  }

  array<float> GroupIcon::updatePortLocation() const
  {
    array<float> r;
    int toIdx=1, fromIdx=1; // port counters
    minsky::Rotate rotate(rotation, x(), y());

    float leftMargin, rightMargin;
    margins(leftMargin,rightMargin);

    float escale=edgeScale();

    vector<VariablePtr> eVars=edgeVariables();
    for (vector<VariablePtr>::iterator v=eVars.begin(); v!=eVars.end(); ++v)
      {
        // calculate the unrotated offset from icon position
        float dx, dy; 
        RenderVariable rv(**v, NULL);
        if (v-eVars.begin()<int(inVariables.size()))
          {
            dx= zoomFactor*(-0.5*width+leftMargin)-escale*(rv.width()+2);
            dy= escale*2*rv.height() * (toIdx>>1) * (toIdx&1? -1:1);
            (r<<=-0.5*width)<<=dy;
            toIdx++;
          }
        else
          {
            dx= zoomFactor*(0.5*width-rightMargin)+escale*rv.width();
            dy= escale*2*rv.height() * (fromIdx>>1) * (fromIdx&1? -1:1);
            (r<<=0.5*width)<<=dy;
            fromIdx++;
          }
        (*v)->rotation=rotation;
        // calculate rotated port positions
        (*v)->moveTo(rotate.x(x()+dx,y()+dy), rotate.y(x()+dx,y()+dy));
        assert((*v)->type()!=VariableType::undefined);
        rv.updatePortLocs(); // ensures ports locations are corrected for curren zoom
      }
    return r;
  }

  struct PortMap: public map<int, int>
  {
    // add mapping from all src ports to dest ports
    template <class T>
    void addPorts(const T& src, const T& dest)
    {
      assert(src.numPorts()==dest.numPorts());
      for (size_t i=0; i<src.numPorts(); ++i)
        insert(make_pair(src.ports()[i],dest.ports()[i]));
    }
  };

  void GroupIcon::copy(const GroupIcon& src)
  {
    //  a map of port correspondences
    PortMap portMap;
    map<int, int> varMap; // map of variable correspondences
    *this=src;
    m_operations.resize(src.m_operations.size());
    m_variables.clear();
    m_wires.clear();
    inVariables.clear();
    outVariables.clear();
    createdVars.clear();
    // TODO revisit this when scoped variables are introduced
    inVarNames=src.inVarNames;
    outVarNames=src.outVarNames;

    // map of integral variable names
    map<string,string> intVarMap;
    // set of "bound" integration variable - do not copy these
    set<int> integrationVars;

    // generate copies of operations
    for (size_t i=0; i<src.m_operations.size(); ++i)
      {
        m_operations[i]=minsky().copyOperation(src.m_operations[i]);
        Operations& op=minsky().operations;
        OperationBase& srcOp=*op[src.m_operations[i]];
        OperationBase& destOp=*op[m_operations[i]];
        destOp.group=id();
        portMap.addPorts(srcOp, destOp);
        // add intVarMap entry if an integral
        if (IntOp* i=dynamic_cast<IntOp*>(&srcOp))
          {
            IntOp& d=dynamic_cast<IntOp&>(destOp);
            intVarMap[i->description()] = d.description();
            integrationVars.insert(i->intVarID());
            d.getIntVar()->group=id();
            d.getIntVar()->setScope(id());
            m_variables.push_back(d.intVarID());
            if (i->coupled()!=d.coupled()) d.toggleCoupled();
            portMap.addPorts(*i->getIntVar(), *d.getIntVar());
            varMap[i->intVarID()]=d.intVarID();
          }
      }
    // generate copies of variables
    for (size_t i=0; i<src.m_variables.size(); ++i)
      if (!integrationVars.count(src.m_variables[i]))
        {
          int srcVar= src.m_variables[i];
          m_variables.push_back(minsky().copyVariable(srcVar));
          const VariablePtr& v=minsky().variables[m_variables.back()];
          v->visible = minsky().variables[src.m_variables[i]]->visible;
          v->group = id();
          if (v->type()==VariableType::integral && intVarMap.count(v->name()))
            // remap the variable name
            v->name(intVarMap[v->name()]);
          portMap.addPorts(*minsky().variables[srcVar], *v);
          varMap[srcVar]=m_variables.back();
        }
  
    // add corresponding wires
    for (size_t i=0; i<src.m_wires.size(); ++i)
      {
        Wire w=minsky().wires[src.m_wires[i]];
        // if the wire connects to an already wired variable, skip adding the wire (ticket #192)
        if (minsky().variables.getVariableIDFromPort(w.to)==-1)
          {
            w.from=portMap[w.from]; w.to=portMap[w.to];
            w.group=id();
            m_wires.push_back(static_cast<PortManager&>(minsky()).addWire(w));
          }
      }

    // add corresponding I/O variables
    for (set<int>::iterator i=src.inVariables.begin(); 
         i!=src.inVariables.end(); ++i)
      {
        minsky().variables[varMap[*i]]->setScope(id());
        inVariables.insert(varMap[*i]);
      }
    for (set<int>::iterator i=src.outVariables.begin(); 
         i!=src.outVariables.end(); ++i)
      {
        minsky().variables[varMap[*i]]->setScope(id());
        outVariables.insert(varMap[*i]);
      }
    for (vector<int>::const_iterator i=src.createdVars.begin();
         i!=src.createdVars.end(); ++i)
      createdVars.push_back(varMap[*i]);

    assert(inVariables.size()==inVarNames.size());
    assert(outVariables.size()==outVarNames.size());
  }


  float GroupIcon::contentBounds(float& x0, float& y0, float& x1, float& y1) const
  {
    float localZoom=1;
    y0=x0=numeric_limits<float>::max();
    y1=x1=-x0;
    vector<int>::const_iterator i=m_operations.begin();
    for (; i!=m_operations.end(); ++i)
      {
        const OperationPtr& op=cminsky().operations[*i];
        assert(op);
        RenderOperation ro(*op);
        localZoom=op->zoomFactor;
        x0=min(x0, op->x() - ro.width()*op->zoomFactor);
        x1=max(x1, op->x() + ro.width()*op->zoomFactor);
        y0=min(y0, op->y() - ro.height()*op->zoomFactor);
        y1=max(y1, op->y() + ro.height()*op->zoomFactor);
      }
    set<int> edgeVars=edgeSet();
    for (i=m_variables.begin(); i!=m_variables.end(); ++i)
      // exclude the edge variables from content bound calc    
      if (edgeVars.count(*i)==0) 
        {
          const VariablePtr& v=cminsky().variables[*i];
          assert(v);
          RenderVariable rv(*v);
          localZoom=v->zoomFactor;
          x0=min(x0, v->x() - rv.width()*v->zoomFactor);
          x1=max(x1, v->x() + rv.width()*v->zoomFactor);
          y0=min(y0, v->y() - rv.height()*v->zoomFactor);
          y1=max(y1, v->y() + rv.height()*v->zoomFactor);
        }

    for (i=m_groups.begin(); i!=m_groups.end(); ++i)
      {
        GroupIcon& g=minsky().groupItems[*i];
        localZoom=g.zoomFactor;
        float w=0.5f*g.width*g.zoomFactor,
          h=0.5f*g.height*g.zoomFactor;
        x0=min(x0, g.x() - w);
        x1=max(x1, g.x() + w);
        y0=min(y0, g.y() - h);
        y1=max(y1, g.y() + h);
      }

    // if there are no contents, result is not finite. In this case,
    // set the content bounds to a 10x10 sized box around the centroid of the I/O variables.

    if (x0==numeric_limits<float>::max())
      {
        float cx=0, cy=0;
        for (int i: inVariables)
          {
            cx+=cminsky().variables[i]->x();
            cy+=cminsky().variables[i]->y();
          }
        for (int i: outVariables)
          {
            cx+=cminsky().variables[i]->x();
            cy+=cminsky().variables[i]->y();
          }
        int n=inVariables.size()+outVariables.size();
        cx/=n;
        cy/=n;
        x0=cx-10;
        x1=cx+10;
        y0=cy-10;
        y1=cy+10;
      }
    else
      {
        // extend width by 2 pixels to allow for the slightly oversized variable icons
        x0-=2*this->localZoom();
        y0-=2*this->localZoom();
        x1+=2*this->localZoom();
        y1+=2*this->localZoom();
      }

    return localZoom;
  }

  void GroupIcon::moveContents(float dx, float dy)
  {
    vector<int>::const_iterator i;
    for (i=m_wires.begin(); i!=m_wires.end(); ++i)
      minsky().wires[*i].move(dx,dy);
    for (i=m_operations.begin(); i!=m_operations.end(); ++i)
      minsky().operations[*i]->move(dx,dy);
    for (i=m_variables.begin(); i!=m_variables.end(); ++i)
      minsky().variables[*i]->move(dx,dy);
    for (i=m_groups.begin(); i!=m_groups.end(); ++i)
      minsky().groupItems[*i].move(dx,dy);
  }

  void GroupIcon::drawVar
  (cairo_t* cairo, const VariablePtr& v) const
  {
    cairo_save(cairo);
    cairo_translate(cairo,v->x()-x(),v->y()-y());
    RenderVariable(*v, cairo).draw();
    cairo_restore(cairo);
  }


  void GroupIcon::drawEdgeVariables
  (cairo_t* cairo) const
  {
    set<int>::const_iterator i=inVariables.begin();
    for (; i!=inVariables.end(); ++i)
      {
        drawVar(cairo, minsky().variables[*i]);
        assert(minsky().variables[*i]->type()!=VariableType::undefined);
      }
    for (i=outVariables.begin(); i!=outVariables.end(); ++i)
      {
        drawVar(cairo, minsky().variables[*i]);
        assert(minsky().variables[*i]->type()!=VariableType::undefined);
      }
  }

  // draw notches in the I/O region to indicate docking capability
  void GroupIcon::drawIORegion(cairo_t* cairo) const
  {
    cairo_save(cairo);
    float left, right;
    margins(left,right);
    left*=zoomFactor;
    right*=zoomFactor;
    float y=0, dy=5*edgeScale();
    set<int>::const_iterator i=inVariables.begin();
    for (; i!=inVariables.end(); ++i)
      y=max(y, fabs(minsky().variables[*i]->m_y)+3*dy);
    cairo_set_source_rgba(cairo,0,1,1,0.5);
    float w=0.5*zoomFactor*width, h=0.5*zoomFactor*height;

    cairo_move_to(cairo,-w,-h);
    // create notch in input region
    cairo_line_to(cairo,-w,y-dy);
    cairo_line_to(cairo,left-w-2,y-dy);
    cairo_line_to(cairo,left-w,y);
    cairo_line_to(cairo,left-w-2,y+dy);
    cairo_line_to(cairo,-w,y+dy);
    cairo_line_to(cairo,-w,h);
    cairo_line_to(cairo,left-w,h);
    cairo_line_to(cairo,left-w,-h);
    cairo_close_path(cairo);
    cairo_fill(cairo);

    for (y=0, i=outVariables.begin(); i!=outVariables.end(); ++i)
      y=max(y, fabs(minsky().variables[*i]->m_y)+3*dy);
    cairo_move_to(cairo,w,-h);
    // create notch in output region
    cairo_line_to(cairo,w,y-dy);
    cairo_line_to(cairo,w-right,y-dy);
    cairo_line_to(cairo,w-right+2,y);
    cairo_line_to(cairo,w-right,y+dy);
    cairo_line_to(cairo,w,y+dy);
    cairo_line_to(cairo,w,h);
    cairo_line_to(cairo,w-right,h);
    cairo_line_to(cairo,w-right,-h);
    cairo_close_path(cairo);
    cairo_fill(cairo);

    cairo_restore(cairo);
  }

  void GroupIcon::margins(float& left, float& right) const
  {
    set<int>::const_iterator i=inVariables.begin();
    float scale=edgeScale()/zoomFactor;
    left=right=10*scale;
    for (; i!=inVariables.end(); ++i)
      {
        assert(minsky().variables.count(*i));
        float w= scale * (2*RenderVariable(*cminsky().variables[*i]).width()+2);
        assert(minsky().variables[*i]->type()!=VariableType::undefined);
        if (w>left) left=w;
      }
    for (i=outVariables.begin(); i!=outVariables.end(); ++i)
      {
        assert(minsky().variables.count(*i));
        float w= scale * (2*RenderVariable(*cminsky().variables[*i]).width()+2);
        assert(minsky().variables[*i]->type()!=VariableType::undefined);
        if (w>right) right=w;
      }
  }

  void GroupIcon::zoom(float xOrigin, float yOrigin,float factor) {
    computeDisplayZoom();
    if (visible)
      {
        if (m_parent==-1)
          {
            ::zoom(m_x, xOrigin, factor);
            ::zoom(m_y, yOrigin, factor);
          }
        else
          {
            m_x*=factor;
            m_y*=factor;
          }
        zoomFactor*=factor;
        for (VariablePtr& v: edgeVariables())
          v->zoomFactor=edgeScale();
        updatePortLocation(); // should force edge wire coordinates to update
      }
  }

  float GroupIcon::computeDisplayZoom()
  {
    float x0, x1, y0, y1, l, r;
    float lz=contentBounds(x0,y0,x1,y1);
    x0=min(x0,x());
    x1=max(x1,x());
    y0=min(y0,y());
    y1=max(y1,y());
    // first compute the value assuming margins are of zero width
    displayZoom = 2*max( max(x1-x(), x()-x0)/width, max(y1-y(), y()-y0)/height );

    // account for shrinking margins
    float readjust=zoomFactor/edgeScale() / (displayZoom>1? displayZoom:1);
    margins(l,r);
    l*=readjust; r*=readjust;
    displayZoom = max(displayZoom, 
                      max((x1-x())/(0.5f*width-r), (x()-x0)/(0.5f*width-l)));
  
    displayZoom*=1.1*rotFactor()/lz;

    // displayZoom should never be less than 1
    displayZoom=max(displayZoom, 1.0f);
    return displayZoom;
  }

  template <class S>
  void GroupIcon::addAnyWires(const S& ports)
  {
    array<int> wiresToCheck;
    for (size_t i=0; i<ports.size(); ++i)
      wiresToCheck <<= minsky().wiresAttachedToPort(ports[i]);
    if (wiresToCheck.size()>0)
      {
        // first build list of contained ports
        set<int> containedPorts;
        set<int> containedWires(m_wires.begin(), m_wires.end());
        for (size_t i=0; i<m_operations.size(); ++i)
          {
            const vector<int>& p=minsky().operations[m_operations[i]]->ports();
            containedPorts.insert(p.begin(), p.end());
          }
        for (size_t i=0; i<m_variables.size(); ++i)
          {
            auto& p=minsky().variables[m_variables[i]]->ports();
            containedPorts.insert(p.begin(), p.end());
          }
        vector<Wire> wiresToAdd;
        float left, bottom, right, top;
        contentBounds(left,bottom,right,top);
        for (array<int>::iterator w=wiresToCheck.begin(); 
             w!=wiresToCheck.end(); ++w)
          {
            if (containedWires.count(*w)) continue; // already there, no
            // need to add
            Wire& wire=minsky().wires[*w];
            if (containedPorts.count(wire.from) && 
                containedPorts.count(wire.to))
              {
                m_wires.push_back(*w);
                if (itemExists("wire",*w))
                  tclcmd()|".wiring.canvas dtag wire"|*w|" groupitems"|wire.group|
                    ";.wiring.canvas addtag groupitems"|id()|
                    " withtag wire"|*w|"\n";
                array<float> coords=wire.coords();
                wire.group=id();
                wire.coords(coords);
                wire.visible=displayContents();
              }
            else if (containedPorts.count(wire.from) &&
                     !containedPorts.count(wire.to))
              {
                const Port& toPort=cminsky().ports[wire.to];
                // this check prevents adding IO variable for a link to a
                // contained group
                if (toPort.x()>right || toPort.x()<left ||
                    toPort.y()>top || toPort.y()<bottom)
                  // wire leaves group, we should insert an output variable
                  addEdgeVariable
                    (outVariables, outVarNames, wiresToAdd, wire.from);
              }
            else if (containedPorts.count(wire.to) &&
                     !containedPorts.count(wire.from))
              {
                const Port& fromPort=cminsky().ports[wire.from];
                // this check prevents adding IO variable for a link to a
                // contained group
                if (fromPort.x()>right || fromPort.x()<left ||
                    fromPort.y()>top || fromPort.y()<bottom)
                  // wire leaves group, we should insert an output variable
                  addEdgeVariable(inVariables, inVarNames, wiresToAdd, wire.to);
              }
          }
        // now add the additional wires to port manager
        for (size_t i=0; i<wiresToAdd.size(); ++i)
          m_wires.push_back(minsky().addWire(wiresToAdd[i]));
      
      }
  }

  template void GroupIcon::addAnyWires(const array<int>& ports);
  template void GroupIcon::addAnyWires(const vector<int>& ports);

  template <class S>
  void GroupIcon::removeAnyWires(const S& ports)
  {
    set<int> portsToCheck(ports.begin(), ports.end());
    vector<int> newWires;
    GroupIcon* parentGroup=parent()>=0? &minsky().groupItems[parent()]: NULL;
    for (vector<int>::iterator i=m_wires.begin(); i!=m_wires.end(); ++i)
      {
        Wire& w=minsky().wires[*i];
        if (!portsToCheck.count(w.from) && !portsToCheck.count(w.to))
          newWires.push_back(*i);
        else
          {
            w.group=parent();
            w.visible=parentGroup? parentGroup->displayContents(): true;
          } 
      }
    m_wires.swap(newWires);
  }

  template void GroupIcon::removeAnyWires(const array<int>& ports);
  template void GroupIcon::removeAnyWires(const vector<int>& ports);

  void GroupIcon::addWire(int id)
  {
    const Wire& w=minsky().wires[id];
    vector<int> ports(2);
    ports[0]=w.from; 
    ports[1]=w.to;
    addAnyWires(ports);
  }

  void GroupIcon::delWire(int id)
  {
    m_wires.erase(remove(m_wires.begin(), m_wires.end(), id), m_wires.end());
  }

  void GroupIcon::addWires(const vector<int>& wires) 
  {
    m_wires.insert(m_wires.end(), wires.begin(), wires.end());
    for (int i: wires)
      {
        assert(minsky().wires.count(i));
        Wire& w=minsky().wires[i];
        array<float> c=w.coords();
        w.group=id();
        w.coords(c);
        w.visible=false;
      }
  }


  int GroupIcon::inIORegion(float x, float y) const
  {
    float left, right;
    margins(left,right);
    float dx=(x-this->x())*cos(rotation*M_PI/180)-
      (y-this->y())*sin(rotation*M_PI/180);
    float w=0.5*width*zoomFactor;
    if (w-right*zoomFactor<dx)
      return 2;
    else if (-w+left*zoomFactor>dx)
      return 1;
    else
      return 0;
  }

  
  void GroupIcon::setName(const string& name)
  {
    m_name=name;
    // update variables values with new name
    for (int v: m_variables)
      {
        const VariableBase& vv=*cminsky().variables[v];
        minsky().variables.values[vv.valueId()].name=
          vv.fqName();
      }
  }

  void GroupIcon::addVariable(const VariableManager::value_type& pv, bool checkIOregions)
  {
    float x=pv->x(), y=pv->y();
    if (pv->group!=id())
      {
        m_variables.push_back(pv.id());
        pv->group=id();
        pv->moveTo(x,y); // adjust to group relative coordinates
      }

    if (checkIOregions)
      {
        // determine if variable is to be added to the interface variable list
        pv->visible=false;
        switch (inIORegion(x,y))
          {
          case 2:
            if (outVarNames.insert(pv->name()).second)
              outVariables.insert(pv.id());
            break;
          case 1:
            if (inVarNames.insert(pv->name()).second)
              inVariables.insert(pv.id());
            break;
          case 0:
            pv->visible=displayContents();
            break;
          }
      }
    else
      pv->visible=displayContents();
    computeDisplayZoom();
    updatePortLocation();
    assert(inVariables.size()==inVarNames.size());
    assert(outVariables.size()==outVarNames.size());
  }

  void GroupIcon::removeVariable(const VariableManager::value_type& pv)
  {
    for (vector<int>::iterator i=m_variables.begin(); i!=m_variables.end(); ++i)
      if (*i==pv.id())
        {
          m_variables.erase(i);

          if (inVariables.erase(pv.id())>0)
            {
              // if removing a variable from inputs, we need to delete outgoing wires
              array<int> outWires=minsky().wiresAttachedToPort(pv->outPort());
              for (array<int>::iterator w=outWires.begin(); w!=outWires.end(); ++w)
                {
                  minsky().deleteWire(*w);
                  if (itemExists("wire",*w))
                    tclcmd() | ".wiring.canvas delete wire"|*w|"\n";
                }
            }
          if (outVariables.erase(pv.id())>0)
            {
              // if removing a variable from outputs, we need to delete incoming wires
              array<int> inWires=minsky().wiresAttachedToPort(pv->inPort());
              for (array<int>::iterator w=inWires.begin(); w!=inWires.end(); ++w)
                {
                  minsky().deleteWire(*w);
                  if (itemExists("wire",*w))
                    tclcmd() | ".wiring.canvas delete wire"|*w|"\n";
                }
            }

          inVarNames.erase(pv->name());
          outVarNames.erase(pv->name());
          //rebase coordinates
          float x=pv->x(), y=pv->y();
          pv->group=parent();
          pv->visible= parent()==-1? true: minsky().groupItems[parent()].displayContents();
          pv->moveTo(x,y); // adjust to group relative coordinates
          if (!ecolab::interpExiting && pv->visible && !itemExists("var",pv.id()))
            tclcmd() << "if [llength [info commands .wiring.canvas]] {newVar"<< pv.id() << "}\n";
          computeDisplayZoom();
          break;
        }
  }

  void GroupIcon::removeVariableById(int id)
  {
    VariableManager::iterator v=minsky().variables.find(id);
    if (v!=minsky().variables.end())
      removeVariable(*v);
  }

  void GroupIcon::addOperation(Operations::value_type& po)
  {
    if (po->group!=id())
      {
        m_operations.push_back(po.id());
        float x=po->x(), y=po->y();
        po->group=id();
        po->moveTo(x,y); // adjust to group relative coordinates
        computeDisplayZoom();
        po->visible=displayContents();
      }
  }

  void GroupIcon::removeOperation(Operations::value_type& po)
  {
    for (vector<int>::iterator i=m_operations.begin(); i!=m_operations.end(); ++i)
      if (*i==po.id())
        {
          m_operations.erase(i);
          computeDisplayZoom();
          if (parent()==-1) //rebase coordinates
            {
              float x=po->x(), y=po->y();
              po->group=-1;
              po->visible=true;
              po->moveTo(x,y); // adjust to group relative coordinates
            }
          computeDisplayZoom();
          break;
        }
  }

  bool GroupIcon::isAncestor(int gid) const
  {
    for (const GroupIcon* g=this; g->parent()!=-1; 
         g=&minsky().groupItems[g->parent()])
      if (g->parent()==gid)
        return true;
    return false;
  }

  bool GroupIcon::addGroup(IntrusiveWrap<int, GroupIcon>& pg)
  {
    // do not add to self, or any ancestor to prevent cycles!
    if (pg.id()==id() || isAncestor(pg.id())) return false; 
    m_groups.push_back(pg.id());
    float x=pg.x(), y=pg.y();
    pg.m_parent=id();
    pg.moveTo(x,y);
    pg.zoom(pg.x(), pg.y(), 
            localZoom()/pg.zoomFactor);
    computeDisplayZoom();
    pg.visible=displayContents();
    return true;
  }

  void GroupIcon::removeGroup(IntrusiveWrap<int, GroupIcon>& pg)
  {
    m_groups.erase(remove(m_groups.begin(), m_groups.end(), pg.id()), m_groups.end());
    computeDisplayZoom();
    if (parent()==-1)
      {
        float x=pg.x(), y=pg.y();
        pg.m_parent=-1;
        pg.moveTo(x,y);
        pg.zoom(pg.x(), pg.y(), minsky().zoomFactor()/pg.zoomFactor);
        pg.visible=true;
      }
  }

  void GroupIcon::setZoom(float factor)
  {
    zoomFactor=factor;
    computeDisplayZoom();
    float lzoom=localZoom();
    vector<int>::iterator i;
    for (i=m_operations.begin(); i!=m_operations.end(); ++i)
      minsky::minsky().operations[*i]->setZoom(lzoom);
    for (i=m_variables.begin(); i!=m_variables.end(); ++i)
      minsky::minsky().variables[*i]->setZoom(lzoom);
    for (i=m_groups.begin(); i!=m_groups.end(); ++i)
      minsky::minsky().groupItems[*i].setZoom(lzoom);
  }

  namespace 
  {
    // TODO: replace this with the Rotate class.

    // transform (x,y) by rotating around (0,0)
    inline void rotate(float& x, float& y, float ca, float sa)
    {
      float x1=ca*x-sa*y, y1=sa*x+ca*y;
      x=x1; y=y1;
    }

    // transform (x,y) by rotating around the origin(ox,oy)
    inline void rotate(float& x, float& y, float ox, float oy, float ca, float sa)
    {
      x-=ox; y-=oy;
      rotate(x,y,ca,sa);
      x+=ox; y+=oy;
    }
  }

  void GroupIcon::rotate(float angle)
  {
    rotation+=angle;
    ::Rotate rotate(angle, x(), y());

    vector<int>::const_iterator i=m_operations.begin();
    for (; i!=m_operations.end(); ++i)
      {
        OperationBase& o=*minsky().operations[*i];
        o.rotation+=angle;
        Point p=rotate(o.x(), o.y());
        o.moveTo(p.x(), p.y());
      }
    for (i=m_variables.begin(); i!=m_variables.end(); ++i)
      {
        VariableBase& v=*minsky().variables[*i];
        v.rotation+=angle;
        Point p=rotate(v.x(), v.y());
        v.moveTo(p.x(), p.y());
      }
    // transform wire coordinates
    for (i=m_wires.begin(); i!=m_wires.end(); ++i)
      {
        Wire& w=minsky().wires[*i];
        array<float> coords=w.coords();
        for (size_t j=0; j<coords.size(); j+=2)
          {
            Point p=rotate(coords[j], coords[j+1]);
            coords[j]=p.x();
            coords[j+1]=p.y();
          }
        w.coords(coords); 
      }
    // transform external ports
    vector<int> externalPorts=ports();
    for (i=externalPorts.begin(); i!=externalPorts.end(); ++i)
      {
        const Port& p=cminsky().ports[*i];
        Point pp=rotate(p.x(), p.y());
        minsky().movePortTo(*i,pp.x(),pp.y());
      }
  }

  void GroupIcon::flipContents()
  {
    Reflect r(rotation,x(),y());

    vector<int>::const_iterator i=m_operations.begin();
    for (; i!=m_operations.end(); ++i)
      {
        OperationBase& o=*minsky().operations[*i];
        o.rotation+=180;
        o.moveTo(r.x(o.x(),o.y()), r.y(o.x(),o.y()));
      }
    set<int> evars=edgeSet();
    for (i=m_variables.begin(); i!=m_variables.end(); ++i)
      if (evars.count(*i)==0)
        {
          VariableBase& v=*minsky().variables[*i];
          v.rotation+=180;
          v.moveTo(r.x(v.x(),v.y()), r.y(v.x(),v.y()));
        }
    for (i=m_groups.begin(); i!=m_groups.end(); ++i)
      if (evars.count(*i)==0)
        {
          GroupIcon& g=minsky().groupItems[*i];
          g.rotation+=180;
          g.moveTo(r.x(g.x(),g.y()), r.y(g.x(),g.y()));
        }
    // transform wire coordinates
    for (i=m_wires.begin(); i!=m_wires.end(); ++i)
      {
        Wire& w=minsky().wires[*i];
        array<float> coords=w.coords();
        for (size_t j=0; j<coords.size(); j+=2)
          {
            float x=r.x(coords[j], coords[j+1]);
            float y=r.y(coords[j], coords[j+1]);
            coords[j]=x;
            coords[j+1]=y;
          }
        w.coords(coords); 
      }
  }

  int GroupIcon::selectVariable(float x, float y) const
  {
    float left, right;
    margins(left, right);
    set<int>::const_iterator i;
    float dx=x-this->x(), dy=y-this->y();
    minsky::rotate(dx,dy, cos(rotation*M_PI/180.0), sin(rotation*M_PI/180.0));
    if (dx < -0.5*width+left)
      // check input variables
      for (i=inVariables.begin(); i!=inVariables.end(); ++i)
        if (RenderVariable(*minsky().variables[*i]).inImage(x,y))
          return *i;
    if (dx > 0.5*width-right)
      // check output variables
      for (i=outVariables.begin(); i!=outVariables.end(); ++i)
        if (RenderVariable(*minsky().variables[*i]).inImage(x,y))
          return *i;
    return -1;
  }

  Polygon GroupIcon::geom() const
  {
    ::Rotate rotate(rotation, x(), y());
    float dx=0.5*width*zoomFactor, dy=0.5*height*zoomFactor;
    Polygon r;
    r+= rotate(x()-dx, y()-dy), rotate(x()-dx, y()+dy), 
      rotate(x()+dx, y()+dy), rotate(x()+dx, y()-dy);
    correct(r);
    return r;
  }

  void GroupIcon::draw(cairo_t* cairo) const
  {
    double angle=rotation * M_PI / 180.0;

    // determine how big the group icon should be to allow
    // sufficient space around the side for the edge variables
    float leftMargin, rightMargin;
    margins(leftMargin, rightMargin);
    leftMargin*=zoomFactor; rightMargin*=zoomFactor;

    unsigned width=zoomFactor*this->width, height=zoomFactor*this->height;
    // bitmap needs to be big enough to allow a rotated
    // icon to fit on the bitmap.
    float rotFactor=this->rotFactor();


    // set clip to indicate icon boundary
    cairo_rotate(cairo, angle);
    cairo_rectangle(cairo,-0.5*width,-0.5*height,width,height);
    cairo_clip(cairo);

   // draw default group icon
    cairo_save(cairo);
    //    cairo_rotate(cairo, angle);

    // display I/O region in grey
    updatePortLocation();
    drawIORegion(cairo);

    cairo_translate(cairo, -0.5*width+leftMargin, -0.5*height);


              
    double scalex=double(width-leftMargin-rightMargin)/width;
    cairo_scale(cairo, scalex, 1);

    // draw a simple frame 
    cairo_rectangle(cairo,0,0,width,height);
    cairo_save(cairo);
    cairo_identity_matrix(cairo);
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
    cairo_restore(cairo);

    if (!displayContents())
      {
        cairo_scale(cairo,width/svgRenderer.width(),height/svgRenderer.height());
        cairo_rectangle(cairo,0, 0,svgRenderer.width(), svgRenderer.height());
        cairo_clip(cairo);
        svgRenderer.render(cairo);
      }
    cairo_restore(cairo);

    cairo_identity_matrix(cairo);

    drawEdgeVariables(cairo);


    // display text label
    if (!name().empty())
      {
        cairo_save(cairo);
        cairo_identity_matrix(cairo);
        cairo_scale(cairo, zoomFactor, zoomFactor);
        cairo_select_font_face
          (cairo, "sans-serif", CAIRO_FONT_SLANT_ITALIC, 
           CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cairo,12);
              
        // extract the bounding box of the text
        cairo_text_extents_t bbox;
        cairo_text_extents(cairo,name().c_str(),&bbox);
        double w=0.5*bbox.width+2; 
        double h=0.5*bbox.height+5;
        double fm=std::fmod(rotation,360);

        // if rotation is in 1st or 3rd quadrant, rotate as
        // normal, otherwise flip the text so it reads L->R
        if ((fm>-90 && fm<90) || fm>270 || fm<-270)
          cairo_rotate(cairo, angle);
        else
          cairo_rotate(cairo, angle+M_PI);
 
        // prepare a background for the text, partially obscuring graphic
        double transparency=displayContents()? 0.25: 1;
        cairo_set_source_rgba(cairo,0,1,1,0.5*transparency);
        cairo_rectangle(cairo,-w,-h,2*w,2*h);
        cairo_fill(cairo);

        // display text
        cairo_move_to(cairo,-w+1,h-4);
        cairo_set_source_rgba(cairo,0,0,0,transparency);
        cairo_show_text(cairo,name().c_str());
        cairo_restore(cairo);
      }

    cairo_identity_matrix(cairo);

    // shouldn't be needed??
    // set clip to indicate icon boundary
    cairo_rotate(cairo, angle);
    cairo_rectangle(cairo,-0.5*width,-0.5*height,width,height);
    cairo_clip(cairo);

    if (mouseFocus)
      drawPorts(*this, ports(), cairo);

    if (selected) drawSelected(cairo);
  }

  void GroupIcon::wtDraw(ecolab::cairo::Surface& cairoSurface)
  {
    // TODO doesn't this just make g a reference to *this?
    GroupIcon& g=minsky::minsky().groupItems[id()];
    double angle  = rotation * M_PI / 180.0;
    double xScale = zoomFactor;
    double yScale = zoomFactor;
    //computeDisplayZoom();

    // determine how big the group icon should be to allow
    // sufficient space around the side for the edge variables
    float leftMargin, rightMargin;
    margins(leftMargin, rightMargin);
    leftMargin *= xScale;
    rightMargin *= xScale;

    unsigned width = xScale  * this->width;
    unsigned height = yScale * this->height;
    // bitmap needs to be big enough to allow a rotated
    // icon to fit on the bitmap.
    float rotFactor = this->rotFactor();
    //resize(rotFactor * width, rotFactor * height);
    cairoSurface.clear();

    // draw default group icon
    cairo_save(cairoSurface.cairo());

    cairo_translate(cairoSurface.cairo(), 0.5 * cairoSurface.width(), 0.5 * cairoSurface.height());
    cairo_rotate(cairoSurface.cairo(), angle);

    // display I/O region in grey
    updatePortLocation();
    drawIORegion(cairoSurface.cairo());

    cairo_translate(cairoSurface.cairo(), (-0.5 * width) + leftMargin, -0.5 * height);


    double scalex = double(width - leftMargin - rightMargin) / width;
    cairo_scale(cairoSurface.cairo(), scalex, 1);

    // draw a simple frame
    cairo_rectangle(cairoSurface.cairo(),0,0,width,height);
    cairo_save(cairoSurface.cairo());
    cairo_identity_matrix(cairoSurface.cairo());
    cairo_set_line_width(cairoSurface.cairo(),1);
    cairo_stroke(cairoSurface.cairo());
    cairo_restore(cairoSurface.cairo());

    if (!displayContents())
      {
        // render below uses bitmap size to determine image
        // size, so we need to scale by 1/rotFactor to get it to
        // correctly fit in the bitmap
        cairo_scale(cairoSurface.cairo(), 1/rotFactor, 1/rotFactor);
        // replace contents with a graphic image
        cairo_scale(cairoSurface.cairo(),width/svgRenderer.width(),height/svgRenderer.height());
        svgRenderer.render(cairoSurface.cairo());
      }
    cairo_restore(cairoSurface.cairo());

    cairo_t* cairo=cairoSurface.cairo();
    cairo_identity_matrix(cairo);
    cairo_translate(cairo, 0.5 * cairoSurface.width(), 0.5 * cairoSurface.height());

    drawEdgeVariables(cairo);
    //  if (displayContents())
    //  {
    //    createOrDeleteContentItems(g.displayContents());
    //    displayContents=g.displayContents();
    //  }

    // display text label
    if (!name().empty())
      {
        cairo_save(cairo);
        cairo_identity_matrix(cairo);
        cairo_translate(cairo, 0.5 * cairoSurface.width(), 0.5 * cairoSurface.height());
        cairo_scale(cairo, xScale, yScale);
        //initMatrix();
        cairo_select_font_face
          (cairo, "sans-serif", CAIRO_FONT_SLANT_ITALIC,
           CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cairo,12);

        // extract the bounding box of the text
        cairo_text_extents_t bbox;
        cairo_text_extents(cairo,g.name().c_str(),&bbox);
        double w=0.5*bbox.width+2;
        double h=0.5*bbox.height+5;
        double fm=std::fmod(g.rotation,360);

        // if rotation is in 1st or 3rd quadrant, rotate as
        // normal, otherwise flip the text so it reads L->R
        if ((fm > -90 && fm < 90) || fm>270 || fm<-270)
          cairo_rotate(cairo, angle);
        else
          cairo_rotate(cairo, angle+M_PI);

        // prepare a background for the text, partially obscuring graphic
        //double transparency=displayContents()? 0.25: 1;
        double transparency=1;
        cairo_set_source_rgba(cairo,0,1,1,0.5*transparency);
        cairo_rectangle(cairo,-w,-h,2*w,2*h);
        cairo_fill(cairo);

        // display text
        cairo_move_to(cairo,-w+1,h-4);
        cairo_set_source_rgba(cairo,0,0,0,transparency);
        cairo_show_text(cairo,g.name().c_str());
        cairo_restore(cairo);
      }
  }

  SVGRenderer GroupIcon::svgRenderer;

  vector<int> GroupIcons::visibleGroups() const
  {
    vector<int> r;
    for (const_iterator i=begin(); i!=end(); ++i)
      if (i->visible)
        r.push_back(i->id());
    return r;
  }

  namespace
  {
    bool insert(set<int>& items, const vector<int>& itemList)
    {
      for (size_t i=0; i<itemList.size(); ++i)
        if (!items.insert(itemList[i]).second)
          return false;
      return true;
    }
  }

  void GroupIcon::select(Selection& sel, float x0, float y0, float x1, float y1)
  {
    if (!displayContents()) return;
    sel.group=id();

    LassoBox lasso(x0,y0,x1,y1);
    for (int w: m_wires)
      if (lasso.contains(cminsky().wires[w]))
        sel.wires.push_back(w);

    for (int vi: m_variables)
      {
        VariablePtr v=cminsky().variables[vi];
        if (lasso.intersects(*v))
          {
            sel.variables.push_back(vi);
            v->selected=true;
          }
      }

    for (int oi: m_operations)
      {
        OperationPtr o=cminsky().operations[oi];
        if (lasso.intersects(*o))
        {
          sel.operations.push_back(oi);
          o->selected=true;
        }
      }
        
    for (int gi: m_groups)
      {
        GroupIcon& g=minsky().groupItems[gi];
        if (lasso.intersects(g))
          {
            sel.groups.push_back(g.id());
            g.selected=true;
          }
      }


  }

  void GroupIcon::centreIconOnContents() 
  {
    float x0, y0, x1, y1;
    contentBounds(x0,y0,x1,y1);
    // amount to shift the icon
    float dx=0.5*(x0+x1)-x(), dy=0.5*(y0+y1)-y();
    m_x+=dx;
    m_y+=dy;
    // compensate contents for icon shift
    moveContents(-dx,-dy);
#ifndef NDEBUG
     contentBounds(x0,y0,x1,y1);
     assert(abs(x()-0.5*(x0+x1))<0.1);
     assert(abs(y()-0.5*(y0+y1))<0.1);
#endif
  }

  void GroupIcon::rehostGlobalVars(int id)
  {
    for (int v: m_variables)
      {
        VariablePtr vp=minsky().variables[v];
        if (vp->scope()==-1)
          vp->setScope(id);
      }
    for (int g: m_groups)
      minsky().groupItems[g].rehostGlobalVars(id);
  }
        



  bool GroupIcons::uniqueGroupMembership() const
  {
    set<int> varItems, opItems, wireItems, groupIds;
    for (GroupIcons::const_iterator g=begin(); g!=end(); ++g)
      if (!::insert(varItems, g->variables()) ||
          !::insert(opItems, g->operations()) ||
          !::insert(wireItems, g->wires()) ||
          !::insert(groupIds, g->groups()))
        return false;
    return true;
  }


}
