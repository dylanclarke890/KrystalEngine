#pragma once

#include "Krystal.Core/Colour/ColourComponents.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>
#include <cmath>
#include <functional>

namespace krys
{
  KRYS_NODISCARD constexpr SRGBA<float> Premultiplied(const SRGBA<float> &color) noexcept
  {
    auto [r, g, b, a] = color.Resolved();
    return {r * a, g * a, b * a, a};
  }

  KRYS_NODISCARD constexpr SRGBA<float> Unpremultiplied(const SRGBA<float> &color) noexcept
  {
    auto [r, g, b, a] = color.Resolved();
    if (!a)
    {
      return color;
    }

    return CreateFromComponentsClampingExceptAlpha<SRGBA<float>>(r / a, g / a, b / a, a);
  }

  KRYS_NODISCARD constexpr SRGBA<uint8> PremultipliedFlooring(SRGBA<uint8> color) noexcept
  {
    auto [r, g, b, a] = color.Resolved();
    if (!a)
    {
      return {0, 0, 0, 0};
    }

    if (a == 255)
    {
      return color;
    }

    return CreateFromComponentsClampingExceptAlpha<SRGBA<uint8>>(
      FastDivideBy255(r * a), FastDivideBy255(g * a), FastDivideBy255(b * a), a);
  }

  KRYS_NODISCARD constexpr SRGBA<uint8> PremultipliedCeiling(SRGBA<uint8> color) noexcept
  {
    auto [r, g, b, a] = color.Resolved();
    if (!a)
    {
      return {0, 0, 0, 0};
    }

    if (a == 255)
    {
      return color;
    }

    return CreateFromComponentsClampingExceptAlpha<SRGBA<uint8>>(
      FastDivideBy255(r * a + 254), FastDivideBy255(g * a + 254), FastDivideBy255(b * a + 254), a);
  }

  KRYS_NODISCARD constexpr SRGBA<uint8> Unpremultiplied(SRGBA<uint8> color) noexcept
  {
    constexpr auto UnpremultipliedComponentByte = [](uint8 c, uint8 a) -> uint16
    {
      return (FastMultiplyBy255(c) + a - 1) / a;
    };

    auto [r, g, b, a] = color.Resolved();
    if (!a || a == 255)
    {
      return color;
    }

    return CreateFromComponentsClampingExceptAlpha<SRGBA<uint8>>(UnpremultipliedComponentByte(r, a),
                                                                 UnpremultipliedComponentByte(g, a),
                                                                 UnpremultipliedComponentByte(b, a), a);
  }

  KRYS_NODISCARD inline uint8 ConvertPrescaledSRGBAFloatToSRGBAByte(float value) noexcept
  {
    return static_cast<uint8>(std::clamp(std::lround(value), 0l, 255l));
  }

  template <typename T>
  KRYS_NODISCARD constexpr T ConvertByteAlphaTo(uint8 value) noexcept;

  template <>
  KRYS_NODISCARD constexpr uint8 ConvertByteAlphaTo<uint8>(uint8 value) noexcept
  {
    return value;
  }

  template <>
  KRYS_NODISCARD constexpr float ConvertByteAlphaTo<float>(uint8 value) noexcept
  {
    return value / 255.0f;
  }

  template <typename T>
  KRYS_NODISCARD T ConvertFloatAlphaTo(float value) noexcept;

  template <>
  KRYS_NODISCARD inline uint8 ConvertFloatAlphaTo<uint8>(float value) noexcept
  {
    return static_cast<uint8>(std::clamp(std::lround(value * 255.0f), 0l, 255l));
  }

  template <>
  KRYS_NODISCARD inline float ConvertFloatAlphaTo<float>(float value) noexcept
  {
    return ClampedAlpha(value);
  }

  template <typename ColourType, typename Functor>
  KRYS_NODISCARD constexpr auto ColourByModifyingEachNonAlphaComponent(const ColourType &color,
                                                                       Functor &&functor) noexcept
  {
    auto components = AsColourComponents(color.Resolved());

    auto copy = components;
    copy[0] = std::invoke(functor, components[0]);
    copy[1] = std::invoke(functor, components[1]);
    copy[2] = std::invoke(std::forward<Functor>(functor), components[2]);

    return CreateFromComponents<ColourType>(copy);
  }

  template <typename ColourType>
  KRYS_NODISCARD constexpr auto ColourWithOverriddenAlpha(const ColourType &color,
                                                          uint8 overrideAlpha) noexcept
  {
    auto copy = color.Unresolved();
    copy.Alpha = ConvertByteAlphaTo<typename ColourType::ComponentType>(overrideAlpha);
    return copy;
  }

  template <typename ColourType>
  KRYS_NODISCARD auto ColourWithOverriddenAlpha(const ColourType &color, float overrideAlpha) noexcept
  {
    auto copy = color.Unresolved();
    copy.Alpha = ConvertFloatAlphaTo<typename ColourType::ComponentType>(overrideAlpha);
    return copy;
  }

  template <typename ColourType>
  KRYS_NODISCARD constexpr auto InvertedColourWithOverriddenAlpha(const ColourType &color,
                                                                  uint8 overrideAlpha) noexcept
  {
    static_assert(ColourType::Model::IsInvertible);

    auto components = AsColourComponents(color.Resolved());
    auto copy = components;

    for (size_t i = 0uz; i < 3uz; ++i)
    {
      copy[i] = ColourType::Model::ComponentInfo[i].Max - components[i];
    }

    copy[3] = ConvertByteAlphaTo<typename ColourType::ComponentType>(overrideAlpha);

    return CreateFromComponents<ColourType>(copy);
  }

  template <typename ColourType>
  KRYS_NODISCARD auto InvertedColourWithOverriddenAlpha(const ColourType &color, float overrideAlpha) noexcept
  {
    static_assert(ColourType::Model::IsInvertible);

    auto components = AsColourComponents(color.Resolved());
    auto copy = components;

    for (size_t i = 0uz; i < 3uz; ++i)
    {
      copy[i] = ColourType::Model::ComponentInfo[i].Max - components[i];
    }

    copy[3] = ConvertFloatAlphaTo<typename ColourType::ComponentType>(overrideAlpha);

    return CreateFromComponents<ColourType>(copy);
  }

  template <typename ColourType>
  requires(UsesLabModel<ColourType> || UsesLCHModel<ColourType> || UsesOKLabModel<ColourType>
           || UsesOKLCHModel<ColourType> || UsesHSLModel<ColourType>)
  KRYS_NODISCARD constexpr bool IsBlack(const ColourType &color) noexcept
  {
    auto resolvedColor = color.Resolved();
    return resolvedColor.Lightness == 0 && resolvedColor.Alpha == AlphaTraits<float>::Opaque;
  }

  template <typename ColourType>
  requires UsesRGBModel<ColourType>
  KRYS_NODISCARD constexpr bool IsBlack(const ColourType &color) noexcept
  {
    auto [c1, c2, c3, alpha] = color.Resolved();
    return c1 == 0 && c2 == 0 && c3 == 0 && alpha == AlphaTraits<typename ColourType::ComponentType>::Opaque;
  }

  template <typename ColourType>
  requires UsesHWBModel<ColourType>
  KRYS_NODISCARD constexpr bool IsBlack(const ColourType &color) noexcept
  {
    auto resolvedColor = color.Resolved();
    return resolvedColor.Blackness == 100 && resolvedColor.Alpha == AlphaTraits<float>::Opaque;
  }

  template <WhitePoint W>
  KRYS_NODISCARD constexpr bool IsBlack(const XYZA<float, W> &color) noexcept
  {
    auto resolvedColor = color.Resolved();
    return resolvedColor.Y == 0 && resolvedColor.Alpha == AlphaTraits<float>::Opaque;
  }

  template <typename ColourType>
  requires(UsesLabModel<ColourType> || UsesLCHModel<ColourType> || UsesHSLModel<ColourType>)
  KRYS_NODISCARD constexpr bool IsWhite(const ColourType &color) noexcept
  {
    auto resolvedColor = color.Resolved();
    return resolvedColor.Lightness == 100 && resolvedColor.Alpha == AlphaTraits<float>::Opaque;
  }

  template <typename ColourType>
  requires(UsesOKLabModel<ColourType> || UsesOKLCHModel<ColourType>)
  KRYS_NODISCARD constexpr bool IsWhite(const ColourType &color) noexcept
  {
    auto resolvedColor = color.Resolved();
    return resolvedColor.Lightness == 1 && resolvedColor.Alpha == AlphaTraits<float>::Opaque;
  }

  template <typename ColourType>
  requires(UsesRGBModel<ColourType> && SameType<typename ColourType::ComponentType, float>)
  KRYS_NODISCARD constexpr bool IsWhite(const ColourType &color) noexcept
  {
    auto [c1, c2, c3, alpha] = color.Resolved();
    return c1 == 1 && c2 == 1 && c3 == 1 && alpha == AlphaTraits<float>::Opaque;
  }

  template <typename ColourType>
  requires(UsesRGBModel<ColourType> && SameType<typename ColourType::ComponentType, uint8>)
  KRYS_NODISCARD constexpr bool IsWhite(const ColourType &color) noexcept
  {
    auto [c1, c2, c3, alpha] = color.Resolved();
    return c1 == 255 && c2 == 255 && c3 == 255 && alpha == AlphaTraits<uint8>::Opaque;
  }

  template <typename ColourType>
  requires UsesHWBModel<ColourType>
  KRYS_NODISCARD constexpr bool IsWhite(const ColourType &color) noexcept
  {
    auto resolvedColor = color.Resolved();
    return resolvedColor.Whiteness == 100 && resolvedColor.Alpha == AlphaTraits<float>::Opaque;
  }

  template <WhitePoint W>
  KRYS_NODISCARD constexpr bool IsWhite(const XYZA<float, W> &color) noexcept
  {
    auto resolvedColor = color.Resolved();
    return resolvedColor.Y == 1 && resolvedColor.Alpha == AlphaTraits<float>::Opaque;
  }
}
