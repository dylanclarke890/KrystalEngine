#include "Krystal.UI/Element.hpp"

namespace Krys::UI
{
  Element::Element(ElementHandle handle, Layout::ConfigRef layoutConfig) : _handle(handle)
  {
    _layoutNode = Layout::NodeCreate(layoutConfig);
    SetBorderWidth(0._px);
    SetPadding(0._px);
    SetMargin(0._px);
    SetPosition(Position::Static);
  }

  Element::~Element()
  {
    Layout::NodeDestroy(_layoutNode);
  }
}