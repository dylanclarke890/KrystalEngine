/*
 * Copyright (C) 2021-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorder.hpp"

namespace krys::boo::display_list
{
  /// @brief Records what's drawn into a display list.
  class RecorderImpl : public Recorder
  {
    KRYS_NON_COPYABLE(RecorderImpl);

  private:
    List<Item> _items;

  public:
    RecorderImpl(const GraphicsContextState &, const RectF &initialClip, const AffineTransform &) noexcept;

    explicit RecorderImpl(SizeF initialClipSize) noexcept : RecorderImpl({}, {{}, initialClipSize}, {})
    {
    }

    ~RecorderImpl() noexcept override;

    KRYS_NODISCARD Ref<const DisplayList> TakeDisplayList() noexcept;

    // This function is deprecated and sign that caller is doing something incorrect. This will be
    // removed once all clients are fixed.
    KRYS_NODISCARD Ref<const DisplayList> CopyDisplayList() noexcept;

    using GraphicsContext::FillRect;
    using GraphicsContext::Scale;
    using GraphicsContext::Translate;

    void Save(GraphicsContextState::Purpose = GraphicsContextState::Purpose::SaveRestore) noexcept final;

    void Restore(GraphicsContextState::Purpose = GraphicsContextState::Purpose::SaveRestore) noexcept final;

    void Translate(float x, float y) noexcept final;

    void Rotate(float angle) noexcept final;

    void Scale(const SizeF &) noexcept final;

    void SetCTM(const AffineTransform &) noexcept final;

    void ConcatCTM(const AffineTransform &) noexcept final;

    void SetLineCap(LineCap) noexcept final;

    void SetLineDash(const DashArray &, float dashOffset) noexcept final;

    void SetLineJoin(LineJoin) noexcept final;

    void SetMiterLimit(float) noexcept final;

    void ResetClip() noexcept final;

    void Clip(const RectF &) noexcept final;

    void ClipRoundedRect(const FloatRoundedRect &) noexcept final;

    void ClipOut(const RectF &) noexcept final;

    void ClipOut(const Path &) noexcept final;

    void ClipOutRoundedRect(const FloatRoundedRect &) noexcept final;

    void ClipPath(const Path &, WindRule = WindRule::EvenOdd) noexcept final;

    void ClipToImageBuffer(ImageBuffer &, const RectF &) noexcept final;

    void BeginTransparencyLayer(float) noexcept final;

    void BeginTransparencyLayer(CompositeOperator, boo::BlendMode = BlendMode::Normal) noexcept final;

    void EndTransparencyLayer() noexcept final;

    void DrawRect(const RectF &, float = 1) noexcept final;

    void DrawLine(const PointF &point1, const PointF &point2) noexcept final;

    void DrawLinesForText(const PointF &, float thickness, Span<const SegmentF>, bool isPrinting,
                          bool doubleLines, boo::StrokeStyle) noexcept final;

    void DrawDotsForDocumentMarker(const RectF &, DocumentMarkerLineStyle) noexcept final;

    void DrawEllipse(const RectF &) noexcept final;

    void DrawPath(const Path &) noexcept final;

    void DrawFocusRing(const Path &, float outlineWidth, const Color &) noexcept final;

    void DrawFocusRing(const List<RectF> &, float outlineOffset, float outlineWidth,
                       const Color &) noexcept final;

    void FillEllipse(const RectF &) noexcept final;

    void FillPath(const Path &) noexcept final;

    void FillRect(const RectF &, RequiresClipToRect = RequiresClipToRect::Yes) noexcept final;

    void FillRect(const RectF &, const Color &) noexcept final;

    void FillRect(const RectF &, Gradient &) noexcept final;

    void FillRect(const RectF &, Gradient &, const AffineTransform &,
                  RequiresClipToRect = RequiresClipToRect::Yes) noexcept final;

    void FillRect(const RectF &, const Color &, CompositeOperator,
                  boo::BlendMode = BlendMode::Normal) noexcept final;

    void FillRoundedRect(const FloatRoundedRect &, const Color &,
                         boo::BlendMode = BlendMode::Normal) noexcept final;

    void FillRectWithRoundedHole(const RectF &, const FloatRoundedRect &, const Color &) noexcept final;

    void DrawGlyphs(const Font &, Span<const GlyphBufferGlyph>, Span<const GlyphBufferAdvance>,
                    const PointF &localAnchor, FontSmoothingMode) noexcept final;

    void DrawGlyphsImmediate(const Font &, Span<const GlyphBufferGlyph>, Span<const GlyphBufferAdvance>,
                             const PointF &localAnchor, FontSmoothingMode) noexcept final;

    void DrawDisplayList(const DisplayList &) noexcept final;

    void DrawControlPart(ControlPart &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
                         const ControlStyle &) noexcept final;

    void DrawNativeImage(NativeImage &, const RectF &destRect, const RectF &srcRect,
                         ImagePaintingOptions) noexcept final;

    void DrawPattern(NativeImage &, const RectF &destRect, const RectF &tileRect, const AffineTransform &,
                     const PointF &phase, const SizeF &spacing, ImagePaintingOptions) noexcept final;

    void DrawFilteredImageBuffer(ImageBuffer *sourceImage, const RectF &sourceImageRect, Filter &,
                                 FilterResults &) noexcept override;

    /// @note Not final: an accelerated buffer's recorder draws a copy of an accelerated source.
    void DrawImageBuffer(ImageBuffer &, const RectF &destRect, const RectF &srcRect,
                         ImagePaintingOptions) noexcept override;

    void DrawPattern(ImageBuffer &, const RectF &destRect, const RectF &tileRect, const AffineTransform &,
                     const PointF &phase, const SizeF &spacing, ImagePaintingOptions) noexcept final;

    void StrokeRect(const RectF &, float) noexcept final;

    void StrokeEllipse(const RectF &) noexcept final;

    void StrokePath(const Path &) noexcept final;

    void ClearRect(const RectF &) noexcept final;

    void ApplyDeviceScaleFactor(float) noexcept final;

    void BeginPage(const RectF &) noexcept final;

    void EndPage() noexcept final;

    void SetUrlForRect(const net::Url &, const RectF &) noexcept final;

    // Appends a deferred placeholder command.
    // The function is called during the display list playback, drawDisplayList().
    // The function may be called multiple times.
    // The function may be called from an arbitrary thread.
    // The function should always produce the same GraphicsContext calls.
    void DrawPlaceholder(Func<void(GraphicsContext &)> &&) noexcept;

  private:
    void AppendStateChangeItemIfNecessary() noexcept final;
  };
}
