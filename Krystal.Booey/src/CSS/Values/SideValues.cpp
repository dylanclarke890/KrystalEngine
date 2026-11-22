#include "Krystal.Booey/CSS/Values/SideValues.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"

namespace krys::boo::css
{
  bool SideValues::operator==(const SideValues &other) const noexcept
  {
    return Top->Equals(*other.Top) && Right->Equals(*other.Right) && Bottom->Equals(*other.Bottom)
           && Left->Equals(*other.Left);
  }

  bool SideValues::AreAllEqual() const noexcept
  {
    return Top->Equals(*Right) && Top->Equals(*Bottom) && Top->Equals(*Left);
  }

  CSSOMString SerialiseSides(const CSSOMString &top, const CSSOMString &right, const CSSOMString &bottom,
                             const CSSOMString &left) noexcept
  {
    // One value gives every side; two give top and bottom, then right and left; three give top, right and
    // left, then bottom. So drop the left side if it's the right side's, then the bottom if it's the top's,
    // then the right if it's the top's.
    const CSSOMString *sides[] {&top, &right, &bottom, &left};
    uint32 count = 4;
    if (left == right)
    {
      count = 3;
      if (bottom == top)
      {
        count = 2;
        if (right == top)
        {
          count = 1;
        }
      }
    }

    StringBuilder builder;
    for (uint32 index = 0; index < count; ++index)
    {
      if (index > 0)
      {
        builder.Append(u8' ');
      }
      builder.Append(*sides[index]);
    }
    return krys::move(builder).ToString();
  }

  CSSOMString SerialiseSides(const SideValues &sides, const SerialisationContext &context) noexcept
  {
    return SerialiseSides(sides.Top->CssText(context), sides.Right->CssText(context),
                          sides.Bottom->CssText(context), sides.Left->CssText(context));
  }

  CSSOMString SerialiseRect(const SideValues &sides, const SerialisationContext &context) noexcept
  {
    StringBuilder builder;
    builder.Append(u8"rect(", sides.Top->CssText(context), u8", ", sides.Right->CssText(context), u8", ",
                   sides.Bottom->CssText(context), u8", ", sides.Left->CssText(context), u8')');
    return krys::move(builder).ToString();
  }
}
