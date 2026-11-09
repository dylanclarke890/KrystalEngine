/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Rik Cabanier (cabanier@adobe.com)
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
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

  bool ParseCompositeAndBlendOperator(utf8_stringview name, CompositeOperator &op,
                                      BlendMode &blendOp) noexcept
  {
    for (uint32 i = 0; i < std::size(compositeOperatorNames); ++i)
    {
      // `darker` isn't a standard operator, and `difference` is a blend mode.
      const auto candidate = static_cast<CompositeOperator>(i);
      if (candidate == CompositeOperator::PlusDarker || candidate == CompositeOperator::Difference)
      {
        continue;
      }
      if (std::ranges::equal(name, compositeOperatorNames[i], {}, {},
                             [](char c) { return static_cast<char8>(c); }))
      {
        op = candidate;
        blendOp = BlendMode::Normal;
        return true;
      }
    }

    if (ParseBlendMode(name, blendOp))
    {
      // For now, blending will always assume source-over. This will be fixed in the future
      op = CompositeOperator::SourceOver;
      return true;
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
