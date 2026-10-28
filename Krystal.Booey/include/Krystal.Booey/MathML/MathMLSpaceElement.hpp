#pragma once

#include "Krystal.Booey/MathML/MathMLPresentationElement.hpp"

namespace krys::boo::mathml
{
  /// @brief The `<mspace>` element: blank space of a given size.
  /// @see https://w3c.github.io/mathml-core/#space-mspace
  class MathMLSpaceElement final : public MathMLPresentationElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLSpaceElement);

  private:
    Maybe<Length> _width;
    Maybe<Length> _height;
    Maybe<Length> _depth;

  public:
    explicit MathMLSpaceElement(dom::Document &document) noexcept
        : MathMLPresentationElement(document, MathMLElementInterface::Space)
    {
    }

    KRYS_NODISCARD const Length &Width() noexcept;
    KRYS_NODISCARD const Length &Height() noexcept;
    KRYS_NODISCARD const Length &Depth() noexcept;

    // FIXME: The element is made a RenderMathMLSpace, once there are MathML renderers.

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLSpaceElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Space;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
