/*
 * Copyright (C) 2004, 2005, 2006 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Paths/WindRule.hpp"
#include "Krystal.Core/Colour/BlendMode.hpp"
#include "Krystal.Core/Colour/GradientSpreadMethod.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  enum class CompositeOperator : uint8
  {
    Clear,
    Copy,
    SourceOver,
    SourceIn,
    SourceOut,
    SourceAtop,
    DestinationOver,
    DestinationIn,
    DestinationOut,
    DestinationAtop,
    XOR,
    PlusDarker,
    PlusLighter,
    Difference
  };

  using krys::BlendMode;

  struct CompositeMode
  {
    CompositeOperator Operation;
    boo::BlendMode BlendMode;

    KRYS_NODISCARD constexpr bool operator==(const CompositeMode &) const noexcept = default;
  };

  enum class DocumentMarkerLineStyleMode : uint8
  {
    TextCheckingDictationPhraseWithAlternatives,
    Spelling,
    Grammar,
    AutocorrectionReplacement,
    DictationAlternatives
  };

  struct DocumentMarkerLineStyle
  {
    DocumentMarkerLineStyleMode Mode;
    boo::Color Color;
  };

  using krys::GradientSpreadMethod;

  // InterpolationQuality::Default
  // For ImagePaintingOptions, it means:
  //  - Use context image interpolation quality.
  // For GraphicsContext CG it means:
  //  - If the CGImage has shouldInterpolate == true, use High
  //  - Else use None
  // For GraphicsContext Cairo it means:
  //  - Use Medium
  //
  // FIXME: Remove InterpolationQuality::Default since it does not mean what it should
  // obviously mean and because the CG context behavior is unusable in general case where
  // the draw call sites cannot track where the native images are generated from.
  enum class InterpolationQuality : uint8
  {
    Default,
    DoNotInterpolate,
    Low,
    Medium,
    High
  };

  enum class LineCap : uint8
  {
    Butt,
    Round,
    Square
  };

  enum class LineJoin : uint8
  {
    Miter,
    Round,
    Bevel
  };

  enum HorizontalAlignment
  {
    AlignLeft,
    AlignRight,
    AlignHCenter
  };

  enum class StrokeStyle : uint8
  {
    NoStroke,
    SolidStroke,
    DottedStroke,
    DashedStroke,
    DoubleStroke,
    WavyStroke,
  };

  enum class TextDrawingMode : uint8
  {
    Fill,
    Stroke,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::TextDrawingMode, 2uz);

namespace krys::boo
{
  using TextDrawingModeFlags = EnumSet<TextDrawingMode>;

  enum TextBaseline
  {
    AlphabeticTextBaseline,
    TopTextBaseline,
    MiddleTextBaseline,
    BottomTextBaseline,
    IdeographicTextBaseline,
    HangingTextBaseline
  };

  enum TextAlign
  {
    StartTextAlign,
    EndTextAlign,
    LeftTextAlign,
    CenterTextAlign,
    RightTextAlign
  };

  /// @brief The name of @p op, or of @p blendOp when it isn't `normal`.
  KRYS_NODISCARD std::string_view CompositeOperatorName(CompositeOperator op, BlendMode blendOp) noexcept;

  /// @brief Sets @p blendMode to the blend mode named @p name.
  /// @return False, leaving @p blendMode as it is, if @p name isn't a blend mode.
  bool ParseBlendMode(utf8_stringview name, BlendMode &blendMode) noexcept;

  /// @brief Sets @p op and @p blendOp to the compositing operator or blend mode named @p name (a blend mode
  /// composites with `source-over`), as `globalCompositeOperation` names them.
  /// @return False, leaving both as they are, if @p name is neither.
  bool ParseCompositeAndBlendOperator(utf8_stringview name, CompositeOperator &op,
                                      BlendMode &blendOp) noexcept;
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, CompositeOperator op) noexcept;
  TextStream &operator<<(TextStream &ts, CompositeMode compositeMode) noexcept;
  TextStream &operator<<(TextStream &ts, InterpolationQuality interpolationQuality) noexcept;
  TextStream &operator<<(TextStream &ts, WindRule rule) noexcept;
  TextStream &operator<<(TextStream &ts, LineCap capStyle) noexcept;
  TextStream &operator<<(TextStream &ts, LineJoin joinStyle) noexcept;
  TextStream &operator<<(TextStream &ts, StrokeStyle strokeStyle) noexcept;
  TextStream &operator<<(TextStream &ts, TextDrawingMode textDrawingMode) noexcept;
}
