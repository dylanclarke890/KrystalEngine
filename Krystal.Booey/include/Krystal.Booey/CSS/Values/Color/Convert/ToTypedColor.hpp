#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/Normalise.hpp"

namespace krys::boo::css
{
  // This file implements support for converting style resolved parsed values (e.g. tuple
  // of `Variant<style::Number<>, style::Percentage<>, ...>`) into typed colors (e.g. `SRGBA<float>`).

  template <typename Descriptor, size_t Index>
  float convertToTypeColorComponent(style::Number<> number)
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);
    constexpr auto multiplier = info.NumberMultiplier;
    constexpr auto min = info.min * info.NumberMultiplier;
    constexpr auto max = info.max * info.NumberMultiplier;

    if constexpr (info.Type == ColorComponentType::Angle)
    {
      return normalizeHue(number.Value);
    }
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity()
                       && info.Max == std::numeric_limits<double>::infinity())
      return number.Value * multiplier;
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity())
      return std::min(number.Value * multiplier, max);
    else if constexpr (info.Max == std::numeric_limits<double>::infinity())
      return std::max(number.Value * multiplier, min);
    else
      return std::clamp(number.Value * multiplier, min, max);
  }

  template <typename Descriptor, size_t Index>
  float convertToTypeColorComponent(style::Percentage<> percent)
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);
    constexpr auto multiplier = info.PercentMultiplier * info.NumberMultiplier;
    constexpr auto min = info.Min * info.NumberMultiplier;
    constexpr auto max = info.Max * info.NumberMultiplier;

    if constexpr (info.Min == -std::numeric_limits<double>::infinity()
                  && info.Max == std::numeric_limits<double>::infinity())
      return percent.Value * multiplier;
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity())
      return std::min(percent.Value * multiplier, max);
    else if constexpr (info.Max == std::numeric_limits<double>::infinity())
      return std::max(percent.Value * multiplier, min);
    else
      return std::clamp(percent.Value * multiplier, min, max);
  }

  template <typename Descriptor, size_t Index>
  float convertToTypeColorComponent(style::Angle<> angle)
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);
    static_assert(info.Type == ColorComponentType::Angle);

    return normalizeHue(angle.Value);
  }

  template <typename Descriptor, size_t Index>
  float convertToTypeColorComponent(CSS::Keyword::None)
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  template <typename Descriptor, size_t Index, typename... Ts>
  float convertToTypeColorComponent(const Variant<Ts...> &variant)
  {
    return WTF::switchOn(variant,
                         [](auto value) { return convertToTypeColorComponent<Descriptor, Index>(value); });
  }

  template <typename Descriptor, size_t Index, typename T>
  float convertToTypeColorComponent(const std::optional<T> &optional, float defaultValue)
  {
    return optional ? convertToTypeColorComponent<Descriptor, Index>(*optional) : defaultValue;
  }

  template <typename Descriptor>
  GetColorType<Descriptor> convertToTypedColor(StyleColorParseType<Descriptor> parsed, double defaultAlpha)
  {
    return {ConvertToTypeColorComponent<Descriptor, 0>(std::get<0>(parsed)),
            ConvertToTypeColorComponent<Descriptor, 1>(std::get<1>(parsed)),
            ConvertToTypeColorComponent<Descriptor, 2>(std::get<2>(parsed)),
            ConvertToTypeColorComponent<Descriptor, 3>(std::get<3>(parsed), defaultAlpha)};
  }

} // namespace WebCore
