#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/WindRule.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include <string_view>

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

  enum class BlendMode : uint8
  {
    Normal = 1, // Start with 1 to match SVG's blendmode enumeration.
    Multiply,
    Screen,
    Darken,
    Lighten,
    Overlay,
    ColorDodge,
    ColorBurn,
    HardLight,
    SoftLight,
    Difference,
    Exclusion,
    Hue,
    Saturation,
    Color,
    Luminosity,
    PlusDarker,
    PlusLighter
  };

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

  enum class GradientSpreadMethod : uint8
  {
    Pad,
    Reflect,
    Repeat
  };

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

  /// @brief The CSS name of @p blendMode, such as `color-dodge`.
  KRYS_NODISCARD constexpr std::string_view BlendModeName(BlendMode blendMode) noexcept
  {
    constexpr std::string_view blendOperatorNames[] = {
      "normal",      "multiply",   "screen",     "darken",     "lighten",     "overlay",
      "color-dodge", "color-burn", "hard-light", "soft-light", "difference",  "exclusion",
      "hue",         "saturation", "color",      "luminosity", "plus-darker", "plus-lighter"};
    return blendOperatorNames[static_cast<uint32>(blendMode) - static_cast<uint32>(BlendMode::Normal)];
  }

  // FIXME: `compositeOperatorName`, `parseBlendMode` and `parseCompositeAndBlendOperator` come with the
  // canvas 2D context.
}
