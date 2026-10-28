#include "Krystal.Booey/MathML/MathMLSpaceElement.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::mathml
{
  const MathMLElement::Length &MathMLSpaceElement::Width() noexcept
  {
    return CachedMathMLLength(MathMLNames::WidthAttr(), _width);
  }

  const MathMLElement::Length &MathMLSpaceElement::Height() noexcept
  {
    return CachedMathMLLength(MathMLNames::HeightAttr(), _height);
  }

  const MathMLElement::Length &MathMLSpaceElement::Depth() noexcept
  {
    return CachedMathMLLength(MathMLNames::DepthAttr(), _depth);
  }

  void MathMLSpaceElement::AttributeChanged(const dom::QualifiedName &name,
                                            Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value) noexcept
  {
    bool affectsLayout = false;
    if (name == MathMLNames::WidthAttr())
    {
      _width.reset();
      affectsLayout = true;
    }
    else if (name == MathMLNames::HeightAttr())
    {
      _height.reset();
      affectsLayout = true;
    }
    else if (name == MathMLNames::DepthAttr())
    {
      _depth.reset();
      affectsLayout = true;
    }

    if (affectsLayout)
    {
      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }

    MathMLPresentationElement::AttributeChanged(name, oldValue, value);
  }
}
