/*
  Javascript binding of the Minsky object for WebAssembly, using
  emscripten::bind. Adapted from the Ravel binding (jsravel.cc).

  The Minsky API is exposed through the RESTProcess classdesc
  descriptor: Javascript calls minsky.call("canvas.mouseDown", "[x,y]"),
  and gets back an object wrapping the result, which can be chained
  or converted to JSON. See minsky.js for a wrapper that mirrors the
  C++ object structure as a Javascript object.

  The model is drawn on an HTML5 canvas through
  mansoura::MansouraHTMLCanvas.
*/

#include "RESTMinsky.h"
#include "cairoItems.h"
#include "minsky_epilogue.h"
#include "mansouraHTMLCanvas.h"
#include "json_pack_base.h"
#include "RESTProcess_base.h"

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

#include <string>
#include <vector>

using namespace std;
using namespace emscripten;
using namespace minsky;
using classdesc::RPPtr;
using classdesc::json_pack_t;
using mansoura::MansouraHTMLCanvas;

// the global Minsky object. Linkage definitions normally provided by RESTService.cc
namespace
{
  RESTMinsky rminsky;
}

namespace minsky
{
  Minsky& minsky() {return rminsky;}
  LocalMinsky::LocalMinsky(Minsky&) {}
  LocalMinsky::~LocalMinsky() {}
  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool) {}
}

namespace classdesc
{
  template <> struct tn<emscripten::val> {
    static std::string name() {return "emscripten::val";}
  };
}

/// C++ exceptions cross the embind boundary as opaque
/// WebAssembly.Exception objects, so convert them into Javascript
/// Errors carrying the message.
template <class F> auto jsGuard(F f) -> decltype(f())
{
  try
    {
      return f();
    }
  catch (const std::exception& ex)
    {
      val::global("Error").new_(std::string(ex.what())).throw_();
    }
  catch (...)
    {
      val::global("Error").new_(std::string("unknown C++ exception")).throw_();
    }
  throw; // not reached: throw_() does not return
}

/// Wraps the result of a call on the Minsky API
struct CppWrapper
{
  RPPtr wrappedObject=make_shared<classdesc::RESTProcessVoid>();
  std::string path;
  CppWrapper()=default;
  CppWrapper(const RPPtr& x, const std::string& path): wrappedObject(x), path(path) {}
  /// call method of this object. \a args is a JSON array of arguments
  CppWrapper call(const std::string& method, const string& args)
  {
    return jsGuard([&]{
      json_pack_t jin;
      read(args,jin);
      const auto command=path+"."+method;
      auto result=wrappedObject->process("."+method, jin);
      rminsky.commandHook(command,jin);
      return CppWrapper(result,command);
    });
  }
  /// JSON representation of the wrapped value
  std::string json() const
  {return jsGuard([&]{return write(wrappedObject->asBuffer());});}
  /// list the methods and members available
  std::vector<string> list() const
  {
    std::vector<string> methods;
    for (auto& i: wrappedObject->list())
      {
        auto n=i.first.find('.');
        if (n==string::npos) continue;
        methods.emplace_back(i.first.substr(n+1));
      }
    return methods;
  }
};

struct JSMinsky: public CppWrapper
{
  MansouraHTMLCanvas man;
  /// @param canvas a CanvasRenderingContext2D
  JSMinsky(const val& canvas):
    CppWrapper(rminsky.registry.at("minsky"),"minsky"), man(canvas)
  {
    // text metrics for contexts without a canvas (item bounding boxes)
    // (preferably with an offscreen canvas, so as not to disturb the visible one's state)
    if (!mansoura::installHTMLTextMeasurer())
      mansoura::installHTMLTextMeasurer(canvas);
  }

  /// Draw the model canvas into the HTML canvas context, covering
  /// \a width by \a height pixels.
  void render(int width, int height)
  {
    jsGuard([&]{renderImpl(width,height); return 0;});
  }

  void renderImpl(int width, int height)
  {
    man.syncState();
    const mansoura::PreserveContext pc(man);
    auto& bg=RenderNativeWindow::backgroundColour;
    man.setSourceRGBA(bg.r,bg.g,bg.b,bg.a);
    man.rectangle(0,0,width,height);
    man.fill();
    man.setSourceRGB(0,0,0);
    drawToContext(rminsky.canvas,man,width,height);
  }
};

EMSCRIPTEN_BINDINGS(Minsky) {
  class_<CppWrapper>("CppWrapper")
    .function("call",&CppWrapper::call)
    .function("json",&CppWrapper::json)
    .function("list",&CppWrapper::list)
    ;

  class_<JSMinsky,base<CppWrapper>>("Minsky")
    .constructor<const val&>()
    .function("render",&JSMinsky::render)
    ;

  register_vector<std::string>("vector<string>");
}
