#pragma once

#include "Krystal.Booey/CSS/Values/Color/AbsoluteColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/AbsoluteColorResolver.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Booey/Style/Values/Color/ResolvedColor.hpp"

namespace krys::boo::style
{
  // NOTE: There is no style::AbsoluteColor<D> type. Style conversion of css::AbsoluteColor<D> yields a
  // style::ResolvedColor.
  template <typename D>
  KRYS_NODISCARD Color ToStyleColor(const css::AbsoluteColor<D> &unresolved,
                                    ColorResolutionState &state) noexcept
  {
    ColorResolutionStateNester nester {state};

    auto resolver =
      css::AbsoluteColorResolver<D> {.Components = unresolved.Components, .NestingLevel = state.NestingLevel};

    return Color {ResolvedColor {css::Resolve(krys::move(resolver), state.ConversionData)}};
  }
}
