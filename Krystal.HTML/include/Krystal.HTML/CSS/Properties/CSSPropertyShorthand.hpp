#pragma once

#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys::HTML
{
  /// @brief Represents a shorthand property and the longhand properties it expands into.
  /// @example `margin` expands into `margin-top`, `margin-left`, `margin-bottom`, `margin-right`.
  class CSSPropertyShorthand
  {
  private:
    Span<const CSSPropertyId> _properties;
    CSSPropertyId _shorthandId {CSSPropertyId::Invalid};

  public:
    CSSPropertyShorthand() noexcept = default;

    template <size_t NumProperties>
    CSSPropertyShorthand(CSSPropertyId id, Span<const CSSPropertyId, NumProperties> properties) noexcept
        : _properties(properties), _shorthandId(id)
    {
      static_assert(NumProperties != std::dynamic_extent);
    }

    KRYS_NODISCARD const CSSPropertyId *begin() const noexcept
    {
      return std::to_address(_properties.begin());
    }

    KRYS_NODISCARD const CSSPropertyId *end() const noexcept
    {
      return std::to_address(_properties.end());
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _properties.size();
    }

    KRYS_NODISCARD CSSPropertyId Id() const noexcept
    {
      return _shorthandId;
    }

    KRYS_NODISCARD Span<const CSSPropertyId> Properties() const noexcept
    {
      return _properties;
    }
  };

  /// @brief Finds the index of the shorthand property in the given list of shorthands. If the shorthand
  /// property is not found, an assertion will fail.
  KRYS_NODISCARD constexpr size_t
    IndexOfShorthandProperty(CSSPropertyId property, Span<const CSSPropertyShorthand> shorthands) noexcept
  {
    for (size_t i = 0uz; i < shorthands.size(); ++i)
    {
      if (shorthands[i].Id() == property)
      {
        return i;
      }
    }

    assert(false);
    return 0uz;
  }

  // The implementation is generated in CSSPropertyShorthandFunctions.cpp.
  KRYS_NODISCARD CSSPropertyShorthand ShorthandForProperty(CSSPropertyId) noexcept;

  using CSSPropertyShorthandList = SmallList<CSSPropertyShorthand, 4uz>;

  /// @brief Gets the list of longhand properties that are set by the given shorthand property.
  KRYS_NODISCARD CSSPropertyShorthand LonghandsForShorthand(CSSPropertyId property) noexcept;

  /// @brief Gets the list of shorthands that set the given longhand property.
  KRYS_NODISCARD CSSPropertyShorthandList MatchingShorthandsForLonghand(CSSPropertyId property) noexcept;

  KRYS_NODISCARD CSSPropertyShorthand TransitionShorthandForParsing() noexcept;
}