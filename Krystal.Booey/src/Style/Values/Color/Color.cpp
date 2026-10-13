#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Color/AbsoluteColor.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorMix.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Booey/Style/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/Style/Values/Color/HexColor.hpp"
#include "Krystal.Booey/Style/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/Style/Values/Color/LightDarkColor.hpp"
#include "Krystal.Booey/Style/Values/Color/RelativeColor.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::style
{
  Color::Color(ColorKind &&kind) noexcept : _value {krys::move(kind)}
  {
  }

  Color::Color(EmptyToken token) noexcept : _value {token}
  {
  }

  Color::Color() noexcept : _value {CurrentColor {}}
  {
  }

  Color::Color(css::keywords::Currentcolor) noexcept : _value {CurrentColor {}}
  {
  }

  Color::Color(boo::Color color) noexcept : _value {ResolvedColor {krys::move(color)}}
  {
  }

  Color::Color(SRGBA<uint8> color) noexcept : _value {ResolvedColor {boo::Color {color}}}
  {
  }

  Color::Color(css::keywords::Transparent) noexcept : _value {ResolvedColor {boo::Color::transparentBlack}}
  {
  }

  Color::Color(css::keywords::Black) noexcept : _value {ResolvedColor {boo::Color::black}}
  {
  }

  Color::Color(css::keywords::White) noexcept : _value {ResolvedColor {boo::Color::white}}
  {
  }

  Color::Color(ResolvedColor &&color) noexcept : _value {krys::move(color)}
  {
  }

  Color::Color(CurrentColor &&color) noexcept : _value {krys::move(color)}
  {
  }

  Color::Color(ColorLayers &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(ColorMix &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(ContrastColor &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::RGBFunctionModernRelative> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::HSLFunctionModern> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::HWBFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::LabFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::LCHFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::OKLabFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::OKLCHFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorRGBFunction<ExtendedA98RGB<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorRGBFunction<ExtendedDisplayP3<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorRGBFunction<ExtendedLinearDisplayP3<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorRGBFunction<ExtendedProPhotoRGB<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorRGBFunction<ExtendedRec2020<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorRGBFunction<ExtendedSRGBA<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorRGBFunction<ExtendedLinearSRGBA<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorXYZFunction<XYZA<float, WhitePoint::D50>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<css::ColorXYZFunction<XYZA<float, WhitePoint::D65>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(const Color &other) noexcept : _value {Copy(other._value)}
  {
  }

  Color &Color::operator=(const Color &other) noexcept
  {
    _value = Copy(other._value);
    return *this;
  }

  Color::Color(Color &&) noexcept = default;
  Color &Color::operator=(Color &&) noexcept = default;
  Color::~Color() noexcept = default;

  bool Color::operator==(const Color &other) const noexcept
  {
    if (_value.index() != other._value.index())
    {
      return false;
    }

    return krys::SwitchOn(
      _value, [&](const EmptyToken &) { return true; },
      [&]<typename T>(const T &color) { return color == std::get<T>(other._value); },
      [&]<typename T>(const UniquePtr<T> &color) { return *color == *std::get<UniquePtr<T>>(other._value); });
  }

  const Color &Color::GetCurrentColor() noexcept
  {
    static NeverDestroyed<Color> color {CurrentColor {}};
    return color.get();
  }

  Color::ColorKind Color::Copy(const ColorKind &other) noexcept
  {
    return krys::SwitchOn(
      other, []<typename T>(const T &color) -> ColorKind { return color; },
      []<typename T>(const UniquePtr<T> &color) -> ColorKind { return CreateUnique<T>(*color); });
  }

  boo::Color Color::ResolveColor(const boo::Color &currentColor) const noexcept
  {
    return SwitchOn([&](const auto &kind) { return style::ResolveColor(kind, currentColor); });
  }

  bool Color::ContainsCurrentColor() const noexcept
  {
    return SwitchOn([](const auto &kind) { return style::ContainsCurrentColor(kind); });
  }

  bool Color::IsCurrentColor() const noexcept
  {
    return std::holds_alternative<CurrentColor>(_value);
  }

  bool Color::IsColorMix() const noexcept
  {
    return std::holds_alternative<UniquePtr<ColorMix>>(_value);
  }

  bool Color::IsContrastColor() const noexcept
  {
    return std::holds_alternative<UniquePtr<ContrastColor>>(_value);
  }

  bool Color::IsRelativeColor() const noexcept
  {
    return krys::SwitchOn(
      _value, []<typename D>(const UniquePtr<RelativeColor<D>> &) { return true; },
      [](const auto &) { return false; });
  }

  bool Color::IsResolvedColor() const noexcept
  {
    return std::holds_alternative<ResolvedColor>(_value);
  }

  const boo::Color &Color::GetResolvedColor() const noexcept
  {
    krys_debug_assert(IsResolvedColor());
    return std::get<ResolvedColor>(_value).Value;
  }

  template <typename T>
  Color::ColorKind Color::CreateIndirectColor(T &&color) noexcept
  {
    return {CreateUnique<decay_t<T>>(krys::move(color))};
  }

  boo::Color ResolveColor(const Color &value, const boo::Color &currentColor) noexcept
  {
    return value.ResolveColor(currentColor);
  }

  bool ContainsCurrentColor(const Color &value) noexcept
  {
    return value.ContainsCurrentColor();
  }

  // MARK: - Serialisation

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                   const Color &value) noexcept
  {
    return krys::SwitchOn(value, [&](const auto &kind)
                          { return style::SerialisationForCSSTokenization(context, kind); });
  }

  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &context,
                                       const Color &value) noexcept
  {
    krys::SwitchOn(value,
                   [&](const auto &kind) { style::SerialisationForCSSTokenization(builder, context, kind); });
  }

  void Serialiser<Color>::operator()(css::CSSOMString &builder, const css::SerialisationContext &,
                                     const RenderStyle &style, const Color &value) noexcept
  {
    // NOTE: The specialization of style::Serialiser is used for computed value serialization, so the resolved
    // "used" value is used.
    builder.append(boo::SerialisationForCSS(style.ColorResolvingCurrentColor(value)));
  }

  // MARK: - Conversion

  Color ToStyleColor(const css::Color &value, ColorResolutionState &state) noexcept
  {
    return krys::SwitchOn(value, [&](const auto &color) { return ToStyleColor(color, state); });
  }

  Color ToStyleColor(const css::Color &value, Ref<const dom::Document> document, const RenderStyle &style,
                     const css::ToLengthConversionData &conversionData,
                     style::ForVisitedLink forVisitedLink) noexcept
  {
    auto resolutionState = ColorResolutionState {
      .Document = krys::move(document),
      .Style = style,
      .ConversionData = conversionData,
      .ForVisitedLink = forVisitedLink,
    };
    return ToStyleColor(value, resolutionState);
  }

  Color ToStyleColor(const css::Color &value, const BuilderState &builderState,
                     style::ForVisitedLink forVisitedLink) noexcept
  {
    return ToStyleColor(value, Ref<const dom::Document>::WithRef(builderState.GetDocument()),
                        builderState.Style(), builderState.CssToLengthConversionData(), forVisitedLink);
  }

  css::Color ToCSS<Color>::operator()(const Color &value, const RenderStyle &style) noexcept
  {
    return css::Color {css::ResolvedColor {style.ColorResolvingCurrentColor(value)}};
  }

  Color ToStyle<css::Color>::operator()(const css::Color &value, const BuilderState &builderState,
                                        style::ForVisitedLink forVisitedLink) noexcept
  {
    return ToStyleColor(value, builderState, forVisitedLink);
  }

  Color ToStyle<css::Color>::operator()(const css::Color &value, const BuilderState &builderState) noexcept
  {
    return ConvertToStyle(value, builderState, style::ForVisitedLink::No);
  }

  // MARK: - Blending

  bool Blending<Color>::Equals(const Color &a, const Color &b, const RenderStyle &aStyle,
                               const RenderStyle &bStyle) noexcept
  {
    if (a.IsCurrentColor() && b.IsCurrentColor())
    {
      return true;
    }
    if (a.IsResolvedColor() && b.IsResolvedColor())
    {
      return a.GetResolvedColor() == b.GetResolvedColor();
    }
    return aStyle.ColorResolvingCurrentColor(a) == bStyle.ColorResolvingCurrentColor(b);
  }

  bool Blending<Color>::CanBlend(const Color &a, const Color &b) noexcept
  {
    // We don't animate on currentcolor-only transition.
    // https://github.com/WebKit/WebKit/blob/main/LayoutTests/imported/w3c/web-platform-tests/css/css-transitions/currentcolor-animation-001.html#L27
    return !(a.IsCurrentColor() && b.IsCurrentColor());
  }

  Color Blending<Color>::Blend(const Color &a, const Color &b, const RenderStyle &aStyle,
                               const RenderStyle &bStyle, const BlendingContext &context) noexcept
  {
    return boo::Blend(aStyle.ColorResolvingCurrentColor(a), bStyle.ColorResolvingCurrentColor(b), context);
  }
}
