#include "Krystal.Booey/MathML/MathMLPaddedElement.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::mathml
{
  const MathMLElement::Length &MathMLPaddedElement::Width() noexcept
  {
    return CachedMathMLLength(MathMLNames::WidthAttr(), _width);
  }

  const MathMLElement::Length &MathMLPaddedElement::Height() noexcept
  {
    return CachedMathMLLength(MathMLNames::HeightAttr(), _height);
  }

  const MathMLElement::Length &MathMLPaddedElement::Depth() noexcept
  {
    return CachedMathMLLength(MathMLNames::DepthAttr(), _depth);
  }

  const MathMLElement::Length &MathMLPaddedElement::Lspace() noexcept
  {
    return CachedMathMLLength(MathMLNames::LspaceAttr(), _lspace);
  }

  const MathMLElement::Length &MathMLPaddedElement::Voffset() noexcept
  {
    return CachedMathMLLength(MathMLNames::VoffsetAttr(), _voffset);
  }

  void MathMLPaddedElement::AttributeChanged(const dom::QualifiedName &name,
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
    else if (name == MathMLNames::LspaceAttr())
    {
      _lspace.reset();
      affectsLayout = true;
    }
    else if (name == MathMLNames::VoffsetAttr())
    {
      _voffset.reset();
      affectsLayout = true;
    }

    if (affectsLayout)
    {
      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }

    MathMLRowElement::AttributeChanged(name, oldValue, value);
  }
}
