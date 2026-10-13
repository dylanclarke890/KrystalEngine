#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorType.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Booey/Style/Values/Color/CurrentColor.hpp"
#include "Krystal.Booey/Style/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  enum class ForVisitedLink : bool;

  // The following style color kinds are forward declared and stored in
  // UniquePtrs to avoid unnecessarily growing the size of Color for the
  // uncommon case of un-resolvability due to currentColor.
  struct ColorLayers;
  struct ColorMix;
  struct ContrastColor;

  template <typename Descriptor>
  struct RelativeColor;

  /// @brief A computed `<color>`: a resolved colour, or one that depends on `currentcolor` and is resolved at
  /// use time.
  struct Color
  {
  private:
    friend struct krys::MarkableTraits<Color>;

    struct EmptyToken
    {
      KRYS_NODISCARD constexpr bool operator==(const EmptyToken &) const noexcept = default;
    };

    // FIXME: Replace Variant with a generic CompactPointerVariant type.
    using ColorKind =
      Variant<EmptyToken, ResolvedColor, CurrentColor, UniquePtr<ColorLayers>, UniquePtr<ColorMix>,
              UniquePtr<ContrastColor>, UniquePtr<RelativeColor<css::RGBFunctionModernRelative>>,
              UniquePtr<RelativeColor<css::HSLFunctionModern>>, UniquePtr<RelativeColor<css::HWBFunction>>,
              UniquePtr<RelativeColor<css::LabFunction>>, UniquePtr<RelativeColor<css::LCHFunction>>,
              UniquePtr<RelativeColor<css::OKLabFunction>>, UniquePtr<RelativeColor<css::OKLCHFunction>>,
              UniquePtr<RelativeColor<css::ColorRGBFunction<ExtendedA98RGB<float>>>>,
              UniquePtr<RelativeColor<css::ColorRGBFunction<ExtendedDisplayP3<float>>>>,
              UniquePtr<RelativeColor<css::ColorRGBFunction<ExtendedLinearDisplayP3<float>>>>,
              UniquePtr<RelativeColor<css::ColorRGBFunction<ExtendedProPhotoRGB<float>>>>,
              UniquePtr<RelativeColor<css::ColorRGBFunction<ExtendedRec2020<float>>>>,
              UniquePtr<RelativeColor<css::ColorRGBFunction<ExtendedSRGBA<float>>>>,
              UniquePtr<RelativeColor<css::ColorRGBFunction<ExtendedLinearSRGBA<float>>>>,
              UniquePtr<RelativeColor<css::ColorXYZFunction<XYZA<float, WhitePoint::D50>>>>,
              UniquePtr<RelativeColor<css::ColorXYZFunction<XYZA<float, WhitePoint::D65>>>>>;

    // The indirect alternatives are never null, except after a move.
    ColorKind _value;

    explicit Color(EmptyToken) noexcept;
    explicit Color(ColorKind &&kind) noexcept;

  public:
    // The default constructor initializes to style::CurrentColor to preserve old behavior,
    // we might want to remove it entirely at some point.
    Color() noexcept;
    Color(css::keywords::Currentcolor) noexcept;

    // Convenience constructors that create style::ResolvedColor.
    Color(boo::Color color) noexcept;
    Color(SRGBA<uint8> color) noexcept;
    Color(css::keywords::Transparent) noexcept;
    Color(css::keywords::Black) noexcept;
    Color(css::keywords::White) noexcept;

    Color(ResolvedColor &&color) noexcept;
    Color(CurrentColor &&color) noexcept;
    explicit Color(ColorLayers &&color) noexcept;
    explicit Color(ColorMix &&color) noexcept;
    explicit Color(ContrastColor &&color) noexcept;
    explicit Color(RelativeColor<css::RGBFunctionModernRelative> &&color) noexcept;
    explicit Color(RelativeColor<css::HSLFunctionModern> &&color) noexcept;
    explicit Color(RelativeColor<css::HWBFunction> &&color) noexcept;
    explicit Color(RelativeColor<css::LabFunction> &&color) noexcept;
    explicit Color(RelativeColor<css::LCHFunction> &&color) noexcept;
    explicit Color(RelativeColor<css::OKLabFunction> &&color) noexcept;
    explicit Color(RelativeColor<css::OKLCHFunction> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorRGBFunction<ExtendedA98RGB<float>>> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorRGBFunction<ExtendedDisplayP3<float>>> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorRGBFunction<ExtendedLinearDisplayP3<float>>> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorRGBFunction<ExtendedProPhotoRGB<float>>> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorRGBFunction<ExtendedRec2020<float>>> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorRGBFunction<ExtendedSRGBA<float>>> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorRGBFunction<ExtendedLinearSRGBA<float>>> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorXYZFunction<XYZA<float, WhitePoint::D50>>> &&color) noexcept;
    explicit Color(RelativeColor<css::ColorXYZFunction<XYZA<float, WhitePoint::D65>>> &&color) noexcept;

    Color(const Color &other) noexcept;
    Color &operator=(const Color &other) noexcept;
    Color(Color &&) noexcept;
    Color &operator=(Color &&) noexcept;

    ~Color() noexcept;

    KRYS_NODISCARD bool operator==(const Color &other) const noexcept;

    KRYS_NODISCARD static const Color &GetCurrentColor() noexcept;

    KRYS_NODISCARD bool ContainsCurrentColor() const noexcept;
    KRYS_NODISCARD bool IsCurrentColor() const noexcept;
    KRYS_NODISCARD bool IsColorMix() const noexcept;
    KRYS_NODISCARD bool IsContrastColor() const noexcept;
    KRYS_NODISCARD bool IsRelativeColor() const noexcept;
    KRYS_NODISCARD bool IsResolvedColor() const noexcept;

    /// @attention The colour must be a resolved colour (`IsResolvedColor()`).
    KRYS_NODISCARD const boo::Color &GetResolvedColor() const noexcept;

    KRYS_NODISCARD boo::Color ResolveColor(const boo::Color &currentColor) const noexcept;

    /// @brief This helper allows us to treat all the alternatives in ColorKind
    /// as const references, pretending the UniquePtrs don't exist.
    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const;

  private:
    template <typename T>
    KRYS_NODISCARD static ColorKind CreateIndirectColor(T &&color) noexcept;
    KRYS_NODISCARD static ColorKind Copy(const ColorKind &other) noexcept;
  };

  KRYS_NODISCARD boo::Color ResolveColor(const Color &value, const boo::Color &currentColor) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const Color &value) noexcept;

  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &context,
                                       const Color &value) noexcept;
  KRYS_NODISCARD css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                                  const Color &value) noexcept;

  template <>
  struct Serialiser<Color>
  {
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const Color &value) noexcept;
  };

  // MARK: - Conversion

  struct ColorResolutionState;

  KRYS_NODISCARD Color ToStyleColor(const css::Color &value, ColorResolutionState &state) noexcept;
  KRYS_NODISCARD Color ToStyleColor(const css::Color &value, Ref<const dom::Document> document,
                                    const RenderStyle &style,
                                    const css::ToLengthConversionData &conversionData,
                                    style::ForVisitedLink forVisitedLink) noexcept;
  KRYS_NODISCARD Color ToStyleColor(const css::Color &value, const BuilderState &builderState,
                                    style::ForVisitedLink forVisitedLink) noexcept;

  template <>
  struct ToCSS<Color>
  {
    KRYS_NODISCARD css::Color operator()(const Color &value, const RenderStyle &style) noexcept;
  };

  template <>
  struct ToStyle<css::Color>
  {
    KRYS_NODISCARD Color operator()(const css::Color &value, const BuilderState &builderState,
                                    style::ForVisitedLink forVisitedLink) noexcept;
    KRYS_NODISCARD Color operator()(const css::Color &value, const BuilderState &builderState) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<Color>
  {
    KRYS_NODISCARD bool Equals(const Color &a, const Color &b, const RenderStyle &aStyle,
                               const RenderStyle &bStyle) noexcept;
    KRYS_NODISCARD bool CanBlend(const Color &a, const Color &b) noexcept;

    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const Color &,
                                                                                const Color &) noexcept
    {
      return true;
    }

    KRYS_NODISCARD Color Blend(const Color &a, const Color &b, const RenderStyle &aStyle,
                               const RenderStyle &bStyle, const BlendingContext &context) noexcept;
  };

  // MARK: - Color Implementation

  template <typename... F>
  decltype(auto) Color::SwitchOn(F &&...f) const
  {
    auto visitor = CreateVisitor(std::forward<F>(f)...);
    using ResultType = decltype(visitor(std::declval<ResolvedColor>()));

    return krys::SwitchOn(
      _value, [&](const EmptyToken &) -> ResultType { krys_unreachable(); },
      [&](const ResolvedColor &resolvedColor) -> ResultType { return visitor(resolvedColor); },
      [&](const CurrentColor &currentColor) -> ResultType { return visitor(currentColor); },
      [&]<typename T>(const UniquePtr<T> &color) -> ResultType { return visitor(*color); });
  }
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::style::Color>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const boo::style::Color &color) noexcept
    {
      return std::holds_alternative<boo::style::Color::EmptyToken>(color._value);
    }

    KRYS_NODISCARD static boo::style::Color EmptyValue() noexcept
    {
      return boo::style::Color(boo::style::Color::EmptyToken());
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Color)
