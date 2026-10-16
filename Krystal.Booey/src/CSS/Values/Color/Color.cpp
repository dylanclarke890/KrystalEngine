#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Color/AbsoluteColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/LightDarkColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/RelativeColor.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  Color::Color(ColorKind &&kind) noexcept : _value {krys::move(kind)}
  {
  }

  Color::Color(EmptyToken token) noexcept : _value {token}
  {
  }

  Color::Color(ResolvedColor &&color) noexcept : _value {krys::move(color)}
  {
  }

  Color::Color(KeywordColor &&color) noexcept : _value {krys::move(color)}
  {
  }

  Color::Color(HexColor &&color) noexcept : _value {krys::move(color)}
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

  Color::Color(LightDarkColor &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<RGBFunctionLegacy<Number<>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<RGBFunctionLegacy<Percentage<>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<RGBFunctionModernAbsolute> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<HSLFunctionLegacy> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<HSLFunctionModern> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<HWBFunction> &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<LabFunction> &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<LCHFunction> &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<OKLabFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<OKLCHFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorRGBFunction<ExtendedA98RGB<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorRGBFunction<ExtendedDisplayP3<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorRGBFunction<ExtendedLinearDisplayP3<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorRGBFunction<ExtendedProPhotoRGB<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorRGBFunction<ExtendedRec2020<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorRGBFunction<ExtendedSRGBA<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorRGBFunction<ExtendedLinearSRGBA<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorXYZFunction<XYZA<float, WhitePoint::D50>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(AbsoluteColor<ColorXYZFunction<XYZA<float, WhitePoint::D65>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<RGBFunctionModernRelative> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<HSLFunctionModern> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<HWBFunction> &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<LabFunction> &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<LCHFunction> &&color) noexcept : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<OKLabFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<OKLCHFunction> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorRGBFunction<ExtendedA98RGB<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorRGBFunction<ExtendedDisplayP3<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorRGBFunction<ExtendedLinearDisplayP3<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorRGBFunction<ExtendedProPhotoRGB<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorRGBFunction<ExtendedRec2020<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorRGBFunction<ExtendedSRGBA<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorRGBFunction<ExtendedLinearSRGBA<float>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorXYZFunction<XYZA<float, WhitePoint::D50>>> &&color) noexcept
      : _value {CreateIndirectColor(krys::move(color))}
  {
  }

  Color::Color(RelativeColor<ColorXYZFunction<XYZA<float, WhitePoint::D65>>> &&color) noexcept
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

  bool Color::IsResolved() const noexcept
  {
    return std::holds_alternative<ResolvedColor>(_value);
  }

  Maybe<ResolvedColor> Color::Resolved() const noexcept
  {
    if (IsResolved())
    {
      return std::get<ResolvedColor>(_value);
    }
    return null;
  }

  bool Color::IsKeyword() const noexcept
  {
    return std::holds_alternative<KeywordColor>(_value);
  }

  Maybe<KeywordColor> Color::Keyword() const noexcept
  {
    if (IsKeyword())
    {
      return std::get<KeywordColor>(_value);
    }
    return null;
  }

  bool Color::IsHex() const noexcept
  {
    return std::holds_alternative<HexColor>(_value);
  }

  Maybe<HexColor> Color::Hex() const noexcept
  {
    if (IsHex())
    {
      return std::get<HexColor>(_value);
    }
    return null;
  }

  boo::Color Color::GetAbsoluteColor() const noexcept
  {
    return SwitchOn([](const ResolvedColor &resolved) -> boo::Color { return resolved.Value; },
                    [](const KeywordColor &keyword) -> boo::Color
                    {
                      if (IsAbsoluteColorKeyword(keyword.Value))
                      {
                        return ColorFromAbsoluteKeyword(keyword.Value);
                      }
                      return {};
                    },
                    [](const HexColor &hex) -> boo::Color { return boo::Color {hex.Value}; },
                    [](const auto &) -> boo::Color { return {}; });
  }

  Color::ColorKind Color::Copy(const ColorKind &other) noexcept
  {
    return krys::SwitchOn(
      other, []<typename T>(const T &color) -> ColorKind { return color; },
      []<typename T>(const UniquePtr<T> &color) -> ColorKind { return CreateUnique<T>(*color); });
  }

  template <typename T>
  Color::ColorKind Color::CreateIndirectColor(T &&color) noexcept
  {
    return {CreateUnique<decay_t<T>>(krys::move(color))};
  }

  boo::Color CreateColor(const Color &value, PlatformColorResolutionState &state) noexcept
  {
    return krys::SwitchOn(value, [&](const auto &color) { return css::CreateColor(color, state); });
  }

  bool ContainsCurrentColor(const Color &value) noexcept
  {
    return krys::SwitchOn(value, [&](const auto &color) { return css::ContainsCurrentColor(color); });
  }

  bool ContainsColorSchemeDependentColor(const Color &value) noexcept
  {
    return krys::SwitchOn(value,
                          [&](const auto &color) { return css::ContainsColorSchemeDependentColor(color); });
  }

  void Serialiser<Color>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                     const Color &value) noexcept
  {
    krys::SwitchOn(value, [&](const auto &color) { Serialise(builder, context, color); });
  }

  void ComputedStyleDependenciesCollector<Color>::operator()(ComputedStyleDependencies &dependencies,
                                                             const Color &value) noexcept
  {
    krys::SwitchOn(value, [&](const auto &color) { CollectComputedStyleDependencies(dependencies, color); });
  }

  IterationStatus
    ValueChildrenVisitor<Color>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                            const Color &value) noexcept
  {
    return krys::SwitchOn(value, [&](const auto &color) { return VisitValueChildren(func, color); });
  }
}
