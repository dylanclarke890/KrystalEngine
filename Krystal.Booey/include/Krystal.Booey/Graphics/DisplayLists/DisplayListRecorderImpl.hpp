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
    RecorderImpl(const GraphicsContextState &, const FloatRect &initialClip,
                 const AffineTransform &) noexcept;
    explicit RecorderImpl(FloatSize initialClipSize) noexcept : RecorderImpl({}, {{}, initialClipSize}, {})
    {
    }
    virtual ~RecorderImpl() noexcept;

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
    void Scale(const FloatSize &) noexcept final;
    void SetCTM(const AffineTransform &) noexcept final;
    void ConcatCTM(const AffineTransform &) noexcept final;

    void SetLineCap(LineCap) noexcept final;
    void SetLineDash(const DashArray &, float dashOffset) noexcept final;
    void SetLineJoin(LineJoin) noexcept final;
    void SetMiterLimit(float) noexcept final;

    void ResetClip() noexcept final;
    void Clip(const FloatRect &) noexcept final;
    void ClipRoundedRect(const FloatRoundedRect &) noexcept final;
    void ClipOut(const FloatRect &) noexcept final;
    void ClipOut(const Path &) noexcept final;
    void ClipOutRoundedRect(const FloatRoundedRect &) noexcept final;
    void ClipPath(const Path &, WindRule = WindRule::EvenOdd) noexcept final;
    void ClipToImageBuffer(ImageBuffer &, const FloatRect &) noexcept final;

    void BeginTransparencyLayer(float) noexcept final;
    void BeginTransparencyLayer(CompositeOperator, boo::BlendMode = BlendMode::Normal) noexcept final;
    void EndTransparencyLayer() noexcept final;

    void DrawRect(const FloatRect &, float = 1) noexcept final;
    void DrawLine(const FloatPoint &point1, const FloatPoint &point2) noexcept final;
    void DrawLinesForText(const FloatPoint &, float thickness, Span<const FloatSegment>, bool isPrinting,
                          bool doubleLines, boo::StrokeStyle) noexcept final;
    void DrawDotsForDocumentMarker(const FloatRect &, DocumentMarkerLineStyle) noexcept final;
    void DrawEllipse(const FloatRect &) noexcept final;
    void DrawPath(const Path &) noexcept final;

    void DrawFocusRing(const Path &, float outlineWidth, const Color &) noexcept final;
    void DrawFocusRing(const List<FloatRect> &, float outlineOffset, float outlineWidth,
                       const Color &) noexcept final;

    void FillEllipse(const FloatRect &) noexcept final;
    void FillPath(const Path &) noexcept final;
    void FillRect(const FloatRect &, RequiresClipToRect = RequiresClipToRect::Yes) noexcept final;
    void FillRect(const FloatRect &, const Color &) noexcept final;
    void FillRect(const FloatRect &, Gradient &) noexcept final;
    void FillRect(const FloatRect &, Gradient &, const AffineTransform &,
                  RequiresClipToRect = RequiresClipToRect::Yes) noexcept final;
    void FillRect(const FloatRect &, const Color &, CompositeOperator,
                  boo::BlendMode = BlendMode::Normal) noexcept final;
    void FillRoundedRect(const FloatRoundedRect &, const Color &,
                         boo::BlendMode = BlendMode::Normal) noexcept final;
    void FillRectWithRoundedHole(const FloatRect &, const FloatRoundedRect &, const Color &) noexcept final;

    void DrawGlyphs(const Font &, Span<const GlyphBufferGlyph>, Span<const GlyphBufferAdvance>,
                    const FloatPoint &localAnchor, FontSmoothingMode) noexcept final;
    void DrawGlyphsImmediate(const Font &, Span<const GlyphBufferGlyph>, Span<const GlyphBufferAdvance>,
                             const FloatPoint &localAnchor, FontSmoothingMode) noexcept final;
    void DrawDisplayList(const DisplayList &) noexcept final;
    void DrawControlPart(ControlPart &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
                         const ControlStyle &) noexcept final;
    void DrawNativeImage(NativeImage &, const FloatRect &destRect, const FloatRect &srcRect,
                         ImagePaintingOptions) noexcept final;
    void DrawPattern(NativeImage &, const FloatRect &destRect, const FloatRect &tileRect,
                     const AffineTransform &, const FloatPoint &phase, const FloatSize &spacing,
                     ImagePaintingOptions) noexcept final;
    void DrawImageBuffer(ImageBuffer &, const FloatRect &destRect, const FloatRect &srcRect,
                         ImagePaintingOptions) noexcept final;
    void DrawPattern(ImageBuffer &, const FloatRect &destRect, const FloatRect &tileRect,
                     const AffineTransform &, const FloatPoint &phase, const FloatSize &spacing,
                     ImagePaintingOptions) noexcept final;

    void StrokeRect(const FloatRect &, float) noexcept final;
    void StrokeEllipse(const FloatRect &) noexcept final;
    void StrokePath(const Path &) noexcept final;

    void ClearRect(const FloatRect &) noexcept final;

    void ApplyDeviceScaleFactor(float) noexcept final;

    void BeginPage(const FloatRect &) noexcept final;
    void EndPage() noexcept final;

    void SetURLForRect(const url::URL &, const FloatRect &) noexcept final;

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
