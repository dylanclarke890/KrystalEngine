#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyleBase+SettersInlines.hpp"

#define SET_STYLE_PROPERTY_BASE(read, value, write)                                                          \
  do                                                                                                         \
  {                                                                                                          \
    if (!CompareEqual(read, value))                                                                          \
    {                                                                                                        \
      write;                                                                                                 \
    }                                                                                                        \
  } while (0)
#define SET_STYLE_PROPERTY(read, write, value) SET_STYLE_PROPERTY_BASE(read, value, write = value)
#define SET(group, variable, value) SET_STYLE_PROPERTY(group->variable, group.Access().variable, value)
#define SET_NESTED(group, parent, variable, value)                                                           \
  SET_STYLE_PROPERTY(group->parent->variable, group.Access().parent.Access().variable, value)
#define SET_DOUBLY_NESTED(group, grandparent, parent, variable, value)                                       \
  SET_STYLE_PROPERTY(group->grandparent->parent->variable,                                                   \
                     group.Access().grandparent.Access().parent.Access().variable, value)
#define SET_NESTED_STRUCT(group, parent, variable, value)                                                    \
  SET_STYLE_PROPERTY(group->parent.variable, group.Access().parent.variable, value)
#define SET_STYLE_PROPERTY_PAIR(read, write, variable1, value1, variable2, value2)                           \
  do                                                                                                         \
  {                                                                                                          \
    auto &readable = *read;                                                                                  \
    if (!CompareEqual(readable.variable1, value1) || !CompareEqual(readable.variable2, value2))              \
    {                                                                                                        \
      auto &writable = write;                                                                                \
      writable.variable1 = value1;                                                                           \
      writable.variable2 = value2;                                                                           \
    }                                                                                                        \
  } while (0)
#define SET_PAIR(group, variable1, value1, variable2, value2)                                                \
  SET_STYLE_PROPERTY_PAIR(group, group.Access(), variable1, value1, variable2, value2)
#define SET_NESTED_PAIR(group, parent, variable1, value1, variable2, value2)                                 \
  SET_STYLE_PROPERTY_PAIR(group->parent, group.Access().parent.Access(), variable1, value1, variable2, value2)
#define SET_DOUBLY_NESTED_PAIR(group, grandparent, parent, variable1, value1, variable2, value2)             \
  SET_STYLE_PROPERTY_PAIR(group->grandparent->parent, group.Access().grandparent.Access().parent.Access(),   \
                          variable1, value1, variable2, value2)

namespace krys::boo::style
{
  // FIXME: - Below are property setters that are not yet generated

  // FIXME: Support setters that need to return a `bool` value to indicate if the property changed.
  inline bool ComputedStyleProperties::SetDirection(TextDirection bidiDirection) noexcept
  {
    if (GetWritingMode().ComputedTextDirection() == bidiDirection)
    {
      return false;
    }
    _inheritedFlags.WritingMode.SetTextDirection(bidiDirection);
    return true;
  }

  inline bool ComputedStyleProperties::SetTextOrientation(TextOrientation textOrientation) noexcept
  {
    if (GetWritingMode().ComputedTextOrientation() == textOrientation)
    {
      return false;
    }
    _inheritedFlags.WritingMode.SetTextOrientation(textOrientation);
    return true;
  }

  inline bool ComputedStyleProperties::SetWritingMode(StyleWritingMode mode) noexcept
  {
    if (GetWritingMode().ComputedWritingMode() == mode)
    {
      return false;
    }
    _inheritedFlags.WritingMode.SetWritingMode(mode);
    return true;
  }

  inline bool ComputedStyleProperties::SetZoom(Zoom zoom) noexcept
  {
    // Clamp the effective zoom value to avoid overflow in derived computations.
    // This matches other engines values for compatibility.
    constexpr float minEffectiveZoom = 1e-6f;
    constexpr float maxEffectiveZoom = 1e6f;
    std::ignore =
      SetUsedZoom(ClampTo<float>(UsedZoom() * Evaluate<float>(zoom), minEffectiveZoom, maxEffectiveZoom));

    if (CompareEqual(_nonInheritedData->RareData->Zoom, zoom))
    {
      return false;
    }
    _nonInheritedData.Access().RareData.Access().Zoom = zoom;
    return true;
  }

  // FIXME: Support properties that set more than one value when set.

  inline void ComputedStyleProperties::SetAppearance(StyleAppearance appearance) noexcept
  {
    SET_NESTED_PAIR(_nonInheritedData, MiscData, Appearance, static_cast<unsigned>(appearance),
                    UsedAppearance, static_cast<unsigned>(appearance));
  }

  inline void ComputedStyleProperties::SetBlendMode(BlendMode mode) noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, EffectiveBlendMode, static_cast<unsigned>(mode));
    SET(_rareInheritedData, IsInSubtreeWithBlendMode, mode != BlendMode::Normal);
  }

  inline void ComputedStyleProperties::SetDisplay(DisplayType value) noexcept
  {
    _nonInheritedFlags.OriginalDisplay = static_cast<unsigned>(value);
    _nonInheritedFlags.EffectiveDisplay = static_cast<unsigned>(value);
  }

  // FIXME: Support generating properties that have their storage spread out

  inline void ComputedStyleProperties::SetSpecifiedZIndex(ZIndex index) noexcept
  {
    SET_NESTED_PAIR(_nonInheritedData, BoxData, HasAutoSpecifiedZIndex, static_cast<uint8>(index._isAuto),
                    SpecifiedZIndexValue, index._value);
  }

  inline void ComputedStyleProperties::SetCursor(Cursor cursor) noexcept
  {
    _inheritedFlags.CursorType = static_cast<unsigned>(cursor.Predefined);
    SET(_rareInheritedData, CursorImages, krys::move(cursor.Images));
  }

  // MARK: Support Font properties

  inline void ComputedStyleProperties::SetTextSpacingTrim(TextSpacingTrim value) noexcept
  {
    auto description = GetFontDescription();
    description.SetTextSpacingTrim(value.Platform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetTextAutospace(TextAutospace value) noexcept
  {
    auto description = GetFontDescription();
    description.SetTextAutospace(ConvertToPlatform(value));
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontSize(float size) noexcept
  {
    // size must be specifiedSize if Text Autosizing is enabled, but computedSize if text
    // zoom is enabled (if neither is enabled it's irrelevant as they're probably the same).

    krys_debug_assert(std::isfinite(size));
    if (!std::isfinite(size) || size < 0)
    {
      size = 0;
    }
    else
    {
      size = std::min(MaximumAllowedFontSize, size);
    }

    auto description = GetFontDescription();
    description.SetSpecifiedSize(size);
    description.SetComputedSize(size);
    SetFontDescription(krys::move(description));

    // Whenever the font size changes, letter-spacing and word-spacing, which are dependent on font-size, must
    // be re-synchronized.
    SynchronizeLetterSpacingWithFontCascade();
    SynchronizeWordSpacingWithFontCascade();
  }

  inline void ComputedStyleProperties::SetFontSizeAdjust(FontSizeAdjust sizeAdjust) noexcept
  {
    auto description = GetFontDescription();
    description.SetFontSizeAdjust(sizeAdjust.Platform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontOpticalSizing(FontOpticalSizing opticalSizing) noexcept
  {
    auto description = GetFontDescription();
    description.SetOpticalSizing(opticalSizing);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontFamily(FontFamilies families) noexcept
  {
    auto description = GetFontDescription();
    description.SetFamilies(families.TakePlatform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontFeatureSettings(FontFeatureSettings settings) noexcept
  {
    auto description = GetFontDescription();
    description.SetFeatureSettings(settings.TakePlatform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontVariationSettings(FontVariationSettings settings) noexcept
  {
    auto description = GetFontDescription();
    description.SetVariationSettings(settings.TakePlatform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontWeight(FontWeight value) noexcept
  {
    auto description = GetFontDescription();
    description.SetWeight(value.Platform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontWidth(FontWidth value) noexcept
  {
    auto description = GetFontDescription();
    description.SetWidth(value.Platform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontStyle(FontStyle style) noexcept
  {
    auto description = GetFontDescription();
    description.SetFontStyleSlope(style.PlatformSlope());
    description.SetFontStyleAxis(style.PlatformAxis());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontPalette(FontPalette value) noexcept
  {
    auto description = GetFontDescription();
    description.SetFontPalette(value.Platform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontKerning(Kerning value) noexcept
  {
    auto description = GetFontDescription();
    description.SetKerning(value);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontSmoothing(FontSmoothingMode value) noexcept
  {
    auto description = GetFontDescription();
    description.SetFontSmoothing(value);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontSynthesisSmallCaps(FontSynthesisLonghandValue value) noexcept
  {
    auto description = GetFontDescription();
    description.SetFontSynthesisSmallCaps(value);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontSynthesisStyle(FontSynthesisLonghandValue value) noexcept
  {
    auto description = GetFontDescription();
    description.SetFontSynthesisStyle(value);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontSynthesisWeight(FontSynthesisLonghandValue value) noexcept
  {
    auto description = GetFontDescription();
    description.SetFontSynthesisWeight(value);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontVariantAlternates(FontVariantAlternates value) noexcept
  {
    auto description = GetFontDescription();
    description.SetVariantAlternates(value.TakePlatform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontVariantCaps(FontVariantCaps value) noexcept
  {
    auto description = GetFontDescription();
    description.SetVariantCaps(value);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontVariantEastAsian(FontVariantEastAsian value) noexcept
  {
    auto description = GetFontDescription();
    description.SetVariantEastAsian(value.GetPlatform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontVariantEmoji(FontVariantEmoji value) noexcept
  {
    auto description = GetFontDescription();
    description.SetVariantEmoji(value);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontVariantLigatures(FontVariantLigatures value) noexcept
  {
    auto description = GetFontDescription();
    description.SetVariantLigatures(value.GetPlatform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontVariantNumeric(FontVariantNumeric value) noexcept
  {
    auto description = GetFontDescription();
    description.SetVariantNumeric(value.GetPlatform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetFontVariantPosition(FontVariantPosition value) noexcept
  {
    auto description = GetFontDescription();
    description.SetVariantPosition(value);
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetLocale(WebkitLocale value) noexcept
  {
    auto description = GetFontDescription();
    description.SetSpecifiedLocale(value.TakePlatform());
    SetFontDescription(krys::move(description));
  }

  inline void ComputedStyleProperties::SetTextRendering(TextRenderingMode value) noexcept
  {
    auto description = GetFontDescription();
    description.SetTextRenderingMode(value);
    SetFontDescription(krys::move(description));
  }
}

#undef SET
#undef SET_DOUBLY_NESTED
#undef SET_DOUBLY_NESTED_PAIR
#undef SET_NESTED
#undef SET_NESTED_PAIR
#undef SET_NESTED_STRUCT
#undef SET_PAIR
#undef SET_STYLE_PROPERTY
#undef SET_STYLE_PROPERTY_BASE
#undef SET_STYLE_PROPERTY_PAIR
