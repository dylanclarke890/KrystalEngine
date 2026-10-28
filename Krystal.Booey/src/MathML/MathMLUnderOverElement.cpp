#include "Krystal.Booey/MathML/MathMLUnderOverElement.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::mathml
{
  MathMLElement::BooleanValue MathMLUnderOverElement::Accent() noexcept
  {
    return CachedBooleanAttribute(MathMLNames::AccentAttr(), _accent);
  }

  MathMLElement::BooleanValue MathMLUnderOverElement::AccentUnder() noexcept
  {
    return CachedBooleanAttribute(MathMLNames::AccentunderAttr(), _accentUnder);
  }

  void MathMLUnderOverElement::AttributeChanged(const dom::QualifiedName &name,
                                                Maybe<dom::DOMStringView> oldValue,
                                                Maybe<dom::DOMStringView> value) noexcept
  {
    bool affectsLayout = false;
    if (name == MathMLNames::AccentAttr())
    {
      _accent.reset();
      affectsLayout = true;
    }
    else if (name == MathMLNames::AccentunderAttr())
    {
      _accentUnder.reset();
      affectsLayout = true;
    }

    if (affectsLayout)
    {
      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }

    MathMLScriptsElement::AttributeChanged(name, oldValue, value);
  }
}
