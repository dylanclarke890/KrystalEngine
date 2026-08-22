#pragma once

#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyFlag.hpp"
#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.HTML/CSS/Properties/Types/IsImplicit.hpp"
#include "Krystal.HTML/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.HTML/CSS/Properties/Types/IsSetFromShorthand.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  /// @brief The maximum number of shorthands that can set a single property. This is usually 'all' and one
  /// other. Logical properties are not counted as shorthands for physical properties and vice versa.
  /// @example `margin-left` has two shorthands: `margin` and `all`. `margin-inline` is not counted as a
  /// shorthand for `margin-left` because it is a logical property and `margin-left` is a physical property.
  /// `margin-inline-start` is also not counted as a shorthand for `margin-left` because it is a longhand
  /// property.
  constexpr inline uint8 MaxShorthandsPerProperty = 4u;

  struct CSSParserContext;

  struct CSSPropertyMetadata
  {
    CSSPropertyId Id : BitCount<CSSPropertyId>() {CSSPropertyId::Invalid};
    CSSPropertyFlag Flags : BitCount<CSSPropertyFlag>() {CSSPropertyFlag::None};

    /// @brief The index into the shorthands vector that this property was set from, if `SetFromShorthand`
    /// flag is set and there is ambiguity (i.e. multiple shorthands set the same property). If there is no
    /// ambiguity, this value will be 0.
    uint8 ShorthandIndex : std::bit_width(MaxShorthandsPerProperty) {0u};

    KRYS_NODISCARD CSSPropertyId ShorthandId() const noexcept;
  };

  class CSSProperty
  {
  private:
    CSSPropertyMetadata _metadata;
    Ref<CSSValue> _value;

  public:
    CSSProperty(const CSSPropertyMetadata &metadata, Ref<CSSValue> &&value) noexcept
        : _metadata(metadata), _value(Krys::Move(value))
    {
    }

    CSSProperty(CSSPropertyId id, Ref<CSSValue> &&value, IsImportant important = IsImportant(false),
                IsImplicit implicit = IsImplicit(false),
                IsSetFromShorthand setFromShorthand = IsSetFromShorthand(false),
                uint8 shorthandIndex = 0u) noexcept
        : _metadata({.Id = id, .ShorthandIndex = shorthandIndex}), _value(Krys::Move(value))
    {
      if (important)
      {
        _metadata.Flags = _metadata.Flags | CSSPropertyFlag::Important;
      }

      if (implicit)
      {
        _metadata.Flags = _metadata.Flags | CSSPropertyFlag::Implicit;
      }

      if (setFromShorthand)
      {
        _metadata.Flags = _metadata.Flags | CSSPropertyFlag::SetFromShorthand;
      }
    }

    KRYS_NODISCARD CSSPropertyId Id() const noexcept
    {
      return _metadata.Id;
    }

    KRYS_NODISCARD CSSPropertyId ShorthandId() const noexcept
    {
      return _metadata.ShorthandId();
    }

    KRYS_NODISCARD IsImportant Important() const noexcept
    {
      return IsImportant(HasFlag(_metadata.Flags, CSSPropertyFlag::Important));
    }

    KRYS_NODISCARD IsImplicit Implicit() const noexcept
    {
      return IsImplicit(HasFlag(_metadata.Flags, CSSPropertyFlag::Implicit));
    }

    KRYS_NODISCARD IsSetFromShorthand SetFromShorthand() const noexcept
    {
      return IsSetFromShorthand(HasFlag(_metadata.Flags, CSSPropertyFlag::SetFromShorthand));
    }

    KRYS_NODISCARD uint8 ShorthandIndex() const noexcept
    {
      return _metadata.ShorthandIndex;
    }

    KRYS_NODISCARD CSSValue &Value() noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD const CSSValue &Value() const noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD static bool IsShorthand(CSSPropertyId id) noexcept
    {
      return static_cast<size_t>(id) >= static_cast<uint16>(FirstShorthandProperty)
             && static_cast<size_t>(id) <= static_cast<uint16>(LastShorthandProperty);
    }

    KRYS_NODISCARD static bool IsLonghand(CSSPropertyId id) noexcept
    {
      // TODO
      return false;
    }

    KRYS_NODISCARD static bool IsInheritedProperty(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool IsCornerShapeProperty(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool IsMarginProperty(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool IsPaddingProperty(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static SmallList<CSSOMString> AliasesForProperty(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool DisablesNativeAppearance(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static char32 ListValuedPropertySeparator(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool AllowsNumberOrIntegerInput(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool IsInLogicalPropertyGroup(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool
      AreInSameLogicalPropertyGroupWithDifferentMappingLogic(CSSPropertyId id1, CSSPropertyId id2) noexcept;

    KRYS_NODISCARD static bool IsDescriptorOnly(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool AcceptsQuirkyColor(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool AcceptsQuirkyLength(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool AcceptsQuirkyAngle(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static Span<const CSSValueId> ValidKeywordsForProperty(CSSPropertyId id) noexcept;

    KRYS_NODISCARD static bool IsKeywordValidForPropertyValues(CSSPropertyId id, CSSValueId keyword,
                                                               const CSSParserContext &context) noexcept;
  };
}