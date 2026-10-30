#pragma once

#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  class Builder;
}

namespace krys::boo::css
{
  /// @brief The value set on each longhand of a shorthand whose value contains `var()` or `env()` references.
  /// The shorthand is parsed again, and the longhand's value picked out, once the references are resolved.
  class PendingSubstitutionValue final : public Value
  {
  private:
    const PropertyId _shorthandPropertyId;
    const Ref<VariableReferenceValue> _shorthandValue;

    mutable List<Property> _cachedPropertyValues;

    PendingSubstitutionValue(PropertyId shorthandPropertyId,
                             Ref<VariableReferenceValue> &&shorthandValue) noexcept
        : Value(ValueType::PendingSubstitution), _shorthandPropertyId(shorthandPropertyId),
          _shorthandValue(krys::move(shorthandValue))
    {
    }

  public:
    KRYS_NODISCARD static Ref<PendingSubstitutionValue>
      Create(PropertyId shorthandPropertyId, Ref<VariableReferenceValue> &&shorthandValue) noexcept
    {
      return AdoptRef(*new PendingSubstitutionValue(shorthandPropertyId, krys::move(shorthandValue)));
    }

    KRYS_NODISCARD VariableReferenceValue &ShorthandValue() const noexcept
    {
      return *_shorthandValue;
    }

    KRYS_NODISCARD PropertyId ShorthandPropertyId() const noexcept
    {
      return _shorthandPropertyId;
    }

    KRYS_NODISCARD bool Equals(const PendingSubstitutionValue &other) const noexcept
    {
      return _shorthandValue.get() == other._shorthandValue.get();
    }

    KRYS_NODISCARD static CSSOMString CustomCSSText(const SerialisationContext &) noexcept
    {
      return {};
    }

    KRYS_NODISCARD RefPtr<Value> ResolveValue(style::Builder &builder, PropertyId propertyId) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*_shorthandValue) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PendingSubstitutionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPendingSubstitutionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
