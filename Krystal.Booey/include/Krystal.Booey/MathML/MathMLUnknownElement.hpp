#pragma once

#include "Krystal.Booey/MathML/MathMLRowElement.hpp"

namespace krys::boo::mathml
{
  /// @brief An element in the MathML namespace that MathML Core doesn't define, laid out as `<mrow>` is.
  /// @see https://w3c.github.io/mathml-core/#new-mathml-elements
  class MathMLUnknownElement final : public MathMLRowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLUnknownElement);

  public:
    explicit MathMLUnknownElement(dom::Document &document) noexcept
        : MathMLRowElement(document, MathMLElementInterface::Unknown, dom::NodeFlags::IsUnknownElement)
    {
    }

  private:
    KRYS_NODISCARD bool IsPresentationMathML() const noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLUnknownElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Unknown;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
