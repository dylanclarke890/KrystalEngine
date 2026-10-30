#pragma once

#include "Krystal.Booey/MathML/MathMLScriptsElement.hpp"

namespace krys::boo::mathml
{
  /// @brief The `<munder>`, `<mover>` and `<munderover>` elements: a base with scripts below and above it.
  /// @see https://w3c.github.io/mathml-core/#underscripts-and-overscripts-munder-mover-munderover
  class MathMLUnderOverElement final : public MathMLScriptsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLUnderOverElement);

  private:
    Maybe<BooleanValue> _accent;
    Maybe<BooleanValue> _accentUnder;

  public:
    MathMLUnderOverElement(dom::Document &document, ScriptType scriptType) noexcept
        : MathMLScriptsElement(document, scriptType, MathMLElementInterface::UnderOver)
    {
    }

    KRYS_NODISCARD BooleanValue Accent() noexcept;
    KRYS_NODISCARD BooleanValue AccentUnder() noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLUnderOverElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::UnderOver;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
