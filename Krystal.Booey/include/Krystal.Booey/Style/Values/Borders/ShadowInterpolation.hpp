#pragma once

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"

namespace krys::boo::style
{
  // Generic implementation of interpolation for shadow lists for use by both `style::BoxShadow` and
  // `style::TextShadow`. https://www.w3.org/TR/web-animations-1/#animating-shadow-lists

  template <typename ShadowsType, typename MatchingShadows>
  struct ShadowInterpolation
  {
    using ShadowListType = typename ShadowsType::List;
    using ShadowType = typename ShadowListType::value_type;

    KRYS_NODISCARD static bool CanInterpolate(const ShadowsType &fromShadows, const ShadowsType &toShadows,
                                              CompositeOperation compositeOperation) noexcept
    {
      if (compositeOperation != CompositeOperation::Replace)
      {
        return true;
      }

      // The only scenario where we can't interpolate is if specified items don't have the same shadow style.

      auto fromLength = fromShadows.size();
      auto toLength = toShadows.size();

      // FIXME: Something like LLVM ADT's zip_shortest
      // (https://llvm.org/doxygen/structllvm_1_1detail_1_1zip__shortest.html) would allow this to be done
      // without indexing:
      //
      // return std::ranges::all_of(
      //     zip_shortest(fromShadows | std::views::reverse, toShadows | std::views::reverse),
      //     [](const auto& pair) {
      //         return shadowStyle(std::get<0>(pair)) == shadowStyle(std::get<1>(pair));
      //     }
      // );

      size_t minLength = std::min(fromLength, toLength);
      for (size_t i = 0; i < minLength; ++i)
      {
        auto fromIndex = fromLength - i - 1;
        auto toIndex = toLength - i - 1;
        if (GetShadowStyle(fromShadows[fromIndex]) != GetShadowStyle(toShadows[toIndex]))
        {
          return false;
        }
      }
      return true;
    }

    KRYS_NODISCARD static ShadowsType Interpolate(const ShadowsType &fromShadows,
                                                  const ShadowsType &toShadows, const RenderStyle &fromStyle,
                                                  const RenderStyle &toStyle,
                                                  const BlendingContext &context) noexcept
    {
      if (context.IsDiscrete)
      {
        krys_debug_assert(!context.Progress || context.Progress == 1.0);
        return context.Progress ? toShadows : fromShadows;
      }

      auto fromLength = fromShadows.size();
      auto toLength = toShadows.size();

      if (!fromLength && !toLength)
      {
        return css::keywords::None {};
      }
      if (fromLength == toLength)
      {
        return BlendMatchedShadowLists(fromShadows, toShadows, fromLength, fromStyle, toStyle, context);
      }
      return BlendMismatchedShadowLists(fromShadows, toShadows, fromLength, toLength, fromStyle, toStyle,
                                        context);
    }

    KRYS_NODISCARD static ShadowsType AddShadowLists(const ShadowsType &fromShadows,
                                                     const ShadowsType &toShadows, size_t fromLength,
                                                     size_t toLength) noexcept
    {
      return ShadowListType::createWithSizeFromGenerator(fromLength + toLength,
                                                         [&](auto index) -> ShadowType
                                                         {
                                                           if (index < toLength)
                                                           {
                                                             return toShadows[index];
                                                           }
                                                           return fromShadows[index - toLength];
                                                         });
    }

    KRYS_NODISCARD static ShadowsType BlendMatchedShadowLists(const ShadowsType &fromShadows,
                                                              const ShadowsType &toShadows, size_t length,
                                                              const RenderStyle &fromStyle,
                                                              const RenderStyle &toStyle,
                                                              const BlendingContext &context) noexcept
    {
      // `from` or `to` might be empty in which case we don't want to do additivity, but do replace instead.
      if (!fromShadows.IsNone() && !toShadows.IsNone()
          && context.CompositeOperation == CompositeOperation::Add)
      {
        return AddShadowLists(fromShadows, toShadows, length, length);
      }

      return ShadowListType::createWithSizeFromGenerator(
        length, [&](auto index) -> ShadowType
        { return style::Blend(fromShadows[index], toShadows[index], fromStyle, toStyle, context); });
    }

    KRYS_NODISCARD static ShadowsType
      BlendMismatchedShadowLists(const ShadowsType &fromShadows, const ShadowsType &toShadows,
                                 size_t fromLength, size_t toLength, const RenderStyle &fromStyle,
                                 const RenderStyle &toStyle, const BlendingContext &context) noexcept
    {
      if (!fromShadows.IsNone() && !toShadows.IsNone()
          && context.CompositeOperation != CompositeOperation::Replace)
      {
        return AddShadowLists(fromShadows, toShadows, fromLength, toLength);
      }

      auto maxLength = std::max(fromLength, toLength);
      return ShadowListType::createWithSizeFromGenerator(
        maxLength,
        [&](auto index) -> ShadowType
        {
          auto indexFromEnd = maxLength - index - 1;
          bool hasFrom = indexFromEnd < fromLength;
          bool hasTo = indexFromEnd < toLength;

          if (hasFrom && hasTo)
          {
            const auto &fromShadow = fromShadows[index - (maxLength - fromLength)];
            const auto &toShadow = toShadows[index - (maxLength - toLength)];
            return style::Blend(fromShadow, toShadow, fromStyle, toStyle, context);
          }
          else if (hasFrom)
          {
            const auto &fromShadow = fromShadows[index - (maxLength - fromLength)];
            const auto &toShadow = MatchingShadows::ShadowForInterpolation(fromShadow);
            return style::Blend(fromShadow, toShadow, fromStyle, toStyle, context);
          }
          else if (hasTo)
          {
            const auto &toShadow = toShadows[index - (maxLength - toLength)];
            const auto &fromShadow = MatchingShadows::ShadowForInterpolation(toShadow);
            return style::Blend(fromShadow, toShadow, fromStyle, toStyle, context);
          }

          krys_unreachable();
        });
    }
  };
}
