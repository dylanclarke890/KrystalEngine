#pragma once

#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::mathml
{
  /// @brief A presentation MathML element: one that lays out its children (as `<mtable>`, `<mtr>`,
  /// `<mtd>`, `<mprescripts>` and `<none>` do) and the base of the other presentation elements.
  class MathMLPresentationElement : public MathMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLPresentationElement);

  public:
    explicit MathMLPresentationElement(
      dom::Document &document, MathMLElementInterface interface = MathMLElementInterface::Presentation,
      dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;

    /// @brief Parses a MathML length, as MathML Core does: a number and an optional unit, with a unitless
    /// value only allowed for "0".
    /// @return A length of type `ParsingFailed` if @p string isn't one.
    KRYS_NODISCARD static Length ParseMathMLLength(dom::DOMStringView string) noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;

  protected:
    KRYS_NODISCARD static Maybe<bool> ToOptionalBool(BooleanValue value) noexcept
    {
      return value == BooleanValue::Default ? Maybe<bool> {} : Maybe<bool> {value == BooleanValue::True};
    }

    KRYS_NODISCARD BooleanValue CachedBooleanAttribute(const dom::QualifiedName &name,
                                                       Maybe<BooleanValue> &attribute) const noexcept;
    KRYS_NODISCARD const Length &CachedMathMLLength(const dom::QualifiedName &name,
                                                    Maybe<Length> &length) const noexcept;

  private:
    KRYS_NODISCARD bool IsPresentationMathML() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD static Length ParseNumberAndUnit(dom::DOMStringView string) noexcept;
  };
}
