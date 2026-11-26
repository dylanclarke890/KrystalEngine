#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Css/Properties/PropertyFlag.hpp"
#include "Krystal.Web/Css/Properties/PropertyId.hpp"
#include "Krystal.Web/Css/Properties/PropertyShorthandFunctions.hpp"
#include "Krystal.Web/Css/Properties/Types/IsImplicit.hpp"
#include "Krystal.Web/Css/Properties/Types/IsImportant.hpp"
#include "Krystal.Web/Css/Properties/Types/IsSetFromShorthand.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"
#include <bit>

namespace krys::web::css
{
  /// @brief Everything a declaration records besides its value.
  struct PropertyMetadata
  {
    PropertyId Id : EnumBitWidth<PropertyId> {PropertyId::Invalid};
    PropertyFlags Flags;

    /// @brief Which of `MatchingShorthandsForLonghand(Id)` set the property, when it was set from a
    /// shorthand.
    uint8 ShorthandIndex : std::bit_width(MaxShorthandsForLonghand - 1) {0u};

    /// @brief The shorthand that set the property, or `PropertyId::Invalid` if it was set directly.
    KRYS_NODISCARD PropertyId ShorthandId() const noexcept;

    KRYS_NODISCARD friend bool operator==(const PropertyMetadata &,
                                          const PropertyMetadata &) noexcept = default;
  };

  /// @brief A declaration of a longhand property: which one, its flags and its value.
  /// @see https://drafts.csswg.org/cssom-1/#css-declaration
  class Property
  {
  private:
    PropertyMetadata _metadata;
    Ref<css::Value> _value;

  public:
    Property(const PropertyMetadata &metadata, Ref<css::Value> value) noexcept
        : _metadata(metadata), _value(krys::move(value))
    {
      krys_debug_assert(_metadata.Id != PropertyId::Invalid);
      krys_debug_assert_msg(!IsShorthand(_metadata.Id), "a declaration is of a longhand");
    }

    /// @param shorthandIndex Which of `MatchingShorthandsForLonghand(id)` set the property, if @p
    /// setFromShorthand.
    Property(PropertyId id, Ref<css::Value> value, IsImportant important = IsImportant::No,
             IsImplicit implicit = IsImplicit::No,
             IsSetFromShorthand setFromShorthand = IsSetFromShorthand::No, uint8 shorthandIndex = 0u) noexcept
        : Property(PropertyMetadata {.Id = id,
                                     .Flags = Flags(important, implicit, setFromShorthand),
                                     .ShorthandIndex = shorthandIndex},
                   krys::move(value))
    {
    }

    KRYS_NODISCARD PropertyId Id() const noexcept
    {
      return _metadata.Id;
    }

    /// @brief The shorthand that set the property, or `PropertyId::Invalid` if it was set directly.
    KRYS_NODISCARD PropertyId ShorthandId() const noexcept
    {
      return _metadata.ShorthandId();
    }

    /// @brief Whether the declaration is `!important`.
    KRYS_NODISCARD IsImportant Important() const noexcept
    {
      return IsImportant(_metadata.Flags.contains(PropertyFlag::Important));
    }

    /// @brief Whether a shorthand set the property without its value being written.
    KRYS_NODISCARD IsImplicit Implicit() const noexcept
    {
      return IsImplicit(_metadata.Flags.contains(PropertyFlag::Implicit));
    }

    KRYS_NODISCARD IsSetFromShorthand SetFromShorthand() const noexcept
    {
      return IsSetFromShorthand(_metadata.Flags.contains(PropertyFlag::SetFromShorthand));
    }

    KRYS_NODISCARD css::Value &Value() noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD const css::Value &Value() const noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD const PropertyMetadata &Metadata() const noexcept
    {
      return _metadata;
    }

    KRYS_NODISCARD bool operator==(const Property &other) const noexcept
    {
      return _metadata == other._metadata && _value->Equals(*other._value);
    }

  private:
    KRYS_NODISCARD constexpr static PropertyFlags Flags(IsImportant important, IsImplicit implicit,
                                                        IsSetFromShorthand setFromShorthand) noexcept
    {
      PropertyFlags flags;
      if (important == IsImportant::Yes)
      {
        flags.Add(PropertyFlag::Important);
      }
      if (implicit == IsImplicit::Yes)
      {
        flags.Add(PropertyFlag::Implicit);
      }
      if (setFromShorthand == IsSetFromShorthand::Yes)
      {
        flags.Add(PropertyFlag::SetFromShorthand);
      }
      return flags;
    }
  };
}
