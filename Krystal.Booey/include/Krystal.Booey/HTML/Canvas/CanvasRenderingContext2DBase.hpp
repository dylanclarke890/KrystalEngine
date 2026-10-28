#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasDirection.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasFillRule.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasFontKerning.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasFontStretch.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasFontVariantCaps.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasLayerContextSwitcher.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasLineCap.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasLineJoin.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPath.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DSettings.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasStyle.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasTextAlign.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasTextBaseline.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasTextRendering.hpp"
#include "Krystal.Booey/HTML/Canvas/ImageSmoothingQuality.hpp"
#include "Krystal.Booey/HTML/ImageDataSettings.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class ByteArrayPixelBuffer;
  class CachedImage;
  class Filter;
  class GraphicsContext;
  class ImageBuffer;
  class IntPoint;
  class IntRect;
  class Path;
  class RenderElement;
  class RenderObject;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::svg
{
  class SVGImageElement;
}

namespace krys::boo::css
{
  class DOMMatrix;
}

namespace krys::boo::html
{
  class CanvasBase;
  class CanvasGradient;
  class CanvasPattern;
  class HTMLCanvasElement;
  class HTMLImageElement;
  class ImageData;
  class Path2D;
  class TextMetrics;

  /// @brief What a 2D context draws images from.
  /// @see https://html.spec.whatwg.org/#canvasimagesource
  // FIXME: `<video>` (B15); `ImageBitmap`, `OffscreenCanvas` and video frames aren't planned.
  using CanvasImageSource =
    Variant<RefPtr<HTMLImageElement>, RefPtr<svg::SVGImageElement>, RefPtr<HTMLCanvasElement>>;

  /// @brief The 2D context's state and drawing, shared by the contexts that draw like it.
  /// @see https://html.spec.whatwg.org/#canvasrenderingcontext2d
  class CanvasRenderingContext2DBase : public CanvasRenderingContext, public CanvasPath
  {
    friend class CanvasFilterContextSwitcher;
    friend class CanvasLayerContextSwitcher;

  public:
    using Direction = CanvasDirection;

    /// @brief The font text is drawn with, kept up to date with the web fonts of the selector it's
    /// resolved against.
    /// @note All font operations go through the proxy, so that it stays registered with the selector of the
    /// font it holds.
    class FontProxy final : public FontSelectorClient
    {
    private:
      FontCascade _font;

    public:
      FontProxy() noexcept = default;
      ~FontProxy() noexcept override;
      FontProxy(const FontProxy &) noexcept;
      FontProxy &operator=(const FontProxy &) noexcept;

      /// @brief Whether a font has been set: the proxy has a selector.
      KRYS_NODISCARD bool Realized() const noexcept
      {
        return _font.GetFontSelector();
      }
      /// @brief Holds @p fontCascade, resolved against @p fontSelector.
      void Initialize(FontSelector &fontSelector, const FontCascade &fontCascade) noexcept;
      KRYS_NODISCARD const FontMetrics &MetricsOfPrimaryFont() const noexcept;
      KRYS_NODISCARD const FontCascadeDescription &GetFontDescription() const noexcept;
      KRYS_NODISCARD float Width(const TextRun &, GlyphOverflow * = nullptr) const noexcept;
      void DrawBidiText(GraphicsContext &, const TextRun &, const FloatPoint &,
                        FontCascade::CustomFontNotReadyAction) const noexcept;

      KRYS_NODISCARD bool IsPopulated() const noexcept
      {
        return _font.Fonts();
      }

      KRYS_NODISCARD const FontCascade &GetFontCascade() const noexcept
      {
        return _font;
      }

      KRYS_NODISCARD float LetterSpacing() const noexcept
      {
        return _font.LetterSpacing();
      }
      void SetLetterSpacing(float letterSpacing) noexcept
      {
        _font.SetLetterSpacing(letterSpacing);
      }

      KRYS_NODISCARD float WordSpacing() const noexcept
      {
        return _font.WordSpacing();
      }
      void SetWordSpacing(float wordSpacing) noexcept
      {
        _font.SetWordSpacing(wordSpacing);
      }

    private:
      void Update(FontSelector &) noexcept;
      void FontsNeedUpdate(FontSelector &) noexcept final;
    };

    struct State final
    {
      State() noexcept;

      dom::DOMString UnparsedStrokeColor;
      dom::DOMString UnparsedFillColor;
      CanvasStyle StrokeStyle;
      CanvasStyle FillStyle;
      double LineWidth;
      boo::LineCap LineCap;
      boo::LineJoin LineJoin;
      double MiterLimit;
      FloatSize ShadowOffset;
      float ShadowBlur;
      Color ShadowColor;
      double GlobalAlpha;
      CompositeOperator GlobalComposite;
      BlendMode GlobalBlend;
      AffineTransform Transform;
      Maybe<AffineTransform> TransformInverse;
      List<double> LineDash;
      double LineDashOffset;
      bool ImageSmoothingEnabled;
      html::ImageSmoothingQuality ImageSmoothingQuality;
      boo::TextAlign TextAlign;
      boo::TextBaseline TextBaseline;
      CanvasRenderingContext2DBase::Direction Direction;

      dom::DOMString FilterString;
      style::Filter Filter;

      dom::DOMString LetterSpacing;
      dom::DOMString WordSpacing;

      dom::DOMString UnparsedFont;
      FontProxy Font;

      CanvasFontKerning FontKerning;
      CanvasFontStretch FontStretch;
      CanvasFontVariantCaps FontVariantCaps;
      CanvasTextRendering TextRendering;
      dom::DOMString Lang;

      RefPtr<CanvasLayerContextSwitcher> TargetSwitcher;

      KRYS_NODISCARD CanvasLineCap GetCanvasLineCap() const noexcept;
      KRYS_NODISCARD CanvasLineJoin GetCanvasLineJoin() const noexcept;
      KRYS_NODISCARD CanvasTextAlign GetCanvasTextAlign() const noexcept;
      KRYS_NODISCARD CanvasTextBaseline GetCanvasTextBaseline() const noexcept;
      KRYS_NODISCARD dom::DOMString GlobalCompositeOperationString() const noexcept;
      KRYS_NODISCARD dom::DOMString ShadowColorString() const noexcept;
      /// @brief The font, serialised without its line height; `10px sans-serif` until one is set.
      KRYS_NODISCARD dom::DOMString FontString() const noexcept;
    };

    using StyleVariant = Variant<dom::DOMString, RefPtr<CanvasGradient>, RefPtr<CanvasPattern>>;

  private:
    enum class DidDrawOption : uint8
    {
      ApplyTransform = 1 << 0,
      ApplyShadow = 1 << 1,
      ApplyClip = 1 << 2,
      ApplyPostProcessing = 1 << 3,
      PreserveCachedContents = 1 << 4,
    };

    static constexpr uint32 MaxSaveCount = 1'024 * 16;

    struct CachedContentsTransparent
    {
    };
    struct CachedContentsUnknown
    {
    };
    struct CachedContentsImageData
    {
      CachedContentsImageData(CanvasRenderingContext2DBase &, Ref<ByteArrayPixelBuffer>) noexcept;

      Ref<ByteArrayPixelBuffer> Pixels;
      UniquePtr<Timer> EvictionTimer;
    };

    SmallList<State, 1> _stateStack;
    FloatRect _dirtyRect;
    uint32 _unrealizedSaveCount {0};
    bool _usesCSSCompatibilityParseMode;
    bool _hasDeferredOperations {false};
    CanvasRenderingContext2DSettings _settings;
    mutable Variant<CachedContentsTransparent, CachedContentsUnknown, CachedContentsImageData>
      _cachedContents;

  protected:
    static constexpr int32 DefaultFontSize = 10;
    static constexpr std::u8string_view DefaultFontFamily = u8"sans-serif";

  public:
    ~CanvasRenderingContext2DBase() noexcept override;

    KRYS_NODISCARD const CanvasRenderingContext2DSettings &GetContextAttributes() const noexcept
    {
      return _settings;
    }

#pragma region CanvasPathDrawingStyles

    KRYS_NODISCARD double LineWidth() const noexcept
    {
      return GetState().LineWidth;
    }
    void SetLineWidth(double) noexcept;

    KRYS_NODISCARD CanvasLineCap LineCap() const noexcept
    {
      return GetState().GetCanvasLineCap();
    }
    void SetLineCap(CanvasLineCap) noexcept;

    KRYS_NODISCARD CanvasLineJoin LineJoin() const noexcept
    {
      return GetState().GetCanvasLineJoin();
    }
    void SetLineJoin(CanvasLineJoin) noexcept;

    KRYS_NODISCARD double MiterLimit() const noexcept
    {
      return GetState().MiterLimit;
    }
    void SetMiterLimit(double) noexcept;

    KRYS_NODISCARD const List<double> &GetLineDash() const noexcept
    {
      return GetState().LineDash;
    }
    void SetLineDash(const List<double> &) noexcept;

    KRYS_NODISCARD double LineDashOffset() const noexcept
    {
      return GetState().LineDashOffset;
    }
    void SetLineDashOffset(double) noexcept;

    KRYS_NODISCARD const dom::DOMString &LetterSpacing() const noexcept
    {
      return GetState().LetterSpacing;
    }
    /// @brief Sets the spacing added after each character, a CSS length; ignored if it isn't one, or uses
    /// viewport, container or line-height units.
    void SetLetterSpacing(dom::DOMStringView) noexcept;

    KRYS_NODISCARD const dom::DOMString &WordSpacing() const noexcept
    {
      return GetState().WordSpacing;
    }
    /// @brief Sets the spacing added after each space, a CSS length; ignored if it isn't one, or uses
    /// viewport, container or line-height units.
    void SetWordSpacing(dom::DOMStringView) noexcept;

#pragma endregion

#pragma region CanvasShadowStyles

    KRYS_NODISCARD float ShadowOffsetX() const noexcept
    {
      return GetState().ShadowOffset.Width();
    }
    void SetShadowOffsetX(float) noexcept;

    KRYS_NODISCARD float ShadowOffsetY() const noexcept
    {
      return GetState().ShadowOffset.Height();
    }
    void SetShadowOffsetY(float) noexcept;

    KRYS_NODISCARD float ShadowBlur() const noexcept
    {
      return GetState().ShadowBlur;
    }
    void SetShadowBlur(float) noexcept;

    KRYS_NODISCARD dom::DOMString ShadowColor() const noexcept
    {
      return GetState().ShadowColorString();
    }
    void SetShadowColor(dom::DOMStringView) noexcept;

#pragma endregion

#pragma region CanvasCompositing

    KRYS_NODISCARD double GlobalAlpha() const noexcept
    {
      return GetState().GlobalAlpha;
    }
    void SetGlobalAlpha(double) noexcept;

    KRYS_NODISCARD dom::DOMString GlobalCompositeOperation() const noexcept
    {
      return GetState().GlobalCompositeOperationString();
    }
    void SetGlobalCompositeOperation(dom::DOMStringView) noexcept;

#pragma endregion

#pragma region CanvasState

    /// @see https://html.spec.whatwg.org/#dom-context-2d-save
    void Save() noexcept
    {
      ++_unrealizedSaveCount;
    }
    /// @see https://html.spec.whatwg.org/#dom-context-2d-restore
    void Restore() noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-reset
    void Reset() noexcept;

#pragma endregion

#pragma region CanvasTransform

    void Scale(double sx, double sy) noexcept;
    void Rotate(double angleInRadians) noexcept;
    void Translate(double tx, double ty) noexcept;
    void Transform(double m11, double m12, double m21, double m22, double dx, double dy) noexcept;

    KRYS_NODISCARD Ref<css::DOMMatrix> GetTransform() const noexcept;
    void SetTransform(double m11, double m12, double m21, double m22, double dx, double dy) noexcept;
    dom::ExceptionOr<void> SetTransform(css::DOMMatrix2DInit && = {}) noexcept;
    void ResetTransform() noexcept;

#pragma endregion

#pragma region CanvasDrawPath

    void BeginPath() noexcept;

    void Fill(CanvasFillRule = CanvasFillRule::Nonzero) noexcept;
    void Stroke() noexcept;
    void Clip(CanvasFillRule = CanvasFillRule::Nonzero) noexcept;

    void Fill(const Path2D &, CanvasFillRule = CanvasFillRule::Nonzero) noexcept;
    void Stroke(const Path2D &) noexcept;
    void Clip(const Path2D &, CanvasFillRule = CanvasFillRule::Nonzero) noexcept;

    KRYS_NODISCARD bool IsPointInPath(double x, double y, CanvasFillRule = CanvasFillRule::Nonzero) noexcept;
    KRYS_NODISCARD bool IsPointInStroke(double x, double y) noexcept;

    KRYS_NODISCARD bool IsPointInPath(const Path2D &, double x, double y,
                                      CanvasFillRule = CanvasFillRule::Nonzero) noexcept;
    KRYS_NODISCARD bool IsPointInStroke(const Path2D &, double x, double y) noexcept;

#pragma endregion

#pragma region CanvasRect

    void ClearRect(double x, double y, double width, double height) noexcept;
    void FillRect(double x, double y, double width, double height) noexcept;
    void StrokeRect(double x, double y, double width, double height) noexcept;

#pragma endregion

#pragma region CanvasFillStrokeStyles

    KRYS_NODISCARD StyleVariant StrokeStyle() const noexcept;
    void SetStrokeStyle(dom::DOMString &&) noexcept;
    void SetStrokeStyle(Ref<CanvasGradient> &&) noexcept;
    void SetStrokeStyle(Ref<CanvasPattern> &&) noexcept;
    KRYS_NODISCARD StyleVariant FillStyle() const noexcept;
    void SetFillStyle(dom::DOMString &&) noexcept;
    void SetFillStyle(Ref<CanvasGradient> &&) noexcept;
    void SetFillStyle(Ref<CanvasPattern> &&) noexcept;

    KRYS_NODISCARD dom::ExceptionOr<Ref<CanvasGradient>> CreateLinearGradient(float x0, float y0, float x1,
                                                                              float y1) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<CanvasGradient>>
      CreateRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<CanvasGradient>> CreateConicGradient(float angleInRadians, float x,
                                                                             float y) noexcept;
    /// @brief A pattern of @p image repeated as @p repetition says; null if the image isn't ready yet.
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<CanvasPattern>>
      CreatePattern(CanvasImageSource &&image, dom::DOMStringView repetition) noexcept;

#pragma endregion

#pragma region CanvasDrawImage

    dom::ExceptionOr<void> DrawImage(CanvasImageSource &&, float dx, float dy) noexcept;
    dom::ExceptionOr<void> DrawImage(CanvasImageSource &&, float dx, float dy, float dw, float dh) noexcept;
    dom::ExceptionOr<void> DrawImage(CanvasImageSource &&, float sx, float sy, float sw, float sh, float dx,
                                     float dy, float dw, float dh) noexcept;

#pragma endregion

#pragma region CanvasImageData

    KRYS_NODISCARD dom::ExceptionOr<Ref<ImageData>> CreateImageData(ImageData &) const noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<ImageData>> CreateImageData(int32 width, int32 height,
                                                                    Maybe<ImageDataSettings>) const noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<ImageData>> GetImageData(int32 sx, int32 sy, int32 sw, int32 sh,
                                                                 Maybe<ImageDataSettings>) const noexcept;
    void PutImageData(ImageData &, int32 dx, int32 dy) noexcept;
    void PutImageData(ImageData &, int32 dx, int32 dy, int32 dirtyX, int32 dirtyY, int32 dirtyWidth,
                      int32 dirtyHeight) noexcept;

#pragma endregion

#pragma region CanvasFilters

    KRYS_NODISCARD const dom::DOMString &FilterString() const noexcept
    {
      return GetState().FilterString;
    }
    /// @brief Sets the filter, a CSS `filter` value; ignored if it isn't one (or is empty).
    /// @see https://html.spec.whatwg.org/#dom-context-2d-filter
    void SetFilterString(dom::DOMStringView) noexcept;

#pragma endregion

#pragma region CanvasImageSmoothing

    KRYS_NODISCARD bool ImageSmoothingEnabled() const noexcept
    {
      return GetState().ImageSmoothingEnabled;
    }
    void SetImageSmoothingEnabled(bool) noexcept;

    KRYS_NODISCARD ImageSmoothingQuality GetImageSmoothingQuality() const noexcept
    {
      return GetState().ImageSmoothingQuality;
    }
    void SetImageSmoothingQuality(ImageSmoothingQuality) noexcept;

#pragma endregion

#pragma region CanvasTextDrawingStyles

    KRYS_NODISCARD CanvasTextAlign TextAlign() const noexcept
    {
      return GetState().GetCanvasTextAlign();
    }
    void SetTextAlign(CanvasTextAlign) noexcept;

    KRYS_NODISCARD CanvasTextBaseline TextBaseline() const noexcept
    {
      return GetState().GetCanvasTextBaseline();
    }
    void SetTextBaseline(CanvasTextBaseline) noexcept;

    void SetDirection(Direction) noexcept;

    KRYS_NODISCARD dom::DOMString Font() const noexcept
    {
      return GetState().FontString();
    }

    /// @see https://html.spec.whatwg.org/#dom-context-2d-lang
    KRYS_NODISCARD const dom::DOMString &Lang() const noexcept
    {
      return GetState().Lang;
    }
    void SetLang(dom::DOMStringView) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-context-2d-fontkerning
    KRYS_NODISCARD CanvasFontKerning FontKerning() const noexcept
    {
      return GetState().FontKerning;
    }
    void SetFontKerning(CanvasFontKerning) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-context-2d-fontstretch
    KRYS_NODISCARD CanvasFontStretch FontStretch() const noexcept
    {
      return GetState().FontStretch;
    }
    void SetFontStretch(CanvasFontStretch) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-context-2d-fontvariantcaps
    KRYS_NODISCARD CanvasFontVariantCaps FontVariantCaps() const noexcept
    {
      return GetState().FontVariantCaps;
    }
    void SetFontVariantCaps(CanvasFontVariantCaps) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-context-2d-textrendering
    KRYS_NODISCARD CanvasTextRendering TextRendering() const noexcept
    {
      return GetState().TextRendering;
    }
    void SetTextRendering(CanvasTextRendering) noexcept;

#pragma endregion

    /// @brief Clears the whole canvas to transparent black, whatever the transform and clip.
    /// @note Doesn't report the canvas drawn: the draws that use it do (scripts' equivalent is `ClearRect`).
    void ClearCanvas() noexcept;

    /// @brief The state stack, with the saves not yet acted on made real.
    KRYS_NODISCARD const SmallList<State, 1> &StateStack() noexcept;

    /// @brief Whether the context draws into an accelerated buffer, whose pixels the compositor shows.
    KRYS_NODISCARD bool IsAccelerated() const noexcept;

    KRYS_NODISCARD bool IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept override;
    KRYS_NODISCARD RefPtr<GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() noexcept override;
    KRYS_NODISCARD bool HasDeferredOperations() const noexcept final;
    void FlushDeferredOperations() noexcept final;
    KRYS_NODISCARD bool NeedsPreparationForDisplay() const noexcept final;
    void PrepareForDisplay() noexcept final;
    void ClearAccumulatedDirtyRect() noexcept final;
    KRYS_NODISCARD DestinationColorSpace ColorSpace() const noexcept final;
    KRYS_NODISCARD bool WillReadFrequently() const noexcept final;

  protected:
    CanvasRenderingContext2DBase(CanvasBase &, Type, CanvasRenderingContext2DSettings &&,
                                 bool usesCSSCompatibilityParseMode) noexcept;

    KRYS_NODISCARD const State &GetState() const noexcept
    {
      return _stateStack.back();
    }
    void RealizeSaves() noexcept;
    KRYS_NODISCARD State &ModifiableState() noexcept
    {
      krys_debug_assert(!_unrealizedSaveCount || _stateStack.size() >= MaxSaveCount);
      return _stateStack.back();
    }

    KRYS_NODISCARD GraphicsContext *DrawingContext() const noexcept;
    KRYS_NODISCARD GraphicsContext *EffectiveDrawingContext() const noexcept;

    KRYS_NODISCARD virtual GraphicsContext *ExistingDrawingContext() const noexcept;

    /// @brief @p filterString parsed as a filter for this context; empty if it isn't one or filters aren't
    /// supported.
    KRYS_NODISCARD virtual Maybe<style::Filter>
      SetFilterStringWithoutUpdatingStyle(dom::DOMStringView) noexcept
    {
      return null;
    }
    /// @brief The filter that applies the state's filter to what's drawn within @p bounds; null if none can.
    KRYS_NODISCARD virtual RefPtr<boo::Filter> CreateFilter(const FloatRect &) const noexcept;
    /// @brief How far the state's filter reaches outside @p bounds.
    KRYS_NODISCARD virtual IntOutsets CalculateFilterOutsets(const FloatRect &) const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual AffineTransform BaseTransform() const noexcept;

    static constexpr EnumSet<DidDrawOption> DefaultDidDrawOptions() noexcept
    {
      return {DidDrawOption::ApplyTransform, DidDrawOption::ApplyShadow, DidDrawOption::ApplyClip,
              DidDrawOption::ApplyPostProcessing};
    }

    static constexpr EnumSet<DidDrawOption> DefaultDidDrawOptionsWithoutPostProcessing() noexcept
    {
      return {DidDrawOption::ApplyTransform, DidDrawOption::ApplyShadow, DidDrawOption::ApplyClip};
    }

    void DidDraw(Maybe<FloatRect>, EnumSet<DidDrawOption> = DefaultDidDrawOptions()) noexcept;
    void DidDrawEntireCanvas(EnumSet<DidDrawOption> options = DefaultDidDrawOptions()) noexcept;
    void DidDraw(bool entireCanvas, const FloatRect &,
                 EnumSet<DidDrawOption> = DefaultDidDrawOptions()) noexcept;
    template <typename TRectProvider>
    void DidDraw(bool entireCanvas, const TRectProvider &,
                 EnumSet<DidDrawOption> = DefaultDidDrawOptions()) noexcept;

    KRYS_NODISCARD bool UsesCSSCompatibilityParseMode() const noexcept
    {
      return _usesCSSCompatibilityParseMode;
    }

    void UpdateStateTransform(const AffineTransform &) noexcept;

    KRYS_NODISCARD bool CanDrawText(double x, double y, bool fill, Maybe<double> maxWidth = null) noexcept;
    void DrawTextUnchecked(const TextRun &, double x, double y, bool fill,
                           Maybe<double> maxWidth = null) noexcept;
    KRYS_NODISCARD Ref<TextMetrics> MeasureTextInternal(const TextRun &) noexcept;

    /// @brief @p text with its space characters (tab, line feed, line tabulation, form feed and carriage
    /// return) replaced with U+0020 SPACE, as text preparation does.
    KRYS_NODISCARD static dom::DOMString NormalizeSpaces(dom::DOMStringView text) noexcept;

    /// @brief Sets the font kerning, rendering mode and locale of @p fontDescription from the text drawing
    /// styles, and its width and caps too when @p includeFontLonghands is true.
    void ApplyTextDrawingStyles(FontCascadeDescription &fontDescription,
                                bool includeFontLonghands = true) const noexcept;
    /// @brief Sets the font stretch and caps styles from @p fontDescription, the font just set.
    void TakeFontLonghands(const FontCascadeDescription &fontDescription) noexcept;

    /// @brief The language `lang`'s `inherit` stands for; empty unless the context's canvas gives one.
    KRYS_NODISCARD virtual StringAtom InheritedLanguage() const noexcept;

    // The relationship between FontCascade and CanvasRenderingContext2D::FontProxy must hold certain
    // invariants. Therefore, all font operations must pass through the proxy.
    KRYS_NODISCARD virtual const FontProxy *GetFontProxy() noexcept
    {
      return nullptr;
    }

  private:
    KRYS_NODISCARD FloatPoint TextOffset(float width, TextDirection) noexcept;
    /// @brief Resolves the font again with the text drawing styles, if one is set.
    void UpdateFontForTextDrawingStyles() noexcept;
    void ApplyLineDash() const noexcept;
    void SetShadow(const FloatSize &offset, float blur, const Color &) noexcept;
    void ApplyShadow() noexcept;
    KRYS_NODISCARD bool ShouldDrawShadows() const noexcept;

    KRYS_NODISCARD bool IsEntireBackingStoreDirty() const noexcept;
    KRYS_NODISCARD FloatRect BackingStoreBounds() const noexcept
    {
      return FloatRect {{}, FloatSize {GetCanvasBase().Size()}};
    }

    void UnwindStateStack() noexcept;
    void RealizeSavesLoop() noexcept;
    void SetStrokeColorImpl(Color &&color, dom::DOMString &&unparsedColor = {}) noexcept;
    void SetFillColorImpl(Color &&color, dom::DOMString &&unparsedColor = {}) noexcept;

    void BeginCompositeLayer() noexcept;
    void EndCompositeLayer() noexcept;

    void FillInternal(const Path &, CanvasFillRule) noexcept;
    void StrokeInternal(const Path &) noexcept;
    void ClipInternal(const Path &, CanvasFillRule) noexcept;

    KRYS_NODISCARD bool IsPointInPathInternal(const Path &, double x, double y, CanvasFillRule) noexcept;
    KRYS_NODISCARD bool IsPointInStrokeInternal(const Path &, double x, double y) noexcept;

    KRYS_NODISCARD bool RectContainsCanvas(const FloatRect &) const noexcept;

    KRYS_NODISCARD Path TransformAreaToDevice(const Path &) const noexcept;
    KRYS_NODISCARD Path TransformAreaToDevice(const FloatRect &) const noexcept;
    KRYS_NODISCARD IntRect CalculateCompositingBufferRect(const FloatRect &area,
                                                          IntSize *croppedOffset) noexcept;
    void CompositeBuffer(ImageBuffer &, const IntRect &bufferRect, CompositeOperator) noexcept;
    template <typename T>
    void FullCanvasCompositedDrawImage(T &image, const FloatRect &dest, const FloatRect &src,
                                       CompositeOperator) noexcept;

    KRYS_NODISCARD dom::ExceptionOr<RefPtr<CanvasPattern>> CreatePattern(CachedImage &, RenderElement *,
                                                                         bool repeatX, bool repeatY) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<CanvasPattern>> CreatePattern(HTMLImageElement &, bool repeatX,
                                                                         bool repeatY) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<CanvasPattern>> CreatePattern(svg::SVGImageElement &, bool repeatX,
                                                                         bool repeatY) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<CanvasPattern>> CreatePattern(CanvasBase &, bool repeatX,
                                                                         bool repeatY) noexcept;

    dom::ExceptionOr<void> DrawImage(HTMLImageElement &, const FloatRect &srcRect,
                                     const FloatRect &dstRect) noexcept;
    dom::ExceptionOr<void> DrawImage(HTMLImageElement &, const FloatRect &srcRect, const FloatRect &dstRect,
                                     const CompositeOperator &, const BlendMode &) noexcept;
    dom::ExceptionOr<void> DrawImage(svg::SVGImageElement &, const FloatRect &srcRect,
                                     const FloatRect &dstRect) noexcept;
    dom::ExceptionOr<void> DrawImage(svg::SVGImageElement &, const FloatRect &srcRect,
                                     const FloatRect &dstRect, const CompositeOperator &,
                                     const BlendMode &) noexcept;
    dom::ExceptionOr<void> DrawImage(CanvasBase &, const FloatRect &srcRect,
                                     const FloatRect &dstRect) noexcept;
    dom::ExceptionOr<void> DrawImage(dom::Document &, CachedImage &, const RenderObject *,
                                     const FloatRect &imageRect, const FloatRect &srcRect,
                                     const FloatRect &dstRect, const CompositeOperator &, const BlendMode &,
                                     ImageOrientation = ImageOrientation::Orientation::FromImage) noexcept;

    /// @brief Keeps @p imageData's pixels, premultiplied, when they're put over the whole canvas, so that
    /// reading them back soon after needn't read the buffer; null if they aren't.
    RefPtr<ByteArrayPixelBuffer> CacheImageDataIfPossible(const ImageData &imageData,
                                                          const IntRect &sourceRect,
                                                          const IntPoint &destinationPosition) noexcept;
    KRYS_NODISCARD RefPtr<ImageData> MakeImageDataIfContentsCached(const IntRect &sourceRect,
                                                                   PredefinedColorSpace) const noexcept;
    void EvictCachedImageData() noexcept;

    KRYS_NODISCARD FloatRect InflatedStrokeRect(const FloatRect &) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CANVAS_RENDERING_CONTEXT(krys::boo::html::CanvasRenderingContext2DBase, Is2d())
