/*
 * Copyright (C) 2015-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  bool AlwaysPageBreak(BreakBetween between) noexcept
  {
    return between >= BreakBetween::Page;
  }

  CSSBoxType TransformBoxToCSSBoxType(TransformBox transformBox) noexcept
  {
    switch (transformBox)
    {
      case TransformBox::StrokeBox:  return CSSBoxType::StrokeBox;
      case TransformBox::ContentBox: return CSSBoxType::ContentBox;
      case TransformBox::BorderBox:  return CSSBoxType::BorderBox;
      case TransformBox::FillBox:    return CSSBoxType::FillBox;
      case TransformBox::ViewBox:    return CSSBoxType::ViewBox;
      default:                       krys_debug_assert_msg(false, "Unexpected transform box"); return CSSBoxType::BorderBox;
    }
  }

  TextStream &operator<<(TextStream &ts, ImageRendering imageRendering) noexcept
  {
    switch (imageRendering)
    {
      case ImageRendering::Auto:            ts << u8"auto"; break;
      case ImageRendering::OptimizeSpeed:   ts << u8"optimizeSpeed"; break;
      case ImageRendering::OptimizeQuality: ts << u8"optimizeQuality"; break;
      case ImageRendering::CrispEdges:      ts << u8"crispEdges"; break;
      case ImageRendering::Pixelated:       ts << u8"pixelated"; break;
    }
    return ts;
  }
}
