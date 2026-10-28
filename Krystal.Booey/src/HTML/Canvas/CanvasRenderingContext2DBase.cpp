#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"
#include "Krystal.Booey/CSS/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/ByteArrayPixelBuffer.hpp"
#include "Krystal.Booey/Graphics/DashArray.hpp"
#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/Pattern.hpp"
#include "Krystal.Booey/Graphics/PixelBufferConversion.hpp"
#include "Krystal.Booey/Graphics/SourceImage.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasFilterContextSwitcher.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/ImageData.hpp"
#include "Krystal.Booey/HTML/TextMetrics.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Visitor.hpp"
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

  bool CanvasRenderingContext2DBase::IsAccelerated() const noexcept
  {
    // The buffer's mode is the drawing context's.
    if (!GetCanvasBase().HasCreatedImageBuffer())
    {
      return false;
    }
    auto *buffer = GetCanvasBase().Buffer();
    return buffer && buffer->GetRenderingMode() == RenderingMode::Accelerated;
  }

  bool CanvasRenderingContext2DBase::IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept
  {
    // Before the first draw (or first access to the drawing buffer), the drawing buffer is transparent black.
    // Currently the canvas does not support alpha == false.
    return !GetCanvasBase().HasCreatedImageBuffer();
  }

  RefPtr<GraphicsLayerContentsDisplayDelegate>
    CanvasRenderingContext2DBase::LayerContentsDisplayDelegate() noexcept
  {
    if (auto *buffer = GetCanvasBase().Buffer())
    {
      return buffer->LayerContentsDisplayDelegate();
    }
    return nullptr;
  }

  bool CanvasRenderingContext2DBase::HasDeferredOperations() const noexcept
  {
    // Any draw may linger in the backend until it's flushed, holding references to the images it draws,
    // which a 2D context can change. Lingering draws aren't held forever: the context takes part in the
    // canvas preparation phase after any change.
    return _hasDeferredOperations;
  }

  void CanvasRenderingContext2DBase::FlushDeferredOperations() noexcept
  {
    _hasDeferredOperations = false;
    if (auto *buffer = GetCanvasBase().Buffer())
    {
      std::ignore = buffer->FlushDrawingContextAsync();
    }
  }

  bool CanvasRenderingContext2DBase::NeedsPreparationForDisplay() const noexcept
  {
    return IsAccelerated();
  }

  void CanvasRenderingContext2DBase::PrepareForDisplay() noexcept
  {
    if (auto *buffer = GetCanvasBase().Buffer())
    {
      buffer->PrepareForDisplay();
    }
  }

  void CanvasRenderingContext2DBase::Reset() noexcept
  {
    UnwindStateStack();

    krys_debug_assert(_stateStack.size() == 1);
    _stateStack.front() = State();

    _path.Clear();
    _unrealizedSaveCount = 0;
    _hasDeferredOperations = false;
    ClearAccumulatedDirtyRect();
    if (auto *c = ExistingDrawingContext())
    {
      GetCanvasBase().ResetGraphicsContextState();
      c->ClearRect(FloatRect {{}, FloatSize(GetCanvasBase().Size())});
      // The canvas is cleared: it's drawn.
      DidDrawEntireCanvas(DefaultDidDrawOptions() | DidDrawOption::PreserveCachedContents);
    }
    _cachedContents.emplace<CachedContentsTransparent>();
  }

  CanvasRenderingContext2DBase::State::State() noexcept
      : StrokeStyle(Color::black), FillStyle(Color::black), LineWidth(1), LineCap(boo::LineCap::Butt),
        LineJoin(boo::LineJoin::Miter), MiterLimit(10), ShadowBlur(0), ShadowColor(Color::transparentBlack),
        GlobalAlpha(1), GlobalComposite(CompositeOperator::SourceOver), GlobalBlend(BlendMode::Normal),
        TransformInverse(AffineTransform {}), LineDashOffset(0), ImageSmoothingEnabled(true),
        ImageSmoothingQuality(DefaultSmoothingQuality), TextAlign(StartTextAlign),
        TextBaseline(AlphabeticTextBaseline), Direction(Direction::Inherit), FilterString(u8"none"),
        Filter(css::keywords::None {}), LetterSpacing(u8"0px"), WordSpacing(u8"0px"),
        UnparsedFont(DefaultFont), FontKerning(CanvasFontKerning::Auto),
        FontStretch(CanvasFontStretch::Normal), FontVariantCaps(CanvasFontVariantCaps::Normal),
        TextRendering(CanvasTextRendering::Auto), Lang(u8"inherit")
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

  void CanvasRenderingContext2DBase::SetStrokeColorImpl(Color &&color,
                                                        dom::DOMString &&unparsedColor) noexcept
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
    UniquePtr<CanvasFilterContextSwitcher> targetSwitcher;
    if (!GetState().Filter.IsNone())
    {
      targetSwitcher = CanvasFilterContextSwitcher::Create(*this, path.FastBoundingRect());
    }

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

    DidDraw(repaintEntireCanvas,
            [&] { return targetSwitcher ? targetSwitcher->ExpandedBounds() : path.FastBoundingRect(); });

    c->SetFillRule(savedFillRule);
  }

  void CanvasRenderingContext2DBase::StrokeInternal(const Path &path) noexcept
  {
    UniquePtr<CanvasFilterContextSwitcher> targetSwitcher;
    if (!GetState().Filter.IsNone())
    {
      targetSwitcher =
        CanvasFilterContextSwitcher::Create(*this, InflatedStrokeRect(path.FastBoundingRect()));
    }

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

    DidDraw(repaintEntireCanvas,
            [&]
            {
              return targetSwitcher ? targetSwitcher->ExpandedBounds()
                                    : InflatedStrokeRect(path.FastBoundingRect());
            });
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

    auto targetSwitcher = CanvasFilterContextSwitcher::Create(*this, rect);

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

    DidDraw(repaintEntireCanvas, targetSwitcher ? targetSwitcher->ExpandedBounds() : rect);
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

    auto targetSwitcher = CanvasFilterContextSwitcher::Create(*this, inflatedStrokeRect);

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

    DidDraw(repaintEntireCanvas, targetSwitcher ? targetSwitcher->ExpandedBounds() : inflatedStrokeRect);
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

  dom::ExceptionOr<RefPtr<CanvasPattern>>
    CanvasRenderingContext2DBase::CreatePattern(CanvasImageSource &&image,
                                                dom::DOMStringView repetition) noexcept
  {
    bool repeatX, repeatY;
    if (!CanvasPattern::ParseRepetitionType(repetition, repeatX, repeatY))
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    return SwitchOn(image, [&](auto &element) -> dom::ExceptionOr<RefPtr<CanvasPattern>>
                    { return this->CreatePattern(*element, repeatX, repeatY); });
  }

  dom::ExceptionOr<RefPtr<CanvasPattern>>
    CanvasRenderingContext2DBase::CreatePattern(CachedImage &cachedImage, RenderElement *renderer,
                                                bool repeatX, bool repeatY) noexcept
  {
    RefPtr<Image> image = ShareRefPtr(cachedImage.ImageForRenderer(renderer));
    if (!image)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    auto nativeImage = image->GetNativeImage();
    if (!nativeImage)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    return RefPtr<CanvasPattern> {
      CanvasPattern::Create(SourceImage(Ref<NativeImage>(krys::move(nativeImage))), repeatX, repeatY)};
  }

  dom::ExceptionOr<RefPtr<CanvasPattern>>
    CanvasRenderingContext2DBase::CreatePattern(HTMLImageElement &imageElement, bool repeatX,
                                                bool repeatY) noexcept
  {
    auto *cachedImage = imageElement.GetCachedImage();

    // If the image loading hasn't started or the image is not complete, it is not fully decodable.
    if (!cachedImage || !imageElement.Complete())
    {
      return RefPtr<CanvasPattern> {};
    }

    if (cachedImage->ErrorOccurred())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    if (cachedImage->GetStatus() == CachedResource::LoadError)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    // Image may have a zero-width or a zero-height.
    float intrinsicWidth = 0;
    float intrinsicHeight = 0;
    FloatSize intrinsicRatio;
    cachedImage->ComputeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    if (intrinsicWidth == 0 || intrinsicHeight == 0)
    {
      return RefPtr<CanvasPattern> {};
    }

    return CreatePattern(*cachedImage, imageElement.Renderer(), repeatX, repeatY);
  }

  dom::ExceptionOr<RefPtr<CanvasPattern>>
    CanvasRenderingContext2DBase::CreatePattern(svg::SVGImageElement &imageElement, bool repeatX,
                                                bool repeatY) noexcept
  {
    auto *cachedImage = imageElement.GetCachedImage();

    // The image loading hasn't started.
    if (!cachedImage)
    {
      return RefPtr<CanvasPattern> {};
    }

    if (cachedImage->ErrorOccurred())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    // The image loading has started but it is not complete.
    if (!cachedImage->GetImage())
    {
      return RefPtr<CanvasPattern> {};
    }

    // Image may have a zero-width or a zero-height.
    float intrinsicWidth = 0;
    float intrinsicHeight = 0;
    FloatSize intrinsicRatio;
    cachedImage->ComputeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    if (intrinsicWidth == 0 || intrinsicHeight == 0)
    {
      return RefPtr<CanvasPattern> {};
    }

    return CreatePattern(*cachedImage, imageElement.Renderer(), repeatX, repeatY);
  }

  dom::ExceptionOr<RefPtr<CanvasPattern>>
    CanvasRenderingContext2DBase::CreatePattern(CanvasBase &canvas, bool repeatX, bool repeatY) noexcept
  {
    if (!canvas.Width() || !canvas.Height())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }
    auto *copiedImage = canvas.CopiedImage();

    if (!copiedImage)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    auto nativeImage = copiedImage->GetNativeImage();
    if (!nativeImage)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    return RefPtr<CanvasPattern> {
      CanvasPattern::Create(SourceImage(Ref<NativeImage>(krys::move(nativeImage))), repeatX, repeatY)};
  }

  enum class ImageSizeType : uint8
  {
    AfterDevicePixelRatio,
    BeforeDevicePixelRatio
  };

  KRYS_NODISCARD static LayoutSize
    Size(CachedImage *cachedImage, RenderElement *renderer,
         ImageSizeType sizeType = ImageSizeType::BeforeDevicePixelRatio) noexcept
  {
    if (!cachedImage)
    {
      return {};
    }
    LayoutSize size = cachedImage->ImageSizeForRenderer(renderer, 1.0f); // FIXME: Not sure about this.
    if (auto *renderImage = DynamicDowncast<RenderImage>(renderer);
        sizeType == ImageSizeType::AfterDevicePixelRatio && renderImage && cachedImage->GetImage()
        && !cachedImage->GetImage()->HasRelativeWidth())
    {
      size.Scale(renderImage->ImageDevicePixelRatio());
    }
    return size;
  }

  KRYS_NODISCARD static LayoutSize
    Size(HTMLImageElement &element, ImageSizeType sizeType = ImageSizeType::BeforeDevicePixelRatio) noexcept
  {
    return Size(element.GetCachedImage(), element.Renderer(), sizeType);
  }

  KRYS_NODISCARD static LayoutSize
    Size(svg::SVGImageElement &element,
         ImageSizeType sizeType = ImageSizeType::BeforeDevicePixelRatio) noexcept
  {
    return Size(element.GetCachedImage(), element.Renderer(), sizeType);
  }

  KRYS_NODISCARD static FloatSize Size(CanvasBase &canvas) noexcept
  {
    return FloatSize(canvas.Size());
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(CanvasImageSource &&image, float dx,
                                                                 float dy) noexcept
  {
    return SwitchOn(
      image,
      [&](RefPtr<HTMLImageElement> &imageElement) -> dom::ExceptionOr<void>
      {
        LayoutSize destRectSize = Size(*imageElement, ImageSizeType::AfterDevicePixelRatio);
        LayoutSize sourceRectSize = Size(*imageElement, ImageSizeType::BeforeDevicePixelRatio);
        return this->DrawImage(*imageElement,
                               FloatRect {0, 0, sourceRectSize.Width(), sourceRectSize.Height()},
                               FloatRect {dx, dy, destRectSize.Width(), destRectSize.Height()});
      },
      [&](RefPtr<svg::SVGImageElement> &imageElement) -> dom::ExceptionOr<void>
      {
        LayoutSize destRectSize = Size(*imageElement, ImageSizeType::AfterDevicePixelRatio);
        LayoutSize sourceRectSize = Size(*imageElement, ImageSizeType::BeforeDevicePixelRatio);
        return this->DrawImage(*imageElement,
                               FloatRect {0, 0, sourceRectSize.Width(), sourceRectSize.Height()},
                               FloatRect {dx, dy, destRectSize.Width(), destRectSize.Height()});
      },
      [&](auto &element) -> dom::ExceptionOr<void>
      {
        FloatSize elementSize = FloatSize(Size(*element));
        return this->DrawImage(*element, FloatRect {0, 0, elementSize.Width(), elementSize.Height()},
                               FloatRect {dx, dy, elementSize.Width(), elementSize.Height()});
      });
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(CanvasImageSource &&image, float dx,
                                                                 float dy, float dw, float dh) noexcept
  {
    return SwitchOn(image,
                    [&](auto &element) -> dom::ExceptionOr<void>
                    {
                      FloatSize elementSize = FloatSize(Size(*element));
                      return this->DrawImage(*element,
                                             FloatRect {0, 0, elementSize.Width(), elementSize.Height()},
                                             FloatRect {dx, dy, dw, dh});
                    });
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(CanvasImageSource &&image, float sx,
                                                                 float sy, float sw, float sh, float dx,
                                                                 float dy, float dw, float dh) noexcept
  {
    return SwitchOn(
      image, [&](auto &element) -> dom::ExceptionOr<void>
      { return this->DrawImage(*element, FloatRect {sx, sy, sw, sh}, FloatRect {dx, dy, dw, dh}); });
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(HTMLImageElement &imageElement,
                                                                 const FloatRect &srcRect,
                                                                 const FloatRect &dstRect) noexcept
  {
    return DrawImage(imageElement, srcRect, dstRect, GetState().GlobalComposite, GetState().GlobalBlend);
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(HTMLImageElement &imageElement,
                                                                 const FloatRect &srcRect,
                                                                 const FloatRect &dstRect,
                                                                 const CompositeOperator &op,
                                                                 const BlendMode &blendMode) noexcept
  {
    if (!imageElement.Complete())
    {
      return {};
    }

    auto *cachedImage = imageElement.GetCachedImage();
    if (!cachedImage)
    {
      return {};
    }

    if (cachedImage->GetStatus() == CachedResource::DecodeError)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"The HTMLImageElement provided is in the 'broken' state."};
    }

    auto imageRect =
      FloatRect(FloatPoint(), FloatSize(Size(imageElement, ImageSizeType::BeforeDevicePixelRatio)));

    // FIXME: The computed style's orientation for an image without a renderer.
    auto orientation = ImageOrientation(ImageOrientation::Orientation::FromImage);
    if (auto *renderer = imageElement.Renderer())
    {
      orientation = renderer->GetImageOrientation();
    }

    return DrawImage(imageElement.NodeDocument(), *cachedImage, imageElement.Renderer(), imageRect, srcRect,
                     dstRect, op, blendMode, orientation);
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(svg::SVGImageElement &imageElement,
                                                                 const FloatRect &srcRect,
                                                                 const FloatRect &dstRect) noexcept
  {
    return DrawImage(imageElement, srcRect, dstRect, GetState().GlobalComposite, GetState().GlobalBlend);
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(svg::SVGImageElement &imageElement,
                                                                 const FloatRect &srcRect,
                                                                 const FloatRect &dstRect,
                                                                 const CompositeOperator &op,
                                                                 const BlendMode &blendMode) noexcept
  {
    auto *cachedImage = imageElement.GetCachedImage();
    if (!cachedImage)
    {
      return {};
    }

    if (cachedImage->GetStatus() == CachedResource::DecodeError)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"The SVGImageElement provided is in the 'broken' state."};
    }

    auto imageRect =
      FloatRect(FloatPoint(), FloatSize(Size(imageElement, ImageSizeType::BeforeDevicePixelRatio)));

    return DrawImage(imageElement.NodeDocument(), *cachedImage, imageElement.Renderer(), imageRect, srcRect,
                     dstRect, op, blendMode);
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(
    dom::Document &, CachedImage &cachedImage, const RenderObject *renderer, const FloatRect &imageRect,
    const FloatRect &srcRect, const FloatRect &dstRect, const CompositeOperator &op,
    const BlendMode &blendMode, ImageOrientation orientation) noexcept
  {
    if (!std::isfinite(dstRect.X()) || !std::isfinite(dstRect.Y()) || !std::isfinite(dstRect.Width())
        || !std::isfinite(dstRect.Height()) || !std::isfinite(srcRect.X()) || !std::isfinite(srcRect.Y())
        || !std::isfinite(srcRect.Width()) || !std::isfinite(srcRect.Height()))
    {
      return {};
    }

    if (!srcRect.Width() || !srcRect.Height())
    {
      return {};
    }

    if (!dstRect.Width() || !dstRect.Height())
    {
      return {};
    }

    auto normalizedSrcRect = NormaliseRect(srcRect);
    auto normalizedDstRect = NormaliseRect(dstRect);

    if (normalizedSrcRect.IsEmpty())
    {
      return {};
    }

    if (normalizedDstRect.IsEmpty())
    {
      return {};
    }

    auto targetSwitcher = CanvasFilterContextSwitcher::Create(*this, normalizedDstRect);

    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return {};
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return {};
    }

    RefPtr<Image> image = ShareRefPtr(cachedImage.ImageForRenderer(renderer));
    if (!image)
    {
      return {};
    }

    auto observer = image->GetImageObserver();
    auto shouldPostProcess = true;

    if (image->DrawsSVGImage())
    {
      image->SetImageObserver(nullptr);
      image->SetContainerSize(imageRect.Size());
    }

    if (auto *bitmap = DynamicDowncast<BitmapImage>(*image))
    {
      // Drawing an animated image to a canvas should draw the first frame.
      if (image->IsAnimated())
      {
        RefPtr<BitmapImage> bitmapImage = BitmapImage::Create(bitmap->GetNativeImage());
        if (!bitmapImage)
        {
          return {};
        }
        image = krys::move(bitmapImage);
      }

      shouldPostProcess = false;
    }

    ImagePaintingOptions options = {op, blendMode, orientation};

    bool repaintEntireCanvas = false;
    if (RectContainsCanvas(normalizedDstRect))
    {
      std::ignore = c->DrawImage(*image, normalizedDstRect, normalizedSrcRect, options);
      repaintEntireCanvas = true;
    }
    else if (IsFullCanvasCompositeMode(op))
    {
      FullCanvasCompositedDrawImage(*image, normalizedDstRect, normalizedSrcRect, op);
      repaintEntireCanvas = true;
    }
    else if (op == CompositeOperator::Copy)
    {
      ClearCanvas();
      std::ignore = c->DrawImage(*image, normalizedDstRect, normalizedSrcRect, options);
      repaintEntireCanvas = true;
    }
    else
    {
      std::ignore = c->DrawImage(*image, normalizedDstRect, normalizedSrcRect, options);
    }

    DidDraw(repaintEntireCanvas, targetSwitcher ? targetSwitcher->ExpandedBounds() : normalizedDstRect,
            shouldPostProcess ? DefaultDidDrawOptions() : DefaultDidDrawOptionsWithoutPostProcessing());

    if (image->DrawsSVGImage())
    {
      image->SetImageObserver(krys::move(observer));
    }

    return {};
  }

  dom::ExceptionOr<void> CanvasRenderingContext2DBase::DrawImage(CanvasBase &sourceCanvas,
                                                                 const FloatRect &srcRect,
                                                                 const FloatRect &dstRect) noexcept
  {
    FloatRect srcCanvasRect = FloatRect(FloatPoint(), FloatSize(sourceCanvas.Size()));

    if (!srcCanvasRect.Width() || !srcCanvasRect.Height())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    if (!srcRect.Width() || !srcRect.Height())
    {
      return {};
    }

    auto normalizedSrcRect = NormaliseRect(srcRect);
    auto normalizedDstRect = NormaliseRect(dstRect);

    if (normalizedSrcRect.IsEmpty())
    {
      return {};
    }

    if (normalizedDstRect.IsEmpty())
    {
      return {};
    }

    auto targetSwitcher = CanvasFilterContextSwitcher::Create(*this, normalizedDstRect);

    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return {};
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return {};
    }

    Ref<CanvasBase> protectedCanvas = ShareRef(sourceCanvas);

    RefPtr<ImageBuffer> buffer = sourceCanvas.MakeRenderingResultsAvailable();
    if (!buffer)
    {
      return {};
    }

    bool repaintEntireCanvas = false;
    if (RectContainsCanvas(normalizedDstRect))
    {
      c->DrawImageBuffer(*buffer, normalizedDstRect, normalizedSrcRect,
                         {GetState().GlobalComposite, GetState().GlobalBlend});
      repaintEntireCanvas = true;
    }
    else if (IsFullCanvasCompositeMode(GetState().GlobalComposite))
    {
      FullCanvasCompositedDrawImage(*buffer, normalizedDstRect, normalizedSrcRect,
                                    GetState().GlobalComposite);
      repaintEntireCanvas = true;
    }
    else if (GetState().GlobalComposite == CompositeOperator::Copy)
    {
      if (&sourceCanvas == &GetCanvasBase())
      {
        if (auto copy = c->CreateImageBuffer(normalizedSrcRect.Size(), 1))
        {
          copy->Context().DrawImageBuffer(*buffer, -normalizedSrcRect.Location());
          ClearCanvas();
          c->DrawImageBuffer(*copy, normalizedDstRect, {{}, normalizedSrcRect.Size()},
                             {GetState().GlobalComposite, GetState().GlobalBlend});
        }
      }
      else
      {
        ClearCanvas();
        c->DrawImageBuffer(*buffer, normalizedDstRect, normalizedSrcRect,
                           {GetState().GlobalComposite, GetState().GlobalBlend});
      }
      repaintEntireCanvas = true;
    }
    else
    {
      c->DrawImageBuffer(*buffer, normalizedDstRect, normalizedSrcRect,
                         {GetState().GlobalComposite, GetState().GlobalBlend});
    }

    auto *sourceContext = sourceCanvas.RenderingContext();
    auto shouldUseDrawOptionsWithoutPostProcessing = sourceContext && sourceContext->Is2d();
    DidDraw(repaintEntireCanvas, targetSwitcher ? targetSwitcher->ExpandedBounds() : normalizedDstRect,
            shouldUseDrawOptionsWithoutPostProcessing ? DefaultDidDrawOptionsWithoutPostProcessing()
                                                      : DefaultDidDrawOptions());

    return {};
  }

  Path CanvasRenderingContext2DBase::TransformAreaToDevice(const Path &path) const noexcept
  {
    Path transformed(path);
    transformed.Transform(GetState().Transform);
    transformed.Transform(BaseTransform());
    return transformed;
  }

  Path CanvasRenderingContext2DBase::TransformAreaToDevice(const FloatRect &rect) const noexcept
  {
    Path path;
    path.AddRect(rect);
    return TransformAreaToDevice(path);
  }

  IntRect CanvasRenderingContext2DBase::CalculateCompositingBufferRect(const FloatRect &area,
                                                                       IntSize *croppedOffset) noexcept
  {
    IntRect canvasRect(0, 0, static_cast<int32>(GetCanvasBase().Width()),
                       static_cast<int32>(GetCanvasBase().Height()));
    canvasRect = BaseTransform().MapRect(canvasRect);
    Path path = TransformAreaToDevice(area);
    IntRect bufferRect = EnclosingIntRect(path.FastBoundingRect());
    IntPoint originalLocation = bufferRect.Location();
    bufferRect.Intersect(canvasRect);
    if (croppedOffset)
    {
      *croppedOffset = originalLocation - bufferRect.Location();
    }
    return bufferRect;
  }

  void CanvasRenderingContext2DBase::CompositeBuffer(ImageBuffer &buffer, const IntRect &bufferRect,
                                                     CompositeOperator op) noexcept
  {
    IntRect canvasRect(0, 0, static_cast<int32>(GetCanvasBase().Width()),
                       static_cast<int32>(GetCanvasBase().Height()));
    canvasRect = BaseTransform().MapRect(canvasRect);

    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }

    c->Save();
    c->SetCTM(AffineTransform());
    c->SetCompositeOperation(op);

    c->Save();
    c->ClipOut(FloatRect(bufferRect));
    c->ClearRect(FloatRect(canvasRect));
    c->Restore();
    c->DrawImageBuffer(buffer, FloatPoint(bufferRect.Location()),
                       ImagePaintingOptions(GetState().GlobalComposite));
    c->Restore();
  }

  static void DrawImageToContext(Image &image, GraphicsContext &context, const FloatRect &dest,
                                 const FloatRect &src, ImagePaintingOptions options) noexcept
  {
    std::ignore = context.DrawImage(image, dest, src, options);
  }

  static void DrawImageToContext(ImageBuffer &imageBuffer, GraphicsContext &context, const FloatRect &dest,
                                 const FloatRect &src, ImagePaintingOptions options) noexcept
  {
    context.DrawImageBuffer(imageBuffer, dest, src, options);
  }

  template <typename T>
  void CanvasRenderingContext2DBase::FullCanvasCompositedDrawImage(T &image, const FloatRect &dest,
                                                                   const FloatRect &src,
                                                                   CompositeOperator op) noexcept
  {
    krys_debug_assert(IsFullCanvasCompositeMode(op));

    IntSize croppedOffset;
    auto bufferRect = CalculateCompositingBufferRect(dest, &croppedOffset);
    if (bufferRect.IsEmpty())
    {
      ClearCanvas();
      return;
    }

    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return;
    }

    auto buffer = c->CreateImageBuffer(FloatSize(bufferRect.Size()));
    if (!buffer)
    {
      return;
    }

    FloatRect adjustedDest = dest;
    adjustedDest.SetLocation(FloatPoint(0, 0));
    AffineTransform effectiveTransform = c->GetCTM();
    IntRect transformedAdjustedRect = EnclosingIntRect(effectiveTransform.MapRect(adjustedDest));
    buffer->Context().Translate(FloatPoint(-transformedAdjustedRect.Location()));
    buffer->Context().Translate(FloatSize(croppedOffset));
    buffer->Context().ConcatCTM(effectiveTransform);
    DrawImageToContext(image, buffer->Context(), adjustedDest, src,
                       ImagePaintingOptions(CompositeOperator::SourceOver));

    CompositeBuffer(*buffer, bufferRect, op);
  }

  // The area of image data a put of 60 by 60 pixels or less keeps, so that reading it back needn't read the
  // buffer.
  static constexpr uint32 PutImageDataCacheAreaLimit = 60 * 60;

  dom::ExceptionOr<Ref<ImageData>>
    CanvasRenderingContext2DBase::CreateImageData(ImageData &existingImageData) const noexcept
  {
    return ImageData::Create(static_cast<uint32>(existingImageData.Width()),
                             static_cast<uint32>(existingImageData.Height()), existingImageData.ColorSpace());
  }

  dom::ExceptionOr<Ref<ImageData>>
    CanvasRenderingContext2DBase::CreateImageData(int32 sw, int32 sh,
                                                  Maybe<ImageDataSettings> settings) const noexcept
  {
    if (!sw || !sh)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    return ImageData::Create(static_cast<uint32>(std::abs(static_cast<int64>(sw))),
                             static_cast<uint32>(std::abs(static_cast<int64>(sh))), _settings.ColorSpace,
                             settings);
  }

  void CanvasRenderingContext2DBase::EvictCachedImageData() noexcept
  {
    _cachedContents.emplace<CachedContentsUnknown>();
  }

  CanvasRenderingContext2DBase::CachedContentsImageData::CachedContentsImageData(
    CanvasRenderingContext2DBase &context, Ref<ByteArrayPixelBuffer> imageData) noexcept
      : Pixels(krys::move(imageData)),
        EvictionTimer(CreateUnique<Timer>(context, &CanvasRenderingContext2DBase::EvictCachedImageData))
  {
    EvictionTimer->StartOneShot(std::chrono::seconds(5));
  }

  RefPtr<ByteArrayPixelBuffer> CanvasRenderingContext2DBase::CacheImageDataIfPossible(
    const ImageData &imageData, const IntRect &sourceRect, const IntPoint &destinationPosition) noexcept
  {
    if (!destinationPosition.IsZero() || !sourceRect.Location().IsZero()
        || sourceRect.Size() != imageData.Size() || sourceRect.Size() != GetCanvasBase().Size())
    {
      return nullptr;
    }

    auto size = imageData.Size();
    if (size.UnclampedArea() > PutImageDataCacheAreaLimit)
    {
      return nullptr;
    }

    if (imageData.ColorSpace() != _settings.ColorSpace)
    {
      return nullptr;
    }

    if (imageData.PixelFormat() != ImageDataPixelFormat::RgbaUnorm8)
    {
      return nullptr;
    }

    // Consider:
    //   * Real putImageData needs premultiply step.
    //   * Retrieve from cache needs to ensure premultiply + unpremultiply was made to simulate the real
    //     putImageData.
    //   * Caching needs at least a memcpy here.
    // Instead of doing the plain memcpy, copy by doing premultiply here.
    // This computation can be used for cache retrieval as well as the real putImageData.
    // We're not doing RGBA -> BGRA swizzle here, as that is not needed for cache retrieval and
    // the swizzle copy can be made at the putImageData copy site.
    auto colorSpace = ToDestinationColorSpace(imageData.ColorSpace());
    uint32 bytesPerRow = static_cast<uint32>(size.Width()) * 4u;
    PixelBufferFormat cachedFormat {AlphaPremultiplication::Premultiplied, PixelFormat::RGBA8, colorSpace};
    auto cachedBuffer = ByteArrayPixelBuffer::TryCreate(cachedFormat, size);
    if (!cachedBuffer)
    {
      return nullptr;
    }
    auto dataAsUint8ClampedArray = imageData.Data().AsUint8ClampedArray();
    ConstPixelBufferConversionView source {
      .Format = {AlphaPremultiplication::Unpremultiplied, PixelFormat::RGBA8, colorSpace},
      .BytesPerRow = bytesPerRow,
      .Rows = dataAsUint8ClampedArray->GetSpan(),
    };
    PixelBufferConversionView destination {
      .Format = cachedFormat,
      .BytesPerRow = bytesPerRow,
      .Rows = cachedBuffer->Data().MutableSpan(),
    };
    ConvertImagePixels(source, destination, size);
    _cachedContents.emplace<CachedContentsImageData>(*this, Ref<ByteArrayPixelBuffer>(cachedBuffer));
    return cachedBuffer;
  }

  RefPtr<ImageData> CanvasRenderingContext2DBase::MakeImageDataIfContentsCached(
    const IntRect &sourceRect, PredefinedColorSpace colorSpace) const noexcept
  {
    if (std::holds_alternative<CachedContentsTransparent>(_cachedContents))
    {
      return ImageData::Create(sourceRect.Size(), colorSpace);
    }
    if (std::holds_alternative<CachedContentsUnknown>(_cachedContents))
    {
      return nullptr;
    }
    // Always consume the cached image data.
    Ref<ByteArrayPixelBuffer> pixelBuffer =
      krys::move(std::get<CachedContentsImageData>(_cachedContents).Pixels);
    _cachedContents.emplace<CachedContentsUnknown>();

    if (sourceRect != IntRect {{}, GetCanvasBase().Size()})
    {
      return nullptr;
    }

    if (GetCanvasBase().Size() != pixelBuffer->Size())
    {
      return nullptr;
    }

    if (colorSpace != _settings.ColorSpace)
    {
      return nullptr;
    }

    auto size = pixelBuffer->Size();
    auto data = pixelBuffer->TakeData();
    uint32 bytesPerRow = static_cast<uint32>(size.Width()) * 4u;
    ConstPixelBufferConversionView source {
      .Format = pixelBuffer->Format(),
      .BytesPerRow = bytesPerRow,
      .Rows = data->GetSpan(),
    };
    PixelBufferConversionView destination {
      .Format = {AlphaPremultiplication::Unpremultiplied, PixelFormat::RGBA8,
                 pixelBuffer->Format().ColorSpace},
      .BytesPerRow = bytesPerRow,
      .Rows = data->MutableSpan(),
    };
    ConvertImagePixels(source, destination, size);
    return ImageData::Create(size, ImageDataArray(krys::move(data)), _settings.ColorSpace);
  }

  dom::ExceptionOr<Ref<ImageData>>
    CanvasRenderingContext2DBase::GetImageData(int32 sx, int32 sy, int32 sw, int32 sh,
                                               Maybe<ImageDataSettings> settings) const noexcept
  {
    if (!sw || !sh)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    // FIXME: A canvas tainted by cross-origin data throws a SecurityError, once there are origins.

    if (sw < 0)
    {
      sx += sw;
      sw = -sw;
    }
    if (sh < 0)
    {
      sy += sh;
      sh = -sh;
    }

    IntRect imageDataRect {sx, sy, sw, sh};
    auto outputImageDataPixelFormat = settings ? settings->Format : ImageDataPixelFormat::RgbaUnorm8;
    auto outputPixelFormat = ToPixelFormat(outputImageDataPixelFormat);

    auto computedColorSpace = ImageData::ComputeColorSpace(settings, _settings.ColorSpace);

    if (outputImageDataPixelFormat == ImageDataPixelFormat::RgbaUnorm8)
    {
      if (auto imageData = MakeImageDataIfContentsCached(imageDataRect, computedColorSpace))
      {
        return Ref<ImageData>(krys::move(imageData));
      }
    }

    RefPtr<ImageBuffer> buffer = GetCanvasBase().MakeRenderingResultsAvailable();
    if (!buffer)
    {
      return ImageData::Create(static_cast<uint32>(imageDataRect.Width()),
                               static_cast<uint32>(imageDataRect.Height()), _settings.ColorSpace, settings);
    }

    PixelBufferFormat format {AlphaPremultiplication::Unpremultiplied, outputPixelFormat,
                              ToDestinationColorSpace(computedColorSpace)};
    RefPtr<PixelBuffer> pixelBuffer = buffer->GetPixelBuffer(format, imageDataRect);
    if (!pixelBuffer)
    {
      // FIXME: A console message: "Unable to get image data from canvas. Requested size was w x h".
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    krys_debug_assert(pixelBuffer->Format().ColorSpace == ToDestinationColorSpace(computedColorSpace));

    if (auto imageData =
          ImageData::Create(Ref<PixelBuffer>(krys::move(pixelBuffer)), outputImageDataPixelFormat))
    {
      return Ref<ImageData>(krys::move(imageData));
    }

    return dom::Exception {dom::ExceptionCode::InvalidStateError};
  }

  void CanvasRenderingContext2DBase::PutImageData(ImageData &data, int32 dx, int32 dy) noexcept
  {
    PutImageData(data, dx, dy, 0, 0, data.Width(), data.Height());
  }

  KRYS_NODISCARD static IntRect ComputeImageDataRect(const ImageBuffer &buffer, IntSize sourceSize,
                                                     IntRect &destRect, IntPoint destOffset) noexcept
  {
    destRect.Intersect(IntRect {{}, sourceSize});
    destRect.MoveBy(destOffset);
    destRect.Intersect(IntRect {{}, buffer.TruncatedLogicalSize()});
    if (destRect.IsEmpty())
    {
      return destRect;
    }
    IntRect sourceRect {destRect};
    sourceRect.MoveBy(-destOffset);
    sourceRect.Intersect(IntRect {{}, sourceSize});
    return sourceRect;
  }

  void CanvasRenderingContext2DBase::PutImageData(ImageData &data, int32 dx, int32 dy, int32 dirtyX,
                                                  int32 dirtyY, int32 dirtyWidth, int32 dirtyHeight) noexcept
  {
    RefPtr<ImageBuffer> buffer = ShareRefPtr(GetCanvasBase().Buffer());
    if (!buffer)
    {
      return;
    }

    if (data.Data().IsDetached())
    {
      return;
    }

    if (dirtyWidth < 0)
    {
      dirtyX += dirtyWidth;
      dirtyWidth = -dirtyWidth;
    }

    if (dirtyHeight < 0)
    {
      dirtyY += dirtyHeight;
      dirtyHeight = -dirtyHeight;
    }

    IntPoint destOffset {dx, dy};
    IntRect destRect {dirtyX, dirtyY, dirtyWidth, dirtyHeight};
    IntRect sourceRect = ComputeImageDataRect(*buffer, data.Size(), destRect, destOffset);

    EnumSet<DidDrawOption> options; // ignore transform, shadow, clip, and post-processing
    if (!sourceRect.IsEmpty())
    {
      RefPtr<PixelBuffer> pixelBuffer = CacheImageDataIfPossible(data, sourceRect, destOffset);
      if (pixelBuffer)
      {
        options.Add(DidDrawOption::PreserveCachedContents);
      }
      else
      {
        pixelBuffer = data.GetByteArrayPixelBuffer();
      }
      buffer->PutPixelBuffer(*pixelBuffer, sourceRect, destOffset);
    }

    DidDraw(FloatRect {destRect}, options);
  }

  void CanvasRenderingContext2DBase::DidDrawEntireCanvas(EnumSet<DidDrawOption> options) noexcept
  {
    DidDraw(BackingStoreBounds(), options);
  }

  void CanvasRenderingContext2DBase::DidDraw(Maybe<FloatRect> rect, EnumSet<DidDrawOption> options) noexcept
  {
    if (!options.Contains(DidDrawOption::PreserveCachedContents))
    {
      _cachedContents.emplace<CachedContentsUnknown>();
    }

    auto *context = EffectiveDrawingContext();
    if (!context)
    {
      return;
    }

    _hasDeferredOperations = true;

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
    auto *context = DrawingContext();
    if (!context)
    {
      return nullptr;
    }
    if (auto targetSwitcher = GetState().TargetSwitcher)
    {
      return targetSwitcher->DrawingContext();
    }
    return context;
  }

  RefPtr<Filter> CanvasRenderingContext2DBase::CreateFilter(const FloatRect &) const noexcept
  {
    return nullptr;
  }

  void CanvasRenderingContext2DBase::SetFilterString(dom::DOMStringView filterString) noexcept
  {
    if (GetState().FilterString == filterString)
    {
      return;
    }

    // Spec: context.filter = "" should leave the current filter unchanged.
    if (filterString.empty())
    {
      return;
    }

    auto filter = SetFilterStringWithoutUpdatingStyle(filterString);
    if (!filter)
    {
      return;
    }

    RealizeSaves();

    // Spec: context.filter = "none" filters will be disabled for the context.
    // Spec: Only parseable inputs should change the current filter.
    ModifiableState().FilterString = dom::DOMString(filterString);
    ModifiableState().Filter = krys::move(*filter);
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

  // MARK: - Text

  dom::DOMString CanvasRenderingContext2DBase::State::FontString() const noexcept
  {
    if (!Font.Realized())
    {
      return dom::DOMString(DefaultFont);
    }

    StringBuilder serializedFont;
    const auto &font = Font.GetFontDescription();

    auto italic = font.FontStyleSlope() ? utf8_stringview {u8"italic "} : utf8_stringview {};
    auto smallCaps = font.VariantCaps() == boo::FontVariantCaps::Small ? utf8_stringview {u8"small-caps "}
                                                                       : utf8_stringview {};
    serializedFont.Append(italic, smallCaps);
    auto weight = static_cast<int32>(static_cast<float>(font.Weight()));
    if (weight == static_cast<int32>(static_cast<float>(BoldWeightValue())))
    {
      serializedFont.Append(u8"bold ");
    }
    else if (weight != static_cast<int32>(static_cast<float>(NormalWeightValue())))
    {
      serializedFont.Append(weight, u8" ");
    }
    serializedFont.Append(font.ComputedSize(), u8"px");

    for (uint32 i = 0; i < font.FamilyCount(); ++i)
    {
      auto family = font.FamilyAt(i).View();
      if (family.starts_with(u8"-webkit-"))
      {
        family = family.substr(8);
      }

      auto separator = i ? utf8_stringview {u8", "} : utf8_stringview {u8" "};
      serializedFont.Append(separator, css::SerialiseFontFamily(css::CSSOMString(family)));
    }

    return krys::move(serializedFont).ToString();
  }

  CanvasRenderingContext2DBase::FontProxy::~FontProxy() noexcept
  {
    if (Realized())
    {
      _font.GetFontSelector()->UnregisterForInvalidationCallbacks(*this);
    }
  }

  CanvasRenderingContext2DBase::FontProxy::FontProxy(const FontProxy &other) noexcept
      : FontSelectorClient(), _font(other._font)
  {
    if (Realized())
    {
      _font.GetFontSelector()->RegisterForInvalidationCallbacks(*this);
    }
  }

  auto CanvasRenderingContext2DBase::FontProxy::operator=(const FontProxy &other) noexcept -> FontProxy &
  {
    if (Realized())
    {
      _font.GetFontSelector()->UnregisterForInvalidationCallbacks(*this);
    }

    _font = other._font;

    if (Realized())
    {
      _font.GetFontSelector()->RegisterForInvalidationCallbacks(*this);
    }

    return *this;
  }

  void CanvasRenderingContext2DBase::FontProxy::Update(FontSelector &selector) noexcept
  {
    // This is an invariant. We should only ever be registered for callbacks on the font's selector.
    krys_debug_assert(&selector == _font.GetFontSelector());
    if (Realized())
    {
      _font.GetFontSelector()->UnregisterForInvalidationCallbacks(*this);
    }
    _font.Update(ShareRefPtr(&selector));
    if (Realized())
    {
      _font.GetFontSelector()->RegisterForInvalidationCallbacks(*this);
    }
    krys_debug_assert(&selector == _font.GetFontSelector());
  }

  void CanvasRenderingContext2DBase::FontProxy::FontsNeedUpdate(FontSelector &selector) noexcept
  {
    krys_debug_assert(&selector == _font.GetFontSelector());
    krys_debug_assert(Realized());

    Update(selector);
  }

  void CanvasRenderingContext2DBase::FontProxy::Initialize(FontSelector &fontSelector,
                                                           const FontCascade &fontCascade) noexcept
  {
    // Beware! _font.GetFontSelector() might not point to the document's font selector!
    krys_debug_assert(fontCascade.GetFontSelector() == &fontSelector);
    if (Realized())
    {
      _font.GetFontSelector()->UnregisterForInvalidationCallbacks(*this);
    }
    _font = fontCascade;
    _font.Update(ShareRefPtr(&fontSelector));
    krys_debug_assert(&fontSelector == _font.GetFontSelector());
    _font.GetFontSelector()->RegisterForInvalidationCallbacks(*this);
  }

  const FontMetrics &CanvasRenderingContext2DBase::FontProxy::MetricsOfPrimaryFont() const noexcept
  {
    return _font.MetricsOfPrimaryFont();
  }

  const FontCascadeDescription &CanvasRenderingContext2DBase::FontProxy::GetFontDescription() const noexcept
  {
    return _font.GetFontDescription();
  }

  float CanvasRenderingContext2DBase::FontProxy::Width(const TextRun &textRun,
                                                       GlyphOverflow *overflow) const noexcept
  {
    return _font.Width(textRun, nullptr, overflow);
  }

  void CanvasRenderingContext2DBase::FontProxy::DrawBidiText(
    GraphicsContext &context, const TextRun &run, const FloatPoint &point,
    FontCascade::CustomFontNotReadyAction action) const noexcept
  {
    context.DrawBidiText(_font, run, point, action);
  }

  // FIXME: The HTML spec currently doesn't define how <length> units should be resolved, so we only
  // allow units where the resolution is straightforward. See https://github.com/whatwg/html/issues/10893.
  KRYS_NODISCARD static bool UnitAllowedForSpacing(css::LengthUnit lengthUnit) noexcept
  {
    using enum css::LengthUnit;

    switch (lengthUnit)
    {
      case Px:
      case Cm:
      case Mm:
      case Q:
      case In:
      case Pt:
      case Pc:
      case Em:
      case QuirkyEm:
      case Ex:
      case Cap:
      case Ch:
      case Ic:
      case Rcap:
      case Rch:
      case Rem:
      case Rex:
      case Ric:      return true;

      case Lh:
      case Rlh:
      case Vw:
      case Vh:
      case Vmin:
      case Vmax:
      case Vb:
      case Vi:
      case Svw:
      case Svh:
      case Svmin:
      case Svmax:
      case Svb:
      case Svi:
      case Lvw:
      case Lvh:
      case Lvmin:
      case Lvmax:
      case Lvb:
      case Lvi:
      case Dvw:
      case Dvh:
      case Dvmin:
      case Dvmax:
      case Dvb:
      case Dvi:
      case Cqw:
      case Cqh:
      case Cqi:
      case Cqb:
      case Cqmin:
      case Cqmax:    return false;
    }

    krys_assert(false);
    return false;
  }

  namespace
  {
    struct ParsedSpacing
    {
      dom::DOMString Serialised;
      double Pixels;
    };
  }

  /// @brief @p spacing parsed as a CSS length and resolved against @p fontCascade; empty if it isn't one, or
  /// has a unit spacing doesn't allow.
  KRYS_NODISCARD static Maybe<ParsedSpacing> ParseSpacing(dom::DOMStringView spacing,
                                                          css::PropertyId property,
                                                          const FontCascade &fontCascade) noexcept
  {
    css::InputStream inputStream {css::CSSOMString(spacing)};
    css::Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto tokenRange = tokenizer.Tokens();
    tokenRange.DiscardWhitespace();

    auto parserContext = css::ParserContext {css::ParserMode::HTMLStandard};
    auto parserState = css::PropertyParserState {.Context = parserContext};

    auto parsedValue = css::MetaConsumer<css::Length<>>::Consume(tokenRange, parserState);
    if (!parsedValue)
    {
      return null;
    }
    tokenRange.DiscardWhitespace();
    if (!tokenRange.IsAtEnd())
    {
      return null;
    }
    auto rawLength = parsedValue->GetRaw();
    if (!rawLength)
    {
      return null;
    }
    if (!UnitAllowedForSpacing(rawLength->Unit))
    {
      return null;
    }

    double pixels =
      style::ComputeUnzoomedNonCalcLengthDouble(rawLength->Value, rawLength->Unit, property, &fontCascade);

    StringBuilder serialised;
    css::Serialise(serialised, css::DefaultSerialisationContext(), *rawLength);
    return ParsedSpacing {.Serialised = krys::move(serialised).ToString(), .Pixels = pixels};
  }

  void CanvasRenderingContext2DBase::SetLetterSpacing(dom::DOMStringView letterSpacing) noexcept
  {
    if (GetState().LetterSpacing == letterSpacing)
    {
      return;
    }
    RealizeSaves();

    auto spacing =
      ParseSpacing(letterSpacing, css::PropertyId::LetterSpacing, GetFontProxy()->GetFontCascade());
    if (!spacing)
    {
      return;
    }

    ModifiableState().LetterSpacing = krys::move(spacing->Serialised);
    ModifiableState().Font.SetLetterSpacing(static_cast<float>(spacing->Pixels));
  }

  void CanvasRenderingContext2DBase::SetWordSpacing(dom::DOMStringView wordSpacing) noexcept
  {
    if (GetState().WordSpacing == wordSpacing)
    {
      return;
    }
    RealizeSaves();

    auto spacing = ParseSpacing(wordSpacing, css::PropertyId::WordSpacing, GetFontProxy()->GetFontCascade());
    if (!spacing)
    {
      return;
    }

    ModifiableState().WordSpacing = krys::move(spacing->Serialised);
    ModifiableState().Font.SetWordSpacing(static_cast<float>(spacing->Pixels));
  }

  KRYS_NODISCARD static Kerning ToKerning(CanvasFontKerning fontKerning) noexcept
  {
    switch (fontKerning)
    {
      case CanvasFontKerning::Auto:   return Kerning::Auto;
      case CanvasFontKerning::Normal: return Kerning::Normal;
      case CanvasFontKerning::None:   return Kerning::NoShift;
    }
    krys_debug_assert(false);
    return Kerning::Auto;
  }

  KRYS_NODISCARD static FontSelectionValue ToFontWidth(CanvasFontStretch fontStretch) noexcept
  {
    switch (fontStretch)
    {
      case CanvasFontStretch::UltraCondensed: return UltraCondensedWidthValue();
      case CanvasFontStretch::ExtraCondensed: return ExtraCondensedWidthValue();
      case CanvasFontStretch::Condensed:      return CondensedWidthValue();
      case CanvasFontStretch::SemiCondensed:  return SemiCondensedWidthValue();
      case CanvasFontStretch::Normal:         return NormalWidthValue();
      case CanvasFontStretch::SemiExpanded:   return SemiExpandedWidthValue();
      case CanvasFontStretch::Expanded:       return ExpandedWidthValue();
      case CanvasFontStretch::ExtraExpanded:  return ExtraExpandedWidthValue();
      case CanvasFontStretch::UltraExpanded:  return UltraExpandedWidthValue();
    }
    krys_debug_assert(false);
    return NormalWidthValue();
  }

  KRYS_NODISCARD static CanvasFontStretch FromFontWidth(FontSelectionValue width) noexcept
  {
    for (auto fontStretch :
         {CanvasFontStretch::UltraCondensed, CanvasFontStretch::ExtraCondensed, CanvasFontStretch::Condensed,
          CanvasFontStretch::SemiCondensed, CanvasFontStretch::SemiExpanded, CanvasFontStretch::Expanded,
          CanvasFontStretch::ExtraExpanded, CanvasFontStretch::UltraExpanded})
    {
      if (ToFontWidth(fontStretch) == width)
      {
        return fontStretch;
      }
    }
    return CanvasFontStretch::Normal;
  }

  KRYS_NODISCARD static boo::FontVariantCaps ToFontVariantCaps(CanvasFontVariantCaps fontVariantCaps) noexcept
  {
    switch (fontVariantCaps)
    {
      case CanvasFontVariantCaps::Normal:        return boo::FontVariantCaps::Normal;
      case CanvasFontVariantCaps::SmallCaps:     return boo::FontVariantCaps::Small;
      case CanvasFontVariantCaps::AllSmallCaps:  return boo::FontVariantCaps::AllSmall;
      case CanvasFontVariantCaps::PetiteCaps:    return boo::FontVariantCaps::Petite;
      case CanvasFontVariantCaps::AllPetiteCaps: return boo::FontVariantCaps::AllPetite;
      case CanvasFontVariantCaps::Unicase:       return boo::FontVariantCaps::Unicase;
      case CanvasFontVariantCaps::TitlingCaps:   return boo::FontVariantCaps::Titling;
    }
    krys_debug_assert(false);
    return boo::FontVariantCaps::Normal;
  }

  KRYS_NODISCARD static CanvasFontVariantCaps
    FromFontVariantCaps(boo::FontVariantCaps fontVariantCaps) noexcept
  {
    switch (fontVariantCaps)
    {
      case boo::FontVariantCaps::Normal:    return CanvasFontVariantCaps::Normal;
      case boo::FontVariantCaps::Small:     return CanvasFontVariantCaps::SmallCaps;
      case boo::FontVariantCaps::AllSmall:  return CanvasFontVariantCaps::AllSmallCaps;
      case boo::FontVariantCaps::Petite:    return CanvasFontVariantCaps::PetiteCaps;
      case boo::FontVariantCaps::AllPetite: return CanvasFontVariantCaps::AllPetiteCaps;
      case boo::FontVariantCaps::Unicase:   return CanvasFontVariantCaps::Unicase;
      case boo::FontVariantCaps::Titling:   return CanvasFontVariantCaps::TitlingCaps;
    }
    krys_debug_assert(false);
    return CanvasFontVariantCaps::Normal;
  }

  KRYS_NODISCARD static TextRenderingMode ToTextRenderingMode(CanvasTextRendering textRendering) noexcept
  {
    switch (textRendering)
    {
      case CanvasTextRendering::Auto:               return TextRenderingMode::Auto;
      case CanvasTextRendering::OptimizeSpeed:      return TextRenderingMode::OptimizeSpeed;
      case CanvasTextRendering::OptimizeLegibility: return TextRenderingMode::OptimizeLegibility;
      case CanvasTextRendering::GeometricPrecision: return TextRenderingMode::GeometricPrecision;
    }
    krys_debug_assert(false);
    return TextRenderingMode::Auto;
  }

  void CanvasRenderingContext2DBase::ApplyTextDrawingStyles(FontCascadeDescription &fontDescription,
                                                            bool includeFontLonghands) const noexcept
  {
    const auto &state = GetState();
    fontDescription.SetKerning(ToKerning(state.FontKerning));
    fontDescription.SetTextRenderingMode(ToTextRenderingMode(state.TextRendering));
    // An empty language is explicitly unknown.
    fontDescription.SetSpecifiedLocale(state.Lang == u8"inherit" ? InheritedLanguage()
                                                                 : StringAtom(state.Lang));
    if (includeFontLonghands)
    {
      fontDescription.SetWidth(ToFontWidth(state.FontStretch));
      fontDescription.SetVariantCaps(ToFontVariantCaps(state.FontVariantCaps));
    }
  }

  void CanvasRenderingContext2DBase::TakeFontLonghands(const FontCascadeDescription &fontDescription) noexcept
  {
    auto &state = ModifiableState();
    state.FontStretch = FromFontWidth(fontDescription.Width());
    state.FontVariantCaps = FromFontVariantCaps(fontDescription.VariantCaps());
  }

  StringAtom CanvasRenderingContext2DBase::InheritedLanguage() const noexcept
  {
    return StringAtom::Empty();
  }

  void CanvasRenderingContext2DBase::UpdateFontForTextDrawingStyles() noexcept
  {
    auto &font = ModifiableState().Font;
    if (!font.Realized())
    {
      return;
    }

    auto fontDescription = FontCascadeDescription(font.GetFontDescription());
    ApplyTextDrawingStyles(fontDescription);

    auto &fontSelector = *font.GetFontCascade().GetFontSelector();
    auto fontCascade = FontCascade(krys::move(fontDescription), font.GetFontCascade());
    fontCascade.Update(ShareRefPtr(&fontSelector));
    font.Initialize(fontSelector, fontCascade);
  }

  void CanvasRenderingContext2DBase::SetLang(dom::DOMStringView lang) noexcept
  {
    if (GetState().Lang == lang)
    {
      return;
    }
    // Set the font first: one set later would undo the styles set before it.
    std::ignore = GetFontProxy();
    RealizeSaves();
    ModifiableState().Lang = dom::DOMString(lang);
    UpdateFontForTextDrawingStyles();
  }

  void CanvasRenderingContext2DBase::SetFontKerning(CanvasFontKerning fontKerning) noexcept
  {
    if (GetState().FontKerning == fontKerning)
    {
      return;
    }
    std::ignore = GetFontProxy();
    RealizeSaves();
    ModifiableState().FontKerning = fontKerning;
    UpdateFontForTextDrawingStyles();
  }

  void CanvasRenderingContext2DBase::SetFontStretch(CanvasFontStretch fontStretch) noexcept
  {
    if (GetState().FontStretch == fontStretch)
    {
      return;
    }
    std::ignore = GetFontProxy();
    RealizeSaves();
    ModifiableState().FontStretch = fontStretch;
    UpdateFontForTextDrawingStyles();
  }

  void CanvasRenderingContext2DBase::SetFontVariantCaps(CanvasFontVariantCaps fontVariantCaps) noexcept
  {
    if (GetState().FontVariantCaps == fontVariantCaps)
    {
      return;
    }
    std::ignore = GetFontProxy();
    RealizeSaves();
    ModifiableState().FontVariantCaps = fontVariantCaps;
    UpdateFontForTextDrawingStyles();
  }

  void CanvasRenderingContext2DBase::SetTextRendering(CanvasTextRendering textRendering) noexcept
  {
    if (GetState().TextRendering == textRendering)
    {
      return;
    }
    std::ignore = GetFontProxy();
    RealizeSaves();
    ModifiableState().TextRendering = textRendering;
    UpdateFontForTextDrawingStyles();
  }

  bool CanvasRenderingContext2DBase::CanDrawText(double x, double y, bool fill,
                                                 Maybe<double> maxWidth) noexcept
  {
    if (!GetFontProxy()->Realized())
    {
      return false;
    }

    auto *c = EffectiveDrawingContext();
    if (!c)
    {
      return false;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return false;
    }
    if (!std::isfinite(x) || !std::isfinite(y))
    {
      return false;
    }
    if (maxWidth && (!std::isfinite(*maxWidth) || *maxWidth <= 0))
    {
      return false;
    }

    // If gradient size is zero, nothing would be painted.
    auto *gradient = c->StrokeGradient();
    if (!fill && gradient && gradient->IsZeroSize())
    {
      return false;
    }

    gradient = c->FillGradient();
    if (fill && gradient && gradient->IsZeroSize())
    {
      return false;
    }

    return true;
  }

  KRYS_NODISCARD static constexpr bool IsSpaceThatNeedsReplacing(char8 c) noexcept
  {
    // According to specification all space characters should be replaced with 0x0020 space character.
    // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-canvas-element.html#text-preparation-algorithm
    // The space characters according to specification are : U+0020, U+0009, U+000A, U+000C, and U+000D.
    // http://www.whatwg.org/specs/web-apps/current-work/multipage/common-microsyntaxes.html#space-character
    // This function returns true for 0x000B also, so that this is backward compatible.
    // Otherwise, the test LayoutTests/canvas/philip/tests/2d.text.draw.space.collapse.space.html will fail
    return c == 0x0009 || c == 0x000A || c == 0x000B || c == 0x000C || c == 0x000D;
  }

  dom::DOMString CanvasRenderingContext2DBase::NormalizeSpaces(dom::DOMStringView text) noexcept
  {
    auto normalized = dom::DOMString(text);
    for (auto &c : normalized)
    {
      if (IsSpaceThatNeedsReplacing(c))
      {
        c = u8' ';
      }
    }
    return normalized;
  }

  void CanvasRenderingContext2DBase::DrawTextUnchecked(const TextRun &textRun, double x, double y, bool fill,
                                                       Maybe<double> maxWidth) noexcept
  {
    const auto &measuringFontProxy = *GetFontProxy();

    // FIXME: Need to turn off font smoothing.
    float fontWidth = measuringFontProxy.Width(textRun);
    const auto &fontMetrics = measuringFontProxy.MetricsOfPrimaryFont();

    bool useMaxWidth = maxWidth && *maxWidth < fontWidth;
    float width = useMaxWidth ? static_cast<float>(*maxWidth) : fontWidth;
    auto location = FloatPoint(static_cast<float>(x), static_cast<float>(y));
    location += TextOffset(width, textRun.Direction());

    // The slop built in to this mask rect matches the heuristic used in FontCGWin.cpp for GDI text.
    auto textRect = FloatRect(
      location.X() - static_cast<float>(fontMetrics.IntHeight()) / 2,
      location.Y() - static_cast<float>(fontMetrics.IntAscent() + fontMetrics.IntLineGap()),
      width + static_cast<float>(fontMetrics.IntHeight()), static_cast<float>(fontMetrics.IntLineSpacing()));
    if (!fill)
    {
      textRect = InflatedStrokeRect(textRect);
    }

    auto targetSwitcher = CanvasFilterContextSwitcher::Create(*this, textRect);

    // FIXME: Need to refetch fontProxy. CanvasFilterContextSwitcher might have called save().
    auto *c = EffectiveDrawingContext();
    const auto &fontProxy = *GetFontProxy();

    c->SetTextDrawingMode(fill ? TextDrawingMode::Fill : TextDrawingMode::Stroke);

    GraphicsContextStateSaver stateSaver(*c);
    if (useMaxWidth)
    {
      c->Translate(location);
      // We draw when fontWidth is 0 so compositing operations (eg, a "copy" op) still work.
      c->Scale(FloatSize((fontWidth > 0 ? (width / fontWidth) : 0), 1));
      location = FloatPoint();
    }

    bool repaintEntireCanvas = false;
    if (IsFullCanvasCompositeMode(GetState().GlobalComposite))
    {
      BeginCompositeLayer();
      fontProxy.DrawBidiText(*c, textRun, location,
                             FontCascade::CustomFontNotReadyAction::UseFallbackIfFontNotReady);
      EndCompositeLayer();
      repaintEntireCanvas = true;
    }
    else if (GetState().GlobalComposite == CompositeOperator::Copy)
    {
      ClearCanvas();
      fontProxy.DrawBidiText(*c, textRun, location,
                             FontCascade::CustomFontNotReadyAction::UseFallbackIfFontNotReady);
      repaintEntireCanvas = true;
    }
    else
    {
      fontProxy.DrawBidiText(*c, textRun, location,
                             FontCascade::CustomFontNotReadyAction::UseFallbackIfFontNotReady);
    }

    DidDraw(repaintEntireCanvas, targetSwitcher ? targetSwitcher->ExpandedBounds() : textRect);
  }

  Ref<TextMetrics> CanvasRenderingContext2DBase::MeasureTextInternal(const TextRun &textRun) noexcept
  {
    auto metrics = TextMetrics::Create();

    const auto &font = *GetFontProxy();
    const auto &fontMetrics = font.MetricsOfPrimaryFont();

    GlyphOverflow glyphOverflow;
    glyphOverflow.ComputeBounds = true;
    float fontWidth = font.Width(textRun, &glyphOverflow);
    metrics->SetWidth(fontWidth);

    auto offset = TextOffset(fontWidth, textRun.Direction());
    auto ascent = fontMetrics.Ascent();
    auto descent = fontMetrics.Descent();

    metrics->SetActualBoundingBoxAscent(glyphOverflow.Top.ToFloat() - offset.Y());
    metrics->SetActualBoundingBoxDescent(glyphOverflow.Bottom.ToFloat() + offset.Y());
    metrics->SetFontBoundingBoxAscent(static_cast<float>(fontMetrics.IntAscent()) - offset.Y());
    metrics->SetFontBoundingBoxDescent(static_cast<float>(fontMetrics.IntDescent()) + offset.Y());
    metrics->SetEmHeightAscent(ascent - offset.Y());
    metrics->SetEmHeightDescent(descent + offset.Y());
    metrics->SetHangingBaseline(ascent - offset.Y());
    metrics->SetAlphabeticBaseline(-offset.Y());
    metrics->SetIdeographicBaseline(-descent - offset.Y());

    metrics->SetActualBoundingBoxLeft(glyphOverflow.Left.ToFloat() - offset.X());
    metrics->SetActualBoundingBoxRight(fontWidth + glyphOverflow.Right.ToFloat() + offset.X());

    return metrics;
  }

  FloatPoint CanvasRenderingContext2DBase::TextOffset(float width, TextDirection direction) noexcept
  {
    const auto &fontMetrics = GetFontProxy()->MetricsOfPrimaryFont();
    FloatPoint offset;

    switch (GetState().TextBaseline)
    {
      case TopTextBaseline:
      case HangingTextBaseline:     offset.SetY(static_cast<float>(fontMetrics.IntAscent())); break;
      case BottomTextBaseline:
      case IdeographicTextBaseline: offset.SetY(static_cast<float>(-fontMetrics.IntDescent())); break;
      case MiddleTextBaseline:
        offset.SetY(static_cast<float>(fontMetrics.IntHeight() / 2 - fontMetrics.IntDescent()));
        break;
      case AlphabeticTextBaseline:
      default:                     break;
    }

    bool isRTL = direction == TextDirection::RTL;
    auto align = GetState().TextAlign;
    if (align == StartTextAlign)
    {
      align = isRTL ? RightTextAlign : LeftTextAlign;
    }
    else if (align == EndTextAlign)
    {
      align = isRTL ? LeftTextAlign : RightTextAlign;
    }

    switch (align)
    {
      case CenterTextAlign: offset.SetX(-width / 2); break;
      case RightTextAlign:  offset.SetX(-width); break;
      default:              break;
    }
    return offset;
  }
}
