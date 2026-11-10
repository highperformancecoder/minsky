/*
  The native build draws the canvas into an operating system window via
  WindowInformation. There are no such windows in a browser, where the
  canvas is drawn into an HTML canvas context instead (see
  drawToContext() and jsminsky.cc), so these members are never
  meant to be reached. They exist to satisfy the linker for
  RenderNativeWindow::renderFrame()/draw().
*/
#include "windowInformation.h"
#include <classdesc_epilogue.h>
#include <stdexcept>

namespace minsky
{
  WindowInformation::WindowInformation(uint64_t, int, int, int, int, double, bool, const std::function<void()>&)
  {throw std::runtime_error("native windows are not available in the wasm build");}
  WindowInformation::~WindowInformation() {}
  bool WindowInformation::render(const std::function<bool(const mansoura::IMansoura&,int,int)>&, const mansoura::Colour&)
  {return false;}
}
