#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Core/Colour/ColourTextStream.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::display_list
{
  void Save::Apply(GraphicsContext &context) const noexcept
  {
    context.Save();
  }

  void Restore::Apply(GraphicsContext &context) const noexcept
  {
    context.Restore();
  }

  void Translate::Apply(GraphicsContext &context) const noexcept
  {
    context.Translate(_x, _y);
  }

  void Translate::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"x", X());
    ts.DumpProperty(u8"y", Y());
  }

  void Rotate::Apply(GraphicsContext &context) const noexcept
  {
    context.Rotate(_angle);
  }

  void Rotate::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"angle", Angle());
  }

  void Scale::Apply(GraphicsContext &context) const noexcept
  {
    context.Scale(_size);
  }

  void Scale::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"size", Amount());
  }

  void SetCTM::Apply(GraphicsContext &context) const noexcept
  {
    context.SetCTM(_transform);
  }

  void SetCTM::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"set-ctm", Transform());
  }

  void ConcatenateCTM::Apply(GraphicsContext &context) const noexcept
  {
    context.ConcatCTM(_transform);
  }

  void ConcatenateCTM::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"ctm", Transform());
  }

  void SetLineCap::Apply(GraphicsContext &context) const noexcept
  {
    context.SetLineCap(_lineCap);
  }

  void SetLineCap::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"line-cap", GetLineCap());
  }

  void SetLineDash::Apply(GraphicsContext &context) const noexcept
  {
    context.SetLineDash(_dashArray, _dashOffset);
  }

  void SetLineDash::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"dash-array", List<DashArrayElement> {GetDashArray().begin(), GetDashArray().end()});
    ts.DumpProperty(u8"dash-offset", DashOffset());
  }

  void SetLineJoin::Apply(GraphicsContext &context) const noexcept
  {
    context.SetLineJoin(_lineJoin);
  }

  void SetLineJoin::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"line-join", GetLineJoin());
  }

  void SetMiterLimit::Apply(GraphicsContext &context) const noexcept
  {
    context.SetMiterLimit(_miterLimit);
  }

  void SetMiterLimit::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"mitre-limit", MiterLimit());
  }

  void Clip::Apply(GraphicsContext &context) const noexcept
  {
    context.Clip(_rect);
  }

  void Clip::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void ClipRoundedRect::Apply(GraphicsContext &context) const noexcept
  {
    context.ClipRoundedRect(_rect);
  }

  void ClipRoundedRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void ClipOut::Apply(GraphicsContext &context) const noexcept
  {
    context.ClipOut(_rect);
  }

  void ClipOut::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void ClipOutRoundedRect::Apply(GraphicsContext &context) const noexcept
  {
    context.ClipOutRoundedRect(_rect);
  }

  void ClipOutRoundedRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void ClipOutToPath::Apply(GraphicsContext &context) const noexcept
  {
    context.ClipOut(_path);
  }

  void ClipOutToPath::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"path", GetPath());
  }

  void ClipPath::Apply(GraphicsContext &context) const noexcept
  {
    context.ClipPath(_path, _windRule);
  }

  void ClipPath::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"path", GetPath());
    ts.DumpProperty(u8"wind-rule", GetWindRule());
  }

  void ResetClip::Apply(GraphicsContext &context) const noexcept
  {
    context.ResetClip();
  }

  void BeginTransparencyLayer::Apply(GraphicsContext &context) const noexcept
  {
    context.BeginTransparencyLayer(_opacity);
  }

  void BeginTransparencyLayer::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"opacity", Opacity());
  }

  void BeginTransparencyLayerWithCompositeMode::Apply(GraphicsContext &context) const noexcept
  {
    context.BeginTransparencyLayer(_compositeMode.Operation, _compositeMode.BlendMode);
  }

  void BeginTransparencyLayerWithCompositeMode::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"composite-operator", GetCompositeMode().Operation);
    ts.DumpProperty(u8"blend-mode", GetCompositeMode().BlendMode);
  }

  void DrawRect::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawRect(_rect, _borderThickness);
  }

  void DrawRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
    ts.DumpProperty(u8"border-thickness", BorderThickness());
  }

  void DrawLine::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawLine(_point1, _point2);
  }

  void DrawLine::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"point-1", Point1());
    ts.DumpProperty(u8"point-2", Point2());
  }

  void DrawDotsForDocumentMarker::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawDotsForDocumentMarker(_rect, _style);
  }

  void DrawDotsForDocumentMarker::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void DrawEllipse::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawEllipse(_rect);
  }

  void DrawEllipse::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void DrawPath::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawPath(_path);
  }

  void DrawPath::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"path", GetPath());
  }

  void DrawFocusRingPath::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawFocusRing(_path, _outlineWidth, _color);
  }

  void DrawFocusRingPath::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"path", GetPath());
    ts.DumpProperty(u8"outline-width", OutlineWidth());
    ts.DumpProperty(u8"color", GetColor());
  }

  void DrawFocusRingRects::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawFocusRing(_rects, _outlineOffset, _outlineWidth, _color);
  }

  void DrawFocusRingRects::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rects", Rects());
    ts.DumpProperty(u8"outline-offset", OutlineOffset());
    ts.DumpProperty(u8"outline-width", OutlineWidth());
    ts.DumpProperty(u8"color", GetColor());
  }

  void FillRect::Apply(GraphicsContext &context) const noexcept
  {
    context.FillRect(_rect, _requiresClipToRect);
  }

  void FillRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
    ts.DumpProperty(u8"requiresClipToRect", _requiresClipToRect == GraphicsContext::RequiresClipToRect::Yes);
  }

  void FillRectWithColor::Apply(GraphicsContext &context) const noexcept
  {
    context.FillRect(_rect, _color);
  }

  void FillRectWithColor::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
    ts.DumpProperty(u8"color", GetColor());
  }

  void FillCompositedRect::Apply(GraphicsContext &context) const noexcept
  {
    context.FillRect(_rect, _color, _op, _blendMode);
  }

  void FillCompositedRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
    ts.DumpProperty(u8"color", GetColor());
    ts.DumpProperty(u8"composite-operation", GetCompositeOperator());
    ts.DumpProperty(u8"blend-mode", GetBlendMode());
  }

  void FillRoundedRect::Apply(GraphicsContext &context) const noexcept
  {
    context.FillRoundedRect(_rect, _color, _blendMode);
  }

  void FillRoundedRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", RoundedRect());
    ts.DumpProperty(u8"color", GetColor());
    ts.DumpProperty(u8"blend-mode", GetBlendMode());
  }

  void FillRectWithRoundedHole::Apply(GraphicsContext &context) const noexcept
  {
    context.FillRectWithRoundedHole(_rect, _roundedHoleRect, _color);
  }

  void FillRectWithRoundedHole::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
    ts.DumpProperty(u8"rounded-hole-rect", RoundedHoleRect());
    ts.DumpProperty(u8"color", GetColor());
  }

  void FillPath::Apply(GraphicsContext &context) const noexcept
  {
    context.FillPath(_path);
  }

  void FillPath::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"path", GetPath());
  }

  void FillEllipse::Apply(GraphicsContext &context) const noexcept
  {
    context.FillEllipse(_rect);
  }

  void FillEllipse::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void StrokeRect::Apply(GraphicsContext &context) const noexcept
  {
    context.StrokeRect(_rect, _lineWidth);
  }

  void StrokeRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
    ts.DumpProperty(u8"line-width", LineWidth());
  }

  void StrokePath::Apply(GraphicsContext &context) const noexcept
  {
    context.StrokePath(_path);
  }

  void StrokePath::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"path", GetPath());
  }

  void StrokeEllipse::Apply(GraphicsContext &context) const noexcept
  {
    context.StrokeEllipse(_rect);
  }

  void StrokeEllipse::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void ClearRect::Apply(GraphicsContext &context) const noexcept
  {
    context.ClearRect(_rect);
  }

  void ClearRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
  }

  void ApplyDeviceScaleFactor::Apply(GraphicsContext &context) const noexcept
  {
    context.ApplyDeviceScaleFactor(_scaleFactor);
  }

  void ApplyDeviceScaleFactor::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"scale-factor", ScaleFactor());
  }

  void BeginPage::Apply(GraphicsContext &context) const noexcept
  {
    context.BeginPage(_pageRect);
  }

  void BeginPage::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"page-rect", PageRect());
  }

  void EndPage::Apply(GraphicsContext &context) const noexcept
  {
    context.EndPage();
  }

  void SetURLForRect::Apply(GraphicsContext &context) const noexcept
  {
    context.SetURLForRect(_link, _destRect);
  }

  void SetURLForRect::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"link", Link().String());
    ts.DumpProperty(u8"dest_rect", DestRect());
  }

  void SetInlineFillColor::Apply(GraphicsContext &context) const noexcept
  {
    context.SetFillColor(GetColor());
  }

  void SetInlineFillColor::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"color", GetColor());
  }

  void SetInlineStroke::Apply(GraphicsContext &context) const noexcept
  {
    if (auto color = GetColor())
    {
      context.SetStrokeColor(*color);
    }
    if (auto thickness = Thickness())
    {
      context.SetStrokeThickness(*thickness);
    }
  }

  void SetInlineStroke::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    if (auto color = GetColor())
    {
      ts.DumpProperty(u8"color", *color);
    }
    if (auto thickness = Thickness())
    {
      ts.DumpProperty(u8"thickness", *thickness);
    }
  }

  SetState::SetState(const GraphicsContextState &state) noexcept : _state(state)
  {
  }

  void SetState::Apply(GraphicsContext &context) const noexcept
  {
    context.MergeLastChanges(_state);
  }

  void SetState::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts << State();
  }

  DrawGlyphs::DrawGlyphs(Ref<const boo::Font> &&font, List<GlyphBufferGlyph> &&glyphs,
                         List<GlyphBufferAdvance> &&advances, const FloatPoint &localAnchor,
                         FontSmoothingMode smoothingMode) noexcept
      : _font(krys::move(font)), _glyphs(krys::move(glyphs)), _advances(krys::move(advances)),
        _localAnchor(localAnchor), _fontSmoothingMode(smoothingMode)
  {
  }

  void DrawGlyphs::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawGlyphs(*_font, _glyphs, _advances, _localAnchor, _fontSmoothingMode);
  }

  void DrawGlyphs::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    // FIXME: dump more stuff.
    ts.DumpProperty(u8"local-anchor", LocalAnchor());
    ts.DumpProperty(u8"font-smoothing-mode", GetFontSmoothingMode());
    ts.DumpProperty(u8"length", Length());
  }

  DrawControlPart::DrawControlPart(ControlPart &part, const FloatRoundedRect &borderRect,
                                   float deviceScaleFactor, const ControlStyle &style) noexcept
      : _part(ShareRef(part)), _borderRect(borderRect), _deviceScaleFactor(deviceScaleFactor), _style(style)
  {
  }

  DrawControlPart::DrawControlPart(const DrawControlPart &) noexcept = default;
  DrawControlPart::DrawControlPart(DrawControlPart &&) noexcept = default;
  DrawControlPart &DrawControlPart::operator=(const DrawControlPart &) noexcept = default;
  DrawControlPart &DrawControlPart::operator=(DrawControlPart &&) noexcept = default;
  DrawControlPart::~DrawControlPart() noexcept = default;

  ControlPart &DrawControlPart::Part() const noexcept
  {
    return *_part;
  }

  StyleAppearance DrawControlPart::GetType() const noexcept
  {
    return _part->GetType();
  }

  DrawDisplayList::DrawDisplayList(Ref<const boo::display_list::DisplayList> &&displayList) noexcept
      : _displayList(krys::move(displayList))
  {
  }

  DrawDisplayList::DrawDisplayList(const DrawDisplayList &) noexcept = default;
  DrawDisplayList::DrawDisplayList(DrawDisplayList &&) noexcept = default;
  DrawDisplayList &DrawDisplayList::operator=(const DrawDisplayList &) noexcept = default;
  DrawDisplayList &DrawDisplayList::operator=(DrawDisplayList &&) noexcept = default;
  DrawDisplayList::~DrawDisplayList() noexcept = default;

  Ref<const boo::display_list::DisplayList> DrawDisplayList::GetDisplayList() const noexcept
  {
    return _displayList;
  }

  ClipToImageBuffer::ClipToImageBuffer(ImageBuffer &imageBuffer, const FloatRect &destinationRect) noexcept
      : _imageBuffer(ShareRef(imageBuffer)), _destinationRect(destinationRect)
  {
  }

  ClipToImageBuffer::ClipToImageBuffer(const ClipToImageBuffer &) noexcept = default;
  ClipToImageBuffer::ClipToImageBuffer(ClipToImageBuffer &&) noexcept = default;
  ClipToImageBuffer &ClipToImageBuffer::operator=(const ClipToImageBuffer &) noexcept = default;
  ClipToImageBuffer &ClipToImageBuffer::operator=(ClipToImageBuffer &&) noexcept = default;
  ClipToImageBuffer::~ClipToImageBuffer() noexcept = default;

  ImageBuffer &ClipToImageBuffer::GetImageBuffer() const noexcept
  {
    return *_imageBuffer;
  }

  void ClipToImageBuffer::Apply(GraphicsContext &context) const noexcept
  {
    context.ClipToImageBuffer(*_imageBuffer, _destinationRect);
  }

  void ClipToImageBuffer::Dump(TextStream &ts, EnumSet<AsTextFlag> flags) const noexcept
  {
    if (flags.Contains(AsTextFlag::IncludeResourceIdentifiers))
    {
      ts.DumpProperty(u8"image-buffer-identifier", _imageBuffer->GetRenderingResourceIdentifier());
    }
    ts.DumpProperty(u8"dest-rect", DestinationRect());
  }

  DrawImageBuffer::DrawImageBuffer(ImageBuffer &imageBuffer, const FloatRect &destRect,
                                   const FloatRect &srcRect, ImagePaintingOptions options) noexcept
      : _imageBuffer(ShareRef(imageBuffer)), _destinationRect(destRect), _srcRect(srcRect), _options(options)
  {
  }

  DrawImageBuffer::DrawImageBuffer(const DrawImageBuffer &) noexcept = default;
  DrawImageBuffer::DrawImageBuffer(DrawImageBuffer &&) noexcept = default;
  DrawImageBuffer &DrawImageBuffer::operator=(const DrawImageBuffer &) noexcept = default;
  DrawImageBuffer &DrawImageBuffer::operator=(DrawImageBuffer &&) noexcept = default;
  DrawImageBuffer::~DrawImageBuffer() noexcept = default;

  ImageBuffer &DrawImageBuffer::GetImageBuffer() const noexcept
  {
    return *_imageBuffer;
  }

  void DrawImageBuffer::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawImageBuffer(*_imageBuffer, _destinationRect, _srcRect, _options);
  }

  void DrawImageBuffer::Dump(TextStream &ts, EnumSet<AsTextFlag> flags) const noexcept
  {
    if (flags.Contains(AsTextFlag::IncludeResourceIdentifiers))
    {
      ts.DumpProperty(u8"image-buffer-identifier", _imageBuffer->GetRenderingResourceIdentifier());
    }
    ts.DumpProperty(u8"source-rect", Source());
    ts.DumpProperty(u8"dest-rect", DestinationRect());
  }

  DrawNativeImage::DrawNativeImage(NativeImage &image, const FloatRect &destRect, const FloatRect &srcRect,
                                   ImagePaintingOptions options) noexcept
      : _image(ShareRef(image)), _destinationRect(destRect), _srcRect(srcRect), _options(options)
  {
  }

  DrawNativeImage::DrawNativeImage(const DrawNativeImage &) noexcept = default;
  DrawNativeImage::DrawNativeImage(DrawNativeImage &&) noexcept = default;
  DrawNativeImage &DrawNativeImage::operator=(const DrawNativeImage &) noexcept = default;
  DrawNativeImage &DrawNativeImage::operator=(DrawNativeImage &&) noexcept = default;
  DrawNativeImage::~DrawNativeImage() noexcept = default;

  NativeImage &DrawNativeImage::GetNativeImage() const noexcept
  {
    return *_image;
  }

  void DrawNativeImage::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawNativeImage(*_image, _destinationRect, _srcRect, _options);
  }

  void DrawNativeImage::Dump(TextStream &ts, EnumSet<AsTextFlag> flags) const noexcept
  {
    if (flags.Contains(AsTextFlag::IncludeResourceIdentifiers))
    {
      ts.DumpProperty(u8"image-identifier", _image->GetRenderingResourceIdentifier());
    }
    ts.DumpProperty(u8"source-rect", Source());
    ts.DumpProperty(u8"dest-rect", DestinationRect());
  }

  DrawPatternNativeImage::DrawPatternNativeImage(NativeImage &image, const FloatRect &destRect,
                                                 const FloatRect &tileRect,
                                                 const AffineTransform &patternTransform,
                                                 const FloatPoint &phase, const FloatSize &spacing,
                                                 ImagePaintingOptions options) noexcept
      : _image(ShareRef(image)), _destination(destRect), _tileRect(tileRect),
        _patternTransform(patternTransform), _phase(phase), _spacing(spacing), _options(options)
  {
  }

  DrawPatternNativeImage::DrawPatternNativeImage(const DrawPatternNativeImage &) noexcept = default;
  DrawPatternNativeImage::DrawPatternNativeImage(DrawPatternNativeImage &&) noexcept = default;
  DrawPatternNativeImage &
    DrawPatternNativeImage::operator=(const DrawPatternNativeImage &) noexcept = default;
  DrawPatternNativeImage &DrawPatternNativeImage::operator=(DrawPatternNativeImage &&) noexcept = default;
  DrawPatternNativeImage::~DrawPatternNativeImage() noexcept = default;

  NativeImage &DrawPatternNativeImage::GetNativeImage() const noexcept
  {
    return *_image;
  }

  void DrawPatternNativeImage::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawPattern(*_image, _destination, _tileRect, _patternTransform, _phase, _spacing, _options);
  }

  void DrawPatternNativeImage::Dump(TextStream &ts, EnumSet<AsTextFlag> flags) const noexcept
  {
    if (flags.Contains(AsTextFlag::IncludeResourceIdentifiers))
    {
      ts.DumpProperty(u8"image-identifier", _image->GetRenderingResourceIdentifier());
    }
    ts.DumpProperty(u8"pattern-transform", PatternTransform());
    ts.DumpProperty(u8"tile-rect", TileRect());
    ts.DumpProperty(u8"dest-rect", DestRect());
    ts.DumpProperty(u8"phase", Phase());
    ts.DumpProperty(u8"spacing", Spacing());
  }

  DrawPatternImageBuffer::DrawPatternImageBuffer(ImageBuffer &image, const FloatRect &destRect,
                                                 const FloatRect &tileRect,
                                                 const AffineTransform &patternTransform,
                                                 const FloatPoint &phase, const FloatSize &spacing,
                                                 ImagePaintingOptions options) noexcept
      : _imageBuffer(ShareRef(image)), _destination(destRect), _tileRect(tileRect),
        _patternTransform(patternTransform), _phase(phase), _spacing(spacing), _options(options)
  {
  }

  DrawPatternImageBuffer::DrawPatternImageBuffer(const DrawPatternImageBuffer &) noexcept = default;
  DrawPatternImageBuffer::DrawPatternImageBuffer(DrawPatternImageBuffer &&) noexcept = default;
  DrawPatternImageBuffer &
    DrawPatternImageBuffer::operator=(const DrawPatternImageBuffer &) noexcept = default;
  DrawPatternImageBuffer &DrawPatternImageBuffer::operator=(DrawPatternImageBuffer &&) noexcept = default;
  DrawPatternImageBuffer::~DrawPatternImageBuffer() noexcept = default;

  ImageBuffer &DrawPatternImageBuffer::GetImageBuffer() const noexcept
  {
    return *_imageBuffer;
  }

  void DrawPatternImageBuffer::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawPattern(*_imageBuffer, _destination, _tileRect, _patternTransform, _phase, _spacing,
                        _options);
  }

  void DrawPatternImageBuffer::Dump(TextStream &ts, EnumSet<AsTextFlag> flags) const noexcept
  {
    if (flags.Contains(AsTextFlag::IncludeResourceIdentifiers))
    {
      ts.DumpProperty(u8"image-identifier", _imageBuffer->GetRenderingResourceIdentifier());
    }
    ts.DumpProperty(u8"pattern-transform", PatternTransform());
    ts.DumpProperty(u8"tile-rect", TileRect());
    ts.DumpProperty(u8"dest-rect", DestRect());
    ts.DumpProperty(u8"phase", Phase());
    ts.DumpProperty(u8"spacing", Spacing());
  }

  void DrawControlPart::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawControlPart(*_part, _borderRect, _deviceScaleFactor, _style);
  }

  void DrawControlPart::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"type", GetType());
    ts.DumpProperty(u8"border-rect", BorderRect());
    ts.DumpProperty(u8"device-scale-factor", DeviceScaleFactor());
    ts.DumpProperty(u8"style", Style());
  }

  void DrawDisplayList::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawDisplayList(*_displayList);
  }

  void DrawDisplayList::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"display-list", *_displayList);
  }

  DrawPlaceholder::DrawPlaceholder(Func<void(GraphicsContext &)> &&function) noexcept
      : _function(krys::move(function))
  {
  }

  void DrawPlaceholder::Apply(GraphicsContext &context) const noexcept
  {
    _function(context);
  }

  void DrawPlaceholder::Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept
  {
  }

  DrawLinesForText::DrawLinesForText(const FloatPoint &point, Span<const FloatSegment> lineSegments,
                                     float thickness, bool printing, bool doubleLines,
                                     boo::StrokeStyle style) noexcept
      : _point(point), _lineSegments(lineSegments.begin(), lineSegments.end()), _thickness(thickness),
        _printing(printing), _doubleLines(doubleLines), _style(style)
  {
  }

  void DrawLinesForText::Apply(GraphicsContext &context) const noexcept
  {
    context.DrawLinesForText(_point, _thickness, _lineSegments, _printing, _doubleLines, _style);
  }

  void DrawLinesForText::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"point", Point());
    ts.DumpProperty(u8"thickness", Thickness());
    ts.DumpProperty(u8"double", DoubleLines());
    ts.DumpProperty(u8"lineSegments", LineSegments());
    ts.DumpProperty(u8"is-printing", IsPrinting());
    ts.DumpProperty(u8"double", DoubleLines());
  }

  FillRectWithGradient::FillRectWithGradient(const FloatRect &rect, boo::Gradient &gradient) noexcept
      : _rect(rect), _gradient(ShareRef(gradient))
  {
  }

  FillRectWithGradient::FillRectWithGradient(FloatRect &&rect, Ref<boo::Gradient> &&gradient) noexcept
      : _rect(krys::move(rect)), _gradient(krys::move(gradient))
  {
  }

  void FillRectWithGradient::Apply(GraphicsContext &context) const noexcept
  {
    context.FillRect(_rect, *_gradient);
  }

  void FillRectWithGradient::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    ts.DumpProperty(u8"rect", Rect());
    ts.DumpProperty(u8"gradient", *_gradient);
  }

  FillRectWithGradientAndSpaceTransform::FillRectWithGradientAndSpaceTransform(
    const FloatRect &rect, boo::Gradient &gradient, const AffineTransform &gradientSpaceTransform,
    GraphicsContext::RequiresClipToRect requiresClipToRect) noexcept
      : _rect(rect), _gradient(ShareRef(gradient)), _gradientSpaceTransform(gradientSpaceTransform),
        _requiresClipToRect(requiresClipToRect)
  {
  }

  FillRectWithGradientAndSpaceTransform::FillRectWithGradientAndSpaceTransform(
    FloatRect &&rect, Ref<boo::Gradient> &&gradient, AffineTransform &&gradientSpaceTransform,
    GraphicsContext::RequiresClipToRect requiresClipToRect) noexcept
      : _rect(krys::move(rect)), _gradient(krys::move(gradient)),
        _gradientSpaceTransform(krys::move(gradientSpaceTransform)), _requiresClipToRect(requiresClipToRect)
  {
  }

  void FillRectWithGradientAndSpaceTransform::Apply(GraphicsContext &context) const noexcept
  {
    context.FillRect(_rect, *_gradient, _gradientSpaceTransform, _requiresClipToRect);
  }

  void FillRectWithGradientAndSpaceTransform::Dump(TextStream &ts, EnumSet<AsTextFlag>) const noexcept
  {
    // FIXME: log gradient.
    ts.DumpProperty(u8"rect", Rect());
    ts.DumpProperty(u8"gradient-space-transform", GradientSpaceTransform());
    ts.DumpProperty(u8"requiresClipToRect", _requiresClipToRect == GraphicsContext::RequiresClipToRect::Yes);
  }

  void EndTransparencyLayer::Apply(GraphicsContext &context) const noexcept
  {
    if (context.IsInTransparencyLayer())
    {
      context.EndTransparencyLayer();
    }
  }
}
