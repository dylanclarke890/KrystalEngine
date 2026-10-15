#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <repeat-style> = repeat-x@(alias=[repeat no-repeat]) | repeat-y@(alias=[no-repeat repeat]) | [repeat |
  // space | round | no-repeat]{1,2} https://www.w3.org/TR/css-backgrounds-3/#typedef-repeat-style
  struct RepeatStyle
  {
    css::MinimallySerialisingSpaceSeparatedPoint<FillRepeat> Values;

    KRYS_NODISCARD FillRepeat X() const noexcept
    {
      return Values.x();
    }
    KRYS_NODISCARD FillRepeat Y() const noexcept
    {
      return Values.y();
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (X() == Y())
      {
        return visitor(X());
      }
      if (X() == FillRepeat::Repeat && Y() == FillRepeat::NoRepeat)
      {
        return visitor(css::keywords::RepeatX {});
      }
      if (X() == FillRepeat::NoRepeat && Y() == FillRepeat::Repeat)
      {
        return visitor(css::keywords::RepeatY {});
      }
      return visitor(Values);
    }

    KRYS_NODISCARD bool operator==(const RepeatStyle &) const noexcept = default;
    bool operator==(FillRepeat other) const noexcept
    {
      return X() == other && Y() == other;
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<RepeatStyle>
  {
    KRYS_NODISCARD RepeatStyle operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::RepeatStyle)
