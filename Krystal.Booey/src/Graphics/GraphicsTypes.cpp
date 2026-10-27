#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Core/Colour/ColourTextStream.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr std::string_view compositeOperatorNames[] {"clear",
                                                         "copy",
                                                         "source-over",
                                                         "source-in",
                                                         "source-out",
                                                         "source-atop",
                                                         "destination-over",
                                                         "destination-in",
                                                         "destination-out",
                                                         "destination-atop",
                                                         "xor",
                                                         "darker",
                                                         "lighter",
                                                         "difference"};
  }

  bool ParseBlendMode(utf8_stringview name, BlendMode &blendMode) noexcept
  {
    for (auto i = static_cast<uint32>(BlendMode::Normal); i <= static_cast<uint32>(BlendMode::PlusLighter);
         ++i)
    {
      auto mode = static_cast<BlendMode>(i);
      auto modeName = BlendModeName(mode);
      if (std::ranges::equal(name, modeName, {}, {}, [](char c) { return static_cast<char8>(c); }))
      {
        blendMode = mode;
        return true;
      }
    }

    return false;
  }

  std::string_view CompositeOperatorName(CompositeOperator op, BlendMode blendOp) noexcept
  {
    krys_debug_assert(static_cast<uint32>(op) < std::size(compositeOperatorNames));
    if (blendOp > BlendMode::Normal)
    {
      return BlendModeName(blendOp);
    }
    return compositeOperatorNames[static_cast<uint32>(op)];
  }

  TextStream &operator<<(TextStream &ts, CompositeOperator op) noexcept
  {
    return ts << compositeOperatorNames[static_cast<uint32>(op)];
  }

  TextStream &operator<<(TextStream &ts, CompositeMode compositeMode) noexcept
  {
    ts.DumpProperty(u8"composite-operation", compositeMode.Operation);
    ts.DumpProperty(u8"blend-mode", compositeMode.BlendMode);
    return ts;
  }

  TextStream &operator<<(TextStream &ts, InterpolationQuality interpolationQuality) noexcept
  {
    switch (interpolationQuality)
    {
      case InterpolationQuality::Default:          ts << "default"; break;
      case InterpolationQuality::DoNotInterpolate: ts << "do-not-interpolate"; break;
      case InterpolationQuality::Low:              ts << "low"; break;
      case InterpolationQuality::Medium:           ts << "medium"; break;
      case InterpolationQuality::High:             ts << "high"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, WindRule rule) noexcept
  {
    switch (rule)
    {
      case WindRule::NonZero: ts << "NON-ZERO"; break;
      case WindRule::EvenOdd: ts << "EVEN-ODD"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, LineCap capStyle) noexcept
  {
    switch (capStyle)
    {
      case LineCap::Butt:   ts << "BUTT"; break;
      case LineCap::Round:  ts << "ROUND"; break;
      case LineCap::Square: ts << "SQUARE"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, LineJoin joinStyle) noexcept
  {
    switch (joinStyle)
    {
      case LineJoin::Miter: ts << "MITER"; break;
      case LineJoin::Round: ts << "ROUND"; break;
      case LineJoin::Bevel: ts << "BEVEL"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, StrokeStyle strokeStyle) noexcept
  {
    switch (strokeStyle)
    {
      case StrokeStyle::NoStroke:     ts << "no-stroke"; break;
      case StrokeStyle::SolidStroke:  ts << "solid-stroke"; break;
      case StrokeStyle::DottedStroke: ts << "dotted-stroke"; break;
      case StrokeStyle::DashedStroke: ts << "dashed-stroke"; break;
      case StrokeStyle::DoubleStroke: ts << "double-stroke"; break;
      case StrokeStyle::WavyStroke:   ts << "wavy-stroke"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, TextDrawingMode textDrawingMode) noexcept
  {
    switch (textDrawingMode)
    {
      case TextDrawingMode::Fill:   ts << "fill"; break;
      case TextDrawingMode::Stroke: ts << "stroke"; break;
    }
    return ts;
  }
}
