#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePool.hpp"
#include "Krystal.Lib/Core/Visitor.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  template <typename CSSType>
  struct CSSValueCreation;

  struct CSSValueCreationInvoker
  {
    template <typename CSSType, typename... Args>
    KRYS_NODISCARD Ref<CSSValue> operator()(CSSValuePool &pool, const CSSType &value,
                                            Args &&...args) const noexcept
    {
      return CSSValueCreation<CSSType> {}(pool, value, std::forward<Args>(args)...);
    }
  };

  constexpr CSSValueCreationInvoker CreateCSSValue {};

  KRYS_NODISCARD Ref<CSSValue> CreatePrimitiveCSSValue(CSSValueId keyword) noexcept;

  KRYS_NODISCARD Ref<CSSValue> CreatePrimitiveCSSValue(const CustomIdentifier &identifer) noexcept;

  KRYS_NODISCARD Ref<CSSValue> CreatePrimitiveCSSValue(const PropertyIdentifier &identifer) noexcept;

  KRYS_NODISCARD Ref<CSSValue> CreatePrimitiveCSSValue(const CSSOMStringAtom &str) noexcept;

  KRYS_NODISCARD Ref<CSSValue> CreatePrimitiveCSSValue(const CSSOMString &str) noexcept;

  KRYS_NODISCARD Ref<CSSValue> CreateFunctionCSSValue(CSSValueId name, Ref<CSSValue> &&args) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<CSSValue> CreateCoalescingPairCSSValue(Ref<CSSValue> &&, Ref<CSSValue> &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<CSSValue>
    CreateCoalescingPairCSSValue<SerializationSeparatorType::Space>(Ref<CSSValue> &&,
                                                                    Ref<CSSValue> &&) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<CSSValue> CreateCoalescingQuadCSSValue(Ref<CSSValue> &&, Ref<CSSValue> &&,
                                                            Ref<CSSValue> &&, Ref<CSSValue> &&) noexcept;
  template <>
  KRYS_NODISCARD Ref<CSSValue> CreateCoalescingQuadCSSValue<SerializationSeparatorType::Space>(
    Ref<CSSValue> &&, Ref<CSSValue> &&, Ref<CSSValue> &&, Ref<CSSValue> &&) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<CSSValue> CreateListCSSValue(CSSValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<CSSValue>
    CreateListCSSValue<SerializationSeparatorType::Space>(CSSValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<CSSValue>
    CreateListCSSValue<SerializationSeparatorType::Comma>(CSSValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<CSSValue>
    CreateListCSSValue<SerializationSeparatorType::Slash>(CSSValueListBuilder &&) noexcept;

  // Constrained for `TreatAsVariantLike`.
  template <VariantLike CSSType>
  struct CSSValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<CSSValue> operator()(CSSValuePool &pool, const CSSType &value, Args &&...args) noexcept
    {
      return Krys::SwitchOn(value, [&](const auto &alternative)
                            { return CreateCSSValue(pool, alternative, std::forward<Args>(args)...); });
    }
  };

  // Constrained for `TreatAsTupleLike`
  template <TupleLike CSSType>
  struct CSSValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<CSSValue> operator()(CSSValuePool &pool, const CSSType &value, Args &&...args) noexcept
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
        CSSValueListBuilder list;

        auto caller = Krys::CreateVisitor(
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

        return CreateListCSSValue<SerializationSeparator<CSSType>>(Krys::Move(list));
      }
    }
  };

  // Constrained for `TreatAsRangeLike`
  template <RangeLike CSSType>
  struct CSSValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<CSSValue> operator()(CSSValuePool &pool, const CSSType &value, Args &&...args) noexcept
    {
      CSSValueListBuilder list;
      for (const auto &element : value)
      {
        list.push_back(CreateCSSValue(pool, element, std::forward<Args>(args)...));
      }

      return CreateListCSSValue<SerializationSeparator<CSSType>>(Krys::Move(list));
    }
  };

  // Specialization for `Constant`.
  template <CSSValueId Id>
  struct CSSValueCreation<ValueKeywordConstant<Id>>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<CSSValue> operator()(CSSValuePool &, const ValueKeywordConstant<Id> &,
                                            Args &&...) noexcept
    {
      return CreatePrimitiveCSSValue(Id);
    }
  };

  // Specialization for `CustomIdentifier`.
  template <>
  struct CSSValueCreation<CustomIdentifier>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<CSSValue> operator()(CSSValuePool &, const CustomIdentifier &customIdentifier,
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
    Ref<CSSValue> operator()(CSSValuePool &, const PropertyIdentifier &propertyIdentifier, Args &&...)
    {
      return CreatePrimitiveCSSValue(propertyIdentifier);
    }
  };

  // Specialization for `CSSOMStringAtom`.
  template <>
  struct CSSValueCreation<CSSOMStringAtom>
  {
    template <typename... Args>
    Ref<CSSValue> operator()(CSSValuePool &, const CSSOMStringAtom &string, Args &&...)
    {
      return CreatePrimitiveCSSValue(string);
    }
  };

  // Specialization for `CSSOMString`.
  template <>
  struct CSSValueCreation<CSSOMString>
  {
    template <typename... Args>
    Ref<CSSValue> operator()(CSSValuePool &, const CSSOMString &string, Args &&...)
    {
      return CreatePrimitiveCSSValue(string);
    }
  };

  // Specialization for `FunctionNotation`.
  template <CSSValueId Name, typename CSSType>
  struct CSSValueCreation<FunctionNotation<Name, CSSType>>
  {
    template <typename... Args>
    Ref<CSSValue> operator()(CSSValuePool &pool, const FunctionNotation<Name, CSSType> &value, Args &&...args)
    {
      return CreateFunctionCSSValue(value.name,
                                  CreateCSSValue(pool, value.parameters, std::forward<Args>(args)...));
    }
  };

}