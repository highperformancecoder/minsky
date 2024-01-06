#include "function.h"
#include "typescriptAPI_base.h"
#include "minsky.h"
#include "minsky.tcd"

#include "assetClass.tcd"
#include "bookmark.h"
#include "bookmark.tcd"
#include "cairoSurfaceImage.tcd"
#include "callableFunction.tcd"
#include "canvas.tcd"
#include "CSVDialog.tcd"
#include "CSVParser.tcd"
#include "constMap.tcd"
#include "dataSpecSchema.tcd"
#include "dataOp.h"
#include "dataOp.tcd"
#include "dimension.tcd"
#include "engNotation.tcd"
#include "equationDisplay.tcd"
#include "evalGodley.tcd"
#include "eventInterface.tcd"
#include "fontDisplay.tcd"
#include "godleyIcon.tcd"
#include "godleyTable.tcd"
#include "godleyTableWindow.tcd"
#include "grid.tcd"
#include "group.tcd"
#include "handleLockInfo.tcd"
#include "hypercube.tcd"
#include "index.tcd"
#include "intOp.tcd"
#include "item.tcd"
#include "itemT.tcd"
#include "itemTab.h"
#include "itemTab.tcd"
#include "lasso.tcd"
#include "lock.tcd"
#include "noteBase.tcd"
#include "operation.tcd"
#include "operationBase.tcd"
#include "operationType.tcd"
#include "pango.tcd"
#include "pannableTab.tcd"
#include "phillipsDiagram.tcd"
#include "plot.tcd"
#include "plotOptions.tcd"
#include "plotWidget.tcd"
#include "polyRESTProcessBase.tcd"
#include "port.h"
#include "port.tcd"
#include "pubTab.tcd"
#include "ravelState.tcd"
#include "renderNativeWindow.tcd"
#include "ravelWrap.tcd"
#include "rungeKutta.tcd"
#include "selection.tcd"
#include "sheet.tcd"
#include "simulation.tcd"
#include "slider.tcd"
#include "SVGItem.tcd"
#include "switchIcon.tcd"
#include "tensorInterface.tcd"
#include "tensorVal.tcd"
#include "units.tcd"
#include "userFunction.h"
#include "userFunction.tcd"
#include "variableInstanceList.h"
#include "variableInstanceList.tcd"
#include "variable.tcd"
#include "variablePane.tcd"
#include "variableType.tcd"
#include "variableValue.tcd"
#include "variableValues.tcd"
#include "wire.tcd"
#include "xvector.tcd"

namespace classdesc_access
{
  // clobbers
  template <class T>
  struct access_typescriptAPI<classdesc::Exclude<T>>:
    public classdesc::NullDescriptor<classdesc::typescriptAPI_t> {};

  template <class T, class V, int N>
  struct access_typescriptAPI<ecolab::TCLAccessor<T,V,N>>:
    public classdesc::NullDescriptor<classdesc::typescriptAPI_t> {};

  template <>
  struct access_typescriptAPI<classdesc::PolyPackBase>:
    public classdesc::NullDescriptor<classdesc::typescriptAPI_t> {};
  
  template <class T>
  struct access_typescriptAPI<classdesc::PolyPack<T>>:
    public classdesc::NullDescriptor<classdesc::typescriptAPI_t> {};
  
  template <>
  struct access_typescriptAPI<std::vector<civita::any>>:
    public classdesc::NullDescriptor<classdesc::typescriptAPI_t> {};
  
  template <class T>
  struct access_typescriptAPI<minsky::Optional<T>>:
    public classdesc::NullDescriptor<classdesc::typescriptAPI_t> {};
}

namespace classdesc
{
  // dummies
  template <> string typeName<cairo_t>() {return "minsky__dummy";}
  template <> string typeName<cairo_surface_t>() {return "minsky__dummy";}
  
  template <>
  struct tn<civita::any>
  {
    static string name() {return "minsky__dummy";}
  };
  template <class T>
  struct tn<boost::geometry::model::d2::point_xy<T>>
  {
    static string name() {return "minsky__dummy";}
  };
  template <>
  struct tn<RESTProcess_t>
  {
    static string name() {return "minsky__dummy";}
  };
  template <class C, class D>
  struct tn<std::chrono::time_point<C,D>>
  {
    static string name() {return "minsky__dummy";}
  };
  template <>
  struct tn<std::istream>
  {
    static string name() {return "minsky__dummy";}
  };
  
  // typescript has difficulties with specialised templates
  template <>
  struct tn<minsky::PannableTab<minsky::EquationDisplay>>
  {
    static string name() {return "EquationDisplay";}
  };

}

#include "minsky_epilogue.h"

namespace classdesc
{
  template <class T, class Base>
  typename enable_if<Not<is_map<Base>>, void>::T
  typescriptAPI(typescriptAPI_t& t, const string& d)
  {
    // bail out of object heirarchy drill down once reaching named base class
    if (typescriptType<Base>()==t[typescriptType<T>()].super) return;
    classdesc_access::access_typescriptAPI<Base>().template type<T>(t,d);
  }

  template <class T> void typescriptAPI_t::addClass() {typescriptAPI<T,T>(*this,"");}
  template <class T, class Base> void typescriptAPI_t::addSubclass()
  {
    operator[](typescriptType<T>()).super=typescriptType<Base>();
    typescriptAPI<T,T>(*this,"");
  }
}

namespace minsky
{
  Minsky& minsky()
  {
    static Minsky s_minsky;
    return s_minsky;
  }

  LocalMinsky::LocalMinsky(Minsky& minsky) {}
  LocalMinsky::~LocalMinsky() {}

  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}
}

using namespace classdesc;
using namespace std;
using namespace minsky;

void exportClass(const std::string& name, const minsky::typescriptAPI_ns::ClassType& klass)
{
      cout << "export class "+name+" extends "+(klass.super.empty()? "CppClass": klass.super)+" {\n";

      // properties
      for (auto& prop: klass.properties)
        cout << "  "<<prop.first <<": "<<prop.second.type<<";\n";

      // constructor
      if (!klass.super.empty())
        {
          cout << "  constructor(prefix: string|"<<klass.super<<"){\n";
          cout << "    if (typeof prefix==='string')\n";
          cout << "      super(prefix";
          if (!klass.valueType.empty())
            cout <<","<<klass.valueType;
          cout << ")\n";
          cout << "    else\n";
          cout << "      super(prefix.$prefix()";
          if (!klass.valueType.empty())
            cout <<","<<klass.valueType;
          cout <<")\n";
        }
      else
        {
          cout << "  constructor(prefix: string){\n";
          cout << "    super(prefix);\n";
        }
      for (auto& prop: klass.properties)
        {
          if (!prop.second.construction.empty())
            cout << "    this."<<prop.first<<"="<<prop.second.construction<<"\n";
          else
            cout << "    this."<<prop.first<<"=new "<<prop.second.type<<"(this.$prefix()+'."<<prop.first<<"');\n"; 
        }
      cout << "  }\n";

      // methods
      for (auto& method: klass.methods)
        {
          cout << "  async "<<method.first<<"(";
          for (size_t i=0; i<method.second.args.size(); ++i)
            {
              if (i>0) cout<<",";
              cout<<method.second.args[i].name<<": "<<method.second.args[i].type;
            }
          cout << "): Promise<"<<method.second.returnType<<"> {return this.$callMethod('"<<method.first<<"'";
          for (auto& arg: method.second.args)
            cout<<","<<arg.name;
          cout<<");}\n";
        }
      cout << "}\n\n";
}

int main()
{
  typescriptAPI_t api;
  api.addClass<Minsky>();

  // supporting types
  api.addClass<Bookmark>();
  api.addClass<civita::Dimension>();
  api.addClass<civita::Hypercube>();
  api.addClass<civita::Index>();
  api.addClass<civita::ITensor>();
  api.addClass<civita::XVector>();
  api.addClass<DataSpecSchema>();
  api.addClass<ecolab::Plot::LineStyle>();
  api.addClass<EngNotation>();
  api.addClass<EventInterface>();
  api.addClass<GroupItems>();
  api.addClass<HandleLockInfo>();
  api.addClass<Port>();
  api.addClass<PubItem>();
  api.addClass<ravel::HandleState>();
  api.addClass<ravel::RavelState>();
  api.addClass<Units>();
  api.addClass<VariablePaneCell>();
  api.addClass<VariableValue>();
  api.addClass<VariableValues>();
  api["VariableValues"].valueType="VariableValue";
  
  // Item subclasses
  api.addSubclass<DataOp,Item>();
  api.addSubclass<Group,Item>();
  api.addSubclass<GodleyIcon,Item>();
  api.addSubclass<IntOp,Item>();
  api.addSubclass<Lock,Item>();
  api.addSubclass<OperationBase,Item>();
  api.addSubclass<PhillipsFlow,Item>();
  api.addSubclass<PhillipsStock,Item>();
  api.addSubclass<PlotWidget,Item>();
  api.addSubclass<Ravel,Item>();
  api.addSubclass<Sheet,Item>();
  api.addSubclass<SwitchIcon,Item>();
  api.addSubclass<VariableBase,Item>();
  api.addSubclass<UserFunction,Item>();

  // RenderNativeWindow subclasses
  api.addSubclass<RenderNativeWindow, EventInterface>();
  api.addSubclass<Canvas,RenderNativeWindow>();
  api.addSubclass<PannableTab<EquationDisplay>,RenderNativeWindow>();
  api.addSubclass<PhillipsDiagram,RenderNativeWindow>();
  api.addSubclass<PubTab,RenderNativeWindow>();

  
  // to prevent Group recursively calling itself on construction
  api["Group"].properties.erase("parent");
  
  cout << "/*\nThis is a built file, please do not edit.\n";
  cout << "See RESTService/typescriptAPI for more information.\n*/\n\n";
  cout << "import {CppClass, Sequence, Container, Map, Pair} from './backend';\n\n";

  // dummy types
  cout << "class minsky__dummy {}\n";
  cout << "class minsky__EventInterface__KeyPressArgs {}\n";
  cout << "class minsky__GodleyIcon__MoveCellArgs {}\n";
  cout << "class minsky__RenderNativeWindow__RenderFrameArgs {}\n";
  cout << "class minsky__VariableType__TypeT {}\n";
  cout << "class civita__ITensor__Args {}\n";
  cout << "class classdesc__json_pack_t {}\n";
  cout << "class classdesc__pack_t {}\n";
  cout << "class classdesc__RESTProcess_t {}\n";
  cout << "class classdesc__TCL_obj_t {}\n";
  cout << "class ecolab__cairo__Surface {}\n";
  //  cout << "class ecolab__Pango {}\n";
  cout<<endl;
  
  // these need to be declared in a specific order
  vector<string> exportFirst{"EventInterface","Item","OperationBase","RenderNativeWindow","VariableBase"};
  for (auto& i: exportFirst) exportClass(i,api[i]);

  cout << "class minsky__Variable<T> extends VariableBase {}\n";

  // then export the rest
  for (auto& i: api)
    if (find(exportFirst.begin(), exportFirst.end(), i.first)==exportFirst.end())
      exportClass(i.first, i.second);

  cout << "export var minsky=new Minsky('minsky');\n";
}
