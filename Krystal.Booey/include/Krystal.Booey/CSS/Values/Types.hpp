#pragma once

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  template <typename CSSType>
  struct CSSValueCreation;

  struct CSSValueCreationInvoker
  {
    template <typename CSSType, typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &pool, const CSSType &value, Args &&...args) const noexcept
    {
      return CSSValueCreation<CSSType> {}(pool, value, std::forward<Args>(args)...);
    }
  };

  constexpr CSSValueCreationInvoker CreateCSSValue {};

  KRYS_NODISCARD Ref<Value> CreatePrimitiveCSSValue(ValueId keyword) noexcept;

  KRYS_NODISCARD Ref<Value> CreatePrimitiveCSSValue(const CustomIdentifier &identifer) noexcept;

  KRYS_NODISCARD Ref<Value> CreatePrimitiveCSSValue(const PropertyIdentifier &identifer) noexcept;

  KRYS_NODISCARD Ref<Value> CreatePrimitiveCSSValue(const CSSOMStringAtom &str) noexcept;

  KRYS_NODISCARD Ref<Value> CreatePrimitiveCSSValue(const CSSOMString &str) noexcept;

  KRYS_NODISCARD Ref<Value> CreateFunctionCSSValue(ValueId name, Ref<Value> &&args) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<Value> CreateCoalescingPairCSSValue(Ref<Value> &&, Ref<Value> &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<Value>
    CreateCoalescingPairCSSValue<SerializationSeparatorType::Space>(Ref<Value> &&, Ref<Value> &&) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<Value> CreateCoalescingQuadCSSValue(Ref<Value> &&, Ref<Value> &&, Ref<Value> &&,
                                                         Ref<Value> &&) noexcept;
  template <>
  KRYS_NODISCARD Ref<Value>
    CreateCoalescingQuadCSSValue<SerializationSeparatorType::Space>(Ref<Value> &&, Ref<Value> &&,
                                                                    Ref<Value> &&, Ref<Value> &&) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<Value> CreateListCSSValue(ValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<Value>
    CreateListCSSValue<SerializationSeparatorType::Space>(ValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<Value>
    CreateListCSSValue<SerializationSeparatorType::Comma>(ValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<Value>
    CreateListCSSValue<SerializationSeparatorType::Slash>(ValueListBuilder &&) noexcept;

  // Constrained for `TreatAsVariantLike`.
  template <VariantLike CSSType>
  struct CSSValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &pool, const CSSType &value, Args &&...args) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &alternative)
                            { return CreateCSSValue(pool, alternative, std::forward<Args>(args)...); });
    }
  };

  // Constrained for `TreatAsTupleLike`
  template <TupleLike CSSType>
  struct CSSValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &pool, const CSSType &value, Args &&...args) noexcept
    {
      if constexpr (std::tuple_size_v<CSSType> == 1
                    && SerializationSeparator<CSSType> == SerializationSeparatorType::None)
      {
        return CreateCSSValue(pool, get<0>(value), std::forward<Args>(args)...);
      }
      else if constexpr (std::tuple_size_v<CSSType> == 2
                         && SerializationCoalescing<CSSType> == SerializationCoalescingType::Minimal)
      {
        return CreateCoalescingPairCSSValue<SerializationSeparator<CSSType>>(
          CreateCSSValue(pool, get<0>(value), std::forward<Args>(args)...),
          CreateCSSValue(pool, get<1>(value), std::forward<Args>(args)...));
      }
      else if constexpr (std::tuple_size_v<CSSType> == 4
                         && SerializationCoalescing<CSSType> == SerializationCoalescingType::Minimal)
      {
        return CreateCoalescingQuadCSSValue<SerializationSeparator<CSSType>>(
          CreateCSSValue(pool, get<0>(value), std::forward<Args>(args)...),
          CreateCSSValue(pool, get<1>(value), std::forward<Args>(args)...),
          CreateCSSValue(pool, get<2>(value), std::forward<Args>(args)...),
          CreateCSSValue(pool, get<3>(value), std::forward<Args>(args)...));
      }
      else
      {
        ValueListBuilder list;

        auto caller = krys::CreateVisitor(
          [&]<OptionalLike T>(const T &element)
          {
            if (!element)
            {
              return;
            }

            list.push_back(CreateCSSValue(pool, *element, std::forward<Args>(args)...));
          },
          [&](const auto &element)
          { list.push_back(CreateCSSValue(pool, element, std::forward<Args>(args)...)); });

        apply([&](const auto &...x) { (..., caller(x)); }, value);

        return CreateListCSSValue<SerializationSeparator<CSSType>>(krys::move(list));
      }
    }
  };

  // Constrained for `TreatAsRangeLike`
  template <RangeLike CSSType>
  struct CSSValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &pool, const CSSType &value, Args &&...args) noexcept
    {
      ValueListBuilder list;
      for (const auto &element : value)
      {
        list.push_back(CreateCSSValue(pool, element, std::forward<Args>(args)...));
      }

      return CreateListCSSValue<SerializationSeparator<CSSType>>(krys::move(list));
    }
  };

  // Specialization for `Constant`.
  template <ValueId Id>
  struct CSSValueCreation<KeywordValueConstant<Id>>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &, const KeywordValueConstant<Id> &, Args &&...) noexcept
    {
      return CreatePrimitiveCSSValue(Id);
    }
  };

  // Specialization for `CustomIdentifier`.
  template <>
  struct CSSValueCreation<CustomIdentifier>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &, const CustomIdentifier &customIdentifier,
                                         Args &&...) noexcept
    {
      return CreatePrimitiveCSSValue(customIdentifier);
    }
  };

  // Specialization for `PropertyIdentifier`.
  template <>
  struct CSSValueCreation<PropertyIdentifier>
  {
    template <typename... Args>
    Ref<Value> operator()(ValuePool &, const PropertyIdentifier &propertyIdentifier, Args &&...)
    {
      return CreatePrimitiveCSSValue(propertyIdentifier);
    }
  };

  // Specialization for `CSSOMStringAtom`.
  template <>
  struct CSSValueCreation<CSSOMStringAtom>
  {
    template <typename... Args>
    Ref<Value> operator()(ValuePool &, const CSSOMStringAtom &string, Args &&...)
    {
      return CreatePrimitiveCSSValue(string);
    }
  };

  // Specialization for `CSSOMString`.
  template <>
  struct CSSValueCreation<CSSOMString>
  {
    template <typename... Args>
    Ref<Value> operator()(ValuePool &, const CSSOMString &string, Args &&...)
    {
      return CreatePrimitiveCSSValue(string);
    }
  };

  // Specialization for `FunctionNotation`.
  template <ValueId Name, typename CSSType>
  struct CSSValueCreation<FunctionNotation<Name, CSSType>>
  {
    template <typename... Args>
    Ref<Value> operator()(ValuePool &pool, const FunctionNotation<Name, CSSType> &value, Args &&...args)
    {
      return CreateFunctionCSSValue(value.name,
                                    CreateCSSValue(pool, value.parameters, std::forward<Args>(args)...));
    }
  };
}