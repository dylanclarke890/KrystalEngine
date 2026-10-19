#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/WindRule.hpp"
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

  // FIXME: `compositeOperatorName`, `parseBlendMode` and `parseCompositeAndBlendOperator` come with the
  // canvas 2D context.
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
