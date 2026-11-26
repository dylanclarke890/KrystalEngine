#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"

namespace krys::web::css
{
  struct SerialisationContext;

  /// @brief A value for each side of a box, in the order CSS lists them: top, right, bottom, left. The values
  /// of the four-sided properties (`margin`, `border-image-slice`) and of `rect()`.
  struct SideValues
  {
    Ref<const Value> Top;
    Ref<const Value> Right;
    Ref<const Value> Bottom;
    Ref<const Value> Left;

    /// @brief @p value on every side.
    KRYS_NODISCARD static SideValues Uniform(const Ref<const Value> &value) noexcept
    {
      return {value, value, value, value};
    }

    /// @brief Whether each side's value equals the same side's in @p other.
    KRYS_NODISCARD bool operator==(const SideValues &other) const noexcept;

    /// @brief Whether every side's value is equal.
    KRYS_NODISCARD bool AreAllEqual() const noexcept;
  };

  /// @brief The serialised values of the four sides, as few as give them all: a side whose value is its
  /// opposite side's is left off the end (`1px 2px` for `1px 2px 1px 2px`), as the four-sided properties'
  /// syntax fills it in.
  /// @see https://drafts.csswg.org/cssom/#serialize-a-css-value
  /// @see https://drafts.csswg.org/css-box-4/#margin-shorthand
  KRYS_NODISCARD CSSOMString SerialiseSides(const CSSOMString &top, const CSSOMString &right,
                                            const CSSOMString &bottom, const CSSOMString &left) noexcept;

  /// @brief @p sides serialised in @p context, as few as give them all.
  KRYS_NODISCARD CSSOMString SerialiseSides(const SideValues &sides,
                                            const SerialisationContext &context) noexcept;

  /// @brief @p sides serialised in @p context as `rect(<top>, <right>, <bottom>, <left>)`.
  /// @see https://drafts.fxtf.org/css-masking/#clip-property
  KRYS_NODISCARD CSSOMString SerialiseRect(const SideValues &sides,
                                           const SerialisationContext &context) noexcept;
}
