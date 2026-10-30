#pragma once

#include "Krystal.Core/Types/List.hpp"
#include <algorithm>
#include <ranges>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A span along a line, from `Begin` to `End`.
  struct FloatSegment
  {
    float Begin {0.f};
    float End {0.f};

    KRYS_NODISCARD constexpr bool operator==(const FloatSegment &) const noexcept = default;

    KRYS_NODISCARD constexpr float Length() const noexcept
    {
      return End - Begin;
    }

    KRYS_NODISCARD constexpr FloatSegment Dilate(float length) const noexcept
    {
      return {Begin - length, End + length};
    }
  };

  // Computes difference between a line segment `a` and list of line segments `bs` that are dilated by
  // dilationAmount.
  // The difference list is further filtered to only results > dilationAmount.
  //
  // List<FloatSegment> intersections = ...;
  // List<FloatSegment> solidLine { { 0, totalWidth } };
  // List<FloatSegment> diff = difference(solidLine, dilate(intersections, dilationAmount));
  // return filter(diff, [](auto&& s) { return s.length() > dilationAmount });
  // <=>
  // return DifferenceWithDilation({ 0, totalWidth }, intersections);
  KRYS_NODISCARD inline List<FloatSegment> DifferenceWithDilation(FloatSegment a, List<FloatSegment> &&bs,
                                                                  float dilationAmount) noexcept
  {
    std::ranges::sort(bs, {}, &FloatSegment::Begin);
    auto result = bs.begin();
    for (auto b : bs)
    {
      if (a.Begin >= a.End)
      {
        break;
      }
      b = b.Dilate(dilationAmount);
      if (b.Begin >= a.End)
      {
        break;
      }
      FloatSegment candidate {a.Begin, b.Begin};
      if (candidate.Length() > dilationAmount)
      {
        *result = candidate;
        result = std::next(result);
      }
      a.Begin = std::max(a.Begin, b.End);
    }
    bs.erase(result, bs.end());
    if (a.Length() > dilationAmount)
    {
      bs.push_back(a);
    }
    return krys::move(bs);
  }

  TextStream &operator<<(TextStream &ts, FloatSegment floatSegment) noexcept;
}
