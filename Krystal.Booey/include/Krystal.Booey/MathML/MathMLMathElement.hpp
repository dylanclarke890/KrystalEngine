#pragma once

#include "Krystal.Booey/MathML/MathMLRowElement.hpp"

namespace krys::boo::mathml
{
  /// @brief The `<math>` element: the root of a formula.
  /// @see https://w3c.github.io/mathml-core/#the-top-level-math-element
  class MathMLMathElement final : public MathMLRowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLMathElement);

  public:
    explicit MathMLMathElement(dom::Document &document) noexcept
        : MathMLRowElement(document, MathMLElementInterface::Math)
    {
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLMathElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Math;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
