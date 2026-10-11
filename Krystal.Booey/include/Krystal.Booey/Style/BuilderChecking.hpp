#pragma once

#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::style
{
  template <typename ValueType>
  RefPtr<const ValueType> RequiredDowncast(BuilderState &, const css::Value &) noexcept;

  template <typename ValueType>
  Maybe<Pair<Ref<const ValueType>, Ref<const ValueType>>> RequiredPairDowncast(BuilderState &,
                                                                               const css::Value &) noexcept;

  template <typename ValueType>
  struct TypedRequiredListIterator
  {
    css::ValueList::iterator iterator;

    using iterator_category = std::forward_iterator_tag;
    using value_type = const ValueType &;
    using difference_type = ptrdiff_t;
    using pointer = const ValueType *;
    using reference = const ValueType &;

    TypedRequiredListIterator(css::ValueList::iterator iterator) : iterator(iterator)
    {
    }

    TypedRequiredListIterator &operator++()
    {
      ++iterator;
      return *this;
    }

    const ValueType &operator*() const
    {
      return downcast<ValueType>(*iterator);
    }

    bool operator!=(const TypedRequiredListIterator &other) const
    {
      return iterator != other.iterator;
    }
  };

  template <typename ListType, typename ValueType>
  struct TypedRequiredList
  {
    Ref<const ListType> list;

    TypedRequiredList(const ListType &list) : list(list)
    {
    }

    size_t size() const
    {
      return list->size();
    }

    const ValueType &item(size_t index) const noexcept
    {
      return Downcast<ValueType>(*list->item(index));
    }

    TypedRequiredListIterator<ValueType> begin() const noexcept
    {
      return list->begin();
    }

    TypedRequiredListIterator<ValueType> end() const noexcept
    {
      return list->end();
    }
  };

  template <typename ListType, typename ValueType, size_t minimumLength = 1uz>
  Maybe<TypedRequiredList<ListType, ValueType>> RequiredListDowncast(BuilderState &state,
                                                                     const css::Value &) noexcept;

  template <css::ValueId, typename ValueType, size_t minimumLength = 1>
  Maybe<TypedRequiredList<css::FunctionValue, ValueType>>
    RequiredFunctionDowncast(BuilderState &, const css::Value &) noexcept;

  template <typename ValueType>
  RefPtr<const ValueType> RequiredDowncast(BuilderState &builderState, const css::Value &value) noexcept
  {
    auto *typedValue = DynamicDowncast<ValueType>(value);
    if (!typedValue) [[unlikely]]
    {
      builderState.SetCurrentPropertyInvalidAtComputedValueTime();
      return nullptr;
    }

    return ShareRefPtr(typedValue);
  }

  template <typename ValueType>
  Maybe<Pair<Ref<const ValueType>, Ref<const ValueType>>>
    RequiredPairDowncast(BuilderState &builderState, const css::Value &value) noexcept
  {
    auto pairValue = RequiredDowncast<css::ValuePair>(builderState, value);
    if (!pairValue) [[unlikely]]
    {
      return {};
    }

    auto firstValue = RequiredDowncast<ValueType>(builderState, pairValue->First());
    if (!firstValue) [[unlikely]]
    {
      return {};
    }

    auto secondValue = RequiredDowncast<ValueType>(builderState, pairValue->Second());
    if (!secondValue) [[unlikely]]
    {
      return {};
    }

    return {{krys::move(firstValue), krys::move(secondValue)}};
  }

  template <typename ListType, typename ValueType, size_t minimumSize>
  Maybe<TypedRequiredList<ListType, ValueType>> RequiredListDowncast(BuilderState &builderState,
                                                                     const css::Value &value) noexcept
  {
    RefPtr listValue = RequiredDowncast<ListType>(builderState, value);
    if (!listValue) [[unlikely]]
    {
      return {};
    }

    if (listValue->size() < minimumSize) [[unlikely]]
    {
      builderState.SetCurrentPropertyInvalidAtComputedValueTime();
      return {};
    }

    for (Ref value : *listValue)
    {
      if (!RequiredDowncast<ValueType>(builderState, value)) [[unlikely]]
      {
        return {};
      }
    }

    return TypedRequiredList<ListType, ValueType> {*listValue};
  }

  template <css::ValueId functionName, typename ValueType, size_t minimumSize>
  Maybe<TypedRequiredList<css::FunctionValue, ValueType>>
    RequiredFunctionDowncast(BuilderState &builderState, const css::Value &value) noexcept
  {
    auto function = RequiredListDowncast<css::FunctionValue, ValueType, minimumSize>(builderState, value);
    if (!function) [[unlikely]]
    {
      return {};
    }

    if (function->List->Name() != functionName)
    {
      builderState.SetCurrentPropertyInvalidAtComputedValueTime();
      return {};
    }

    return function;
  }
}