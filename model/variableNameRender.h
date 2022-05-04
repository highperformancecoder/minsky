#ifndef VARIABLE_NAME_RENDER_H
#define VARIABLE_NAME_RENDER_H
#include "renderNativeWindow.h"
#include "variable.h"
#include "classdesc_access.h"

namespace minsky
{
class VariableNameRender: public RenderNativeWindow
  {
    VariablePtr var;
    double m_width, m_height;
    CLASSDESC_ACCESS(VariableNameRender);
  public:
    VariableNameRender() {}
    VariableNameRender(const VariableValue& var);
    bool redraw(int x0, int y0, int width, int height) override;
    bool operator<(const VariableNameRender& x) const {return var&&x.var? var->valueId()<x.var->valueId(): true;}
    double width() const {return m_width;}
    double height() const {return m_height;}
    void emplace() const; ///< emplace a copy of this variable on the canvas
  };
} 

#include "variableNameRender.cd"
#endif
