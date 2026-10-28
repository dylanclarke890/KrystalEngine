#pragma once

#include "Krystal.Booey/MathML/MathMLRowElement.hpp"

namespace krys::boo::mathml
{
  /// @brief The `<mpadded>` element: a row with its size and position adjusted.
  /// @see https://w3c.github.io/mathml-core/#adjust-space-around-content-mpadded
  class MathMLPaddedElement final : public MathMLRowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLPaddedElement);

  private:
    Maybe<Length> _width;
    Maybe<Length> _height;
    Maybe<Length> _depth;
    Maybe<Length> _lspace;
    Maybe<Length> _voffset;

  public:
    explicit MathMLPaddedElement(dom::Document &document) noexcept
        : MathMLRowElement(document, MathMLElementInterface::Padded)
    {
    }

    // FIXME: Pseudo-units are not supported yet (https://bugs.webkit.org/show_bug.cgi?id=85730).
    KRYS_NODISCARD const Length &Width() noexcept;
    KRYS_NODISCARD const Length &Height() noexcept;
    KRYS_NODISCARD const Length &Depth() noexcept;
    KRYS_NODISCARD const Length &Lspace() noexcept;
    KRYS_NODISCARD const Length &Voffset() noexcept;

    // FIXME: The element is made a RenderMathMLPadded, once there are MathML renderers.

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLPaddedElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Padded;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
