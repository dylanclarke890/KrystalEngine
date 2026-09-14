#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyFlag.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImplicit.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsSetFromShorthand.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  /// @brief The average number of shorthands that can set a single property (max is currently 6). This is
  /// usually 'all' and one other. Logical properties are not counted as shorthands for physical properties
  /// and vice versa.
  /// @example `margin-left` has two shorthands: `margin` and `all`. `margin-inline` is not counted as a
  /// shorthand for `margin-left` because it is a logical property and `margin-left` is a physical property.
  /// `margin-inline-start` is also not counted as a shorthand for `margin-left` because it is a longhand
  /// property.
  constexpr uint8 MaxShorthandsPerProperty = 4u;

  struct ParserContext;

  struct PropertyMetadata
  {
    PropertyId Id : BitCount<PropertyId>() {PropertyId::Invalid};
    PropertyFlag Flags : BitCount<PropertyFlag>() {PropertyFlag::None};

    /// @brief The index into the shorthands vector that this property was set from, if `SetFromShorthand`
    /// flag is set and there is ambiguity (i.e. multiple shorthands set the same property). If there is no
    /// ambiguity, this value will be 0.
    uint8 ShorthandIndex : std::bit_width(MaxShorthandsPerProperty) {0u};

    KRYS_NODISCARD PropertyId ShorthandId() const noexcept;
  };

  class Property
  {
  private:
    PropertyMetadata _metadata;
    Ref<css::Value> _value;

  public:
    Property(const PropertyMetadata &metadata, Ref<css::Value> &&value) noexcept
        : _metadata(metadata), _value(krys::move(value))
    {
    }

    Property(PropertyId id, Ref<css::Value> &&value, IsImportant important = IsImportant(false),
             IsImplicit implicit = IsImplicit(false),
             IsSetFromShorthand setFromShorthand = IsSetFromShorthand(false),
             uint8 shorthandIndex = 0u) noexcept
        : _metadata({.Id = id, .ShorthandIndex = shorthandIndex}), _value(krys::move(value))
    {
      if (important)
      {
        _metadata.Flags = _metadata.Flags | PropertyFlag::Important;
      }

      if (implicit)
      {
        _metadata.Flags = _metadata.Flags | PropertyFlag::Implicit;
      }

      if (setFromShorthand)
      {
        _metadata.Flags = _metadata.Flags | PropertyFlag::SetFromShorthand;
      }
    }

    KRYS_NODISCARD PropertyId Id() const noexcept
    {
      return _metadata.Id;
    }

    KRYS_NODISCARD PropertyId ShorthandId() const noexcept
    {
      return _metadata.ShorthandId();
    }

    KRYS_NODISCARD IsImportant Important() const noexcept
    {
      return IsImportant(HasFlag(_metadata.Flags, PropertyFlag::Important));
    }

    KRYS_NODISCARD IsImplicit Implicit() const noexcept
    {
      return IsImplicit(HasFlag(_metadata.Flags, PropertyFlag::Implicit));
    }

    KRYS_NODISCARD IsSetFromShorthand SetFromShorthand() const noexcept
    {
      return IsSetFromShorthand(HasFlag(_metadata.Flags, PropertyFlag::SetFromShorthand));
    }

    KRYS_NODISCARD uint8 ShorthandIndex() const noexcept
    {
      return _metadata.ShorthandIndex;
    }

    KRYS_NODISCARD css::Value &Value() noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD const css::Value &Value() const noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD static PropertyId ResolveDirectionAwareProperty(PropertyId id,
                                                                   WritingMode writingMode) noexcept;

    KRYS_NODISCARD static PropertyId UnresolvePhysicalProperty(PropertyId id,
                                                               WritingMode writingMode) noexcept;

    KRYS_NODISCARD static bool IsShorthand(PropertyId id) noexcept
    {
      return static_cast<size_t>(id) >= static_cast<uint16>(FirstShorthandProperty)
             && static_cast<size_t>(id) <= static_cast<uint16>(LastShorthandProperty);
    }

    KRYS_NODISCARD static bool IsLonghand(PropertyId id) noexcept
    {
      // TODO
      return false;
    }

    KRYS_NODISCARD static bool IsInheritedProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsBorderColorProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsBorderRadiusProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsBorderStyleProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsBorderWidthProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsCornerShapeProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsContainIntrinsicSizeProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsInsetProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsMinSizeProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsMaxSizeProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsOverflowProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsOverscrollBehaviorProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsScrollMarginProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsScrollPaddingProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsSizeProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsMarginProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsPaddingProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static SmallList<CSSOMString> AliasesForProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool DisablesNativeAppearance(PropertyId id) noexcept;

    KRYS_NODISCARD static char32 ListValuedPropertySeparator(PropertyId id) noexcept;

    KRYS_NODISCARD static bool AllowsNumberOrIntegerInput(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsInLogicalPropertyGroup(PropertyId id) noexcept;

    KRYS_NODISCARD static bool
      AreInSameLogicalPropertyGroupWithDifferentMappingLogic(PropertyId id1, PropertyId id2) noexcept;

    KRYS_NODISCARD static bool IsDescriptorOnly(PropertyId id) noexcept;

    KRYS_NODISCARD static bool AcceptsQuirkyColor(PropertyId id) noexcept;

    KRYS_NODISCARD static bool AcceptsQuirkyLength(PropertyId id) noexcept;

    KRYS_NODISCARD static bool AcceptsQuirkyAngle(PropertyId id) noexcept;

    KRYS_NODISCARD static Span<const ValueId> ValidKeywordsForProperty(PropertyId id) noexcept;

    KRYS_NODISCARD static bool IsKeywordValidForPropertyValues(PropertyId id, ValueId keyword,
                                                               const ParserContext &context) noexcept;
  };
}