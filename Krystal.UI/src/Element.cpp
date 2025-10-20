#include "Krystal.UI/Element.hpp"

namespace Krys::UI
{
  Element::Element(Layout::ConfigRef layoutConfig)
  {
    _layoutNode = Layout::NodeCreate(layoutConfig);
    SetBorderWidth(0._px);
    SetPadding(0._px);
    SetMargin(0._px);
  }

  Element::~Element()
  {
    Layout::NodeDestroy(_layoutNode);
  }
}