/*
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Graphics/GraphicsContext.hpp"

namespace krys::web
{
  enum class NullGraphicsContextPaintInvalidationReasons : uint8
  {
    None,
    InvalidatingControlTints,
    InvalidatingImagesWithAsyncDecodes,
    DetectingContentfulPaint,
  };

  /// @brief A graphics context that draws nothing, for painting that only computes (invalidation, contentful
  /// paint detection).
  class NullGraphicsContext : public GraphicsContext
  {
  public:
    using PaintInvalidationReasons = NullGraphicsContextPaintInvalidationReasons;

  private:
    const PaintInvalidationReasons _paintInvalidationReasons {PaintInvalidationReasons::None};

  public:
    NullGraphicsContext() noexcept = default;

    NullGraphicsContext(PaintInvalidationReasons reasons) noexcept : _paintInvalidationReasons(reasons)
    {
    }

  private:
    KRYS_NODISCARD bool PaintingDisabled() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool PerformingPaintInvalidation() const noexcept final
    {
      return _paintInvalidationReasons != PaintInvalidationReasons::None;
    }

    KRYS_NODISCARD bool InvalidatingControlTints() const noexcept final
    {
      return _paintInvalidationReasons == PaintInvalidationReasons::InvalidatingControlTints;
    }

    KRYS_NODISCARD bool InvalidatingImagesWithAsyncDecodes() const noexcept final
    {
      return _paintInvalidationReasons == PaintInvalidationReasons::InvalidatingImagesWithAsyncDecodes;
    }

    KRYS_NODISCARD bool DetectingContentfulPaint() const noexcept final
    {
      return _paintInvalidationReasons == PaintInvalidationReasons::DetectingContentfulPaint;
    }

    void DidUpdateState(GraphicsContextState &) noexcept final
    {
    }

    KRYS_NODISCARD RectI ClipBounds() const noexcept final
    {
      return {};
    }

    void DrawRect(const RectF &, float = 1) noexcept final
    {
    }

    void DrawLine(const PointF &, const PointF &) noexcept final
    {
    }

    void DrawEllipse(const RectF &) noexcept final
    {
    }

    void FillPath(const Path &) noexcept final
    {
    }

    void StrokePath(const Path &) noexcept final
    {
    }

    void FillRect(const RectF &, RequiresClipToRect) noexcept final
    {
    }

    void FillRect(const RectF &, Gradient &, const AffineTransform &, RequiresClipToRect) noexcept final
    {
    }

    void FillRect(const RectF &, const Color &) noexcept final
    {
    }

    void FillRoundedRectImpl(const FloatRoundedRect &, const Color &) noexcept final
    {
    }

    void StrokeRect(const RectF &, float) noexcept final
    {
    }

    void ClipPath(const Path &, WindRule = WindRule::EvenOdd) noexcept final
    {
    }

    void ClipToImageBuffer(ImageBuffer &, const RectF &) noexcept final
    {
    }

    void DrawLinesForText(const PointF &, float, Span<const SegmentF>, bool, bool,
                          web::StrokeStyle) noexcept final
    {
    }

    void SetLineCap(LineCap) noexcept override
    {
    }

    void SetLineDash(const DashArray &, float) noexcept override
    {
    }

    void SetLineJoin(LineJoin) noexcept override
    {
    }

    void SetMiterLimit(float) noexcept override
    {
    }

    void ClipOut(const Path &) noexcept final
    {
    }

    void Scale(const SizeF &) noexcept final
    {
    }

    void Rotate(float) noexcept final
    {
    }

    void Translate(float, float) noexcept final
    {
    }

    void ConcatCTM(const AffineTransform &) noexcept final
    {
    }

    void SetCTM(const AffineTransform &) noexcept final
    {
    }

    KRYS_NODISCARD AffineTransform
      GetCTM(IncludeDeviceScale = PossiblyIncludeDeviceScale) const noexcept final
    {
      return {};
    }

    void ClearRect(const RectF &) noexcept final
    {
    }

    void ResetClip() noexcept final
    {
    }

    void Clip(const RectF &) noexcept final
    {
    }

    void ClipOut(const RectF &) noexcept final
    {
    }

    void Save(GraphicsContextState::Purpose = GraphicsContextState::Purpose::SaveRestore) noexcept final
    {
    }

    void Restore(GraphicsContextState::Purpose = GraphicsContextState::Purpose::SaveRestore) noexcept final
    {
    }

    void DrawRaisedEllipse(const RectF &, const Color &, const Color &) noexcept final
    {
    }

    void DrawGlyphs(const Font &, Span<const GlyphBufferGlyph>, Span<const GlyphBufferAdvance>,
                    const PointF &, FontSmoothingMode) noexcept final
    {
    }

    void DrawDisplayList(const display_list::DisplayList &) noexcept final
    {
    }

    void DrawDotsForDocumentMarker(const RectF &, DocumentMarkerLineStyle) noexcept final
    {
    }

    void DrawFocusRing(const Path &, float, const Color &) noexcept final
    {
    }

    void DrawFocusRing(const List<RectF> &, float, float, const Color &) noexcept final
    {
    }

    void ClipRoundedRect(const FloatRoundedRect &) noexcept final
    {
    }

    void ClipOutRoundedRect(const FloatRoundedRect &) noexcept final
    {
    }

    void FillRect(const RectF &, Gradient &) noexcept final
    {
    }

    void FillRect(const RectF &, const Color &, CompositeOperator,
                  web::BlendMode = BlendMode::Normal) noexcept final
    {
    }

    void FillRoundedRect(const FloatRoundedRect &, const Color &,
                         web::BlendMode = BlendMode::Normal) noexcept final
    {
    }

    void FillRectWithRoundedHole(const RectF &, const FloatRoundedRect &, const Color &) noexcept final
    {
    }

    ImageDrawResult DrawImage(Image &, const RectF &, const RectF &,
                              ImagePaintingOptions = ImagePaintingOptions {
                                img::ImageOrientation::Orientation::FromImage}) noexcept final
    {
      return ImageDrawResult::DidNothing;
    }

    ImageDrawResult DrawTiledImage(Image &, const RectF &, const PointF &, const SizeF &, const SizeF &,
                                   ImagePaintingOptions = {}) noexcept final
    {
      return ImageDrawResult::DidNothing;
    }

    void DrawNativeImage(NativeImage &, const RectF &, const RectF &,
                         ImagePaintingOptions = {}) noexcept final
    {
    }

    void DrawPattern(NativeImage &, const RectF &, const RectF &, const AffineTransform &, const PointF &,
                     const SizeF &, ImagePaintingOptions = {}) noexcept final
    {
    }

    void DrawImageBuffer(ImageBuffer &, const RectF &, const RectF &,
                         ImagePaintingOptions = {}) noexcept final
    {
    }

    void DrawFilteredImageBuffer(ImageBuffer *, const RectF &, Filter &, FilterResults &) noexcept final
    {
    }

    void DrawPattern(ImageBuffer &, const RectF &, const RectF &, const AffineTransform &, const PointF &,
                     const SizeF &, ImagePaintingOptions = {}) noexcept final
    {
    }

    ImageDrawResult DrawTiledImage(Image &, const RectF &, const RectF &, const SizeF &, Image::TileRule,
                                   Image::TileRule, ImagePaintingOptions = {}) noexcept final
    {
      return ImageDrawResult::DidNothing;
    }
  };
}
