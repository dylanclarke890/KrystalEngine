#include "Krystal.Booey/MathML/MathMLFractionElement.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::mathml
{
  const MathMLElement::Length &MathMLFractionElement::LineThickness() noexcept
  {
    return CachedMathMLLength(MathMLNames::LinethicknessAttr(), _lineThickness);
  }

  void MathMLFractionElement::AttributeChanged(const dom::QualifiedName &name,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == MathMLNames::LinethicknessAttr())
    {
      _lineThickness.reset();
      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }

    MathMLRowElement::AttributeChanged(name, oldValue, value);
  }
}
