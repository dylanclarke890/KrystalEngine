#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyleBase.hpp"

namespace krys::boo::style
{
  inline ComputedStyleBase::ComputedStyleBase(ComputedStyleBase &&) noexcept = default;
  inline ComputedStyleBase &ComputedStyleBase::operator=(ComputedStyleBase &&) noexcept = default;

  inline ComputedStyleBase::ComputedStyleBase(CreateDefaultStyleTag) noexcept
      : _nonInheritedData(StyleNonInheritedData::Create()),
        _rareInheritedData(StyleRareInheritedData::Create()), _inheritedData(StyleInheritedData::Create()),
        _svgStyle(SVGRenderStyle::Create())
  {
    _inheritedFlags.WritingMode =
      WritingMode(ComputedStyle::InitialWritingMode(), ComputedStyle::InitialDirection(),
                  ComputedStyle::InitialTextOrientation());
    _inheritedFlags.EmptyCells = static_cast<unsigned>(ComputedStyle::InitialEmptyCells());
    _inheritedFlags.CaptionSide = static_cast<unsigned>(ComputedStyle::InitialCaptionSide());
    _inheritedFlags.ListStylePosition = static_cast<unsigned>(ComputedStyle::InitialListStylePosition());
    _inheritedFlags.Visibility = static_cast<unsigned>(ComputedStyle::InitialVisibility());
    _inheritedFlags.TextAlign = static_cast<unsigned>(ComputedStyle::InitialTextAlign());
    _inheritedFlags.TextTransform = ComputedStyle::InitialTextTransform().ToRaw();
    _inheritedFlags.TextDecorationLineInEffect = ComputedStyle::InitialTextDecorationLine().ToRaw();
    _inheritedFlags.CursorType = static_cast<unsigned>(ComputedStyle::InitialCursor().Predefined);
    _inheritedFlags.WhiteSpaceCollapse = static_cast<unsigned>(ComputedStyle::InitialWhiteSpaceCollapse());
    _inheritedFlags.TextWrapMode = static_cast<unsigned>(ComputedStyle::InitialTextWrapMode());
    _inheritedFlags.TextWrapStyle = static_cast<unsigned>(ComputedStyle::InitialTextWrapStyle());
    _inheritedFlags.BorderCollapse = static_cast<unsigned>(ComputedStyle::InitialBorderCollapse());
    _inheritedFlags.RtlOrdering = static_cast<unsigned>(ComputedStyle::InitialRTLOrdering());
    _inheritedFlags.PrintColorAdjust = static_cast<unsigned>(ComputedStyle::InitialPrintColorAdjust());
    _inheritedFlags.PointerEvents = static_cast<unsigned>(ComputedStyle::InitialPointerEvents());
    _inheritedFlags.InsideLink = static_cast<unsigned>(boo::InsideLink::NotInside);

    _nonInheritedFlags.EffectiveDisplay = static_cast<unsigned>(ComputedStyle::InitialDisplay());
    _nonInheritedFlags.OriginalDisplay = static_cast<unsigned>(ComputedStyle::InitialDisplay());
    _nonInheritedFlags.OverflowX = static_cast<unsigned>(ComputedStyle::InitialOverflowX());
    _nonInheritedFlags.OverflowY = static_cast<unsigned>(ComputedStyle::InitialOverflowY());
    _nonInheritedFlags.Clear = static_cast<unsigned>(ComputedStyle::InitialClear());
    _nonInheritedFlags.Position = static_cast<unsigned>(ComputedStyle::InitialPosition());
    _nonInheritedFlags.UnicodeBidi = static_cast<unsigned>(ComputedStyle::InitialUnicodeBidi());
    _nonInheritedFlags.Floating = static_cast<unsigned>(ComputedStyle::InitialFloating());
    _nonInheritedFlags.TextDecorationLine = ComputedStyle::InitialTextDecorationLine().ToRaw();
    _nonInheritedFlags.UsesViewportUnits = false;
    _nonInheritedFlags.UsesContainerUnits = false;
    _nonInheritedFlags.UseTreeCountingFunctions = false;
    _nonInheritedFlags.HasExplicitlyInheritedProperties = false;
    _nonInheritedFlags.DisallowsFastPathInheritance = false;
    _nonInheritedFlags.EmptyState = false;
    _nonInheritedFlags.FirstChildState = false;
    _nonInheritedFlags.LastChildState = false;
    _nonInheritedFlags.IsLink = false;
    _nonInheritedFlags.PseudoElementType = 0;
    _nonInheritedFlags.PseudoBits = 0;

    static_assert((sizeof(InheritedFlags) <= 8), "InheritedFlags does not grow");
    static_assert((sizeof(NonInheritedFlags) <= 12), "NonInheritedFlags does not grow");
  }

  inline ComputedStyleBase::ComputedStyleBase(const ComputedStyleBase &other, CloneTag) noexcept
      : _nonInheritedData(other._nonInheritedData), _nonInheritedFlags(other._nonInheritedFlags),
        _rareInheritedData(other._rareInheritedData), _inheritedData(other._inheritedData),
        _inheritedFlags(other._inheritedFlags), _svgStyle(other._svgStyle)
  {
  }

  inline ComputedStyleBase::ComputedStyleBase(ComputedStyleBase &a, ComputedStyleBase &&b) noexcept
      : _nonInheritedData(std::exchange(a._nonInheritedData, krys::move(b._nonInheritedData))),
        _nonInheritedFlags(std::exchange(a._nonInheritedFlags, b._nonInheritedFlags)),
        _rareInheritedData(std::exchange(a._rareInheritedData, krys::move(b._rareInheritedData))),
        _inheritedData(std::exchange(a._inheritedData, krys::move(b._inheritedData))),
        _inheritedFlags(std::exchange(a._inheritedFlags, b._inheritedFlags)),
        _cachedPseudoStyles(std::exchange(a._cachedPseudoStyles, krys::move(b._cachedPseudoStyles))),
        _svgStyle(std::exchange(a._svgStyle, krys::move(b._svgStyle)))
  {
  }

  inline void
    ComputedStyleBase::NonInheritedFlags::CopyNonInheritedFrom(const NonInheritedFlags &other) noexcept
  {
    // Only some flags are copied because NonInheritedFlags contains things that are not actually style data.
    EffectiveDisplay = other.EffectiveDisplay;
    OriginalDisplay = other.OriginalDisplay;
    OverflowX = other.OverflowX;
    OverflowY = other.OverflowY;
    Clear = other.Clear;
    Position = other.Position;
    UnicodeBidi = other.UnicodeBidi;
    Floating = other.Floating;
    TextDecorationLine = other.TextDecorationLine;
    UsesViewportUnits = other.UsesViewportUnits;
    UsesContainerUnits = other.UsesContainerUnits;
    UseTreeCountingFunctions = other.UseTreeCountingFunctions;
    HasExplicitlyInheritedProperties = other.HasExplicitlyInheritedProperties;
    DisallowsFastPathInheritance = other.DisallowsFastPathInheritance;
  }
}
