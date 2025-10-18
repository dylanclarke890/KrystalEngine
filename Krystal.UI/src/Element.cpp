#include "Krystal.UI/Element.hpp"

namespace Krys::UI
{
  void Element::CreateLayoutNode(Layout::ConfigRef layoutConfig)
  {
    _layoutNode = Layout::NodeCreate(layoutConfig);
  }

  Element::~Element()
  {
    Layout::NodeDestroy(_layoutNode);
  }
}