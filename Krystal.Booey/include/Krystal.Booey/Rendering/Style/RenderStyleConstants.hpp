#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  // Fill, Stroke, ViewBox are just used for SVG.
  enum class CSSBoxType : uint8
  {
    BoxMissing = 0,
    MarginBox,
    BorderBox,
    PaddingBox,
    ContentBox,
    FillBox,
    StrokeBox,
    ViewBox
  };

  /// @brief The pseudo-elements that have their own style (those not backed by an element).
  enum class PseudoElementType : uint8
  {
    FirstLine,
    FirstLetter,
    GrammarError,
    Highlight,
    Marker,
    Before,
    After,
    Selection,
    Backdrop,
    SpellingError,
    TargetText,
    Checkmark,
    ViewTransition,
    ViewTransitionGroup,
    ViewTransitionImagePair,
    ViewTransitionOld,
    ViewTransitionNew,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::PseudoElementType, 17uz);
