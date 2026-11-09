/*
 * Copyright (C) 2023-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class GraphicsContext;
}

namespace krys::boo::display_list
{
  class ApplyDeviceScaleFactor;
  class BeginTransparencyLayer;
  class BeginTransparencyLayerWithCompositeMode;
  class ClearRect;
  class Clip;
  class ClipRoundedRect;
  class ClipOut;
  class ClipOutRoundedRect;
  class ClipOutToPath;
  class ClipPath;
  class ClipToImageBuffer;
  class ConcatenateCTM;
  class DrawDotsForDocumentMarker;
  class DrawEllipse;
  class DrawFilteredImageBuffer;
  class DrawFocusRingPath;
  class DrawFocusRingRects;
  class DrawGlyphs;
  class DrawControlPart;
  class DrawDisplayList;
  class DrawImageBuffer;
  class DrawNativeImage;
  class DrawPatternImageBuffer;
  class DrawPatternNativeImage;
  class DrawPlaceholder;
  class DrawLine;
  class DrawLinesForText;
  class DrawPath;
  class DrawRect;
  class EndTransparencyLayer;
  class FillCompositedRect;
  class FillEllipse;
  class FillPath;
  class FillRect;
  class FillRectWithColor;
  class FillRectWithGradient;
  class FillRectWithGradientAndSpaceTransform;
  class FillRectWithRoundedHole;
  class FillRoundedRect;
  class ResetClip;
  class Restore;
  class Rotate;
  class Save;
  class Scale;
  class SetCTM;
  class SetInlineFillColor;
  class SetInlineStroke;
  class SetLineCap;
  class SetLineDash;
  class SetLineJoin;
  class SetMiterLimit;
  class SetState;
  class StrokeEllipse;
  class StrokePath;
  class StrokeRect;
  class Translate;
  class BeginPage;
  class EndPage;
  class SetURLForRect;

  // FIXME: The system image item (DrawSystemImage).
  using Item =
    Variant<ApplyDeviceScaleFactor, BeginTransparencyLayer, BeginTransparencyLayerWithCompositeMode,
            ClearRect, Clip, ClipRoundedRect, ClipOut, ClipOutRoundedRect, ClipOutToPath, ClipPath,
            ClipToImageBuffer, ConcatenateCTM, DrawControlPart, DrawDotsForDocumentMarker, DrawEllipse,
            DrawFilteredImageBuffer, DrawFocusRingPath, DrawFocusRingRects, DrawGlyphs, DrawDisplayList,
            DrawImageBuffer, DrawNativeImage, DrawPatternNativeImage, DrawPatternImageBuffer, DrawPlaceholder,
            DrawLine, DrawLinesForText, DrawPath, DrawRect, EndTransparencyLayer, FillCompositedRect,
            FillEllipse, FillPath, FillRect, FillRectWithColor, FillRectWithGradient,
            FillRectWithGradientAndSpaceTransform, FillRectWithRoundedHole, FillRoundedRect, ResetClip,
            Restore, Rotate, Save, Scale, SetCTM, SetInlineFillColor, SetInlineStroke, SetLineCap,
            SetLineDash, SetLineJoin, SetMiterLimit, SetState, StrokeEllipse, StrokePath, StrokeRect,
            Translate, BeginPage, EndPage, SetURLForRect>;

  enum class AsTextFlag : uint8
  {
    IncludePlatformOperations,
    IncludeResourceIdentifiers,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::display_list::AsTextFlag, 2uz);

namespace krys::boo::display_list
{
  void ApplyItem(GraphicsContext &, const Item &) noexcept;
  KRYS_NODISCARD bool ShouldDumpItem(const Item &, EnumSet<AsTextFlag>) noexcept;
  void DumpItem(TextStream &, const Item &, EnumSet<AsTextFlag>) noexcept;
  TextStream &operator<<(TextStream &, const Item &) noexcept;
}
