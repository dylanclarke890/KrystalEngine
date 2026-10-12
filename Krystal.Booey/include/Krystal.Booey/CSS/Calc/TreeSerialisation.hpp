#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  struct SerialisationContext;
}

namespace krys::boo::css::calc
{
  struct Child;
  struct Tree;

  struct SerialisationOptions
  {
    // `Range` represents the allowed numeric range for the calculated result.
    css::Range Range;

    // `SerialisationContext` is the context used for CSS serialisation state.
    const css::SerialisationContext &SerialisationContext;
  };

  // https://drafts.csswg.org/css-values-4/#serialize-a-math-function
  void SerialisationForCSS(CSSOMString &builder, const Tree &tree,
                           const SerialisationOptions &options) noexcept;

  KRYS_NODISCARD CSSOMString SerialisationForCSS(const Tree &tree,
                                                 const SerialisationOptions &options) noexcept;

  void SerialisationForCSS(CSSOMString &builder, const Child &child,
                           const SerialisationOptions &options) noexcept;

  KRYS_NODISCARD CSSOMString SerialisationForCSS(const Child &child,
                                                 const SerialisationOptions &options) noexcept;
}