#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  /// @brief Represents a shorthand property and the longhand properties it expands into.
  /// @example `margin` expands into `margin-top`, `margin-left`, `margin-bottom`, `margin-right`.
  class PropertyShorthand
  {
  private:
    Span<const PropertyId> _properties;
    PropertyId _shorthandId {PropertyId::Invalid};

  public:
    PropertyShorthand() noexcept = default;

    template <size_t NumProperties>
    PropertyShorthand(PropertyId id, Span<const PropertyId, NumProperties> properties) noexcept
        : _properties(properties), _shorthandId(id)
    {
      static_assert(NumProperties != std::dynamic_extent);
    }

    KRYS_NODISCARD const PropertyId *begin() const noexcept
    {
      return std::to_address(_properties.begin());
    }

    KRYS_NODISCARD const PropertyId *end() const noexcept
    {
      return std::to_address(_properties.end());
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _properties.size();
    }

    KRYS_NODISCARD PropertyId Id() const noexcept
    {
      return _shorthandId;
    }

    KRYS_NODISCARD Span<const PropertyId> Properties() const noexcept
    {
      return _properties;
    }
  };

  /// @brief Finds the index of the shorthand property in the given list of shorthands. If the shorthand
  /// property is not found, an assertion will fail.
  KRYS_NODISCARD constexpr size_t
    IndexOfShorthandProperty(PropertyId property, Span<const PropertyShorthand> shorthands) noexcept
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

  // The implementation is generated in PropertyShorthandFunctions.cpp.
  KRYS_NODISCARD PropertyShorthand ShorthandForProperty(PropertyId) noexcept;

  using CSSPropertyShorthandList = SmallList<PropertyShorthand, 4uz>;

  /// @brief Gets the list of longhand properties that are set by the given shorthand property.
  KRYS_NODISCARD PropertyShorthand LonghandsForShorthand(PropertyId property) noexcept;

  /// @brief Gets the list of shorthands that set the given longhand property.
  KRYS_NODISCARD CSSPropertyShorthandList MatchingShorthandsForLonghand(PropertyId property) noexcept;

  KRYS_NODISCARD PropertyShorthand TransitionShorthandForParsing() noexcept;
}