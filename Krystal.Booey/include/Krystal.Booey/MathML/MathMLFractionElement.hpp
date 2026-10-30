#pragma once

#include "Krystal.Booey/MathML/MathMLRowElement.hpp"

namespace krys::boo::mathml
{
  /// @brief The `<mfrac>` element: a fraction, its numerator centred over its denominator.
  /// @see https://w3c.github.io/mathml-core/#fractions-mfrac
  class MathMLFractionElement final : public MathMLRowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLFractionElement);

  private:
    Maybe<Length> _lineThickness;

  public:
    explicit MathMLFractionElement(dom::Document &document) noexcept
        : MathMLRowElement(document, MathMLElementInterface::Fraction)
    {
    }

    KRYS_NODISCARD const Length &LineThickness() noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLFractionElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Fraction;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
