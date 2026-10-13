#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/HexColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  // The following color kinds are forward declared and stored in UniquePtrs to
  // avoid unnecessarily growing the size of Color for the uncommon cases.
  struct ColorLayers;
  struct ColorMix;
  struct ContrastColor;
  struct LightDarkColor;

  template <typename Descriptor>
  struct AbsoluteColor;

  template <typename Descriptor>
  struct RelativeColor;

  /// @brief A specified CSS `<color>`.
  class Color
  {
    friend struct krys::MarkableTraits<Color>;

    struct EmptyToken
    {
      KRYS_NODISCARD constexpr bool operator==(const EmptyToken &) const noexcept = default;
    };

    // FIXME: Replace Variant with a generic CompactPointerVariant type.
    using ColorKind = Variant<
      // EmptyToken is a special value used by Markable to represent empty state.
      EmptyToken, ResolvedColor, KeywordColor, HexColor, UniquePtr<ColorLayers>, UniquePtr<ColorMix>,
      UniquePtr<ContrastColor>, UniquePtr<LightDarkColor>,
      UniquePtr<AbsoluteColor<RGBFunctionLegacy<Number<>>>>,
      UniquePtr<AbsoluteColor<RGBFunctionLegacy<Percentage<>>>>,
      UniquePtr<AbsoluteColor<RGBFunctionModernAbsolute>>, UniquePtr<AbsoluteColor<HSLFunctionLegacy>>,
      UniquePtr<AbsoluteColor<HSLFunctionModern>>, UniquePtr<AbsoluteColor<HWBFunction>>,
      UniquePtr<AbsoluteColor<LabFunction>>, UniquePtr<AbsoluteColor<LCHFunction>>,
      UniquePtr<AbsoluteColor<OKLabFunction>>, UniquePtr<AbsoluteColor<OKLCHFunction>>,
      UniquePtr<AbsoluteColor<ColorRGBFunction<ExtendedA98RGB<float>>>>,
      UniquePtr<AbsoluteColor<ColorRGBFunction<ExtendedDisplayP3<float>>>>,
      UniquePtr<AbsoluteColor<ColorRGBFunction<ExtendedLinearDisplayP3<float>>>>,
      UniquePtr<AbsoluteColor<ColorRGBFunction<ExtendedProPhotoRGB<float>>>>,
      UniquePtr<AbsoluteColor<ColorRGBFunction<ExtendedRec2020<float>>>>,
      UniquePtr<AbsoluteColor<ColorRGBFunction<ExtendedSRGBA<float>>>>,
      UniquePtr<AbsoluteColor<ColorRGBFunction<ExtendedLinearSRGBA<float>>>>,
      UniquePtr<AbsoluteColor<ColorXYZFunction<XYZA<float, WhitePoint::D50>>>>,
      UniquePtr<AbsoluteColor<ColorXYZFunction<XYZA<float, WhitePoint::D65>>>>,
      UniquePtr<RelativeColor<RGBFunctionModernRelative>>, UniquePtr<RelativeColor<HSLFunctionModern>>,
      UniquePtr<RelativeColor<HWBFunction>>, UniquePtr<RelativeColor<LabFunction>>,
      UniquePtr<RelativeColor<LCHFunction>>, UniquePtr<RelativeColor<OKLabFunction>>,
      UniquePtr<RelativeColor<OKLCHFunction>>,
      UniquePtr<RelativeColor<ColorRGBFunction<ExtendedA98RGB<float>>>>,
      UniquePtr<RelativeColor<ColorRGBFunction<ExtendedDisplayP3<float>>>>,
      UniquePtr<RelativeColor<ColorRGBFunction<ExtendedLinearDisplayP3<float>>>>,
      UniquePtr<RelativeColor<ColorRGBFunction<ExtendedProPhotoRGB<float>>>>,
      UniquePtr<RelativeColor<ColorRGBFunction<ExtendedRec2020<float>>>>,
      UniquePtr<RelativeColor<ColorRGBFunction<ExtendedSRGBA<float>>>>,
      UniquePtr<RelativeColor<ColorRGBFunction<ExtendedLinearSRGBA<float>>>>,
      UniquePtr<RelativeColor<ColorXYZFunction<XYZA<float, WhitePoint::D50>>>>,
      UniquePtr<RelativeColor<ColorXYZFunction<XYZA<float, WhitePoint::D65>>>>>;

  private:
    // The indirect alternatives are never null.
    ColorKind _value;

    explicit Color(EmptyToken) noexcept;
    explicit Color(ColorKind &&kind) noexcept;

  public:
    explicit Color(ResolvedColor &&color) noexcept;
    explicit Color(KeywordColor &&color) noexcept;
    explicit Color(HexColor &&color) noexcept;
    explicit Color(ColorLayers &&color) noexcept;
    explicit Color(ColorMix &&color) noexcept;
    explicit Color(ContrastColor &&color) noexcept;
    explicit Color(LightDarkColor &&color) noexcept;
    explicit Color(AbsoluteColor<RGBFunctionLegacy<Number<>>> &&color) noexcept;
    explicit Color(AbsoluteColor<RGBFunctionLegacy<Percentage<>>> &&color) noexcept;
    explicit Color(AbsoluteColor<RGBFunctionModernAbsolute> &&color) noexcept;
    explicit Color(AbsoluteColor<HSLFunctionLegacy> &&color) noexcept;
    explicit Color(AbsoluteColor<HSLFunctionModern> &&color) noexcept;
    explicit Color(AbsoluteColor<HWBFunction> &&color) noexcept;
    explicit Color(AbsoluteColor<LabFunction> &&color) noexcept;
    explicit Color(AbsoluteColor<LCHFunction> &&color) noexcept;
    explicit Color(AbsoluteColor<OKLabFunction> &&color) noexcept;
    explicit Color(AbsoluteColor<OKLCHFunction> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorRGBFunction<ExtendedA98RGB<float>>> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorRGBFunction<ExtendedDisplayP3<float>>> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorRGBFunction<ExtendedLinearDisplayP3<float>>> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorRGBFunction<ExtendedProPhotoRGB<float>>> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorRGBFunction<ExtendedRec2020<float>>> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorRGBFunction<ExtendedSRGBA<float>>> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorRGBFunction<ExtendedLinearSRGBA<float>>> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorXYZFunction<XYZA<float, WhitePoint::D50>>> &&color) noexcept;
    explicit Color(AbsoluteColor<ColorXYZFunction<XYZA<float, WhitePoint::D65>>> &&color) noexcept;
    explicit Color(RelativeColor<RGBFunctionModernRelative> &&color) noexcept;
    explicit Color(RelativeColor<HSLFunctionModern> &&color) noexcept;
    explicit Color(RelativeColor<HWBFunction> &&color) noexcept;
    explicit Color(RelativeColor<LabFunction> &&color) noexcept;
    explicit Color(RelativeColor<LCHFunction> &&color) noexcept;
    explicit Color(RelativeColor<OKLabFunction> &&color) noexcept;
    explicit Color(RelativeColor<OKLCHFunction> &&color) noexcept;
    explicit Color(RelativeColor<ColorRGBFunction<ExtendedA98RGB<float>>> &&color) noexcept;
    explicit Color(RelativeColor<ColorRGBFunction<ExtendedDisplayP3<float>>> &&color) noexcept;
    explicit Color(RelativeColor<ColorRGBFunction<ExtendedLinearDisplayP3<float>>> &&color) noexcept;
    explicit Color(RelativeColor<ColorRGBFunction<ExtendedProPhotoRGB<float>>> &&color) noexcept;
    explicit Color(RelativeColor<ColorRGBFunction<ExtendedRec2020<float>>> &&color) noexcept;
    explicit Color(RelativeColor<ColorRGBFunction<ExtendedSRGBA<float>>> &&color) noexcept;
    explicit Color(RelativeColor<ColorRGBFunction<ExtendedLinearSRGBA<float>>> &&color) noexcept;
    explicit Color(RelativeColor<ColorXYZFunction<XYZA<float, WhitePoint::D50>>> &&color) noexcept;
    explicit Color(RelativeColor<ColorXYZFunction<XYZA<float, WhitePoint::D65>>> &&color) noexcept;

    Color(const Color &other) noexcept;
    Color &operator=(const Color &other) noexcept;
    Color(Color &&) noexcept;
    Color &operator=(Color &&) noexcept;

    ~Color() noexcept;

    KRYS_NODISCARD bool operator==(const Color &other) const noexcept;

    KRYS_NODISCARD bool IsResolved() const noexcept;
    KRYS_NODISCARD Maybe<ResolvedColor> Resolved() const noexcept;
    KRYS_NODISCARD bool IsKeyword() const noexcept;
    KRYS_NODISCARD Maybe<KeywordColor> Keyword() const noexcept;
    KRYS_NODISCARD bool IsHex() const noexcept;
    KRYS_NODISCARD Maybe<HexColor> Hex() const noexcept;

    /// @brief Return an absolute color if possible, otherwise an invalid color.
    /// @see https://drafts.csswg.org/css-color-5/#absolute-color
    KRYS_NODISCARD boo::Color GetAbsoluteColor() const noexcept;

    /// @brief This helper allows us to treat all the alternatives in ColorKind
    /// as const references, pretending the UniquePtrs don't exist.
    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const;

  private:
    template <typename T>
    KRYS_NODISCARD static ColorKind CreateIndirectColor(T &&color) noexcept;
    KRYS_NODISCARD static ColorKind Copy(const ColorKind &other) noexcept;
  };

  KRYS_NODISCARD boo::Color CreateColor(const Color &value, PlatformColorResolutionState &state) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const Color &value) noexcept;
  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const Color &value) noexcept;

  template <>
  struct Serialiser<Color>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context, const Color &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<Color>
  {
    void operator()(ComputedStyleDependencies &dependencies, const Color &value) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<Color>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const Color &value) noexcept;
  };

  template <typename... F>
  decltype(auto) Color::SwitchOn(F &&...f) const
  {
    auto visitor = CreateVisitor(std::forward<F>(f)...);
    using ResultType = decltype(visitor(std::declval<KeywordColor>()));

    return krys::SwitchOn(
      _value, [&](const EmptyToken &) -> ResultType { krys_unreachable(); },
      [&]<typename T>(const T &color) -> ResultType { return visitor(color); },
      [&]<typename T>(const UniquePtr<T> &color) -> ResultType { return visitor(*color); });
  }
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::css::Color>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const boo::css::Color &color) noexcept
    {
      return std::holds_alternative<boo::css::Color::EmptyToken>(color._value);
    }

    KRYS_NODISCARD static boo::css::Color EmptyValue() noexcept
    {
      return boo::css::Color(boo::css::Color::EmptyToken());
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::Color)
