#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Graphics/DashArray.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItem.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FloatSegment.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/GlyphBufferMembers.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::display_list
{
  class DisplayList;

  class Save
  {
  public:
    static constexpr std::string_view Name = "save";

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept
    {
    }
  };

  class Restore
  {
  public:
    static constexpr std::string_view Name = "restore";

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept
    {
    }
  };

  class Translate
  {
  private:
    float _x;
    float _y;

  public:
    static constexpr std::string_view Name = "translate";

    Translate(float x, float y) noexcept : _x(x), _y(y)
    {
    }

    KRYS_NODISCARD float X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD float Y() const noexcept
    {
      return _y;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class Rotate
  {
  private:
    float _angle;

  public:
    static constexpr std::string_view Name = "rotate";

    explicit Rotate(float angle) noexcept : _angle(angle)
    {
    }

    KRYS_NODISCARD float Angle() const noexcept
    {
      return _angle;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class Scale
  {
  private:
    FloatSize _size;

  public:
    static constexpr std::string_view Name = "scale";

    explicit Scale(const FloatSize &size) noexcept : _size(size)
    {
    }

    KRYS_NODISCARD const FloatSize &Amount() const noexcept
    {
      return _size;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class SetCTM
  {
  private:
    AffineTransform _transform;

  public:
    static constexpr std::string_view Name = "set-ctm";

    explicit SetCTM(const AffineTransform &transform) noexcept : _transform(transform)
    {
    }

    KRYS_NODISCARD const AffineTransform &Transform() const noexcept
    {
      return _transform;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ConcatenateCTM
  {
  private:
    AffineTransform _transform;

  public:
    static constexpr std::string_view Name = "concatenate-ctm";

    explicit ConcatenateCTM(const AffineTransform &transform) noexcept : _transform(transform)
    {
    }

    KRYS_NODISCARD const AffineTransform &Transform() const noexcept
    {
      return _transform;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class SetInlineFillColor
  {
  private:
    PackedColour::RGBA _colorData;

  public:
    static constexpr std::string_view Name = "set-inline-fill-color";

    explicit SetInlineFillColor(PackedColour::RGBA colorData) noexcept : _colorData(colorData)
    {
    }
    explicit SetInlineFillColor(SRGBA<uint8> colorData) noexcept
        : _colorData(*boo::Color(colorData).TryGetAsPackedInline())
    {
    }

    KRYS_NODISCARD boo::Color GetColor() const noexcept
    {
      return {AsSRGBA(ColorData())};
    }
    KRYS_NODISCARD const PackedColour::RGBA &ColorData() const noexcept
    {
      return _colorData;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class SetInlineStroke
  {
  private:
    Maybe<PackedColour::RGBA> _colorData;
    Maybe<float> _thickness;

  public:
    static constexpr std::string_view Name = "set-inline-stroke";

    SetInlineStroke(Maybe<PackedColour::RGBA> colorData, Maybe<float> thickness = std::nullopt) noexcept
        : _colorData(colorData), _thickness(thickness)
    {
      krys_debug_assert(_colorData || _thickness);
    }
    explicit SetInlineStroke(float thickness) noexcept : _thickness(thickness)
    {
    }
    explicit SetInlineStroke(SRGBA<uint8> colorData) noexcept : _colorData(PackedColour::RGBA(colorData))
    {
    }

    KRYS_NODISCARD Maybe<boo::Color> GetColor() const noexcept
    {
      return _colorData ? Maybe<boo::Color>(AsSRGBA(*_colorData)) : std::nullopt;
    }
    KRYS_NODISCARD Maybe<PackedColour::RGBA> ColorData() const noexcept
    {
      return _colorData;
    }
    KRYS_NODISCARD Maybe<float> Thickness() const noexcept
    {
      return _thickness;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class SetState
  {
  private:
    GraphicsContextState _state;

  public:
    static constexpr std::string_view Name = "set-state";

    explicit SetState(const GraphicsContextState &) noexcept;

    KRYS_NODISCARD GraphicsContextState &State() noexcept
    {
      return _state;
    }
    KRYS_NODISCARD const GraphicsContextState &State() const noexcept
    {
      return _state;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class SetLineCap
  {
  private:
    LineCap _lineCap;

  public:
    static constexpr std::string_view Name = "set-line-cap";

    explicit SetLineCap(LineCap lineCap) noexcept : _lineCap(lineCap)
    {
    }

    KRYS_NODISCARD LineCap GetLineCap() const noexcept
    {
      return _lineCap;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class SetLineDash
  {
  private:
    DashArray _dashArray;
    float _dashOffset;

  public:
    static constexpr std::string_view Name = "set-line-dash";

    SetLineDash(const DashArray &dashArray, float dashOffset) noexcept
        : _dashArray(dashArray), _dashOffset(dashOffset)
    {
    }

    KRYS_NODISCARD const DashArray &GetDashArray() const noexcept
    {
      return _dashArray;
    }
    KRYS_NODISCARD float DashOffset() const noexcept
    {
      return _dashOffset;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class SetLineJoin
  {
  private:
    LineJoin _lineJoin;

  public:
    static constexpr std::string_view Name = "set-line-join";

    explicit SetLineJoin(LineJoin lineJoin) noexcept : _lineJoin(lineJoin)
    {
    }

    KRYS_NODISCARD LineJoin GetLineJoin() const noexcept
    {
      return _lineJoin;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class SetMiterLimit
  {
  private:
    float _miterLimit;

  public:
    static constexpr std::string_view Name = "set-miter-limit";

    explicit SetMiterLimit(float miterLimit) noexcept : _miterLimit(miterLimit)
    {
    }

    KRYS_NODISCARD float MiterLimit() const noexcept
    {
      return _miterLimit;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class Clip
  {
  private:
    FloatRect _rect;

  public:
    static constexpr std::string_view Name = "clip";

    explicit Clip(const FloatRect &rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ClipRoundedRect
  {
  private:
    FloatRoundedRect _rect;

  public:
    static constexpr std::string_view Name = "clip-rounded-rect";

    explicit ClipRoundedRect(const FloatRoundedRect &rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD const FloatRoundedRect &Rect() const noexcept
    {
      return _rect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ClipOut
  {
  private:
    FloatRect _rect;

  public:
    static constexpr std::string_view Name = "clip-out";

    explicit ClipOut(const FloatRect &rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ClipOutRoundedRect
  {
  private:
    FloatRoundedRect _rect;

  public:
    static constexpr std::string_view Name = "clip-out-rounded-rect";

    explicit ClipOutRoundedRect(const FloatRoundedRect &rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD const FloatRoundedRect &Rect() const noexcept
    {
      return _rect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ClipOutToPath
  {
  private:
    boo::Path _path;

  public:
    static constexpr std::string_view Name = "clip-out-to-path";

    explicit ClipOutToPath(const boo::Path &path) noexcept : _path(path)
    {
    }

    KRYS_NODISCARD const boo::Path &GetPath() const noexcept
    {
      return _path;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ClipPath
  {
  private:
    boo::Path _path;
    boo::WindRule _windRule;

  public:
    static constexpr std::string_view Name = "clip-path";

    ClipPath(const boo::Path &path, boo::WindRule windRule) noexcept : _path(path), _windRule(windRule)
    {
    }

    KRYS_NODISCARD const boo::Path &GetPath() const noexcept
    {
      return _path;
    }
    KRYS_NODISCARD boo::WindRule GetWindRule() const noexcept
    {
      return _windRule;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ResetClip
  {
  public:
    static constexpr std::string_view Name = "reset-clip";

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept
    {
    }
  };

  class DrawGlyphs
  {
  private:
    Ref<const boo::Font> _font;
    List<GlyphBufferGlyph> _glyphs;
    List<GlyphBufferAdvance> _advances;
    FloatPoint _localAnchor;
    FontSmoothingMode _fontSmoothingMode;

  public:
    static constexpr std::string_view Name = "draw-glyphs";

    DrawGlyphs(Ref<const boo::Font> &&font, List<GlyphBufferGlyph> &&glyphs,
               List<GlyphBufferAdvance> &&advances, const FloatPoint &localAnchor,
               FontSmoothingMode smoothingMode) noexcept;

    KRYS_NODISCARD Ref<const boo::Font> GetFont() const noexcept
    {
      return _font;
    }
    KRYS_NODISCARD const List<GlyphBufferGlyph> &Glyphs() const noexcept
    {
      return _glyphs;
    }
    KRYS_NODISCARD const List<GlyphBufferAdvance> &Advances() const noexcept
    {
      return _advances;
    }
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _glyphs.size();
    }
    KRYS_NODISCARD FloatPoint LocalAnchor() const noexcept
    {
      return _localAnchor;
    }
    KRYS_NODISCARD FontSmoothingMode GetFontSmoothingMode() const noexcept
    {
      return _fontSmoothingMode;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawDisplayList
  {
  private:
    Ref<const boo::display_list::DisplayList> _displayList;

  public:
    static constexpr std::string_view Name = "draw-display-list";

    explicit DrawDisplayList(Ref<const boo::display_list::DisplayList> &&) noexcept;
    DrawDisplayList(const DrawDisplayList &) noexcept;
    DrawDisplayList(DrawDisplayList &&) noexcept;
    DrawDisplayList &operator=(const DrawDisplayList &) noexcept;
    DrawDisplayList &operator=(DrawDisplayList &&) noexcept;
    ~DrawDisplayList() noexcept;

    KRYS_NODISCARD Ref<const boo::display_list::DisplayList> GetDisplayList() const noexcept;

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawImageBuffer
  {
  private:
    Ref<ImageBuffer> _imageBuffer;
    FloatRect _destinationRect;
    FloatRect _srcRect;
    ImagePaintingOptions _options;

  public:
    static constexpr std::string_view Name = "draw-image-buffer";

    DrawImageBuffer(ImageBuffer &, const FloatRect &destRect, const FloatRect &srcRect,
                    ImagePaintingOptions) noexcept;
    DrawImageBuffer(const DrawImageBuffer &) noexcept;
    DrawImageBuffer(DrawImageBuffer &&) noexcept;
    DrawImageBuffer &operator=(const DrawImageBuffer &) noexcept;
    DrawImageBuffer &operator=(DrawImageBuffer &&) noexcept;
    ~DrawImageBuffer() noexcept;

    KRYS_NODISCARD ImageBuffer &GetImageBuffer() const noexcept;
    KRYS_NODISCARD FloatRect Source() const noexcept
    {
      return _srcRect;
    }
    KRYS_NODISCARD FloatRect DestinationRect() const noexcept
    {
      return _destinationRect;
    }
    KRYS_NODISCARD ImagePaintingOptions Options() const noexcept
    {
      return _options;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawNativeImage
  {
  private:
    Ref<NativeImage> _image;
    FloatRect _destinationRect;
    FloatRect _srcRect;
    ImagePaintingOptions _options;

  public:
    static constexpr std::string_view Name = "draw-native-image";

    DrawNativeImage(NativeImage &, const FloatRect &destRect, const FloatRect &srcRect,
                    ImagePaintingOptions) noexcept;
    DrawNativeImage(const DrawNativeImage &) noexcept;
    DrawNativeImage(DrawNativeImage &&) noexcept;
    DrawNativeImage &operator=(const DrawNativeImage &) noexcept;
    DrawNativeImage &operator=(DrawNativeImage &&) noexcept;
    ~DrawNativeImage() noexcept;

    KRYS_NODISCARD NativeImage &GetNativeImage() const noexcept;
    KRYS_NODISCARD FloatRect DestinationRect() const noexcept
    {
      return _destinationRect;
    }
    KRYS_NODISCARD FloatRect Source() const noexcept
    {
      return _srcRect;
    }
    KRYS_NODISCARD ImagePaintingOptions Options() const noexcept
    {
      return _options;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawPatternNativeImage
  {
  private:
    Ref<NativeImage> _image;
    FloatRect _destination;
    FloatRect _tileRect;
    AffineTransform _patternTransform;
    FloatPoint _phase;
    FloatSize _spacing;
    ImagePaintingOptions _options;

  public:
    static constexpr std::string_view Name = "draw-pattern-native-image";

    DrawPatternNativeImage(NativeImage &, const FloatRect &destRect, const FloatRect &tileRect,
                           const AffineTransform &patternTransform, const FloatPoint &phase,
                           const FloatSize &spacing, ImagePaintingOptions) noexcept;
    DrawPatternNativeImage(const DrawPatternNativeImage &) noexcept;
    DrawPatternNativeImage(DrawPatternNativeImage &&) noexcept;
    DrawPatternNativeImage &operator=(const DrawPatternNativeImage &) noexcept;
    DrawPatternNativeImage &operator=(DrawPatternNativeImage &&) noexcept;
    ~DrawPatternNativeImage() noexcept;

    KRYS_NODISCARD NativeImage &GetNativeImage() const noexcept;
    KRYS_NODISCARD FloatRect DestRect() const noexcept
    {
      return _destination;
    }
    KRYS_NODISCARD FloatRect TileRect() const noexcept
    {
      return _tileRect;
    }
    KRYS_NODISCARD const AffineTransform &PatternTransform() const noexcept
    {
      return _patternTransform;
    }
    KRYS_NODISCARD FloatPoint Phase() const noexcept
    {
      return _phase;
    }
    KRYS_NODISCARD FloatSize Spacing() const noexcept
    {
      return _spacing;
    }
    KRYS_NODISCARD ImagePaintingOptions Options() const noexcept
    {
      return _options;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawPatternImageBuffer
  {
  private:
    Ref<ImageBuffer> _imageBuffer;
    FloatRect _destination;
    FloatRect _tileRect;
    AffineTransform _patternTransform;
    FloatPoint _phase;
    FloatSize _spacing;
    ImagePaintingOptions _options;

  public:
    static constexpr std::string_view Name = "draw-pattern-imagebuffer";

    DrawPatternImageBuffer(ImageBuffer &, const FloatRect &destRect, const FloatRect &tileRect,
                           const AffineTransform &patternTransform, const FloatPoint &phase,
                           const FloatSize &spacing, ImagePaintingOptions) noexcept;
    DrawPatternImageBuffer(const DrawPatternImageBuffer &) noexcept;
    DrawPatternImageBuffer(DrawPatternImageBuffer &&) noexcept;
    DrawPatternImageBuffer &operator=(const DrawPatternImageBuffer &) noexcept;
    DrawPatternImageBuffer &operator=(DrawPatternImageBuffer &&) noexcept;
    ~DrawPatternImageBuffer() noexcept;

    KRYS_NODISCARD ImageBuffer &GetImageBuffer() const noexcept;
    KRYS_NODISCARD FloatRect DestRect() const noexcept
    {
      return _destination;
    }
    KRYS_NODISCARD FloatRect TileRect() const noexcept
    {
      return _tileRect;
    }
    KRYS_NODISCARD const AffineTransform &PatternTransform() const noexcept
    {
      return _patternTransform;
    }
    KRYS_NODISCARD FloatPoint Phase() const noexcept
    {
      return _phase;
    }
    KRYS_NODISCARD FloatSize Spacing() const noexcept
    {
      return _spacing;
    }
    KRYS_NODISCARD ImagePaintingOptions Options() const noexcept
    {
      return _options;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawPlaceholder
  {
  private:
    Func<void(GraphicsContext &)> _function;

  public:
    static constexpr std::string_view Name = "draw-placeholder";

    explicit DrawPlaceholder(Func<void(GraphicsContext &)> &&) noexcept;

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class BeginTransparencyLayer
  {
  private:
    float _opacity;

  public:
    static constexpr std::string_view Name = "begin-transparency-layer";

    explicit BeginTransparencyLayer(float opacity) noexcept : _opacity(opacity)
    {
    }

    KRYS_NODISCARD float Opacity() const noexcept
    {
      return _opacity;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class BeginTransparencyLayerWithCompositeMode
  {
  private:
    boo::CompositeMode _compositeMode;

  public:
    static constexpr std::string_view Name = "begin-transparency-layer-with-composite-mode";

    explicit BeginTransparencyLayerWithCompositeMode(boo::CompositeMode compositeMode) noexcept
        : _compositeMode(compositeMode)
    {
    }

    KRYS_NODISCARD boo::CompositeMode GetCompositeMode() const noexcept
    {
      return _compositeMode;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class EndTransparencyLayer
  {
  public:
    static constexpr std::string_view Name = "end-transparency-layer";

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept
    {
    }
  };

  class DrawRect
  {
  private:
    FloatRect _rect;
    float _borderThickness;

  public:
    static constexpr std::string_view Name = "draw-rect";

    DrawRect(FloatRect rect, float borderThickness) noexcept : _rect(rect), _borderThickness(borderThickness)
    {
    }

    KRYS_NODISCARD FloatRect Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD float BorderThickness() const noexcept
    {
      return _borderThickness;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawLine
  {
  private:
    FloatPoint _point1;
    FloatPoint _point2;

  public:
    static constexpr std::string_view Name = "draw-line";

    DrawLine(FloatPoint point1, FloatPoint point2) noexcept : _point1(point1), _point2(point2)
    {
    }

    KRYS_NODISCARD FloatPoint Point1() const noexcept
    {
      return _point1;
    }
    KRYS_NODISCARD FloatPoint Point2() const noexcept
    {
      return _point2;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawLinesForText
  {
  private:
    FloatPoint _point;
    List<FloatSegment> _lineSegments;
    float _thickness;
    bool _printing;
    bool _doubleLines;
    boo::StrokeStyle _style;

  public:
    static constexpr std::string_view Name = "draw-lines-for-text";

    DrawLinesForText(const FloatPoint &, Span<const FloatSegment> lineSegments, float thickness,
                     bool printing, bool doubleLines, boo::StrokeStyle) noexcept;

    KRYS_NODISCARD FloatPoint Point() const noexcept
    {
      return _point;
    }
    KRYS_NODISCARD float Thickness() const noexcept
    {
      return _thickness;
    }
    KRYS_NODISCARD const List<FloatSegment> &LineSegments() const noexcept
    {
      return _lineSegments;
    }
    KRYS_NODISCARD bool IsPrinting() const noexcept
    {
      return _printing;
    }
    KRYS_NODISCARD bool DoubleLines() const noexcept
    {
      return _doubleLines;
    }
    KRYS_NODISCARD boo::StrokeStyle Style() const noexcept
    {
      return _style;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawDotsForDocumentMarker
  {
  private:
    FloatRect _rect;
    DocumentMarkerLineStyle _style;

  public:
    static constexpr std::string_view Name = "draw-dots-for-document-marker";

    DrawDotsForDocumentMarker(FloatRect rect, DocumentMarkerLineStyle style) noexcept
        : _rect(rect), _style(style)
    {
    }

    KRYS_NODISCARD FloatRect Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD DocumentMarkerLineStyle Style() const noexcept
    {
      return _style;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawEllipse
  {
  private:
    FloatRect _rect;

  public:
    static constexpr std::string_view Name = "draw-ellipse";

    explicit DrawEllipse(FloatRect rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD FloatRect Rect() const noexcept
    {
      return _rect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawPath
  {
  private:
    boo::Path _path;

  public:
    static constexpr std::string_view Name = "draw-path";

    explicit DrawPath(const boo::Path &path) noexcept : _path(path)
    {
    }

    KRYS_NODISCARD const boo::Path &GetPath() const noexcept
    {
      return _path;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawFocusRingPath
  {
  private:
    boo::Path _path;
    float _outlineWidth;
    boo::Color _color;

  public:
    static constexpr std::string_view Name = "draw-focus-ring-path";

    DrawFocusRingPath(const boo::Path &path, float outlineWidth, const boo::Color &color) noexcept
        : _path(path), _outlineWidth(outlineWidth), _color(color)
    {
    }

    KRYS_NODISCARD const boo::Path &GetPath() const noexcept
    {
      return _path;
    }
    KRYS_NODISCARD float OutlineWidth() const noexcept
    {
      return _outlineWidth;
    }
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class DrawFocusRingRects
  {
  private:
    List<FloatRect> _rects;
    float _outlineOffset;
    float _outlineWidth;
    boo::Color _color;

  public:
    static constexpr std::string_view Name = "draw-focus-ring-rects";

    DrawFocusRingRects(const List<FloatRect> &rects, float outlineOffset, float outlineWidth,
                       const boo::Color &color) noexcept
        : _rects(rects), _outlineOffset(outlineOffset), _outlineWidth(outlineWidth), _color(color)
    {
    }

    KRYS_NODISCARD const List<FloatRect> &Rects() const noexcept
    {
      return _rects;
    }
    KRYS_NODISCARD float OutlineOffset() const noexcept
    {
      return _outlineOffset;
    }
    KRYS_NODISCARD float OutlineWidth() const noexcept
    {
      return _outlineWidth;
    }
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillRect
  {
  private:
    FloatRect _rect;
    GraphicsContext::RequiresClipToRect _requiresClipToRect;

  public:
    static constexpr std::string_view Name = "fill-rect";

    FillRect(const FloatRect &rect, GraphicsContext::RequiresClipToRect requiresClipToRect) noexcept
        : _rect(rect), _requiresClipToRect(requiresClipToRect)
    {
    }

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD GraphicsContext::RequiresClipToRect GetRequiresClipToRect() const noexcept
    {
      return _requiresClipToRect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillRectWithColor
  {
  private:
    FloatRect _rect;
    boo::Color _color;

  public:
    static constexpr std::string_view Name = "fill-rect-with-color";

    FillRectWithColor(FloatRect rect, const boo::Color &color) noexcept : _rect(rect), _color(color)
    {
    }

    KRYS_NODISCARD FloatRect Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillRectWithGradient
  {
  private:
    FloatRect _rect;
    const Ref<boo::Gradient> _gradient;

  public:
    static constexpr std::string_view Name = "fill-rect-with-gradient";

    FillRectWithGradient(const FloatRect &, boo::Gradient &) noexcept;
    FillRectWithGradient(FloatRect &&, Ref<boo::Gradient> &&) noexcept;

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD const boo::Gradient &GetGradient() const noexcept
    {
      return *_gradient;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillRectWithGradientAndSpaceTransform
  {
  private:
    FloatRect _rect;
    const Ref<boo::Gradient> _gradient;
    AffineTransform _gradientSpaceTransform;
    GraphicsContext::RequiresClipToRect _requiresClipToRect;

  public:
    static constexpr std::string_view Name = "fill-rect-with-gradient-and-space-transform";

    FillRectWithGradientAndSpaceTransform(const FloatRect &, boo::Gradient &, const AffineTransform &,
                                          GraphicsContext::RequiresClipToRect) noexcept;
    FillRectWithGradientAndSpaceTransform(FloatRect &&, Ref<boo::Gradient> &&, AffineTransform &&,
                                          GraphicsContext::RequiresClipToRect) noexcept;

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD const boo::Gradient &GetGradient() const noexcept
    {
      return *_gradient;
    }
    KRYS_NODISCARD const AffineTransform &GradientSpaceTransform() const noexcept
    {
      return _gradientSpaceTransform;
    }
    KRYS_NODISCARD GraphicsContext::RequiresClipToRect GetRequiresClipToRect() const noexcept
    {
      return _requiresClipToRect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillCompositedRect
  {
  private:
    FloatRect _rect;
    boo::Color _color;
    CompositeOperator _op;
    boo::BlendMode _blendMode;

  public:
    static constexpr std::string_view Name = "fill-composited-rect";

    FillCompositedRect(FloatRect rect, const boo::Color &color, CompositeOperator op,
                       boo::BlendMode blendMode) noexcept
        : _rect(rect), _color(color), _op(op), _blendMode(blendMode)
    {
    }

    KRYS_NODISCARD FloatRect Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }
    KRYS_NODISCARD CompositeOperator GetCompositeOperator() const noexcept
    {
      return _op;
    }
    KRYS_NODISCARD boo::BlendMode GetBlendMode() const noexcept
    {
      return _blendMode;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillRoundedRect
  {
  private:
    FloatRoundedRect _rect;
    boo::Color _color;
    boo::BlendMode _blendMode;

  public:
    static constexpr std::string_view Name = "fill-rounded-rect";

    FillRoundedRect(const FloatRoundedRect &rect, const boo::Color &color, boo::BlendMode blendMode) noexcept
        : _rect(rect), _color(color), _blendMode(blendMode)
    {
    }

    KRYS_NODISCARD const FloatRoundedRect &RoundedRect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }
    KRYS_NODISCARD boo::BlendMode GetBlendMode() const noexcept
    {
      return _blendMode;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillRectWithRoundedHole
  {
  private:
    FloatRect _rect;
    FloatRoundedRect _roundedHoleRect;
    boo::Color _color;

  public:
    static constexpr std::string_view Name = "fill-rect-with-rounded-hole";

    FillRectWithRoundedHole(const FloatRect &rect, const FloatRoundedRect &roundedHoleRect,
                            const boo::Color &color) noexcept
        : _rect(rect), _roundedHoleRect(roundedHoleRect), _color(color)
    {
    }

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD const FloatRoundedRect &RoundedHoleRect() const noexcept
    {
      return _roundedHoleRect;
    }
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillPath
  {
  private:
    boo::Path _path;

  public:
    static constexpr std::string_view Name = "fill-path";

    explicit FillPath(const boo::Path &path) noexcept : _path(path)
    {
    }

    KRYS_NODISCARD const boo::Path &GetPath() const noexcept
    {
      return _path;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class FillEllipse
  {
  private:
    FloatRect _rect;

  public:
    static constexpr std::string_view Name = "fill-ellipse";

    explicit FillEllipse(FloatRect rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD FloatRect Rect() const noexcept
    {
      return _rect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class StrokeRect
  {
  private:
    FloatRect _rect;
    float _lineWidth;

  public:
    static constexpr std::string_view Name = "stroke-rect";

    StrokeRect(FloatRect rect, float lineWidth) noexcept : _rect(rect), _lineWidth(lineWidth)
    {
    }

    KRYS_NODISCARD FloatRect Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD float LineWidth() const noexcept
    {
      return _lineWidth;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class StrokePath
  {
  private:
    boo::Path _path;

  public:
    static constexpr std::string_view Name = "stroke-path";

    explicit StrokePath(const boo::Path &path) noexcept : _path(path)
    {
    }

    KRYS_NODISCARD const boo::Path &GetPath() const noexcept
    {
      return _path;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class StrokeEllipse
  {
  private:
    FloatRect _rect;

  public:
    static constexpr std::string_view Name = "stroke-ellipse";

    explicit StrokeEllipse(const FloatRect &rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ClearRect
  {
  private:
    FloatRect _rect;

  public:
    static constexpr std::string_view Name = "clear-rect";

    explicit ClearRect(const FloatRect &rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class ApplyDeviceScaleFactor
  {
  private:
    float _scaleFactor;

  public:
    static constexpr std::string_view Name = "apply-device-scale-factor";

    explicit ApplyDeviceScaleFactor(float scaleFactor) noexcept : _scaleFactor(scaleFactor)
    {
    }

    KRYS_NODISCARD float ScaleFactor() const noexcept
    {
      return _scaleFactor;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class BeginPage
  {
  private:
    FloatRect _pageRect;

  public:
    static constexpr std::string_view Name = "begin-page";

    explicit BeginPage(const FloatRect &pageRect) noexcept : _pageRect(pageRect)
    {
    }

    KRYS_NODISCARD const FloatRect &PageRect() const noexcept
    {
      return _pageRect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };

  class EndPage
  {
  public:
    static constexpr std::string_view Name = "end-page";

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept
    {
    }
  };

  class SetURLForRect
  {
  private:
    url::URL _link;
    FloatRect _destRect;

  public:
    static constexpr std::string_view Name = "set-URL-for-rect";

    SetURLForRect(const url::URL &link, const FloatRect &destRect) noexcept : _link(link), _destRect(destRect)
    {
    }

    KRYS_NODISCARD const url::URL &Link() const noexcept
    {
      return _link;
    }
    KRYS_NODISCARD const FloatRect &DestRect() const noexcept
    {
      return _destRect;
    }

    void Apply(GraphicsContext &) const noexcept;
    void Dump(TextStream &, EnumSet<AsTextFlag>) const noexcept;
  };
}
