#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'text-emphasis-style'> = none | [ [ filled | open ] || [ dot | circle | double-circle | triangle |
  // sesame ] ] | <string> https://drafts.csswg.org/css-text-decor/#propdef-text-emphasis-style
  struct TextEmphasisStyle
  {
    struct Shape
    {
      TextEmphasisFill Fill {TextEmphasisFill::Filled};
      TextEmphasisMark Mark;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

        if (Fill == TextEmphasisFill::Filled)
        {
          return visitor(Mark);
        }
        return visitor(css::SpaceSeparatedTuple {Fill, Mark});
      }

      KRYS_NODISCARD bool operator==(const Shape &) const noexcept = default;
    };

    Variant<css::keywords::None, Shape, css::CSSOMStringAtom> Value;

    TextEmphasisStyle(css::keywords::None keyword) noexcept : Value {keyword}
    {
    }

    TextEmphasisStyle(Shape shape) noexcept : Value {shape}
    {
    }

    TextEmphasisStyle(css::CSSOMStringAtom &&customMark) noexcept : Value {krys::move(customMark)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return HoldsAlternative<css::keywords::None>(Value);
    }

    // css::CSSOMString representation of the mark.
    KRYS_NODISCARD const css::CSSOMStringAtom &MarkString() const noexcept;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const TextEmphasisStyle &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TextEmphasisStyle>
  {
    KRYS_NODISCARD TextEmphasisStyle operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextEmphasisStyle::Shape)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextEmphasisStyle)
