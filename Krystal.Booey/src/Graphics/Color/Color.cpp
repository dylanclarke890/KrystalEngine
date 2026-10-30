#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorLuminance.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  constexpr static auto lightenedBlack = SRGBA<uint8> {84, 84, 84};
  constexpr static auto darkenedWhite = SRGBA<uint8> {171, 171, 171};

  Color Color::Lightened() const noexcept
  {
    // Hardcode this common case for speed.
    if (IsInline() && AsInline() == black)
    {
      return lightenedBlack;
    }

    auto [r, g, b, a] = ToColorTypeLossy<SRGBA<float>>().Resolved();
    float v = std::max({r, g, b});

    if (v == 0.0f)
    {
      return lightenedBlack.ColourWithAlphaByte(AlphaByte());
    }

    float multiplier = std::min(1.0f, v + 0.33f) / v;

    return ConvertColour<SRGBA<uint8>>(SRGBA<float> {multiplier * r, multiplier * g, multiplier * b, a});
  }

  Color Color::Darkened() const noexcept
  {
    // Hardcode this common case for speed.
    if (IsInline() && AsInline() == white)
    {
      return darkenedWhite;
    }

    auto [r, g, b, a] = ToColorTypeLossy<SRGBA<float>>().Resolved();

    float v = std::max({r, g, b});
    float multiplier = std::max(0.0f, (v - 0.33f) / v);

    return ConvertColour<SRGBA<uint8>>(SRGBA<float> {multiplier * r, multiplier * g, multiplier * b, a});
  }

  double Color::Lightness() const noexcept
  {
    // FIXME: Replace remaining uses with luminance.
    auto [r, g, b, a] = ToColorTypeLossy<SRGBA<float>>().Resolved();
    auto [min, max] = std::minmax({r, g, b});
    return 0.5 * (max + min);
  }

  double Color::Luminance() const noexcept
  {
    return RelativeLuminance(*this);
  }

  bool Color::AnyComponentIsNone() const noexcept
  {
    return CallOnUnderlyingType(
      [&]<typename ColorType>(const ColorType &underlyingColor)
      {
        if constexpr (SameType<ColorType, SRGBA<uint8>>)
        {
          return false;
        }
        else
        {
          return underlyingColor.Unresolved().AnyComponentIsNone();
        }
      });
  }

  Color Color::ColorWithAlpha(float alpha) const noexcept
  {
    return CallOnUnderlyingType(
      [&](const auto &underlyingColor) -> Color
      {
        auto result = ColourWithOverriddenAlpha(underlyingColor, alpha);

        // FIXME: Why is preserving the semantic bit desired and/or correct here?
        if (IsSemantic())
        {
          return {result, ColorFlags::Semantic};
        }

        return {result};
      });
  }

  Color Color::InvertedColorWithAlpha(float alpha) const noexcept
  {
    return CallOnUnderlyingType(
      [&]<typename ColorType>(const ColorType &underlyingColor) -> Color
      {
        // FIXME: Determine if there is a meaningful understanding of inversion that works
        // better for non-invertible color types like Lab or consider removing this in favor
        // of alternatives.
        if constexpr (ColorType::Model::IsInvertible)
        {
          return InvertedColourWithOverriddenAlpha(underlyingColor, alpha);
        }
        else
        {
          return InvertedColourWithOverriddenAlpha(ConvertColour<SRGBA<float>>(underlyingColor), alpha);
        }
      });
  }

  Color Color::SemanticColor() const noexcept
  {
    if (IsSemantic())
    {
      return *this;
    }

    if (IsOutOfLine())
    {
      return {ProtectedAsOutOfLine(), Space(), ColorFlags::Semantic};
    }

    return {AsInline(), ColorFlags::Semantic};
  }

  ColourComponents<float, 4>
    Color::ToResolvedColorComponentsInColorSpace(ColourSpace outputColorSpace) const noexcept
  {
    auto [inputColorSpace, components] = ColorSpaceAndResolvedColorComponents();
    return ConvertAndResolveColourComponents(inputColorSpace, components, outputColorSpace);
  }

  Pair<ColourSpace, ColourComponents<float, 4>> Color::ColorSpaceAndResolvedColorComponents() const noexcept
  {
    if (IsOutOfLine())
    {
      return {Space(), ResolveColourComponents(ProtectedAsOutOfLine()->ResolvedComponents())};
    }

    return {ColourSpace::SRGB, AsColourComponents(ConvertColour<SRGBA<float>>(AsInline()).Resolved())};
  }

  bool Color::IsBlackColor(const Color &color) noexcept
  {
    return color.CallOnUnderlyingType([](const auto &underlyingColor)
                                      { return krys::IsBlack(underlyingColor); });
  }

  bool Color::IsWhiteColor(const Color &color) noexcept
  {
    return color.CallOnUnderlyingType([](const auto &underlyingColor)
                                      { return krys::IsWhite(underlyingColor); });
  }

  Color::DebugRGBAData Color::DebugRGBA() const noexcept
  {
    auto [r, g, b, a] = ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    return {r, g, b, a};
  }

  utf8_string Color::DebugDescription() const noexcept
  {
    return SerialisationForRenderTreeAsText(*this);
  }

  TextStream &operator<<(TextStream &ts, const Color &color) noexcept
  {
    return ts << SerialisationForRenderTreeAsText(color);
  }
}
