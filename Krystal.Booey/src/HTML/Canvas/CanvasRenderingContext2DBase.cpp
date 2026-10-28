#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"
#include "Krystal.Booey/CSS/DOMMatrix.hpp"
#include "Krystal.Booey/Graphics/DashArray.hpp"
#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Pattern.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>

namespace krys::boo::html
{
  static constexpr ImageSmoothingQuality DefaultSmoothingQuality = ImageSmoothingQuality::Low;

  static constexpr std::u8string_view DefaultFont = u8"10px sans-serif";

  KRYS_NODISCARD static CanvasLineCap ToCanvasLineCap(LineCap lineCap) noexcept
  {
    switch (lineCap)
    {
      case LineCap::Butt:   return CanvasLineCap::Butt;
      case LineCap::Round:  return CanvasLineCap::Round;
      case LineCap::Square: return CanvasLineCap::Square;
    }
    krys_debug_assert(false);
    return CanvasLineCap::Butt;
  }

  KRYS_NODISCARD static LineCap FromCanvasLineCap(CanvasLineCap canvasLineCap) noexcept
  {
    switch (canvasLineCap)
    {
      case CanvasLineCap::Butt:   return LineCap::Butt;
      case CanvasLineCap::Round:  return LineCap::Round;
      case CanvasLineCap::Square: return LineCap::Square;
    }
    krys_debug_assert(false);
    return LineCap::Butt;
  }

  KRYS_NODISCARD static CanvasLineJoin ToCanvasLineJoin(LineJoin lineJoin) noexcept
  {
    switch (lineJoin)
    {
      case LineJoin::Round: return CanvasLineJoin::Round;
      case LineJoin::Bevel: return CanvasLineJoin::Bevel;
      case LineJoin::Miter: return CanvasLineJoin::Miter;
    }
    krys_debug_assert(false);
    return CanvasLineJoin::Round;
  }

  KRYS_NODISCARD static LineJoin FromCanvasLineJoin(CanvasLineJoin canvasLineJoin) noexcept
  {
    switch (canvasLineJoin)
    {
      case CanvasLineJoin::Round: return LineJoin::Round;
      case CanvasLineJoin::Bevel: return LineJoin::Bevel;
      case CanvasLineJoin::Miter: return LineJoin::Miter;
    }
    krys_debug_assert(false);
    return LineJoin::Round;
  }

  KRYS_NODISCARD static CanvasTextAlign ToCanvasTextAlign(TextAlign textAlign) noexcept
  {
    switch (textAlign)
    {
      case StartTextAlign:  return CanvasTextAlign::Start;
      case EndTextAlign:    return CanvasTextAlign::End;
      case LeftTextAlign:   return CanvasTextAlign::Left;
      case RightTextAlign:  return CanvasTextAlign::Right;
      case CenterTextAlign: return CanvasTextAlign::Center;
    }
    krys_debug_assert(false);
    return CanvasTextAlign::Start;
  }

  KRYS_NODISCARD static TextAlign FromCanvasTextAlign(CanvasTextAlign canvasTextAlign) noexcept
  {
    switch (canvasTextAlign)
    {
      case CanvasTextAlign::Start:  return StartTextAlign;
      case CanvasTextAlign::End:    return EndTextAlign;
      case CanvasTextAlign::Left:   return LeftTextAlign;
      case CanvasTextAlign::Right:  return RightTextAlign;
      case CanvasTextAlign::Center: return CenterTextAlign;
    }
    krys_debug_assert(false);
    return StartTextAlign;
  }

  KRYS_NODISCARD static CanvasTextBaseline ToCanvasTextBaseline(TextBaseline textBaseline) noexcept
  {
    switch (textBaseline)
    {
      case TopTextBaseline:         return CanvasTextBaseline::Top;
      case HangingTextBaseline:     return CanvasTextBaseline::Hanging;
      case MiddleTextBaseline:      return CanvasTextBaseline::Middle;
      case AlphabeticTextBaseline:  return CanvasTextBaseline::Alphabetic;
      case IdeographicTextBaseline: return CanvasTextBaseline::Ideographic;
      case BottomTextBaseline:      return CanvasTextBaseline::Bottom;
    }
    krys_debug_assert(false);
    return CanvasTextBaseline::Top;
  }

  KRYS_NODISCARD static TextBaseline FromCanvasTextBaseline(CanvasTextBaseline canvasTextBaseline) noexcept
  {
    switch (canvasTextBaseline)
    {
      case CanvasTextBaseline::Top:         return TopTextBaseline;
      case CanvasTextBaseline::Hanging:     return HangingTextBaseline;
      case CanvasTextBaseline::Middle:      return MiddleTextBaseline;
      case CanvasTextBaseline::Alphabetic:  return AlphabeticTextBaseline;
      case CanvasTextBaseline::Ideographic: return IdeographicTextBaseline;
      case CanvasTextBaseline::Bottom:      return BottomTextBaseline;
    }
    krys_debug_assert(false);
    return TopTextBaseline;
  }

  CanvasRenderingContext2DBase::CanvasRenderingContext2DBase(CanvasBase &canvas, Type type,
                                                             CanvasRenderingContext2DSettings &&settings,
                                                             bool usesCSSCompatibilityParseMode) noexcept
      : CanvasRenderingContext(canvas, type), _stateStack(1),
        _usesCSSCompatibilityParseMode(usesCSSCompatibilityParseMode), _settings(krys::move(settings))
  {
    krys_debug_assert(Is2d());
  }

  void CanvasRenderingContext2DBase::UnwindStateStack() noexcept
  {
    // Ensure that the state stack in the ImageBuffer's context
    // is cleared before destruction, to avoid assertions in the
    // GraphicsContext dtor.
    const size_t stackSize = _stateStack.size();
    if (stackSize <= 1)
    {
      return;
    }

    // We need to keep the last state because it is tracked by CanvasBase's context state saver.
    auto *context = ExistingDrawingContext();
    while (_stateStack.size() > 1)
    {
      _stateStack.pop_back();
      if (context)
      {
        context->Restore();
      }
    }
  }

  CanvasRenderingContext2DBase::~CanvasRenderingContext2DBase() noexcept
  {
#if KRYS_DEBUG
    UnwindStateStack();
#endif
  }

  bool CanvasRenderingContext2DBase::IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept
  {
    // Before the first draw (or first access to the drawing buffer), the drawing buffer is transparent black.
    // Currently the canvas does not support alpha == false.
    return !GetCanvasBase().HasCreatedImageBuffer();
  }

  void CanvasRenderingContext2DBase::Reset() noexcept
  {
    UnwindStateStack();

    krys_debug_assert(_stateStack.size() == 1);
    _stateStack.front() = State();

    _path.Clear();
    _unrealizedSaveCount = 0;
    ClearAccumulatedDirtyRect();
    if (auto *c = ExistingDrawingContext())
    {
      GetCanvasBase().ResetGraphicsContextState();
      c->ClearRect(FloatRect {{}, FloatSize(GetCanvasBase().Size())});
    }
  }

  CanvasRenderingContext2DBase::State::State() noexcept
      : StrokeStyle(Color::black), FillStyle(Color::black), LineWidth(1), LineCap(boo::LineCap::Butt),
        LineJoin(boo::LineJoin::Miter), MiterLimit(10), ShadowBlur(0), ShadowColor(Color::transparentBlack),
        GlobalAlpha(1), GlobalComposite(CompositeOperator::SourceOver), GlobalBlend(BlendMode::Normal),
        TransformInverse(AffineTransform {}), LineDashOffset(0), ImageSmoothingEnabled(true),
        ImageSmoothingQuality(DefaultSmoothingQuality), TextAlign(StartTextAlign),
        TextBaseline(AlphabeticTextBaseline), Direction(Direction::Inherit), FilterString(u8"none"),
        LetterSpacing(u8"0px"), WordSpacing(u8"0px"), UnparsedFont(DefaultFont)
  {
  }

  CanvasLineCap CanvasRenderingContext2DBase::State::GetCanvasLineCap() const noexcept
  {
    return ToCanvasLineCap(LineCap);
  }

  CanvasLineJoin CanvasRenderingContext2DBase::State::GetCanvasLineJoin() const noexcept
  {
    return ToCanvasLineJoin(LineJoin);
  }

  CanvasTextAlign CanvasRenderingContext2DBase::State::GetCanvasTextAlign() const noexcept
  {
    return ToCanvasTextAlign(TextAlign);
  }

  CanvasTextBaseline CanvasRenderingContext2DBase::State::GetCanvasTextBaseline() const noexcept
  {
    return ToCanvasTextBaseline(TextBaseline);
  }

  dom::DOMString CanvasRenderingContext2DBase::State::GlobalCompositeOperationString() const noexcept
  {
    const auto name = CompositeOperatorName(GlobalComposite, GlobalBlend);
    return dom::DOMString(name.begin(), name.end());
  }

  dom::DOMString CanvasRenderingContext2DBase::State::ShadowColorString() const noexcept
  {
    return SerialisationForHTML(ShadowColor);
  }

  void CanvasRenderingContext2DBase::RealizeSaves() noexcept
  {
    if (_unrealizedSaveCount)
    {
      RealizeSavesLoop();
    }

    // There's no console, so saving more than the limit is ignored without a message.
  }

  void CanvasRenderingContext2DBase::RealizeSavesLoop() noexcept
  {
    krys_debug_assert(_unrealizedSaveCount);
    krys_debug_assert(_stateStack.size() >= 1);
    GraphicsContext *context = DrawingContext();
    do
    {
      if (_stateStack.size() > MaxSaveCount)
      {
        break;
      }
      _stateStack.push_back(GetState());
      if (context)
      {
        context->Save();
      }
    } while (--_unrealizedSaveCount);
  }

  void CanvasRenderingContext2DBase::Restore() noexcept
  {
    if (_unrealizedSaveCount)
    {
      --_unrealizedSaveCount;
      return;
    }
    krys_debug_assert(_stateStack.size() >= 1);
    if (_stateStack.size() <= 1)
    {
      return;
    }
    _path.Transform(GetState().Transform);
    _stateStack.pop_back();
    const auto &state = GetState();
    _hasInvertibleTransform = state.TransformInverse.has_value();
    if (state.TransformInverse.has_value())
    {
      _path.Transform(*state.TransformInverse);
    }
    GraphicsContext *c = DrawingContext();
    if (!c)
    {
      return;
    }
    c->Restore();
  }

  void CanvasRenderingContext2DBase::SetStrokeColorImpl(Color &&color, dom::DOMString &&unparsedColor) noexcept
  {
    krys_debug_assert(color.IsValid());
    if (GetState().StrokeStyle.GetColor() == color)
    {
      return;
    }

    RealizeSaves();
    if (auto *c = EffectiveDrawingContext())
    {
      c->SetStrokeColor(color);
    }
    auto &state = ModifiableState();
    state.StrokeStyle = krys::move(color);
    state.UnparsedStrokeColor = krys::move(unparsedColor);
  }

  void CanvasRenderingContext2DBase::SetFillColorImpl(Color &&color, dom::DOMString &&unparsedColor) noexcept
  {
    krys_debug_assert(color.IsValid());
    if (GetState().FillStyle.GetColor() == color)
    {
      return;
    }

    RealizeSaves();
    if (auto *c = EffectiveDrawingContext())
    {
      c->SetFillColor(color);
    }
    auto &state = ModifiableState();
    state.FillStyle = krys::move(color);
    state.UnparsedFillColor = krys::move(unparsedColor);
  }

  void CanvasRenderingContext2DBase::SetLineWidth(double width) noexcept
  {
    if (!(std::isfinite(width) && width > 0))
    {
      return;
    }
    if (GetState().LineWidth == width)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().LineWidth = width;
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    c->SetStrokeThickness(static_cast<float>(width));
  }

  void CanvasRenderingContext2DBase::SetLineCap(CanvasLineCap canvasLineCap) noexcept
  {
    auto lineCap = FromCanvasLineCap(canvasLineCap);
    if (GetState().LineCap == lineCap)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().LineCap = lineCap;
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    c->SetLineCap(lineCap);
  }

  void CanvasRenderingContext2DBase::SetLineJoin(CanvasLineJoin canvasLineJoin) noexcept
  {
    auto lineJoin = FromCanvasLineJoin(canvasLineJoin);
    if (GetState().LineJoin == lineJoin)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().LineJoin = lineJoin;
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    c->SetLineJoin(lineJoin);
  }

  void CanvasRenderingContext2DBase::SetMiterLimit(double limit) noexcept
  {
    if (!(std::isfinite(limit) && limit > 0))
    {
      return;
    }
    if (GetState().MiterLimit == limit)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().MiterLimit = limit;
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    c->SetMiterLimit(static_cast<float>(limit));
  }

  void CanvasRenderingContext2DBase::SetShadowOffsetX(float x) noexcept
  {
    if (!std::isfinite(x))
    {
      return;
    }
    if (GetState().ShadowOffset.Width() == x)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().ShadowOffset.SetWidth(x);
    ApplyShadow();
  }

  void CanvasRenderingContext2DBase::SetShadowOffsetY(float y) noexcept
  {
    if (!std::isfinite(y))
    {
      return;
    }
    if (GetState().ShadowOffset.Height() == y)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().ShadowOffset.SetHeight(y);
    ApplyShadow();
  }

  void CanvasRenderingContext2DBase::SetShadowBlur(float blur) noexcept
  {
    if (!(std::isfinite(blur) && blur >= 0))
    {
      return;
    }
    if (GetState().ShadowBlur == blur)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().ShadowBlur = blur;
    ApplyShadow();
  }

  void CanvasRenderingContext2DBase::SetShadowColor(dom::DOMStringView colorString) noexcept
  {
    Color color = ParseColor(colorString, GetCanvasBase());
    if (!color.IsValid())
    {
      return;
    }
    if (GetState().ShadowColor == color)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().ShadowColor = color;
    ApplyShadow();
  }

  KRYS_NODISCARD static bool LineDashSequenceIsValid(const List<double> &dash) noexcept
  {
    for (const auto value : dash)
    {
      if (!std::isfinite(value) || value < 0)
      {
        return false;
      }
    }
    return true;
  }

  void CanvasRenderingContext2DBase::SetLineDash(const List<double> &dash) noexcept
  {
    if (!LineDashSequenceIsValid(dash))
    {
      return;
    }

    RealizeSaves();
    ModifiableState().LineDash = dash;
    // Spec requires the concatenation of two copies the dash list when the
    // number of elements is odd
    if (dash.size() % 2)
    {
      auto &lineDash = ModifiableState().LineDash;
      lineDash.insert(lineDash.end(), dash.begin(), dash.end());
    }

    ApplyLineDash();
  }

  void CanvasRenderingContext2DBase::SetLineDashOffset(double offset) noexcept
  {
    if (!std::isfinite(offset) || GetState().LineDashOffset == offset)
    {
      return;
    }

    RealizeSaves();
    ModifiableState().LineDashOffset = offset;
    ApplyLineDash();
  }

  void CanvasRenderingContext2DBase::ApplyLineDash() const noexcept
  {
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    DashArray convertedLineDash(GetState().LineDash.size());
    for (size_t i = 0; i < GetState().LineDash.size(); ++i)
    {
      convertedLineDash[i] = static_cast<DashArrayElement>(GetState().LineDash[i]);
    }
    c->SetLineDash(convertedLineDash, static_cast<float>(GetState().LineDashOffset));
  }

  void CanvasRenderingContext2DBase::SetGlobalAlpha(double alpha) noexcept
  {
    if (!(alpha >= 0 && alpha <= 1))
    {
      return;
    }
    if (GetState().GlobalAlpha == alpha)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().GlobalAlpha = alpha;
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    c->SetAlpha(static_cast<float>(alpha));
  }

  void CanvasRenderingContext2DBase::SetGlobalCompositeOperation(dom::DOMStringView operation) noexcept
  {
    CompositeOperator op = CompositeOperator::SourceOver;
    BlendMode blendMode = BlendMode::Normal;
    if (!ParseCompositeAndBlendOperator(operation, op, blendMode))
    {
      return;
    }
    if ((GetState().GlobalComposite == op) && (GetState().GlobalBlend == blendMode))
    {
      return;
    }
    RealizeSaves();
    ModifiableState().GlobalComposite = op;
    ModifiableState().GlobalBlend = blendMode;
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    c->SetCompositeOperation(op, blendMode);
  }

  void CanvasRenderingContext2DBase::Scale(double sx, double sy) noexcept
  {
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    if (!std::isfinite(sx) || !std::isfinite(sy))
    {
      return;
    }
    float floatX = ClampTo<float>(sx);
    float floatY = ClampTo<float>(sy);
    AffineTransform newTransform = GetState().Transform;
    newTransform.ScaleNonUniform(floatX, floatY);
    if (GetState().Transform == newTransform)
    {
      return;
    }

    RealizeSaves();
    UpdateStateTransform(newTransform);
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    c->Scale(FloatSize(floatX, floatY));
    _path.Transform(AffineTransform().ScaleNonUniform(1.0 / floatX, 1.0 / floatY));
  }

  void CanvasRenderingContext2DBase::Rotate(double angleInRadians) noexcept
  {
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    if (!std::isfinite(angleInRadians))
    {
      return;
    }

    AffineTransform newTransform = GetState().Transform;
    newTransform.RotateRadians(angleInRadians);
    if (GetState().Transform == newTransform)
    {
      return;
    }

    RealizeSaves();
    UpdateStateTransform(newTransform); // Rotate never causes non-invertible matrices.
    c->Rotate(static_cast<float>(angleInRadians));
    _path.Transform(AffineTransform().RotateRadians(-angleInRadians));
  }

  void CanvasRenderingContext2DBase::Translate(double tx, double ty) noexcept
  {
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    if (!std::isfinite(tx) || !std::isfinite(ty))
    {
      return;
    }

    AffineTransform newTransform = GetState().Transform;
    newTransform.Translate(tx, ty);
    if (GetState().Transform == newTransform)
    {
      return;
    }

    RealizeSaves();
    UpdateStateTransform(newTransform);
    // Translate may end up making infinities which are non-invertible.
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }
    c->Translate(static_cast<float>(tx), static_cast<float>(ty));
    _path.Transform(AffineTransform().Translate(-tx, -ty));
  }

  void CanvasRenderingContext2DBase::Transform(double m11, double m12, double m21, double m22, double dx,
                                               double dy) noexcept
  {
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    if (!std::isfinite(m11) || !std::isfinite(m21) || !std::isfinite(dx) || !std::isfinite(m12)
        || !std::isfinite(m22) || !std::isfinite(dy))
    {
      return;
    }

    AffineTransform transform(m11, m12, m21, m22, dx, dy);
    AffineTransform newTransform = GetState().Transform * transform;
    if (GetState().Transform == newTransform)
    {
      return;
    }

    RealizeSaves();
    UpdateStateTransform(newTransform);
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }
    c->ConcatCTM(
      transform); // Note: concat with the incoming transform, not the full transform (newTransform).
    auto inverse = transform.Inverse();
    krys_debug_assert(inverse);
    if (inverse)
    {
      _path.Transform(*inverse);
    }
  }

  Ref<css::DOMMatrix> CanvasRenderingContext2DBase::GetTransform() const noexcept
  {
    return css::DOMMatrix::Create(GetState().Transform.ToTransformationMatrix(),
                                  css::DOMMatrixReadOnly::Is2D::Yes);
  }

  void CanvasRenderingContext2DBase::SetTransform(double m11, double m12, double m21, double m22, double dx,
                                                  double dy) noexcept
  {
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }

    if (!std::isfinite(m11) || !std::isfinite(m21) || !std::isfinite(dx) || !std::isfinite(m12)
        || !std::isfinite(m22) || !std::isfinite(dy))
    {
      return;
    }

    ResetTransform();
    Transform(m11, m12, m21, m22, dx, dy);
  }

  dom::ExceptionOr<void>
    CanvasRenderingContext2DBase::SetTransform(css::DOMMatrix2DInit &&matrixInit) noexcept
  {
    auto checkValid = css::DOMMatrixReadOnly::ValidateAndFixup(matrixInit);
    if (checkValid.HasException())
    {
      return checkValid.ReleaseException();
    }

    SetTransform(matrixInit.M11.value(), matrixInit.M12.value(), matrixInit.M21.value(),
                 matrixInit.M22.value(), matrixInit.M41.value(), matrixInit.M42.value());
    return {};
  }

  void CanvasRenderingContext2DBase::ResetTransform() noexcept
  {
    GraphicsContext *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }

    if (HasInvertibleTransform())
    {
      _path.Transform(GetState().Transform);
    }

    RealizeSaves();

    c->SetCTM(BaseTransform());
    UpdateStateTransform({});
  }

  void CanvasRenderingContext2DBase::BeginPath() noexcept
  {
    _path.Clear();
  }

  KRYS_NODISCARD static bool ValidateRectForCanvas(double &x, double &y, double &width,
                                                   double &height) noexcept
  {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) || !std::isfinite(height))
    {
      return false;
    }

    if (!width && !height)
    {
      return false;
    }

    if (width < 0)
    {
      width = -width;
      x -= width;
    }

    if (height < 0)
    {
      height = -height;
      y -= height;
    }

    return true;
  }

  KRYS_NODISCARD static bool IsFullCanvasCompositeMode(CompositeOperator op) noexcept
  {
    // See 4.8.11.1.3 Compositing
    // CompositeOperator::SourceAtop and CompositeOperator::DestinationOut are not listed here as the
    // platforms already implement the specification's behavior.
    return op == CompositeOperator::SourceIn || op == CompositeOperator::SourceOut
           || op == CompositeOperator::DestinationIn || op == CompositeOperator::DestinationAtop;
  }

  KRYS_NODISCARD static WindRule ToWindRule(CanvasFillRule rule) noexcept
  {
    return rule == CanvasFillRule::Nonzero ? WindRule::NonZero : WindRule::EvenOdd;
  }

  void CanvasRenderingContext2DBase::Fill(CanvasFillRule windingRule) noexcept
  {
    FillInternal(_path, windingRule);
  }

  void CanvasRenderingContext2DBase::Stroke() noexcept
  {
    StrokeInternal(_path);
  }

  void CanvasRenderingContext2DBase::Clip(CanvasFillRule windingRule) noexcept
  {
    ClipInternal(_path, windingRule);
  }

  void CanvasRenderingContext2DBase::Fill(const Path2D &path, CanvasFillRule windingRule) noexcept
  {
    FillInternal(path.GetPath(), windingRule);
  }

  void CanvasRenderingContext2DBase::Stroke(const Path2D &path) noexcept
  {
    StrokeInternal(path.GetPath());
  }

  void CanvasRenderingContext2DBase::Clip(const Path2D &path, CanvasFillRule windingRule) noexcept
  {
    ClipInternal(path.GetPath(), windingRule);
  }

  void CanvasRenderingContext2DBase::FillInternal(const Path &path, CanvasFillRule windingRule) noexcept
  {
    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    // If gradient size is zero, then paint nothing.
    auto *gradient = c->FillGradient();
    if (gradient && gradient->IsZeroSize())
    {
      return;
    }

    if (path.IsEmpty())
    {
      return;
    }

    auto savedFillRule = c->FillRule();
    c->SetFillRule(ToWindRule(windingRule));

    bool repaintEntireCanvas = false;
    if (IsFullCanvasCompositeMode(GetState().GlobalComposite))
    {
      BeginCompositeLayer();
      c->FillPath(path);
      EndCompositeLayer();
      repaintEntireCanvas = true;
    }
    else if (GetState().GlobalComposite == CompositeOperator::Copy)
    {
      ClearCanvas();
      c->FillPath(path);
      repaintEntireCanvas = true;
    }
    else
    {
      c->FillPath(path);
    }

    DidDraw(repaintEntireCanvas, [&] { return path.FastBoundingRect(); });

    c->SetFillRule(savedFillRule);
  }

  void CanvasRenderingContext2DBase::StrokeInternal(const Path &path) noexcept
  {
    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    // If gradient size is zero, then paint nothing.
    auto *gradient = c->StrokeGradient();
    if (gradient && gradient->IsZeroSize())
    {
      return;
    }

    if (path.IsEmpty())
    {
      return;
    }

    bool repaintEntireCanvas = false;
    if (IsFullCanvasCompositeMode(GetState().GlobalComposite))
    {
      BeginCompositeLayer();
      c->StrokePath(path);
      EndCompositeLayer();
      repaintEntireCanvas = true;
    }
    else if (GetState().GlobalComposite == CompositeOperator::Copy)
    {
      ClearCanvas();
      c->StrokePath(path);
      repaintEntireCanvas = true;
    }
    else
    {
      c->StrokePath(path);
    }

    DidDraw(repaintEntireCanvas, [&] { return InflatedStrokeRect(path.FastBoundingRect()); });
  }

  void CanvasRenderingContext2DBase::ClipInternal(const Path &path, CanvasFillRule windingRule) noexcept
  {
    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    RealizeSaves();
    c->ClipPath(path, ToWindRule(windingRule));
  }

  void CanvasRenderingContext2DBase::BeginCompositeLayer() noexcept
  {
    auto *context = EffectiveDrawingContext();
    context->BeginTransparencyLayer(GetState().GlobalComposite, GetState().GlobalBlend);
  }

  void CanvasRenderingContext2DBase::EndCompositeLayer() noexcept
  {
    auto *context = EffectiveDrawingContext();
    context->EndTransparencyLayer();
  }

  bool CanvasRenderingContext2DBase::IsPointInPath(double x, double y, CanvasFillRule windingRule) noexcept
  {
    return IsPointInPathInternal(_path, x, y, windingRule);
  }

  bool CanvasRenderingContext2DBase::IsPointInStroke(double x, double y) noexcept
  {
    return IsPointInStrokeInternal(_path, x, y);
  }

  bool CanvasRenderingContext2DBase::IsPointInPath(const Path2D &path, double x, double y,
                                                   CanvasFillRule windingRule) noexcept
  {
    return IsPointInPathInternal(path.GetPath(), x, y, windingRule);
  }

  bool CanvasRenderingContext2DBase::IsPointInStroke(const Path2D &path, double x, double y) noexcept
  {
    return IsPointInStrokeInternal(path.GetPath(), x, y);
  }

  bool CanvasRenderingContext2DBase::IsPointInPathInternal(const Path &path, double x, double y,
                                                           CanvasFillRule windingRule) noexcept
  {
    if (!std::isfinite(x) || !std::isfinite(y))
    {
      return false;
    }

    if (!EffectiveDrawingContext())
    {
      return false;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return false;
    }

    const auto &state = GetState();
    auto transformedPoint = state.Transform.Inverse()
                              .value_or(AffineTransform {})
                              .MapPoint(FloatPoint(static_cast<float>(x), static_cast<float>(y)));
    krys_debug_assert(std::isfinite(transformedPoint.X()) && std::isfinite(transformedPoint.Y()));

    return path.Contains(transformedPoint, ToWindRule(windingRule));
  }

  bool CanvasRenderingContext2DBase::IsPointInStrokeInternal(const Path &path, double x, double y) noexcept
  {
    if (!std::isfinite(x) || !std::isfinite(y))
    {
      return false;
    }

    if (!EffectiveDrawingContext())
    {
      return false;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return false;
    }

    const auto &state = GetState();
    auto transformedPoint = state.Transform.Inverse()
                              .value_or(AffineTransform {})
                              .MapPoint(FloatPoint(static_cast<float>(x), static_cast<float>(y)));
    krys_debug_assert(std::isfinite(transformedPoint.X()) && std::isfinite(transformedPoint.Y()));

    return path.StrokeContains(transformedPoint,
                               [&state](GraphicsContext &context)
                               {
                                 context.SetStrokeThickness(static_cast<float>(state.LineWidth));
                                 context.SetLineCap(state.LineCap);
                                 context.SetLineJoin(state.LineJoin);
                                 context.SetMiterLimit(static_cast<float>(state.MiterLimit));
                                 const auto &lineDash = state.LineDash;
                                 DashArray convertedLineDash(lineDash.size());
                                 for (size_t i = 0; i < lineDash.size(); ++i)
                                 {
                                   convertedLineDash[i] = static_cast<DashArrayElement>(lineDash[i]);
                                 }
                                 context.SetLineDash(convertedLineDash,
                                                     static_cast<float>(state.LineDashOffset));
                               });
  }

  void CanvasRenderingContext2DBase::ClearRect(double x, double y, double width, double height) noexcept
  {
    if (!ValidateRectForCanvas(x, y, width, height))
    {
      return;
    }
    auto *context = EffectiveDrawingContext();
    if (!context)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }
    FloatRect rect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                   static_cast<float>(height));

    bool saved = false;
    if (ShouldDrawShadows())
    {
      context->Save();
      saved = true;
      context->SetDropShadow({FloatSize(), 0, Color::transparentBlack, ShadowRadiusMode::Legacy});
    }
    if (GetState().GlobalAlpha != 1)
    {
      if (!saved)
      {
        context->Save();
        saved = true;
      }
      context->SetAlpha(1);
    }
    if (GetState().GlobalComposite != CompositeOperator::SourceOver)
    {
      if (!saved)
      {
        context->Save();
        saved = true;
      }
      context->SetCompositeOperation(CompositeOperator::SourceOver);
    }
    context->ClearRect(rect);
    if (saved)
    {
      context->Restore();
    }
    DidDraw(rect, DefaultDidDrawOptionsWithoutPostProcessing());
  }

  void CanvasRenderingContext2DBase::FillRect(double x, double y, double width, double height) noexcept
  {
    if (!ValidateRectForCanvas(x, y, width, height))
    {
      return;
    }

    FloatRect rect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                   static_cast<float>(height));

    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    // from the HTML5 Canvas spec:
    // If x0 = x1 and y0 = y1, then the linear gradient must paint nothing
    // If x0 = x1 and y0 = y1 and r0 = r1, then the radial gradient must paint nothing
    auto *gradient = c->FillGradient();
    if (gradient && gradient->IsZeroSize())
    {
      return;
    }

    bool repaintEntireCanvas = false;
    if (RectContainsCanvas(rect))
    {
      const bool needsCompositeLayer =
        ShouldDrawShadows() && IsFullCanvasCompositeMode(GetState().GlobalComposite);
      if (needsCompositeLayer)
      {
        BeginCompositeLayer();
      }
      c->FillRect(rect);
      if (needsCompositeLayer)
      {
        EndCompositeLayer();
      }
      repaintEntireCanvas = true;
    }
    else if (IsFullCanvasCompositeMode(GetState().GlobalComposite))
    {
      BeginCompositeLayer();
      c->FillRect(rect);
      EndCompositeLayer();
      repaintEntireCanvas = true;
    }
    else if (GetState().GlobalComposite == CompositeOperator::Copy)
    {
      ClearCanvas();
      c->FillRect(rect);
      repaintEntireCanvas = true;
    }
    else
    {
      c->FillRect(rect);
    }

    DidDraw(repaintEntireCanvas, rect);
  }

  void CanvasRenderingContext2DBase::StrokeRect(double x, double y, double width, double height) noexcept
  {
    if (!ValidateRectForCanvas(x, y, width, height))
    {
      return;
    }

    FloatRect rect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                   static_cast<float>(height));
    FloatRect inflatedStrokeRect = rect;
    inflatedStrokeRect.Inflate(static_cast<float>(GetState().LineWidth / 2));

    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }
    if (!(GetState().LineWidth >= 0))
    {
      return;
    }

    // If gradient size is zero, then paint nothing.
    auto *gradient = c->StrokeGradient();
    if (gradient && gradient->IsZeroSize())
    {
      return;
    }

    const auto lineWidth = static_cast<float>(GetState().LineWidth);
    bool repaintEntireCanvas = false;
    if (IsFullCanvasCompositeMode(GetState().GlobalComposite))
    {
      BeginCompositeLayer();
      c->StrokeRect(rect, lineWidth);
      EndCompositeLayer();
      repaintEntireCanvas = true;
    }
    else if (GetState().GlobalComposite == CompositeOperator::Copy)
    {
      ClearCanvas();
      c->StrokeRect(rect, lineWidth);
      repaintEntireCanvas = true;
    }
    else
    {
      c->StrokeRect(rect, lineWidth);
    }

    DidDraw(repaintEntireCanvas, inflatedStrokeRect);
  }

  void CanvasRenderingContext2DBase::SetShadow(const FloatSize &offset, float blur,
                                               const Color &color) noexcept
  {
    if (GetState().ShadowOffset == offset && GetState().ShadowBlur == blur && GetState().ShadowColor == color)
    {
      return;
    }
    bool wasDrawingShadows = ShouldDrawShadows();
    RealizeSaves();
    ModifiableState().ShadowOffset = offset;
    ModifiableState().ShadowBlur = blur;
    ModifiableState().ShadowColor = color;
    if (!wasDrawingShadows && !ShouldDrawShadows())
    {
      return;
    }
    ApplyShadow();
  }

  void CanvasRenderingContext2DBase::ApplyShadow() noexcept
  {
    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }

    if (ShouldDrawShadows())
    {
      auto shadowOffset = c->PlatformShadowOffset(GetState().ShadowOffset);
      c->SetDropShadow(
        {shadowOffset, GetState().ShadowBlur, GetState().ShadowColor, ShadowRadiusMode::Legacy});
    }
    else
    {
      c->SetDropShadow({{}, 0, Color::transparentBlack, ShadowRadiusMode::Legacy});
    }
  }

  bool CanvasRenderingContext2DBase::ShouldDrawShadows() const noexcept
  {
    return GetState().ShadowColor.IsVisible() && (GetState().ShadowBlur || !GetState().ShadowOffset.IsZero());
  }

  void CanvasRenderingContext2DBase::ClearCanvas() noexcept
  {
    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }

    c->Save();
    c->SetCTM(BaseTransform());
    c->ClearRect(FloatRect(0, 0, static_cast<float>(GetCanvasBase().Width()),
                           static_cast<float>(GetCanvasBase().Height())));
    c->Restore();
  }

  bool CanvasRenderingContext2DBase::RectContainsCanvas(const FloatRect &rect) const noexcept
  {
    FloatQuad quad(rect);
    FloatQuad canvasQuad(FloatRect(0, 0, static_cast<float>(GetCanvasBase().Width()),
                                   static_cast<float>(GetCanvasBase().Height())));
    return GetState().Transform.MapQuad(quad).ContainsQuad(canvasQuad);
  }

  KRYS_NODISCARD static CanvasRenderingContext2DBase::StyleVariant
    ToStyleVariant(const CanvasStyle &style) noexcept
  {
    return style.Visit([](const dom::DOMString &string) -> CanvasRenderingContext2DBase::StyleVariant
                       { return string; },
                       [](const Ref<CanvasGradient> &gradient) -> CanvasRenderingContext2DBase::StyleVariant
                       { return RefPtr<CanvasGradient>(gradient); },
                       [](const Ref<CanvasPattern> &pattern) -> CanvasRenderingContext2DBase::StyleVariant
                       { return RefPtr<CanvasPattern>(pattern); });
  }

  CanvasRenderingContext2DBase::StyleVariant CanvasRenderingContext2DBase::StrokeStyle() const noexcept
  {
    return ToStyleVariant(GetState().StrokeStyle);
  }

  void CanvasRenderingContext2DBase::SetStrokeStyle(dom::DOMString &&colorString) noexcept
  {
    if (colorString == GetState().UnparsedStrokeColor)
    {
      return;
    }

    auto color = ParseColor(colorString, GetCanvasBase());
    if (!color.IsValid())
    {
      return;
    }

    SetStrokeColorImpl(krys::move(color), krys::move(colorString));
  }

  void CanvasRenderingContext2DBase::SetStrokeStyle(Ref<CanvasGradient> &&gradient) noexcept
  {
    RealizeSaves();
    if (auto *c = EffectiveDrawingContext())
    {
      c->SetStrokeGradient(ShareRef(gradient->GetGradient()));
    }
    auto &state = ModifiableState();
    state.StrokeStyle = krys::move(gradient);
    state.UnparsedStrokeColor = {};
  }

  void CanvasRenderingContext2DBase::SetStrokeStyle(Ref<CanvasPattern> &&pattern) noexcept
  {
    RealizeSaves();
    if (auto *c = EffectiveDrawingContext())
    {
      c->SetStrokePattern(ShareRef(pattern->GetPattern()));
    }
    auto &state = ModifiableState();
    state.StrokeStyle = krys::move(pattern);
    state.UnparsedStrokeColor = {};
  }

  CanvasRenderingContext2DBase::StyleVariant CanvasRenderingContext2DBase::FillStyle() const noexcept
  {
    return ToStyleVariant(GetState().FillStyle);
  }

  void CanvasRenderingContext2DBase::SetFillStyle(dom::DOMString &&colorString) noexcept
  {
    if (colorString == GetState().UnparsedFillColor)
    {
      return;
    }

    auto color = ParseColor(colorString, GetCanvasBase());
    if (!color.IsValid())
    {
      return;
    }

    SetFillColorImpl(krys::move(color), krys::move(colorString));
  }

  void CanvasRenderingContext2DBase::SetFillStyle(Ref<CanvasGradient> &&gradient) noexcept
  {
    RealizeSaves();
    if (auto *c = EffectiveDrawingContext())
    {
      c->SetFillGradient(ShareRef(gradient->GetGradient()));
    }
    auto &state = ModifiableState();
    state.FillStyle = krys::move(gradient);
    state.UnparsedFillColor = {};
  }

  void CanvasRenderingContext2DBase::SetFillStyle(Ref<CanvasPattern> &&pattern) noexcept
  {
    RealizeSaves();
    if (auto *c = EffectiveDrawingContext())
    {
      c->SetFillPattern(ShareRef(pattern->GetPattern()));
    }
    auto &state = ModifiableState();
    state.FillStyle = krys::move(pattern);
    state.UnparsedFillColor = {};
  }

  dom::ExceptionOr<Ref<CanvasGradient>>
    CanvasRenderingContext2DBase::CreateLinearGradient(float x0, float y0, float x1, float y1) noexcept
  {
    if (!std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(x1) || !std::isfinite(y1))
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    return CanvasGradient::Create(FloatPoint(x0, y0), FloatPoint(x1, y1));
  }

  dom::ExceptionOr<Ref<CanvasGradient>> CanvasRenderingContext2DBase::CreateRadialGradient(float x0, float y0,
                                                                                           float r0, float x1,
                                                                                           float y1,
                                                                                           float r1) noexcept
  {
    if (!std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(r0) || !std::isfinite(x1)
        || !std::isfinite(y1) || !std::isfinite(r1))
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    if (r0 < 0 || r1 < 0)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    return CanvasGradient::Create(FloatPoint(x0, y0), r0, FloatPoint(x1, y1), r1);
  }

  dom::ExceptionOr<Ref<CanvasGradient>>
    CanvasRenderingContext2DBase::CreateConicGradient(float angleInRadians, float x, float y) noexcept
  {
    if (!std::isfinite(angleInRadians) || !std::isfinite(x) || !std::isfinite(y))
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // Angle starts from x-axis for consistency within canvas methods. See
    // https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-createconicgradient
    angleInRadians = NormaliseAngleInRadians(angleInRadians) + piOverTwoFloat;
    return CanvasGradient::Create(FloatPoint(x, y), angleInRadians);
  }

  void CanvasRenderingContext2DBase::DidDrawEntireCanvas(EnumSet<DidDrawOption> options) noexcept
  {
    DidDraw(BackingStoreBounds(), options);
  }

  void CanvasRenderingContext2DBase::DidDraw(Maybe<FloatRect> rect, EnumSet<DidDrawOption> options) noexcept
  {
    auto *context = EffectiveDrawingContext();
    if (!context)
    {
      return;
    }

    if (!rect)
    {
      GetCanvasBase().DidDraw(null);
      return;
    }

    auto dirtyRect = rect.value();
    if (dirtyRect.IsEmpty())
    {
      return;
    }

    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    if (options.Contains(DidDrawOption::ApplyTransform))
    {
      dirtyRect = GetState().Transform.MapRect(dirtyRect);
    }

    if (options.Contains(DidDrawOption::ApplyShadow) && GetState().ShadowColor.IsVisible())
    {
      // The shadow gets applied after transformation
      auto shadowRect = dirtyRect;
      shadowRect.Move(GetState().ShadowOffset);
      shadowRect.Inflate(GetState().ShadowBlur);
      dirtyRect.Unite(shadowRect);
    }

    // FIXME: This does not apply the clip because we have no way of reading the clip out of the
    // GraphicsContext.
    if (_dirtyRect.Contains(dirtyRect))
    {
      GetCanvasBase().DidDraw(null);
    }
    else
    {
      // Inflate dirty rect to cover antialiasing on image buffers.
      if (context->ShouldAntialias())
      {
        dirtyRect.Inflate(1);
      }
      _dirtyRect.Unite(dirtyRect);
      GetCanvasBase().DidDraw(_dirtyRect);
    }
  }

  void CanvasRenderingContext2DBase::DidDraw(bool entireCanvas, const FloatRect &rect,
                                             EnumSet<DidDrawOption> options) noexcept
  {
    DidDraw(entireCanvas, [&] { return rect; }, options);
  }

  template <typename TRectProvider>
  void CanvasRenderingContext2DBase::DidDraw(bool entireCanvas, const TRectProvider &rectProvider,
                                             EnumSet<DidDrawOption> options) noexcept
  {
    if (IsEntireBackingStoreDirty())
    {
      DidDraw(null, options);
    }
    else if (entireCanvas)
    {
      EnumSet<DidDrawOption> didDrawEntireCanvasOptions {DidDrawOption::ApplyClip};
      if (options.Contains(DidDrawOption::ApplyPostProcessing))
      {
        didDrawEntireCanvasOptions.Add(DidDrawOption::ApplyPostProcessing);
      }
      DidDrawEntireCanvas(didDrawEntireCanvasOptions);
    }
    else
    {
      DidDraw(Maybe<FloatRect>(rectProvider()), options);
    }
  }

  void CanvasRenderingContext2DBase::ClearAccumulatedDirtyRect() noexcept
  {
    _dirtyRect = {};
  }

  bool CanvasRenderingContext2DBase::IsEntireBackingStoreDirty() const noexcept
  {
    return _dirtyRect == BackingStoreBounds();
  }

  const SmallList<CanvasRenderingContext2DBase::State, 1> &CanvasRenderingContext2DBase::StateStack() noexcept
  {
    RealizeSaves();
    return _stateStack;
  }

  GraphicsContext *CanvasRenderingContext2DBase::DrawingContext() const noexcept
  {
    if (auto *buffer = GetCanvasBase().Buffer())
    {
      return &buffer->Context();
    }
    return nullptr;
  }

  GraphicsContext *CanvasRenderingContext2DBase::ExistingDrawingContext() const noexcept
  {
    if (!GetCanvasBase().HasCreatedImageBuffer())
    {
      return nullptr;
    }
    return DrawingContext();
  }

  GraphicsContext *CanvasRenderingContext2DBase::EffectiveDrawingContext() const noexcept
  {
    return DrawingContext();
  }

  AffineTransform CanvasRenderingContext2DBase::BaseTransform() const noexcept
  {
    krys_debug_assert(GetCanvasBase().HasCreatedImageBuffer());
    return GetCanvasBase().Buffer()->BaseTransform();
  }

  FloatRect CanvasRenderingContext2DBase::InflatedStrokeRect(const FloatRect &rect) const noexcept
  {
    // Fast approximation of the stroke's bounding rect.
    // This yields a slightly oversized rect but is very fast
    // compared to Path::strokeBoundingRect().
    static const float root2 = std::sqrt(2.f);
    auto delta = static_cast<float>(GetState().LineWidth / 2);
    if (GetState().LineJoin == boo::LineJoin::Miter)
    {
      delta *= static_cast<float>(GetState().MiterLimit);
    }
    else if (GetState().LineCap == boo::LineCap::Square)
    {
      delta *= root2;
    }
    auto inflatedStrokeRect = rect;
    inflatedStrokeRect.Inflate(delta);
    return inflatedStrokeRect;
  }

  KRYS_NODISCARD static InterpolationQuality
    SmoothingToInterpolationQuality(ImageSmoothingQuality quality) noexcept
  {
    switch (quality)
    {
      case ImageSmoothingQuality::Low:    return InterpolationQuality::Low;
      case ImageSmoothingQuality::Medium: return InterpolationQuality::Medium;
      case ImageSmoothingQuality::High:   return InterpolationQuality::High;
    }

    krys_debug_assert(false);
    return InterpolationQuality::Low;
  }

  void CanvasRenderingContext2DBase::SetImageSmoothingQuality(ImageSmoothingQuality quality) noexcept
  {
    if (quality == GetState().ImageSmoothingQuality)
    {
      return;
    }

    RealizeSaves();
    ModifiableState().ImageSmoothingQuality = quality;

    if (!GetState().ImageSmoothingEnabled)
    {
      return;
    }

    if (auto *context = EffectiveDrawingContext())
    {
      context->SetImageInterpolationQuality(SmoothingToInterpolationQuality(quality));
    }
  }

  void CanvasRenderingContext2DBase::SetImageSmoothingEnabled(bool enabled) noexcept
  {
    if (enabled == GetState().ImageSmoothingEnabled)
    {
      return;
    }

    RealizeSaves();
    ModifiableState().ImageSmoothingEnabled = enabled;
    auto *c = EffectiveDrawingContext();
    if (c)
    {
      c->SetImageInterpolationQuality(enabled
                                        ? SmoothingToInterpolationQuality(GetState().ImageSmoothingQuality)
                                        : InterpolationQuality::DoNotInterpolate);
    }
  }

  void CanvasRenderingContext2DBase::SetTextAlign(CanvasTextAlign canvasTextAlign) noexcept
  {
    auto textAlign = FromCanvasTextAlign(canvasTextAlign);
    if (GetState().TextAlign == textAlign)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().TextAlign = textAlign;
  }

  void CanvasRenderingContext2DBase::SetTextBaseline(CanvasTextBaseline canvasTextBaseline) noexcept
  {
    auto textBaseline = FromCanvasTextBaseline(canvasTextBaseline);
    if (GetState().TextBaseline == textBaseline)
    {
      return;
    }
    RealizeSaves();
    ModifiableState().TextBaseline = textBaseline;
  }

  void CanvasRenderingContext2DBase::SetDirection(Direction direction) noexcept
  {
    if (GetState().Direction == direction)
    {
      return;
    }

    RealizeSaves();
    ModifiableState().Direction = direction;
  }

  void CanvasRenderingContext2DBase::UpdateStateTransform(const AffineTransform &transform) noexcept
  {
    auto &state = ModifiableState();
    state.Transform = transform;
    state.TransformInverse = transform.Inverse();
    _hasInvertibleTransform = state.TransformInverse.has_value();
  }

  DestinationColorSpace CanvasRenderingContext2DBase::ColorSpace() const noexcept
  {
    return ToDestinationColorSpace(_settings.ColorSpace);
  }

  bool CanvasRenderingContext2DBase::WillReadFrequently() const noexcept
  {
    return _settings.WillReadFrequently;
  }
}
