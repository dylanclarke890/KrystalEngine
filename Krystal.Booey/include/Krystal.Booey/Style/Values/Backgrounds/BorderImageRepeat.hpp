#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'border-image-repeat'> = [ stretch | repeat | round | space ]{1,2}
  // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-repeat
  struct BorderImageRepeat
  {
    css::MinimallySerialisingSpaceSeparatedSize<NinePieceImageRule> Values {NinePieceImageRule::Stretch,
                                                                            NinePieceImageRule::Stretch};

    constexpr BorderImageRepeat(
      css::MinimallySerialisingSpaceSeparatedSize<NinePieceImageRule> values) noexcept
        : Values {values}
    {
    }
    constexpr BorderImageRepeat(NinePieceImageRule horizontalRule, NinePieceImageRule verticalRule) noexcept
        : Values {horizontalRule, verticalRule}
    {
    }
    constexpr BorderImageRepeat(NinePieceImageRule rule) noexcept : Values {rule, rule}
    {
    }
    constexpr BorderImageRepeat(css::keywords::Stretch) noexcept
        : BorderImageRepeat {NinePieceImageRule::Stretch}
    {
    }
    constexpr BorderImageRepeat(css::keywords::Repeat) noexcept
        : BorderImageRepeat {NinePieceImageRule::Repeat}
    {
    }
    constexpr BorderImageRepeat(css::keywords::Round) noexcept : BorderImageRepeat {NinePieceImageRule::Round}
    {
    }
    constexpr BorderImageRepeat(css::keywords::Space) noexcept : BorderImageRepeat {NinePieceImageRule::Space}
    {
    }

    KRYS_NODISCARD constexpr NinePieceImageRule HorizontalRule() const noexcept
    {
      return Values.Width();
    }
    KRYS_NODISCARD constexpr NinePieceImageRule VerticalRule() const noexcept
    {
      return Values.Height();
    }

    KRYS_NODISCARD constexpr bool operator==(const BorderImageRepeat &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(BorderImageRepeat, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<BorderImageRepeat>
  {
    KRYS_NODISCARD BorderImageRepeat operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::BorderImageRepeat)
