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
    PropertyId _shorthandId {PropertyId::Invalid};
    Span<const PropertyId> _properties;

  public:
    PropertyShorthand() noexcept = default;

    template <size_t NumProperties>
    PropertyShorthand(PropertyId id, Span<const PropertyId, NumProperties> properties) noexcept
        : _shorthandId(id), _properties(properties)
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

  /// @brief Custom PropertyShorthand function.
  KRYS_NODISCARD PropertyShorthand TransitionShorthandForParsing() noexcept;

  /// @brief Returns an empty value if the property is not a shorthand.
  /// @note The implementation is generated in PropertyShorthandFunctions.cpp.
  KRYS_NODISCARD PropertyShorthand ShorthandForProperty(PropertyId id) noexcept;

  using PropertyShorthandList = SmallList<PropertyShorthand, 4uz>;

  /// @brief Returns the list of shorthands for a given longhand.
  /// @note The implementation is generated in PropertyShorthandFunctions.cpp.
  KRYS_NODISCARD PropertyShorthandList MatchingShorthandsForLonghand(PropertyId id) noexcept;

  /// @brief Finds the index of `shorthandId` in `shorthands`, which must contain it.
  KRYS_NODISCARD uint32 IndexOfShorthandForLonghand(PropertyId shorthandId,
                                                    const PropertyShorthandList &shorthands) noexcept;
}