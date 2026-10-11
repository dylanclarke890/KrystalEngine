#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"

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
    /// @brief The allowed numeric range for the calculated result.
    css::Range range;

    /// @brief The context used for CSS serialization state.
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