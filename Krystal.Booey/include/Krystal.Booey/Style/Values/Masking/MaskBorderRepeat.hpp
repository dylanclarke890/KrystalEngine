#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'mask-border-repeat'> = [ stretch | repeat | round | space ]{1,2}
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border-repeat
  struct MaskBorderRepeat
  {
    css::MinimallySerialisingSpaceSeparatedSize<NinePieceImageRule> Values {NinePieceImageRule::Stretch,
                                                                            NinePieceImageRule::Stretch};

    constexpr MaskBorderRepeat(
      css::MinimallySerialisingSpaceSeparatedSize<NinePieceImageRule> values) noexcept
        : Values {values}
    {
    }
    constexpr MaskBorderRepeat(NinePieceImageRule horizontalRule, NinePieceImageRule verticalRule) noexcept
        : Values {horizontalRule, verticalRule}
    {
    }
    constexpr MaskBorderRepeat(NinePieceImageRule rule) noexcept : Values {rule, rule}
    {
    }
    constexpr MaskBorderRepeat(css::keywords::Stretch) noexcept
        : MaskBorderRepeat {NinePieceImageRule::Stretch}
    {
    }
    constexpr MaskBorderRepeat(css::keywords::Repeat) noexcept : MaskBorderRepeat {NinePieceImageRule::Repeat}
    {
    }
    constexpr MaskBorderRepeat(css::keywords::Round) noexcept : MaskBorderRepeat {NinePieceImageRule::Round}
    {
    }
    constexpr MaskBorderRepeat(css::keywords::Space) noexcept : MaskBorderRepeat {NinePieceImageRule::Space}
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

    KRYS_NODISCARD constexpr bool operator==(const MaskBorderRepeat &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(MaskBorderRepeat, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MaskBorderRepeat>
  {
    KRYS_NODISCARD MaskBorderRepeat operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::MaskBorderRepeat)
