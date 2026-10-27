#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"

namespace krys::boo::display_list
{
  RecorderImpl::RecorderImpl(const GraphicsContextState &state, const FloatRect &initialClip,
                             const AffineTransform &initialCTM) noexcept
      : Recorder(state, initialClip, initialCTM)
  {
  }

  RecorderImpl::~RecorderImpl() noexcept
  {
    krys_debug_assert(StateStack().size() == 1); // If this fires, it indicates mismatched save/restore.
  }

  Ref<const DisplayList> RecorderImpl::TakeDisplayList() noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.shrink_to_fit();
    return DisplayList::Create(krys::move(_items));
  }

  Ref<const DisplayList> RecorderImpl::CopyDisplayList() noexcept
  {
    AppendStateChangeItemIfNecessary();
    return DisplayList::Create(List<Item>(_items));
  }

  void RecorderImpl::Save(GraphicsContextState::Purpose purpose) noexcept
  {
    UpdateStateForSave(purpose);
    _items.push_back(display_list::Save());
  }

  void RecorderImpl::Restore(GraphicsContextState::Purpose purpose) noexcept
  {
    if (StateStack().size() <= 1)
    {
      return;
    }

    if (!UpdateStateForRestore(purpose))
    {
      return;
    }
    _items.push_back(display_list::Restore());
  }

  void RecorderImpl::Translate(float x, float y) noexcept
  {
    if (!UpdateStateForTranslate(x, y))
    {
      return;
    }
    _items.push_back(display_list::Translate(x, y));
  }

  void RecorderImpl::Rotate(float angle) noexcept
  {
    if (!UpdateStateForRotate(angle))
    {
      return;
    }
    _items.push_back(display_list::Rotate(angle));
  }

  void RecorderImpl::Scale(const FloatSize &scale) noexcept
  {
    if (!UpdateStateForScale(scale))
    {
      return;
    }
    _items.push_back(display_list::Scale(scale));
  }

  void RecorderImpl::SetCTM(const AffineTransform &transform) noexcept
  {
    UpdateStateForSetCTM(transform);
    _items.push_back(display_list::SetCTM(transform));
  }

  void RecorderImpl::ConcatCTM(const AffineTransform &transform) noexcept
  {
    if (!UpdateStateForConcatCTM(transform))
    {
      return;
    }
    _items.push_back(ConcatenateCTM(transform));
  }

  void RecorderImpl::SetLineCap(LineCap lineCap) noexcept
  {
    _items.push_back(display_list::SetLineCap(lineCap));
  }

  void RecorderImpl::SetLineDash(const DashArray &array, float dashOffset) noexcept
  {
    _items.push_back(display_list::SetLineDash(array, dashOffset));
  }

  void RecorderImpl::SetLineJoin(LineJoin join) noexcept
  {
    _items.push_back(display_list::SetLineJoin(join));
  }

  void RecorderImpl::SetMiterLimit(float limit) noexcept
  {
    _items.push_back(display_list::SetMiterLimit(limit));
  }

  void RecorderImpl::ResetClip() noexcept
  {
    UpdateStateForResetClip();
    _items.push_back(display_list::ResetClip());
    Clip(InitialClip());
  }

  void RecorderImpl::Clip(const FloatRect &clipRect) noexcept
  {
    UpdateStateForClip(clipRect);
    _items.push_back(display_list::Clip(clipRect));
  }

  void RecorderImpl::ClipRoundedRect(const FloatRoundedRect &clipRect) noexcept
  {
    UpdateStateForClipRoundedRect(clipRect);
    _items.push_back(display_list::ClipRoundedRect(clipRect));
  }

  void RecorderImpl::ClipOut(const FloatRect &clipRect) noexcept
  {
    UpdateStateForClipOut(clipRect);
    _items.push_back(display_list::ClipOut(clipRect));
  }

  void RecorderImpl::ClipOutRoundedRect(const FloatRoundedRect &clipRect) noexcept
  {
    UpdateStateForClipOutRoundedRect(clipRect);
    _items.push_back(display_list::ClipOutRoundedRect(clipRect));
  }

  void RecorderImpl::ClipOut(const Path &path) noexcept
  {
    UpdateStateForClipOut(path);
    _items.push_back(ClipOutToPath(path));
  }

  void RecorderImpl::ClipPath(const Path &path, WindRule rule) noexcept
  {
    UpdateStateForClipPath(path);
    _items.push_back(display_list::ClipPath(path, rule));
  }

  void RecorderImpl::ClipToImageBuffer(ImageBuffer &imageBuffer, const FloatRect &destinationRect) noexcept
  {
    UpdateStateForClipToImageBuffer(destinationRect);
    _items.push_back(display_list::ClipToImageBuffer(imageBuffer, destinationRect));
  }

  void RecorderImpl::DrawGlyphs(const Font &font, Span<const GlyphBufferGlyph> glyphs,
                                Span<const GlyphBufferAdvance> advances, const FloatPoint &localAnchor,
                                FontSmoothingMode smoothingMode) noexcept
  {
    DrawGlyphsImmediate(font, glyphs, advances, localAnchor, smoothingMode);
  }

  void RecorderImpl::DrawGlyphsImmediate(const Font &font, Span<const GlyphBufferGlyph> glyphs,
                                         Span<const GlyphBufferAdvance> advances,
                                         const FloatPoint &localAnchor,
                                         FontSmoothingMode smoothingMode) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawGlyphs(
      ShareRef(font), List<GlyphBufferGlyph>(glyphs.begin(), glyphs.end()),
      List<GlyphBufferAdvance>(advances.begin(), advances.end()), localAnchor, smoothingMode));
  }

  void RecorderImpl::DrawDisplayList(const DisplayList &displayList) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawDisplayList(ShareRef(displayList)));
  }

  void RecorderImpl::DrawControlPart(ControlPart &part, const FloatRoundedRect &borderRect,
                                     float deviceScaleFactor, const ControlStyle &style) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawControlPart(part, borderRect, deviceScaleFactor, style));
  }

  void RecorderImpl::DrawNativeImage(NativeImage &image, const FloatRect &destRect, const FloatRect &srcRect,
                                     ImagePaintingOptions options) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawNativeImage(image, destRect, srcRect, options));
  }

  void RecorderImpl::DrawPattern(NativeImage &image, const FloatRect &destRect, const FloatRect &tileRect,
                                 const AffineTransform &patternTransform, const FloatPoint &phase,
                                 const FloatSize &spacing, ImagePaintingOptions options) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawPatternNativeImage(image, destRect, tileRect, patternTransform, phase,
                                                          spacing, options));
  }

  void RecorderImpl::DrawFilteredImageBuffer(ImageBuffer *sourceImage, const FloatRect &sourceImageRect,
                                             Filter &filter, FilterResults &) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(
      display_list::DrawFilteredImageBuffer(ShareRefPtr(sourceImage), sourceImageRect, filter));
  }

  void RecorderImpl::DrawImageBuffer(ImageBuffer &imageBuffer, const FloatRect &destRect,
                                     const FloatRect &srcRect, ImagePaintingOptions options) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawImageBuffer(imageBuffer, destRect, srcRect, options));
  }

  void RecorderImpl::DrawPattern(ImageBuffer &imageBuffer, const FloatRect &destRect,
                                 const FloatRect &tileRect, const AffineTransform &patternTransform,
                                 const FloatPoint &phase, const FloatSize &spacing,
                                 ImagePaintingOptions options) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawPatternImageBuffer(imageBuffer, destRect, tileRect, patternTransform,
                                                          phase, spacing, options));
  }

  void RecorderImpl::BeginTransparencyLayer(float opacity) noexcept
  {
    UpdateStateForBeginTransparencyLayer(opacity);
    _items.push_back(display_list::BeginTransparencyLayer(opacity));
  }

  void RecorderImpl::BeginTransparencyLayer(CompositeOperator compositeOperator,
                                            boo::BlendMode blendMode) noexcept
  {
    UpdateStateForBeginTransparencyLayer(compositeOperator, blendMode);
    _items.push_back(BeginTransparencyLayerWithCompositeMode({compositeOperator, blendMode}));
  }

  void RecorderImpl::EndTransparencyLayer() noexcept
  {
    if (UpdateStateForEndTransparencyLayer())
    {
      _items.push_back(display_list::EndTransparencyLayer());
    }
  }

  void RecorderImpl::DrawRect(const FloatRect &rect, float lineWidth) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawRect(rect, lineWidth));
  }

  void RecorderImpl::DrawLine(const FloatPoint &point1, const FloatPoint &point2) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawLine(point1, point2));
  }

  void RecorderImpl::DrawLinesForText(const FloatPoint &point, float thickness,
                                      Span<const FloatSegment> lineSegments, bool printing, bool doubleLines,
                                      boo::StrokeStyle style) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(
      display_list::DrawLinesForText(point, lineSegments, thickness, printing, doubleLines, style));
  }

  void RecorderImpl::DrawDotsForDocumentMarker(const FloatRect &rect, DocumentMarkerLineStyle style) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawDotsForDocumentMarker(rect, style));
  }

  void RecorderImpl::DrawEllipse(const FloatRect &rect) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawEllipse(rect));
  }

  void RecorderImpl::DrawPath(const Path &path) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::DrawPath(path));
  }

  void RecorderImpl::DrawFocusRing(const Path &path, float outlineWidth, const Color &color) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(DrawFocusRingPath(path, outlineWidth, color));
  }

  void RecorderImpl::DrawFocusRing(const List<FloatRect> &rects, float outlineOffset, float outlineWidth,
                                   const Color &color) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(DrawFocusRingRects(rects, outlineOffset, outlineWidth, color));
  }

  void RecorderImpl::FillRect(const FloatRect &rect, RequiresClipToRect requiresClipToRect) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::FillRect(rect, requiresClipToRect));
  }

  void RecorderImpl::FillRect(const FloatRect &rect, const Color &color) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(FillRectWithColor(rect, color));
  }

  void RecorderImpl::FillRect(const FloatRect &rect, Gradient &gradient) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(FillRectWithGradient(rect, gradient));
  }

  void RecorderImpl::FillRect(const FloatRect &rect, Gradient &gradient,
                              const AffineTransform &gradientSpaceTransform,
                              RequiresClipToRect requiresClipToRect) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(
      FillRectWithGradientAndSpaceTransform(rect, gradient, gradientSpaceTransform, requiresClipToRect));
  }

  void RecorderImpl::FillRect(const FloatRect &rect, const Color &color, CompositeOperator op,
                              boo::BlendMode mode) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(FillCompositedRect(rect, color, op, mode));
  }

  void RecorderImpl::FillRoundedRect(const FloatRoundedRect &rect, const Color &color,
                                     boo::BlendMode mode) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::FillRoundedRect(rect, color, mode));
  }

  void RecorderImpl::FillRectWithRoundedHole(const FloatRect &rect, const FloatRoundedRect &roundedRect,
                                             const Color &color) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::FillRectWithRoundedHole(rect, roundedRect, color));
  }

  void RecorderImpl::FillPath(const Path &path) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::FillPath(path));
  }

  void RecorderImpl::FillEllipse(const FloatRect &rect) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::FillEllipse(rect));
  }

  void RecorderImpl::StrokeRect(const FloatRect &rect, float width) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::StrokeRect(rect, width));
  }

  void RecorderImpl::StrokePath(const Path &path) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::StrokePath(path));
  }

  void RecorderImpl::StrokeEllipse(const FloatRect &rect) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::StrokeEllipse(rect));
  }

  void RecorderImpl::ClearRect(const FloatRect &rect) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::ClearRect(rect));
  }

  void RecorderImpl::ApplyDeviceScaleFactor(float scaleFactor) noexcept
  {
    UpdateStateForApplyDeviceScaleFactor(scaleFactor);
    _items.push_back(display_list::ApplyDeviceScaleFactor(scaleFactor));
  }

  void RecorderImpl::BeginPage(const FloatRect &pageRect) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::BeginPage(pageRect));
  }

  void RecorderImpl::EndPage() noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::EndPage());
  }

  void RecorderImpl::SetURLForRect(const url::URL &link, const FloatRect &destRect) noexcept
  {
    AppendStateChangeItemIfNecessary();
    _items.push_back(display_list::SetURLForRect(link, destRect));
  }

  void RecorderImpl::AppendStateChangeItemIfNecessary() noexcept
  {
    auto &state = CurrentState().State;
    auto changes = state.Changes();
    if (!changes)
    {
      return;
    }

    auto recordFullItem = [&]
    {
      _items.push_back(SetState(state));
      state.DidApplyChanges();
      CurrentState().LastDrawingState = state;
    };

    if (!changes.ContainsOnly({GraphicsContextState::Change::FillBrush,
                               GraphicsContextState::Change::StrokeBrush,
                               GraphicsContextState::Change::StrokeThickness}))
    {
      recordFullItem();
      return;
    }

    Maybe<PackedColour::RGBA> fillColor;
    if (changes.Contains(GraphicsContextState::Change::FillBrush))
    {
      fillColor = state.FillBrush().PackedColor();
      if (!fillColor)
      {
        recordFullItem();
        return;
      }
    }

    Maybe<PackedColour::RGBA> strokeColor;
    if (changes.Contains(GraphicsContextState::Change::StrokeBrush))
    {
      strokeColor = state.StrokeBrush().PackedColor();
      if (!strokeColor)
      {
        recordFullItem();
        return;
      }
    }

    Maybe<float> strokeThickness;
    if (changes.Contains(GraphicsContextState::Change::StrokeThickness))
    {
      strokeThickness = state.StrokeThickness();
    }

    if (fillColor)
    {
      _items.push_back(SetInlineFillColor(*fillColor));
    }

    if (strokeColor || strokeThickness)
    {
      _items.push_back(SetInlineStroke(strokeColor, strokeThickness));
    }

    state.DidApplyChanges();
    CurrentState().LastDrawingState = state;
  }

  void RecorderImpl::DrawPlaceholder(Func<void(GraphicsContext &)> &&function) noexcept
  {
    _items.push_back(display_list::DrawPlaceholder(krys::move(function)));
  }
}
