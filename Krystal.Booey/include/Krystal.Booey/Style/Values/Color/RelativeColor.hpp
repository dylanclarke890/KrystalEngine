#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/RelativeColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/RelativeColorResolver.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Booey/Style/Values/Color/ResolvedColor.hpp"

namespace krys::boo::style
{
  template <typename D, size_t Index>
  using RelativeColorComponent = css::GetCSSColorParseTypeWithCalcAndSymbolsComponentResult<D, Index>;

  /// @brief A relative colour whose origin couldn't be resolved during style building because it depends on
  /// `currentcolor`.
  template <typename D>
  struct RelativeColor
  {
    using Descriptor = D;

    style::Color Origin;
    css::CSSColorParseTypeWithCalcAndSymbols<Descriptor> Components;

    KRYS_NODISCARD bool operator==(const RelativeColor<Descriptor> &) const noexcept = default;
  };

  template <typename D>
  KRYS_NODISCARD style::Color ToStyleColor(const css::RelativeColor<D> &unresolved,
                                           ColorResolutionState &state) noexcept
  {
    ColorResolutionStateNester nester {state};

    auto origin = ToStyleColor(unresolved.Origin, state);
    if (!origin.IsResolvedColor())
    {
      // If the origin is not absolute, we cannot fully resolve the color yet.
      // Instead, we simplify the calc values using the conversion data, and
      // return a style::RelativeColor to be resolved at use time.
      return style::Color {RelativeColor<D> {
        .Origin = krys::move(origin),
        .Components =
          css::SimplifyUnevaluatedCalc<D>(unresolved.Components, state.ConversionData, css::SymbolTable {}),
      }};
    }

    // If the origin is absolute, we can fully resolve the entire color.
    auto color = css::Resolve(css::RelativeColorResolver<D> {.Origin = origin.GetResolvedColor(),
                                                             .Components = unresolved.Components},
                              state.ConversionData);

    return {ResolvedColor {krys::move(color)}};
  }

  template <typename D>
  KRYS_NODISCARD boo::Color ResolveColor(const RelativeColor<D> &relative,
                                         const boo::Color &currentColor) noexcept
  {
    return css::ResolveNoConversionDataRequired(css::RelativeColorResolver<D> {
      .Origin = relative.Origin.ResolveColor(currentColor), .Components = relative.Components});
  }

  template <typename D>
  KRYS_NODISCARD bool ContainsCurrentColor(const RelativeColor<D> &relative) noexcept
  {
    return style::ContainsCurrentColor(relative.Origin);
  }

  template <typename D>
  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &context,
                                       const RelativeColor<D> &relative) noexcept
  {
    using ColorType = typename D::ColorType;

    if constexpr (D::UsesColorFunctionForSerialisation)
    {
      builder.append(u8"color(from ");
      SerialisationForCSSTokenization(builder, context, relative.Origin);
      builder.push_back(u8' ');
      builder.append(boo::Serialisation(ColorSpaceFor<ColorType>));
    }
    else
    {
      builder.append(D::SerialisationFunctionName);
      builder.append(u8"(from ");
      SerialisationForCSSTokenization(builder, context, relative.Origin);
    }

    const auto &[c1, c2, c3, alpha] = relative.Components;

    builder.push_back(u8' ');
    css::Serialise(builder, context, c1);
    builder.push_back(u8' ');
    css::Serialise(builder, context, c2);
    builder.push_back(u8' ');
    css::Serialise(builder, context, c3);

    if (alpha)
    {
      builder.append(u8" / ");
      css::Serialise(builder, context, *alpha);
    }

    builder.push_back(u8')');
  }

  template <typename D>
  KRYS_NODISCARD css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                                  const RelativeColor<D> &relative) noexcept
  {
    css::CSSOMString builder;
    SerialisationForCSSTokenization(builder, context, relative);
    return builder;
  }
}
