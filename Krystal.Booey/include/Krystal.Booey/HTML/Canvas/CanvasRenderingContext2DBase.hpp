#pragma once

#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasDirection.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasFillRule.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasLineCap.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasLineJoin.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPath.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DSettings.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasStyle.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasTextAlign.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasTextBaseline.hpp"
#include "Krystal.Booey/HTML/Canvas/ImageSmoothingQuality.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  class GraphicsContext;
}

namespace krys::boo::css
{
  class DOMMatrix;
}

namespace krys::boo::html
{
  class CanvasGradient;
  class CanvasPattern;
  class Path2D;

  /// @brief The 2D context's state and drawing, shared by the contexts that draw like it.
  /// @see https://html.spec.whatwg.org/#canvasrenderingcontext2d
  // FIXME: Images (`drawImage`, `createPattern`) and pixels (`ImageData`) come with B14c, text (`font`,
  // `letterSpacing`, `wordSpacing`, `fillText`, `strokeText`, `measureText`) with B14d, and `filter` with
  // B14e.
  class CanvasRenderingContext2DBase : public CanvasRenderingContext, public CanvasPath
  {
  public:
    using Direction = CanvasDirection;

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

      dom::DOMString LetterSpacing;
      dom::DOMString WordSpacing;

      dom::DOMString UnparsedFont;

      KRYS_NODISCARD CanvasLineCap GetCanvasLineCap() const noexcept;
      KRYS_NODISCARD CanvasLineJoin GetCanvasLineJoin() const noexcept;
      KRYS_NODISCARD CanvasTextAlign GetCanvasTextAlign() const noexcept;
      KRYS_NODISCARD CanvasTextBaseline GetCanvasTextBaseline() const noexcept;
      KRYS_NODISCARD dom::DOMString GlobalCompositeOperationString() const noexcept;
      KRYS_NODISCARD dom::DOMString ShadowColorString() const noexcept;
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

    SmallList<State, 1> _stateStack;
    FloatRect _dirtyRect;
    uint32 _unrealizedSaveCount {0};
    bool _usesCSSCompatibilityParseMode;
    CanvasRenderingContext2DSettings _settings;

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

#pragma endregion

    /// @brief Clears the whole canvas to transparent black, whatever the transform and clip.
    void ClearCanvas() noexcept;

    /// @brief The state stack, with the saves not yet acted on made real.
    KRYS_NODISCARD const SmallList<State, 1> &StateStack() noexcept;

    KRYS_NODISCARD bool IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept override;
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

  private:
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

    KRYS_NODISCARD FloatRect InflatedStrokeRect(const FloatRect &) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CANVAS_RENDERING_CONTEXT(krys::boo::html::CanvasRenderingContext2DBase, Is2d())
