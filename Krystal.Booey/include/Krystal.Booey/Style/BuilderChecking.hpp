#pragma once

#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <iterator>

// FIXME: Include `Style/BuilderState.hpp` instead of the seam once the builder is ported.

namespace krys::boo::style
{
  namespace detail_builder_checking
  {
    /// @brief Downcasts a value known to be a `ValueType` (a no-op for `css::Value` itself).
    template <typename ValueType>
    KRYS_NODISCARD const ValueType &DowncastValue(const css::Value &value) noexcept
    {
      if constexpr (SameType<ValueType, css::Value>)
      {
        return value;
      }
      else
      {
        return Downcast<ValueType>(value);
      }
    }
  }

  template <typename ValueType>
  KRYS_NODISCARD const ValueType *RequiredDowncast(BuilderState &, const css::Value &) noexcept;

  template <typename ValueType>
  KRYS_NODISCARD Maybe<Pair<Ref<const ValueType>, Ref<const ValueType>>>
    RequiredPairDowncast(BuilderState &, const css::Value &) noexcept;

  template <typename ValueType>
  struct TypedRequiredListIterator
  {
    using iterator_category = std::forward_iterator_tag;
    using value_type = const ValueType &;
    using difference_type = ptrdiff_t;
    using pointer = const ValueType *;
    using reference = const ValueType &;

    css::ValueList::iterator Iterator;

    TypedRequiredListIterator(css::ValueList::iterator iterator) noexcept : Iterator(iterator)
    {
    }

    TypedRequiredListIterator &operator++() noexcept
    {
      ++Iterator;
      return *this;
    }

    KRYS_NODISCARD const ValueType &operator*() const noexcept
    {
      return detail_builder_checking::DowncastValue<ValueType>(*Iterator);
    }

    KRYS_NODISCARD bool operator!=(const TypedRequiredListIterator &other) const noexcept
    {
      return Iterator != other.Iterator;
    }
  };

  template <typename ListType, typename ValueType>
  struct TypedRequiredList
  {
    Ref<const ListType> List;

    TypedRequiredList(const ListType &list) noexcept : List(ShareRef(list))
    {
    }

    KRYS_NODISCARD uint32 size() const noexcept
    {
      return List->size();
    }

    KRYS_NODISCARD const ValueType &item(uint32 index) const noexcept KRYS_LIFETIME_BOUND
    {
      return detail_builder_checking::DowncastValue<ValueType>(*List->Item(index));
    }

    KRYS_NODISCARD TypedRequiredListIterator<ValueType> begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return List->begin();
    }

    KRYS_NODISCARD TypedRequiredListIterator<ValueType> end() const noexcept KRYS_LIFETIME_BOUND
    {
      return List->end();
    }
  };

  template <typename ListType, typename ValueType, uint32 MinimumLength = 1>
  KRYS_NODISCARD Maybe<TypedRequiredList<ListType, ValueType>>
    RequiredListDowncast(BuilderState &, const css::Value &) noexcept;

  template <css::ValueId, typename ValueType, uint32 MinimumLength = 1>
  KRYS_NODISCARD Maybe<TypedRequiredList<css::FunctionValue, ValueType>>
    RequiredFunctionDowncast(BuilderState &, const css::Value &) noexcept;

  template <typename ValueType>
  inline const ValueType *RequiredDowncast(BuilderState &builderState, const css::Value &value) noexcept
  {
    if constexpr (SameType<ValueType, css::Value>)
    {
      return &value;
    }
    else
    {
      auto *typedValue = DynamicDowncast<ValueType>(value);
      if (!typedValue) KRYS_UNLIKELY
      {
        builderState.SetCurrentPropertyInvalidAtComputedValueTime();
        return nullptr;
      }
      return typedValue;
    }
  }

  template <typename ValueType>
  inline Maybe<Pair<Ref<const ValueType>, Ref<const ValueType>>>
    RequiredPairDowncast(BuilderState &builderState, const css::Value &value) noexcept
  {
    auto *pairValue = RequiredDowncast<css::ValuePair>(builderState, value);
    if (!pairValue) KRYS_UNLIKELY
    {
      return null;
    }
    auto *firstValue = RequiredDowncast<ValueType>(builderState, pairValue->First());
    if (!firstValue) KRYS_UNLIKELY
    {
      return null;
    }
    auto *secondValue = RequiredDowncast<ValueType>(builderState, pairValue->Second());
    if (!secondValue) KRYS_UNLIKELY
    {
      return null;
    }
    return Pair<Ref<const ValueType>, Ref<const ValueType>> {ShareRef(*firstValue), ShareRef(*secondValue)};
  }

  template <typename ListType, typename ValueType, uint32 MinimumSize>
  inline Maybe<TypedRequiredList<ListType, ValueType>> RequiredListDowncast(BuilderState &builderState,
                                                                            const css::Value &value) noexcept
  {
    auto *listValue = RequiredDowncast<ListType>(builderState, value);
    if (!listValue) KRYS_UNLIKELY
    {
      return null;
    }
    if (listValue->size() < MinimumSize) KRYS_UNLIKELY
    {
      builderState.SetCurrentPropertyInvalidAtComputedValueTime();
      return null;
    }
    for (const auto &item : *listValue)
    {
      if (!RequiredDowncast<ValueType>(builderState, item)) KRYS_UNLIKELY
      {
        return null;
      }
    }
    return TypedRequiredList<ListType, ValueType> {*listValue};
  }

  template <css::ValueId FunctionName, typename ValueType, uint32 MinimumSize>
  inline Maybe<TypedRequiredList<css::FunctionValue, ValueType>>
    RequiredFunctionDowncast(BuilderState &builderState, const css::Value &value) noexcept
  {
    auto function = RequiredListDowncast<css::FunctionValue, ValueType, MinimumSize>(builderState, value);
    if (!function) KRYS_UNLIKELY
    {
      return null;
    }
    if (function->List->Name() != FunctionName)
    {
      builderState.SetCurrentPropertyInvalidAtComputedValueTime();
      return null;
    }
    return function;
  }
}
