#include "Krystal.Booey/Rendering/SVG/SVGInlineFlowBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"

namespace krys::boo
{
  FloatRect SVGInlineFlowBox::CalculateBoundaries() const noexcept
  {
    FloatRect childRect;
    for (auto *child = FirstChild(); child; child = child->NextOnLine())
    {
      if (auto *textBox = DynamicDowncast<SVGInlineTextBox>(child))
      {
        childRect.Unite(textBox->CalculateBoundaries());
        continue;
      }
      if (auto *flowBox = DynamicDowncast<SVGInlineFlowBox>(child))
      {
        childRect.Unite(flowBox->CalculateBoundaries());
        continue;
      }
    }
    return childRect;
  }
}
