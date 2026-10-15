#include "Krystal.Booey/Style/Values/Images/Gradient.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/Graphics/Color/ColorInterpolation.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/GradientColorStop.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>

namespace krys::boo::style
{
  // MARK: - IsRepeatingGradient

  template <css::ValueId>
  constexpr bool IsRepeatingGradient = false;
  template <>
  constexpr bool IsRepeatingGradient<css::ValueId::RepeatingLinearGradient> = true;
  template <>
  constexpr bool IsRepeatingGradient<css::ValueId::WebkitRepeatingLinearGradient> = true;
  template <>
  constexpr bool IsRepeatingGradient<css::ValueId::RepeatingRadialGradient> = true;
  template <>
  constexpr bool IsRepeatingGradient<css::ValueId::WebkitRepeatingRadialGradient> = true;
  template <>
  constexpr bool IsRepeatingGradient<css::ValueId::RepeatingConicGradient> = true;

  template <css::ValueId Name, typename T>
  KRYS_NODISCARD constexpr static bool IsRepeating(const css::FunctionNotation<Name, T> &) noexcept
  {
    return IsRepeatingGradient<Name>;
  }

  // MARK: - Conversion: Style -> CSS

  template <typename CSSStop, typename StyleStop>
  KRYS_NODISCARD static CSSStop ToCSSColorStop(const StyleStop &stop, const RenderStyle &style) noexcept
  {
    return CSSStop {ConvertToCSS(stop.Color, style), ConvertToCSS(stop.Position, style)};
  }

  css::GradientAngularColorStop
    ToCSS<GradientAngularColorStop>::operator()(const GradientAngularColorStop &stop,
                                                const RenderStyle &style) noexcept
  {
    return ToCSSColorStop<css::GradientAngularColorStop>(stop, style);
  }

  css::GradientLinearColorStop ToCSS<GradientLinearColorStop>::operator()(const GradientLinearColorStop &stop,
                                                                          const RenderStyle &style) noexcept
  {
    return ToCSSColorStop<css::GradientLinearColorStop>(stop, style);
  }

  css::GradientDeprecatedColorStop
    ToCSS<GradientDeprecatedColorStop>::operator()(const GradientDeprecatedColorStop &stop,
                                                   const RenderStyle &style) noexcept
  {
    return ToCSSColorStop<css::GradientDeprecatedColorStop>(stop, style);
  }

  // MARK: - Conversion: CSS -> Style

  template <typename T>
  decltype(auto) ToStyleColorStop(const T &stop, const BuilderState &state) noexcept
  {
    return GradientColorStop {ConvertToStyle(stop.Color, state), ConvertToStyle(stop.Position, state)};
  }

  GradientAngularColorStop
    ToStyle<css::GradientAngularColorStop>::operator()(const css::GradientAngularColorStop &stop,
                                                       const BuilderState &state) noexcept
  {
    return ToStyleColorStop(stop, state);
  }

  GradientLinearColorStop
    ToStyle<css::GradientLinearColorStop>::operator()(const css::GradientLinearColorStop &stop,
                                                      const BuilderState &state) noexcept
  {
    return ToStyleColorStop(stop, state);
  }

  GradientDeprecatedColorStop
    ToStyle<css::GradientDeprecatedColorStop>::operator()(const css::GradientDeprecatedColorStop &stop,
                                                          const BuilderState &state) noexcept
  {
    return ToStyleColorStop(stop, state);
  }

  // MARK: - Platform Gradient Resolution

  KRYS_NODISCARD static boo::Color ResolveColorStopColor(const Color &styleColor, const RenderStyle &style,
                                                         bool hasColorFilter) noexcept
  {
    if (hasColorFilter)
    {
      return style.ColorWithColorFilter(styleColor);
    }
    return style.ColorResolvingCurrentColor(styleColor);
  }

  KRYS_NODISCARD static boo::Color ResolveColorStopColor(const Markable<Color> &styleColor,
                                                         const RenderStyle &style,
                                                         bool hasColorFilter) noexcept
  {
    if (!styleColor)
    {
      return {};
    }
    return ResolveColorStopColor(*styleColor, style, hasColorFilter);
  }

  KRYS_NODISCARD static Maybe<float>
    ResolveColorStopPosition(const GradientLinearColorStop::PositionType &position,
                             float gradientLength) noexcept
  {
    if (!position)
    {
      return std::nullopt;
    }

    return krys::SwitchOn(
      *position,
      [&](const typename LengthPercentage<>::Dimension &length) -> Maybe<float>
      {
        if (gradientLength <= 0)
        {
          return 0.0f;
        }
        return length.ResolveZoom(style::ZoomNeeded {}) / gradientLength;
      },
      [&](const typename LengthPercentage<>::Percentage &percentage) -> Maybe<float>
      { return static_cast<float>(percentage.Value / 100.0); },
      [&](const typename LengthPercentage<>::Calc &calc) -> Maybe<float>
      {
        if (gradientLength <= 0)
        {
          return 0.0f;
        }
        return style::Evaluate<float>(calc, gradientLength, style::ZoomNeeded {}) / gradientLength;
      });
  }

  KRYS_NODISCARD static Maybe<float>
    ResolveColorStopPosition(const GradientAngularColorStop::PositionType &position, float) noexcept
  {
    if (!position)
    {
      return std::nullopt;
    }

    return krys::SwitchOn(
      *position, [](const typename AnglePercentage<>::Dimension &angle) -> Maybe<float>
      { return static_cast<float>(angle.Value / 360.0); },
      [](const typename AnglePercentage<>::Percentage &percentage) -> Maybe<float>
      { return static_cast<float>(percentage.Value / 100.0); },
      [&](const typename AnglePercentage<>::Calc &calc) -> Maybe<float>
      { return style::Evaluate<float>(calc, 100.0f, style::ZoomNeeded {}); });
  }

  KRYS_NODISCARD static float
    ResolveColorStopPosition(const GradientDeprecatedColorStop::PositionType &position) noexcept
  {
    return static_cast<float>(position.Value.Value);
  }

  struct ResolvedGradientStop
  {
    boo::Color Color;
    Maybe<float> Offset;

    KRYS_NODISCARD bool IsSpecified() const noexcept
    {
      return Offset.has_value();
    }
    KRYS_NODISCARD bool IsMidpoint() const noexcept
    {
      return !Color.IsValid();
    }
  };

  class LinearGradientAdapter
  {
  public:
    explicit LinearGradientAdapter(boo::Gradient::LinearData &data) noexcept : _data(data)
    {
    }

    KRYS_NODISCARD float GradientLength() const noexcept
    {
      auto gradientSize = _data.Point0 - _data.Point1;
      return gradientSize.DiagonalLength();
    }

    KRYS_NODISCARD constexpr static float MaxExtent(float) noexcept
    {
      return 1;
    }

    void NormalizeStopsAndEndpointsOutsideRange(List<ResolvedGradientStop> &stops,
                                                ColorInterpolationMethod) noexcept
    {
      float firstOffset = *stops.front().Offset;
      float lastOffset = *stops.back().Offset;
      if (firstOffset != lastOffset)
      {
        float scale = lastOffset - firstOffset;

        for (auto &stop : stops)
        {
          stop.Offset = (*stop.Offset - firstOffset) / scale;
        }

        auto p0 = _data.Point0;
        auto p1 = _data.Point1;
        _data.Point0 = {p0.X() + firstOffset * (p1.X() - p0.X()), p0.Y() + firstOffset * (p1.Y() - p0.Y())};
        _data.Point1 = {p1.X() + (lastOffset - 1) * (p1.X() - p0.X()),
                        p1.Y() + (lastOffset - 1) * (p1.Y() - p0.Y())};
      }
      else
      {
        // There's a single position that is outside the scale, clamp the positions to 1.
        for (auto &stop : stops)
        {
          stop.Offset = 1.0f;
        }
      }
    }

  private:
    boo::Gradient::LinearData &_data;
  };

  class RadialGradientAdapter
  {
  public:
    explicit RadialGradientAdapter(boo::Gradient::RadialData &data, FloatSize size) noexcept
        : _data(data), _size(size)
    {
    }

    KRYS_NODISCARD float GradientLength() const noexcept
    {
      return _data.EndRadius;
    }

    // Radial gradients may need to extend further than the endpoints, because they have
    // to repeat out to the corners of the box.
    KRYS_NODISCARD float MaxExtent(float gradientLength) const noexcept
    {
      auto maxLengthForRepeat = DistanceToFarthestCorner(_data.Point1, _size);
      if (maxLengthForRepeat > gradientLength)
      {
        return gradientLength > 0 ? maxLengthForRepeat / gradientLength : 0;
      }
      return 1;
    }

    void NormalizeStopsAndEndpointsOutsideRange(List<ResolvedGradientStop> &stops,
                                                ColorInterpolationMethod colorInterpolationMethod) noexcept
    {
      auto numberOfStops = stops.size();

      // Rather than scaling the points < 0, we truncate them, so only scale according to the largest point.
      float firstOffset = 0;
      float lastOffset = *stops.back().Offset;
      float scale = lastOffset - firstOffset;

      // Reset points below 0 to the first visible color.
      size_t firstZeroOrGreaterIndex = numberOfStops;
      for (size_t i = 0; i < numberOfStops; ++i)
      {
        if (*stops[i].Offset >= 0)
        {
          firstZeroOrGreaterIndex = i;
          break;
        }
      }

      if (firstZeroOrGreaterIndex > 0)
      {
        if (firstZeroOrGreaterIndex < numberOfStops && *stops[firstZeroOrGreaterIndex].Offset > 0)
        {
          float prevOffset = *stops[firstZeroOrGreaterIndex - 1].Offset;
          float nextOffset = *stops[firstZeroOrGreaterIndex].Offset;

          float interStopProportion = -prevOffset / (nextOffset - prevOffset);
          auto blendedColor = InterpolateColors(
            colorInterpolationMethod, stops[firstZeroOrGreaterIndex - 1].Color, 1.0f - interStopProportion,
            stops[firstZeroOrGreaterIndex].Color, interStopProportion);

          // Clamp the positions to 0 and set the color.
          for (size_t i = 0; i < firstZeroOrGreaterIndex; ++i)
          {
            stops[i].Offset = 0.0f;
            stops[i].Color = blendedColor;
          }
        }
        else
        {
          // All stops are below 0; just clamp them.
          for (size_t i = 0; i < firstZeroOrGreaterIndex; ++i)
          {
            stops[i].Offset = 0.0f;
          }
        }
      }

      for (auto &stop : stops)
      {
        *stop.Offset /= scale;
      }

      _data.StartRadius *= scale;
      _data.EndRadius *= scale;
    }

  private:
    boo::Gradient::RadialData &_data;
    FloatSize _size;
  };

  class ConicGradientAdapter
  {
  public:
    KRYS_NODISCARD constexpr static float GradientLength() noexcept
    {
      return 1;
    }
    KRYS_NODISCARD constexpr static float MaxExtent(float) noexcept
    {
      return 1;
    }

    void NormalizeStopsAndEndpointsOutsideRange(List<ResolvedGradientStop> &stops,
                                                ColorInterpolationMethod colorInterpolationMethod) noexcept
    {
      size_t numberOfStops = stops.size();
      size_t lastStopIndex = numberOfStops - 1;

      Maybe<size_t> firstZeroOrGreaterIndex;
      for (size_t i = 0; i < numberOfStops; ++i)
      {
        if (*stops[i].Offset >= 0)
        {
          firstZeroOrGreaterIndex = i;
          break;
        }
      }

      if (firstZeroOrGreaterIndex)
      {
        size_t index = *firstZeroOrGreaterIndex;
        if (index > 0)
        {
          float previousOffset = *stops[index - 1].Offset;
          float nextOffset = *stops[index].Offset;

          float interStopProportion = -previousOffset / (nextOffset - previousOffset);
          auto blendedColor =
            InterpolateColors(colorInterpolationMethod, stops[index - 1].Color, 1.0f - interStopProportion,
                              stops[index].Color, interStopProportion);

          // Clamp the positions to 0 and set the color.
          for (size_t i = 0; i < index; ++i)
          {
            stops[i].Offset = 0.0f;
            stops[i].Color = blendedColor;
          }
        }
      }
      else
      {
        // All stop offsets below 0, clamp them.
        for (auto &stop : stops)
        {
          stop.Offset = 0.0f;
        }
      }

      Maybe<size_t> lastOneOrLessIndex;
      for (auto i = static_cast<ptrdiff_t>(lastStopIndex); i >= 0; --i)
      {
        if (*stops[i].Offset <= 1)
        {
          lastOneOrLessIndex = i;
          break;
        }
      }

      if (lastOneOrLessIndex)
      {
        size_t index = *lastOneOrLessIndex;
        if (index < lastStopIndex)
        {
          float previousOffset = *stops[index].Offset;
          float nextOffset = *stops[index + 1].Offset;

          float interStopProportion = (1 - previousOffset) / (nextOffset - previousOffset);
          auto blendedColor =
            InterpolateColors(colorInterpolationMethod, stops[index].Color, 1.0f - interStopProportion,
                              stops[index + 1].Color, interStopProportion);

          // Clamp the positions to 1 and set the color.
          for (size_t i = index + 1; i < numberOfStops; ++i)
          {
            stops[i].Offset = 1.0f;
            stops[i].Color = blendedColor;
          }
        }
      }
      else
      {
        // All stop offsets above 1, clamp them.
        for (auto &stop : stops)
        {
          stop.Offset = 1.0f;
        }
      }
    }
  };

  template <typename GradientAdapter, typename StyleGradient>
  KRYS_NODISCARD GradientColorStops ComputeStopsForDeprecatedVariants(GradientAdapter &,
                                                                      const StyleGradient &styleGradient,
                                                                      const RenderStyle &style) noexcept
  {
    bool hasColorFilter = style.HasAppleColorFilter();
    GradientColorStops::StopVector result;
    for (const auto &stop : styleGradient.Parameters.Stops)
    {
      result.push_back(
        {ResolveColorStopPosition(stop.Position), ResolveColorStopColor(stop.Color, style, hasColorFilter)});
    }

    std::ranges::stable_sort(result, {}, &boo::GradientColorStop::Offset);

    return GradientColorStops::Sorted {krys::move(result)};
  }

  template <typename GradientAdapter, typename StyleGradient>
  KRYS_NODISCARD GradientColorStops ComputeStops(GradientAdapter &gradientAdapter,
                                                 const StyleGradient &styleGradient,
                                                 const RenderStyle &style) noexcept
  {
    bool hasColorFilter = style.HasAppleColorFilter();

    size_t numberOfStops = styleGradient.Parameters.Stops.size();
    List<ResolvedGradientStop> stops(numberOfStops);

    float gradientLength = gradientAdapter.GradientLength();

    for (size_t i = 0; i < numberOfStops; ++i)
    {
      auto &stop = styleGradient.Parameters.Stops[i];

      stops[i].Color = ResolveColorStopColor(stop.Color, style, hasColorFilter);

      auto offset = ResolveColorStopPosition(stop.Position, gradientLength);
      if (offset)
      {
        stops[i].Offset = *offset;
      }
      else
      {
        // If the first color-stop does not have a position, its position defaults to 0%.
        // If the last color-stop does not have a position, its position defaults to 100%.
        if (!i)
        {
          stops[i].Offset = 0.0f;
        }
        else if (numberOfStops > 1 && i == numberOfStops - 1)
        {
          stops[i].Offset = 1.0f;
        }
      }

      // If a color-stop has a position that is less than the specified position of any
      // color-stop before it in the list, its position is changed to be equal to the
      // largest specified position of any color-stop before it.
      if (stops[i].IsSpecified() && i > 0)
      {
        size_t prevSpecifiedIndex;
        for (prevSpecifiedIndex = i - 1; prevSpecifiedIndex; --prevSpecifiedIndex)
        {
          if (stops[prevSpecifiedIndex].IsSpecified())
          {
            break;
          }
        }

        if (*stops[i].Offset < *stops[prevSpecifiedIndex].Offset)
        {
          stops[i].Offset = stops[prevSpecifiedIndex].Offset;
        }
      }
    }

    krys_debug_assert(stops[0].IsSpecified() && stops[numberOfStops - 1].IsSpecified());

    // If any color-stop still does not have a position, then, for each run of adjacent
    // color-stops without positions, set their positions so that they are evenly spaced
    // between the preceding and following color-stops with positions.
    if (numberOfStops > 2)
    {
      size_t unspecifiedRunStart = 0;
      bool inUnspecifiedRun = false;

      for (size_t i = 0; i < numberOfStops; ++i)
      {
        if (!stops[i].IsSpecified() && !inUnspecifiedRun)
        {
          unspecifiedRunStart = i;
          inUnspecifiedRun = true;
        }
        else if (stops[i].IsSpecified() && inUnspecifiedRun)
        {
          size_t unspecifiedRunEnd = i;

          if (unspecifiedRunStart < unspecifiedRunEnd)
          {
            float lastSpecifiedOffset = *stops[unspecifiedRunStart - 1].Offset;
            float nextSpecifiedOffset = *stops[unspecifiedRunEnd].Offset;
            float delta =
              (nextSpecifiedOffset - lastSpecifiedOffset) / (unspecifiedRunEnd - unspecifiedRunStart + 1);

            for (size_t j = unspecifiedRunStart; j < unspecifiedRunEnd; ++j)
            {
              stops[j].Offset = lastSpecifiedOffset + (j - unspecifiedRunStart + 1) * delta;
            }
          }

          inUnspecifiedRun = false;
        }
      }
    }

    // Walk over the color stops, look for midpoints and add stops as needed.
    // If mid < 50%, add 2 stops to the left and 6 to the right
    // else add 6 stops to the left and 2 to the right.
    // Stops on the side with the most stops start midway because the curve approximates
    // a line in that region. We then add 5 more color stops on that side to minimize the change
    // how the luminance changes at each of the color stops. We don't have to add as many on the other side
    // since it becomes small which increases the differentiation of luminance which hides the color stops.
    // Even with 4 extra color stops, it *is* possible to discern the steps when the gradient is large and has
    // large luminance differences between midpoint and color stop. If this becomes an issue, we can consider
    // making this algorithm a bit smarter.

    // Midpoints that coincide with color stops are treated specially since they don't require
    // extra stops and generate hard lines.
    for (size_t x = 1; x < stops.size() - 1;)
    {
      if (!stops[x].IsMidpoint())
      {
        ++x;
        continue;
      }

      // Find previous and next color so we know what to interpolate between.
      // We already know they have a color since we checked for that earlier.
      auto color1 = stops[x - 1].Color;
      auto color2 = stops[x + 1].Color;
      // Likewise find the position of previous and next color stop.
      float offset1 = *stops[x - 1].Offset;
      float offset2 = *stops[x + 1].Offset;
      float offset = *stops[x].Offset;

      // Check if everything coincides or the midpoint is exactly in the middle.
      // If so, ignore the midpoint.
      if (offset - offset1 == offset2 - offset)
      {
        stops.erase(stops.begin() + static_cast<ptrdiff_t>(x));
        continue;
      }

      // Check if we coincide with the left color stop.
      if (offset1 == offset)
      {
        // Morph the midpoint to a regular stop with the color of the next color stop.
        stops[x].Color = color2;
        continue;
      }

      // Check if we coincide with the right color stop.
      if (offset2 == offset)
      {
        // Morph the midpoint to a regular stop with the color of the previous color stop.
        stops[x].Color = color1;
        continue;
      }

      float midpoint = (offset - offset1) / (offset2 - offset1);
      std::array<ResolvedGradientStop, 9> newStops;
      if (midpoint > .5f)
      {
        for (size_t y = 0; y < 6; ++y)
        {
          newStops[y].Offset = offset1 + (offset - offset1) * (7 + y) / 13;
        }

        newStops[6].Offset = offset;
        newStops[7].Offset = offset + (offset2 - offset) / 3;
        newStops[8].Offset = offset + (offset2 - offset) * 2 / 3;
      }
      else
      {
        newStops[0].Offset = offset1 + (offset - offset1) / 3;
        newStops[1].Offset = offset1 + (offset - offset1) * 2 / 3;
        newStops[2].Offset = offset;

        for (size_t y = 1; y < 7; ++y)
        {
          newStops[y + 2].Offset = offset + (offset2 - offset) * y / 13;
        }
      }
      // calculate colors
      for (size_t y = 0; y < 9; ++y)
      {
        float relativeOffset = (*newStops[y].Offset - offset1) / (offset2 - offset1);
        float multiplier = std::pow(relativeOffset, std::log(.5f) / std::log(midpoint));
        newStops[y].Color = InterpolateColors(styleGradient.Parameters.ColorInterpolationMethod.Method,
                                              color1, 1.0f - multiplier, color2, multiplier);
      }

      stops.erase(stops.begin() + static_cast<ptrdiff_t>(x));
      stops.insert(stops.begin() + static_cast<ptrdiff_t>(x), newStops.begin(), newStops.end());
      x += 9;
    }

    numberOfStops = stops.size();

    // If the gradient is repeating, repeat the color stops.
    // We can't just push this logic down into the platform-specific Gradient code,
    // because we have to know the extent of the gradient, and possible move the end points.
    if (IsRepeating(styleGradient) && numberOfStops > 1)
    {
      float maxExtent = gradientAdapter.MaxExtent(gradientLength);
      // If the difference in the positions of the first and last color-stops is 0,
      // the gradient defines a solid-color image with the color of the last color-stop in the rule.
      float gradientRange = *stops.back().Offset - *stops.front().Offset;
      if (maxExtent > 1)
      {
        gradientRange /= maxExtent;
      }
      if (!gradientRange)
      {
        stops.front().Offset = 0.0f;
        stops.front().Color = stops.back().Color;
        stops.resize(1);
        numberOfStops = 1;
      }
      else if (std::abs(gradientRange) < (1.0f / (2 << 15)))
      {
        // If the gradient range is too small, the subsequent replication of stops
        // across the complete [0, maxExtent] range can challenging to complete both
        // because of potentially-expensive initial traversal across the [0, first-offset]
        // and [last-offset, maxExtent] ranges as well as likely exorbitant memory consumption
        // needed for all such generated stops. In case of such a gradient range the initial
        // Vector of stops remains unchanged, and additional stops for the purpose of the
        // repeating nature of the gradient are not computed.
      }
      else
      {
        // Since the gradient range is deemed big enough, the amount of necessary stops is
        // calculated for both the [0, first-offset] and the [last-offset, maxExtent] ranges.
        size_t numberOfGeneratedStopsBeforeFirst = 0;
        size_t numberOfGeneratedStopsAfterLast = 0;

        if (*stops.front().Offset > 0)
        {
          float currOffset = *stops.front().Offset;
          size_t srcStopOrdinal = numberOfStops - 1;

          while (true)
          {
            ++numberOfGeneratedStopsBeforeFirst;
            if (currOffset < 0)
            {
              break;
            }

            if (srcStopOrdinal)
            {
              currOffset -= *stops[srcStopOrdinal].Offset - *stops[srcStopOrdinal - 1].Offset;
            }
            srcStopOrdinal = (srcStopOrdinal + numberOfStops - 1) % numberOfStops;
          }
        }

        if (*stops.back().Offset < maxExtent)
        {
          float currOffset = *stops.back().Offset;
          size_t srcStopOrdinal = 0;

          while (true)
          {
            ++numberOfGeneratedStopsAfterLast;
            if (currOffset > maxExtent)
            {
              break;
            }

            if (srcStopOrdinal < numberOfStops - 1)
            {
              currOffset += *stops[srcStopOrdinal + 1].Offset - *stops[srcStopOrdinal].Offset;
            }
            srcStopOrdinal = (srcStopOrdinal + 1) % numberOfStops;
          }
        }

        // With the number of stops necessary for the repeating gradient now known, we can impose
        // some reasonable limit to prevent generation of memory-expensive amounts of gradient stops.
        size_t numberOfGeneratedStops =
          numberOfStops + numberOfGeneratedStopsBeforeFirst + numberOfGeneratedStopsAfterLast;
        if (numberOfGeneratedStops > (2 << 15))
        {
          // More than 65536 gradient stops are expected. Let's fall back to the initially-provided
          // Vector of stops, effectively meaning the repetition of stops is not applied.
        }
        else
        {
          // An affordable amount of gradient stops is determined. A separate Vector object is constructed
          // accordingly, first generating the repeated stops in the [0, first-offset] range, then adding
          // the original stops, and finally generating the repeated stops in the [last-offset, maxExtent]
          // range. The resulting Vector is then moved in to replace the original stops.
          List<ResolvedGradientStop> generatedStops;
          generatedStops.reserve(numberOfGeneratedStops);

          if (numberOfGeneratedStopsBeforeFirst > 0)
          {
            float currOffset = *stops.front().Offset;
            size_t srcStopOrdinal = numberOfStops - 1;

            for (size_t i = 0; i < numberOfGeneratedStopsBeforeFirst; ++i)
            {
              auto newStop = stops[srcStopOrdinal];
              newStop.Offset = currOffset;
              generatedStops.push_back(newStop);

              if (srcStopOrdinal)
              {
                currOffset -= *stops[srcStopOrdinal].Offset - *stops[srcStopOrdinal - 1].Offset;
              }
              srcStopOrdinal = (srcStopOrdinal + numberOfStops - 1) % numberOfStops;
            }

            std::ranges::reverse(generatedStops);
          }

          generatedStops.insert(generatedStops.end(), stops.begin(), stops.end());

          if (numberOfGeneratedStopsAfterLast > 0)
          {
            float currOffset = *stops.back().Offset;
            size_t srcStopOrdinal = 0;

            for (size_t i = 0; i < numberOfGeneratedStopsAfterLast; ++i)
            {
              auto newStop = stops[srcStopOrdinal];
              newStop.Offset = currOffset;
              generatedStops.push_back(newStop);

              if (srcStopOrdinal < numberOfStops - 1)
              {
                currOffset += *stops[srcStopOrdinal + 1].Offset - *stops[srcStopOrdinal].Offset;
              }
              srcStopOrdinal = (srcStopOrdinal + 1) % numberOfStops;
            }
          }

          stops = krys::move(generatedStops);
        }
      }
    }

    // If the gradient goes outside the 0-1 range, normalize it by moving the endpoints, and adjusting the
    // stops.
    if (stops.size() > 1 && (*stops.front().Offset < 0 || *stops.back().Offset > 1))
    {
      gradientAdapter.NormalizeStopsAndEndpointsOutsideRange(
        stops, styleGradient.Parameters.ColorInterpolationMethod.Method);
    }

    GradientColorStops::StopVector result;
    result.reserve(stops.size());
    for (const auto &stop : stops)
    {
      result.push_back({*stop.Offset, stop.Color});
    }
    return GradientColorStops::Sorted {krys::move(result)};
  }

  KRYS_NODISCARD static inline float PositionFromValue(LengthWrapperBaseDerived auto const &coordinate,
                                                       float widthOrHeight) noexcept
  {
    return Evaluate<float>(coordinate, widthOrHeight, style::ZoomNeeded {});
  }

  KRYS_NODISCARD static inline float PositionFromValue(const NumberOrPercentage<> &coordinate,
                                                       float widthOrHeight) noexcept
  {
    return krys::SwitchOn(
      coordinate, [&](Number<> number) -> float { return static_cast<float>(number.Value); },
      [&](Percentage<> percentage) -> float
      { return static_cast<float>(percentage.Value) / 100.0f * widthOrHeight; });
  }

  template <typename Position>
  KRYS_NODISCARD static inline FloatPoint ComputeEndPoint(const Position &value,
                                                          const FloatSize &size) noexcept
  {
    return {PositionFromValue(get<0>(value), size.Width()), PositionFromValue(get<1>(value), size.Height())};
  }

  // Compute the endpoints so that a gradient of the given angle covers a box of the given size.
  KRYS_NODISCARD static std::pair<FloatPoint, FloatPoint> EndPointsFromAngle(float angleDeg,
                                                                             const FloatSize &size) noexcept
  {
    angleDeg = ToPositiveAngle(angleDeg);

    if (!angleDeg)
    {
      return {{0, size.Height()}, {0, 0}};
    }

    if (angleDeg == 90)
    {
      return {{0, 0}, {size.Width(), 0}};
    }

    if (angleDeg == 180)
    {
      return {{0, 0}, {0, size.Height()}};
    }

    if (angleDeg == 270)
    {
      return {{size.Width(), 0}, {0, 0}};
    }

    // angleDeg is a "bearing angle" (0deg = N, 90deg = E),
    // but tan expects 0deg = E, 90deg = N.
    float slope = std::tan(deg2rad(90 - angleDeg));

    // We find the endpoint by computing the intersection of the line formed by the slope,
    // and a line perpendicular to it that intersects the corner.
    float perpendicularSlope = -1 / slope;

    // Compute start corner relative to center, in Cartesian space (+y = up).
    float halfHeight = size.Height() / 2;
    float halfWidth = size.Width() / 2;
    FloatPoint endCorner;
    if (angleDeg < 90)
    {
      endCorner.Set(halfWidth, halfHeight);
    }
    else if (angleDeg < 180)
    {
      endCorner.Set(halfWidth, -halfHeight);
    }
    else if (angleDeg < 270)
    {
      endCorner.Set(-halfWidth, -halfHeight);
    }
    else
    {
      endCorner.Set(-halfWidth, halfHeight);
    }

    // Compute c (of y = mx + c) using the corner point.
    float c = endCorner.Y() - perpendicularSlope * endCorner.X();
    float endX = c / (slope - perpendicularSlope);
    float endY = perpendicularSlope * endX + c;

    // We computed the end point, so set the second point,
    // taking into account the moved origin and the fact
    // that we're in drawing space (+y = down). Reflect
    // around the center for the start point.

    return {FloatPoint(halfWidth - endX, halfHeight + endY), FloatPoint(halfWidth + endX, halfHeight - endY)};
  }

  KRYS_NODISCARD static std::pair<FloatPoint, FloatPoint>
    EndPointsFromAngleForPrefixedVariants(float angleDeg, const FloatSize &size) noexcept
  {
    // Prefixed gradients use "polar coordinate" angles, rather than "bearing" angles.
    return EndPointsFromAngle(90 - angleDeg, size);
  }

  KRYS_NODISCARD static float ResolveRadius(const LengthPercentage<css::NonNegative> &radius,
                                            float widthOrHeight) noexcept
  {
    return Evaluate<float>(radius, widthOrHeight, style::ZoomNeeded {});
  }

  struct DistanceToCorner
  {
    float distance;
    FloatPoint corner;
  };

  KRYS_NODISCARD static DistanceToCorner FindDistanceToClosestCorner(const FloatPoint &p,
                                                                     const FloatSize &size) noexcept
  {
    FloatPoint topLeft;
    float topLeftDistance = FloatSize(p - topLeft).DiagonalLength();

    FloatPoint topRight {size.Width(), 0};
    float topRightDistance = FloatSize(p - topRight).DiagonalLength();

    FloatPoint bottomLeft {0, size.Height()};
    float bottomLeftDistance = FloatSize(p - bottomLeft).DiagonalLength();

    FloatPoint bottomRight {size.Width(), size.Height()};
    float bottomRightDistance = FloatSize(p - bottomRight).DiagonalLength();

    FloatPoint corner = topLeft;
    float minDistance = topLeftDistance;

    if (topRightDistance < minDistance)
    {
      minDistance = topRightDistance;
      corner = topRight;
    }

    if (bottomLeftDistance < minDistance)
    {
      minDistance = bottomLeftDistance;
      corner = bottomLeft;
    }

    if (bottomRightDistance < minDistance)
    {
      minDistance = bottomRightDistance;
      corner = bottomRight;
    }
    return {minDistance, corner};
  }

  KRYS_NODISCARD static DistanceToCorner FindDistanceToFarthestCorner(const FloatPoint &p,
                                                                      const FloatSize &size) noexcept
  {
    FloatPoint topLeft;
    float topLeftDistance = FloatSize(p - topLeft).DiagonalLength();

    FloatPoint topRight {size.Width(), 0};
    float topRightDistance = FloatSize(p - topRight).DiagonalLength();

    FloatPoint bottomLeft {0, size.Height()};
    float bottomLeftDistance = FloatSize(p - bottomLeft).DiagonalLength();

    FloatPoint bottomRight {size.Width(), size.Height()};
    float bottomRightDistance = FloatSize(p - bottomRight).DiagonalLength();

    FloatPoint corner = topLeft;
    float maxDistance = topLeftDistance;

    if (topRightDistance > maxDistance)
    {
      maxDistance = topRightDistance;
      corner = topRight;
    }

    if (bottomLeftDistance > maxDistance)
    {
      maxDistance = bottomLeftDistance;
      corner = bottomLeft;
    }

    if (bottomRightDistance > maxDistance)
    {
      maxDistance = bottomRightDistance;
      corner = bottomRight;
    }

    return {maxDistance, corner};
  }

  // Compute horizontal radius of ellipse with center at 0,0 which passes through p, and has
  // width/height given by aspectRatio.
  KRYS_NODISCARD static inline float HorizontalEllipseRadius(const FloatSize &p, float aspectRatio) noexcept
  {
    // x^2/a^2 + y^2/b^2 = 1
    // a/b = aspectRatio, b = a/aspectRatio
    // a = sqrt(x^2 + y^2/(1/r^2))
    return std::hypot(p.Width(), p.Height() * aspectRatio);
  }

  // MARK: - Linear create.

  template <css::ValueId Name>
  KRYS_NODISCARD static Ref<boo::Gradient>
    CreatePlatformGradient(const css::FunctionNotation<Name, LinearGradient> &linear, const FloatSize &size,
                           const RenderStyle &style) noexcept
  {
    krys_debug_assert(!size.IsEmpty());

    auto [point0, point1] = krys::SwitchOn(
      linear.Parameters.GradientLine, [&](const Angle<> &angle) -> std::pair<FloatPoint, FloatPoint>
      { return EndPointsFromAngle(static_cast<float>(angle.Value), size); },
      [&](const Horizontal &horizontal) -> std::pair<FloatPoint, FloatPoint>
      {
        return krys::SwitchOn(
          horizontal, [&](css::keywords::Left) -> std::pair<FloatPoint, FloatPoint>
          { return {{size.Width(), 0}, {0, 0}}; },
          [&](css::keywords::Right) -> std::pair<FloatPoint, FloatPoint>
          { return {{0, 0}, {size.Width(), 0}}; });
      },
      [&](const Vertical &vertical) -> std::pair<FloatPoint, FloatPoint>
      {
        return krys::SwitchOn(
          vertical, [&](css::keywords::Top) -> std::pair<FloatPoint, FloatPoint>
          { return {{0, size.Height()}, {0, 0}}; },
          [&](css::keywords::Bottom) -> std::pair<FloatPoint, FloatPoint>
          { return {{0, 0}, {0, size.Height()}}; });
      },
      [&](const css::SpaceSeparatedTuple<Horizontal, Vertical> &pair) -> std::pair<FloatPoint, FloatPoint>
      {
        float rise = size.Width();
        float run = size.Height();
        if (std::holds_alternative<css::keywords::Left>(get<0>(pair)))
        {
          run *= -1;
        }
        if (std::holds_alternative<css::keywords::Bottom>(get<1>(pair)))
        {
          rise *= -1;
        }
        // Compute angle, and flip it back to "bearing angle" degrees.
        float angle = 90 - rad2deg(std::atan2(rise, run));
        return EndPointsFromAngle(angle, size);
      });

    boo::Gradient::LinearData data {point0, point1};
    LinearGradientAdapter adapter {data};
    auto stops = ComputeStops(adapter, linear, style);

    return boo::Gradient::Create(krys::move(data), linear.Parameters.ColorInterpolationMethod.Method,
                                 GradientSpreadMethod::Pad, krys::move(stops));
  }

  // MARK: - Prefixed Linear create.

  template <css::ValueId Name>
  KRYS_NODISCARD static Ref<boo::Gradient>
    CreatePlatformGradient(const css::FunctionNotation<Name, PrefixedLinearGradient> &linear,
                           const FloatSize &size, const RenderStyle &style) noexcept
  {
    krys_debug_assert(!size.IsEmpty());

    auto [point0, point1] = krys::SwitchOn(
      linear.Parameters.GradientLine, [&](const Angle<> &angle) -> std::pair<FloatPoint, FloatPoint>
      { return EndPointsFromAngleForPrefixedVariants(static_cast<float>(angle.Value), size); },
      [&](const Horizontal &horizontal) -> std::pair<FloatPoint, FloatPoint>
      {
        return krys::SwitchOn(
          horizontal, [&](css::keywords::Left) -> std::pair<FloatPoint, FloatPoint>
          { return {{0, 0}, {size.Width(), 0}}; },
          [&](css::keywords::Right) -> std::pair<FloatPoint, FloatPoint>
          { return {{size.Width(), 0}, {0, 0}}; });
      },
      [&](const Vertical vertical) -> std::pair<FloatPoint, FloatPoint>
      {
        return krys::SwitchOn(
          vertical, [&](css::keywords::Top) -> std::pair<FloatPoint, FloatPoint>
          { return {{0, 0}, {0, size.Height()}}; },
          [&](css::keywords::Bottom) -> std::pair<FloatPoint, FloatPoint>
          { return {{0, size.Height()}, {0, 0}}; });
      },
      [&](const css::SpaceSeparatedTuple<Horizontal, Vertical> &pair) -> std::pair<FloatPoint, FloatPoint>
      {
        return std::visit(
          krys::CreateVisitor(
            [&](css::keywords::Left, css::keywords::Top) -> std::pair<FloatPoint, FloatPoint>
            { return {{0, 0}, {size.Width(), size.Height()}}; },
            [&](css::keywords::Left, css::keywords::Bottom) -> std::pair<FloatPoint, FloatPoint>
            { return {{0, size.Height()}, {size.Width(), 0}}; },
            [&](css::keywords::Right, css::keywords::Top) -> std::pair<FloatPoint, FloatPoint>
            { return {{size.Width(), 0}, {0, size.Height()}}; },
            [&](css::keywords::Right, css::keywords::Bottom) -> std::pair<FloatPoint, FloatPoint>
            { return {{size.Width(), size.Height()}, {0, 0}}; }),
          get<0>(pair), get<1>(pair));
      });

    boo::Gradient::LinearData data {point0, point1};
    LinearGradientAdapter adapter {data};
    auto stops = ComputeStops(adapter, linear, style);

    return boo::Gradient::Create(krys::move(data), linear.Parameters.ColorInterpolationMethod.Method,
                                 GradientSpreadMethod::Pad, krys::move(stops));
  }

  // MARK: - Deprecated Linear create.

  template <css::ValueId Name>
  KRYS_NODISCARD static Ref<boo::Gradient>
    CreatePlatformGradient(const css::FunctionNotation<Name, DeprecatedLinearGradient> &linear,
                           const FloatSize &size, const RenderStyle &style) noexcept
  {
    krys_debug_assert(!size.IsEmpty());

    auto point0 = ComputeEndPoint(get<0>(linear.Parameters.GradientLine), size);
    auto point1 = ComputeEndPoint(get<1>(linear.Parameters.GradientLine), size);

    boo::Gradient::LinearData data {point0, point1};
    LinearGradientAdapter adapter {data};
    auto stops = ComputeStopsForDeprecatedVariants(adapter, linear, style);

    return boo::Gradient::Create(krys::move(data), linear.Parameters.ColorInterpolationMethod.Method,
                                 GradientSpreadMethod::Pad, krys::move(stops));
  }

  // MARK: - Radial create.

  template <css::ValueId Name>
  KRYS_NODISCARD static Ref<boo::Gradient>
    CreatePlatformGradient(const css::FunctionNotation<Name, RadialGradient> &radial, const FloatSize &size,
                           const RenderStyle &style) noexcept
  {
    krys_debug_assert(!size.IsEmpty());

    auto computeCenterPoint = [&](const Maybe<Position> &position) -> FloatPoint
    {
      return position ? ComputeEndPoint(*position, size) : FloatPoint {size.Width() / 2, size.Height() / 2};
    };

    auto computeCircleRadius =
      [&](const Variant<RadialGradient::Circle::Length, RadialGradient::Extent> &circleLengthOrExtent,
          FloatPoint centerPoint) -> std::pair<float, float>
    {
      return krys::SwitchOn(
        circleLengthOrExtent,
        [&](const RadialGradient::Circle::Length &circleLength) -> std::pair<float, float>
        { return {circleLength.ResolveZoom(style::ZoomNeeded {}), 1.0f}; },
        [&](const RadialGradient::Extent &extent) -> std::pair<float, float>
        {
          return krys::SwitchOn(
            extent, [&](css::keywords::ClosestSide) -> std::pair<float, float>
            { return {DistanceToClosestSide(centerPoint, size), 1.0f}; },
            [&](css::keywords::FarthestSide) -> std::pair<float, float>
            { return {DistanceToFarthestSide(centerPoint, size), 1.0f}; },
            [&](css::keywords::ClosestCorner) -> std::pair<float, float>
            { return {DistanceToClosestCorner(centerPoint, size), 1.0f}; },
            [&](css::keywords::FarthestCorner) -> std::pair<float, float>
            { return {DistanceToFarthestCorner(centerPoint, size), 1.0f}; });
        });
    };

    auto computeEllipseRadii =
      [&](const Variant<RadialGradient::Ellipse::SizeType, RadialGradient::Extent> &ellipseSizeOrExtent,
          FloatPoint centerPoint) -> std::pair<float, float>
    {
      return krys::SwitchOn(
        ellipseSizeOrExtent,
        [&](const RadialGradient::Ellipse::SizeType &ellipseSize) -> std::pair<float, float>
        {
          auto xDist = ResolveRadius(get<0>(ellipseSize), size.Width());
          auto yDist = ResolveRadius(get<1>(ellipseSize), size.Height());
          return {xDist, xDist / yDist};
        },
        [&](const RadialGradient::Extent &extent) -> std::pair<float, float>
        {
          return krys::SwitchOn(
            extent,
            [&](css::keywords::ClosestSide) -> std::pair<float, float>
            {
              float xDist = std::min(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::min(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {xDist, xDist / yDist};
            },
            [&](css::keywords::FarthestSide) -> std::pair<float, float>
            {
              float xDist = std::max(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::max(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {xDist, xDist / yDist};
            },
            [&](css::keywords::ClosestCorner) -> std::pair<float, float>
            {
              auto [distance, corner] = FindDistanceToClosestCorner(centerPoint, size);
              // If <shape> is ellipse, the gradient-shape has the same ratio of width to height
              // that it would if closest-side or farthest-side were specified, as appropriate.
              float xDist = std::min(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::min(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {HorizontalEllipseRadius(corner - centerPoint, xDist / yDist), xDist / yDist};
            },
            [&](css::keywords::FarthestCorner) -> std::pair<float, float>
            {
              auto [distance, corner] = FindDistanceToFarthestCorner(centerPoint, size);
              // If <shape> is ellipse, the gradient-shape has the same ratio of width to height
              // that it would if closest-side or farthest-side were specified, as appropriate.
              float xDist = std::max(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::max(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {HorizontalEllipseRadius(corner - centerPoint, xDist / yDist), xDist / yDist};
            });
        });
    };

    auto data = krys::SwitchOn(
      radial.Parameters.GradientBox,
      [&](const RadialGradient::Ellipse &ellipse)
      {
        auto centerPoint = computeCenterPoint(ellipse.Position);
        auto [endRadius, aspectRatio] = computeEllipseRadii(ellipse.Size, centerPoint);

        return boo::Gradient::RadialData {centerPoint, centerPoint, 0, endRadius, aspectRatio};
      },
      [&](const RadialGradient::Circle &circle)
      {
        auto centerPoint = computeCenterPoint(circle.Position);
        auto [endRadius, aspectRatio] = computeCircleRadius(circle.Size, centerPoint);

        return boo::Gradient::RadialData {centerPoint, centerPoint, 0, endRadius, aspectRatio};
      });

    RadialGradientAdapter adapter {data, size};
    auto stops = ComputeStops(adapter, radial, style);

    return boo::Gradient::Create(krys::move(data), radial.Parameters.ColorInterpolationMethod.Method,
                                 GradientSpreadMethod::Pad, krys::move(stops));
  }

  // MARK: - Prefixed Radial create.

  template <css::ValueId Name>
  KRYS_NODISCARD static Ref<boo::Gradient>
    CreatePlatformGradient(const css::FunctionNotation<Name, PrefixedRadialGradient> &radial,
                           const FloatSize &size, const RenderStyle &style) noexcept
  {
    krys_debug_assert(!size.IsEmpty());

    auto computeCenterPoint = [&](const Maybe<Position> &position) -> FloatPoint
    {
      return position ? ComputeEndPoint(*position, size) : FloatPoint {size.Width() / 2, size.Height() / 2};
    };

    auto computeEllipseRadii = [&](const Variant<PrefixedRadialGradient::Ellipse::SizeType,
                                                 PrefixedRadialGradient::Extent> &ellipseSizeOrExtent,
                                   FloatPoint centerPoint) -> std::pair<float, float>
    {
      return krys::SwitchOn(
        ellipseSizeOrExtent,
        [&](const PrefixedRadialGradient::Ellipse::SizeType &ellipseSize) -> std::pair<float, float>
        {
          auto xDist = ResolveRadius(get<0>(ellipseSize), size.Width());
          auto yDist = ResolveRadius(get<1>(ellipseSize), size.Height());
          return {xDist, xDist / yDist};
        },
        [&](const PrefixedRadialGradient::Extent &extent) -> std::pair<float, float>
        {
          return krys::SwitchOn(
            extent,
            [&](css::keywords::ClosestSide) -> std::pair<float, float>
            {
              float xDist = std::min(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::min(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {xDist, xDist / yDist};
            },
            [&](css::keywords::Contain) -> std::pair<float, float>
            {
              float xDist = std::min(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::min(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {xDist, xDist / yDist};
            },
            [&](css::keywords::FarthestSide) -> std::pair<float, float>
            {
              float xDist = std::max(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::max(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {xDist, xDist / yDist};
            },
            [&](css::keywords::ClosestCorner) -> std::pair<float, float>
            {
              auto [distance, corner] = FindDistanceToClosestCorner(centerPoint, size);
              // If <shape> is ellipse, the gradient-shape has the same ratio of width to height
              // that it would if closest-side or farthest-side were specified, as appropriate.
              float xDist = std::min(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::min(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {HorizontalEllipseRadius(corner - centerPoint, xDist / yDist), xDist / yDist};
            },
            [&](css::keywords::FarthestCorner) -> std::pair<float, float>
            {
              auto [distance, corner] = FindDistanceToFarthestCorner(centerPoint, size);
              // If <shape> is ellipse, the gradient-shape has the same ratio of width to height
              // that it would if closest-side or farthest-side were specified, as appropriate.
              float xDist = std::max(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::max(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {HorizontalEllipseRadius(corner - centerPoint, xDist / yDist), xDist / yDist};
            },
            [&](css::keywords::Cover) -> std::pair<float, float>
            {
              auto [distance, corner] = FindDistanceToFarthestCorner(centerPoint, size);
              // If <shape> is ellipse, the gradient-shape has the same ratio of width to height
              // that it would if closest-side or farthest-side were specified, as appropriate.
              float xDist = std::max(centerPoint.X(), size.Width() - centerPoint.X());
              float yDist = std::max(centerPoint.Y(), size.Height() - centerPoint.Y());
              return {HorizontalEllipseRadius(corner - centerPoint, xDist / yDist), xDist / yDist};
            });
        });
    };

    auto computeCircleRadius = [&](const PrefixedRadialGradient::Extent &extent,
                                   FloatPoint centerPoint) -> std::pair<float, float>
    {
      return krys::SwitchOn(
        extent,
        [&](css::keywords::ClosestSide) -> std::pair<float, float>
        {
          return {std::min({centerPoint.X(), size.Width() - centerPoint.X(), centerPoint.Y(),
                            size.Height() - centerPoint.Y()}),
                  1.0f};
        },
        [&](css::keywords::Contain) -> std::pair<float, float>
        {
          return {std::min({centerPoint.X(), size.Width() - centerPoint.X(), centerPoint.Y(),
                            size.Height() - centerPoint.Y()}),
                  1.0f};
        },
        [&](css::keywords::FarthestSide) -> std::pair<float, float>
        {
          return {std::max({centerPoint.X(), size.Width() - centerPoint.X(), centerPoint.Y(),
                            size.Height() - centerPoint.Y()}),
                  1.0f};
        },
        [&](css::keywords::ClosestCorner) -> std::pair<float, float>
        { return {DistanceToClosestCorner(centerPoint, size), 1.0f}; },
        [&](css::keywords::FarthestCorner) -> std::pair<float, float>
        { return {DistanceToFarthestCorner(centerPoint, size), 1.0f}; },
        [&](css::keywords::Cover) -> std::pair<float, float>
        { return {DistanceToFarthestCorner(centerPoint, size), 1.0f}; });
    };

    auto data = krys::SwitchOn(
      radial.Parameters.GradientBox,
      [&](const PrefixedRadialGradient::Ellipse &ellipse)
      {
        auto centerPoint = computeCenterPoint(ellipse.Position);
        auto [endRadius, aspectRatio] = computeEllipseRadii(
          ellipse.Size.value_or(PrefixedRadialGradient::Extent {css::keywords::Cover {}}), centerPoint);

        return boo::Gradient::RadialData {centerPoint, centerPoint, 0, endRadius, aspectRatio};
      },
      [&](const PrefixedRadialGradient::Circle &circle)
      {
        auto centerPoint = computeCenterPoint(circle.Position);
        auto [endRadius, aspectRatio] = computeCircleRadius(
          circle.Size.value_or(PrefixedRadialGradient::Extent {css::keywords::Cover {}}), centerPoint);

        return boo::Gradient::RadialData {centerPoint, centerPoint, 0, endRadius, aspectRatio};
      });

    RadialGradientAdapter adapter {data, size};
    auto stops = ComputeStops(adapter, radial, style);

    return boo::Gradient::Create(krys::move(data), radial.Parameters.ColorInterpolationMethod.Method,
                                 GradientSpreadMethod::Pad, krys::move(stops));
  }

  // MARK: - Deprecated Radial create.

  template <css::ValueId Name>
  KRYS_NODISCARD static Ref<boo::Gradient>
    CreatePlatformGradient(const css::FunctionNotation<Name, DeprecatedRadialGradient> &radial,
                           const FloatSize &size, const RenderStyle &style) noexcept
  {
    krys_debug_assert(!size.IsEmpty());

    auto firstPoint = ComputeEndPoint(radial.Parameters.GradientBox.First, size);
    auto secondPoint = ComputeEndPoint(radial.Parameters.GradientBox.Second, size);

    auto firstRadius = static_cast<float>(radial.Parameters.GradientBox.FirstRadius.Value);
    auto secondRadius = static_cast<float>(radial.Parameters.GradientBox.SecondRadius.Value);
    auto aspectRatio = 1.0f;

    boo::Gradient::RadialData data {firstPoint, secondPoint, firstRadius, secondRadius, aspectRatio};
    RadialGradientAdapter adapter {data, size};
    auto stops = ComputeStopsForDeprecatedVariants(adapter, radial, style);

    return boo::Gradient::Create(krys::move(data), radial.Parameters.ColorInterpolationMethod.Method,
                                 GradientSpreadMethod::Pad, krys::move(stops));
  }

  // MARK: - Conic create.

  template <css::ValueId Name>
  KRYS_NODISCARD static Ref<boo::Gradient>
    CreatePlatformGradient(const css::FunctionNotation<Name, ConicGradient> &conic, const FloatSize &size,
                           const RenderStyle &style) noexcept
  {
    krys_debug_assert(!size.IsEmpty());

    auto computeCenterPoint = [&](const Maybe<Position> &position) -> FloatPoint
    {
      return position ? ComputeEndPoint(*position, size) : FloatPoint {size.Width() / 2, size.Height() / 2};
    };

    auto centerPoint = computeCenterPoint(conic.Parameters.GradientBox.Position);
    float angleRadians = conic.Parameters.GradientBox.Angle
                           ? static_cast<float>(css::ConvertToValueInUnitsOf<css::AngleUnit::Rad>(
                               *conic.Parameters.GradientBox.Angle))
                           : 0;

    boo::Gradient::ConicData data {centerPoint, angleRadians};
    ConicGradientAdapter adapter;
    auto stops = ComputeStops(adapter, conic, style);

    return boo::Gradient::Create(krys::move(data), conic.Parameters.ColorInterpolationMethod.Method,
                                 GradientSpreadMethod::Pad, krys::move(stops));
  }

  // MARK: - createPlatformGradient

  KRYS_NODISCARD Ref<boo::Gradient> CreatePlatformGradient(const Gradient &gradient, const FloatSize &size,
                                                           const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(gradient,
                          [&](auto &gradient) { return CreatePlatformGradient(gradient, size, style); });
  }

  // MARK: - stopsAreCacheable

  KRYS_NODISCARD static bool StopColorIsCacheable(const Color &stopColor) noexcept
  {
    return !ContainsCurrentColor(stopColor);
  }

  KRYS_NODISCARD static bool StopColorIsCacheable(const Markable<Color> &stopColor) noexcept
  {
    return !stopColor || StopColorIsCacheable(*stopColor);
  }

  template <typename Gradient>
  KRYS_NODISCARD static bool StopsAreCacheable(const Gradient &gradient) noexcept
  {
    return std::ranges::all_of(gradient.Parameters.Stops,
                               [](auto &stop) { return StopColorIsCacheable(stop.Color); });
  }

  KRYS_NODISCARD bool StopsAreCacheable(const Gradient &gradient) noexcept
  {
    return krys::SwitchOn(gradient, [](auto &gradient) { return StopsAreCacheable(gradient); });
  }

  // MARK: - isOpaque

  template <typename T>
  KRYS_NODISCARD static bool IsOpaque(const T &gradient, const RenderStyle &style) noexcept
  {
    bool hasColorFilter = style.HasAppleColorFilter();

    return std::ranges::all_of(
      gradient.Parameters.Stops,
      [&](auto &stop) { return ResolveColorStopColor(stop.Color, style, hasColorFilter).IsOpaque(); });
  }

  KRYS_NODISCARD bool IsOpaque(const Gradient &gradient, const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(gradient, [&](auto &gradient) { return IsOpaque(gradient, style); });
  }
}
