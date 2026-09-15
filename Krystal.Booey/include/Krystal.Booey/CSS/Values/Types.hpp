#pragma once

#include "Krystal.Booey/Core/IterationStatus.hpp"
#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/URL/Url.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/Apply.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  class ValuePool;

#pragma region Serialization

  struct SerializationContext;

  template <typename CSSType>
  struct Serializer;

  struct SerializeInvoker
  {
    template <typename CSSType, typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context, const CSSType &value,
                    Rest &&...rest) const noexcept
    {
      Serializer<CSSType> {}(builder, context, value, std::forward<Rest>(rest)...);
    }

    template <typename CSSType, typename... Rest>
    KRYS_NODISCARD CSSOMString operator()(const SerializationContext &context, const CSSType &value,
                                          Rest &&...rest) const noexcept
    {
      CSSOMString builder;
      this->operator()(builder, context, value, std::forward<Rest>(rest)...);
      return builder;
    }
  };

  constexpr SerializeInvoker Serialize {};

  void SerializeCustomIdentifier(CSSOMString &, const SerializationContext &,
                                 const CustomIdentifier &) noexcept;

  void SerializePropertyIdentifier(CSSOMString &, const SerializationContext &,
                                   const PropertyIdentifier &) noexcept;

  void SerializeString(CSSOMString &, const SerializationContext &, const CSSOMStringAtom &) noexcept;

  void SerializeString(CSSOMString &, const SerializationContext &, const CSSOMString &) noexcept;

  template <typename CSSType, typename... Rest>
  void SerializeOnOptionalLike(CSSOMString &builder, const SerializationContext &context,
                               const CSSType &value, Rest &&...rest) noexcept
  {
    if (!value)
    {
      return;
    }

    Serialize(builder, context, *value, std::forward<Rest>(rest)...);
  }

  template <typename CSSType, typename... Rest>
  void SerializeOnTupleLike(CSSOMString &builder, const SerializationContext &context, const CSSType &value,
                            string separator, Rest &&...rest) noexcept
  {
    string swappedSeparator = "";
    auto caller = krys::CreateVisitor(
      [&]<OptionalLike T>(const T &element)
      {
        if (!element)
        {
          return;
        }

        builder.append(std::exchange(swappedSeparator, separator));
        Serialize(builder, context, *element, rest...);
      },
      [&](const auto &element)
      {
        builder.append(std::exchange(swappedSeparator, separator));
        Serialize(builder, context, element, rest...);
      });

    std::apply([&](const auto &...x) { (..., caller(x)); }, value);
  }

  template <typename CSSType, typename... Rest>
  void SerializeOnTupleLikeCoalescing(CSSOMString &builder, const SerializationContext &context,
                                      const CSSType &value, string separator, Rest &&...rest) noexcept
  {
    if constexpr (std::tuple_size_v<CSSType> == 2)
    {
      if (get<0>(value) != get<1>(value))
      {
        SerializeOnTupleLike(builder, context, std::tuple {get<0>(value), get<1>(value)}, separator,
                             std::forward<Rest>(rest)...);
        return;
      }

      Serialize(builder, context, get<0>(value), std::forward<Rest>(rest)...);
    }
    else if constexpr (std::tuple_size_v<CSSType> == 4)
    {
      if (get<3>(value) != get<1>(value))
      {
        SerializeOnTupleLike(builder, context,
                             std::tuple {get<0>(value), get<1>(value), get<2>(value), get<3>(value)},
                             separator, std::forward<Rest>(rest)...);
        return;
      }

      if (get<2>(value) != get<0>(value))
      {
        SerializeOnTupleLike(builder, context, std::tuple {get<0>(value), get<1>(value), get<2>(value)},
                             separator, std::forward<Rest>(rest)...);
        return;
      }

      if (get<1>(value) != get<0>(value))
      {
        SerializeOnTupleLike(builder, context, std::tuple {get<0>(value), get<1>(value)}, separator,
                             std::forward<Rest>(rest)...);
        return;
      }
      Serialize(builder, context, get<0>(value), std::forward<Rest>(rest)...);
    }
  }

  template <typename CSSType, typename... Rest>
  void SerializeOnRangeLike(CSSOMString &builder, const SerializationContext &context, const CSSType &value,
                            string separator, Rest &&...rest) noexcept
  {
    string swappedSeparator = "";
    for (const auto &element : value)
    {
      builder.append(std::exchange(swappedSeparator, separator));
      Serialize(builder, context, element, rest...);
    }
  }

  template <typename CSSType, typename... Rest>
  void SerializeOnVariantLike(CSSOMString &builder, const SerializationContext &context, const CSSType &value,
                              Rest &&...rest) noexcept
  {
    krys::SwitchOn(value, [&](const auto &alternative)
                   { Serialize(builder, context, alternative, std::forward<Rest>(rest)...); });
  }

  // Constrained for `TreatAsEmptyLike`.
  template <EmptyLike CSSType>
  struct Serializer<CSSType>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &, const CSSType &, Rest &&...) noexcept
    {
    }
  };

  // Constrained for `TreatAsOptionalLike`.
  template <OptionalLike CSSType>
  struct Serializer<CSSType>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context, const CSSType &value,
                    Rest &&...rest) noexcept
    {
      SerializeOnOptionalLike(builder, context, value, std::forward<Rest>(rest)...);
    }
  };

  // Constrained for `TreatAsTupleLike`.
  template <TupleLike CSSType>
  struct Serializer<CSSType>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context, const CSSType &value,
                    Rest &&...rest) noexcept
    {
      if constexpr (SerializationCoalescing<CSSType> == SerializationCoalescingType::Minimal)
      {
        SerializeOnTupleLikeCoalescing(builder, context, value, SerializationSeparatorString<CSSType>,
                                       std::forward<Rest>(rest)...);
      }
      else
      {
        SerializeOnTupleLike(builder, context, value, SerializationSeparatorString<CSSType>,
                             std::forward<Rest>(rest)...);
      }
    }
  };

  // Constrained for `TreatAsRangeLike`.
  template <RangeLike CSSType>
  struct Serializer<CSSType>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context, const CSSType &value,
                    Rest &&...rest) noexcept
    {
      SerializeOnRangeLike(builder, context, value, SerializationSeparatorString<CSSType>,
                           std::forward<Rest>(rest)...);
    }
  };

  // Constrained for `TreatAsVariantLike`.
  template <VariantLike CSSType>
  struct Serializer<CSSType>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context, const CSSType &value,
                    Rest &&...rest) noexcept
    {
      SerializeOnVariantLike(builder, context, value, std::forward<Rest>(rest)...);
    }
  };

  // Specialization for `KeywordValueConstant`.
  template <ValueId C>
  struct Serializer<KeywordValueConstant<C>>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &, const KeywordValueConstant<C> &value,
                    Rest &&...) noexcept
    {
      builder.append(ToLower(value.value));
    }
  };

  // Specialization for `CustomIdentifier`.
  template <>
  struct Serializer<CustomIdentifier>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context, const CustomIdentifier &value,
                    Rest &&...) noexcept
    {
      SerializeCustomIdentifier(builder, context, value);
    }
  };

  // Specialization for `PropertyIdentifier`.
  template <>
  struct Serializer<PropertyIdentifier>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context,
                    const PropertyIdentifier &value, Rest &&...) noexcept
    {
      SerializePropertyIdentifier(builder, context, value);
    }
  };

  // Specialization for `CSSOMString`.
  template <>
  struct Serializer<CSSOMString>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context, const CSSOMString &value,
                    Rest &&...) noexcept
    {
      SerializeString(builder, context, value);
    }
  };

  // Specialization for `CSSOMStringAtom`.
  template <>
  struct Serializer<CSSOMStringAtom>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context, const CSSOMStringAtom &value,
                    Rest &&...) noexcept
    {
      SerializeString(builder, context, value);
    }
  };

  // Specialization for `FunctionNotation`.
  template <ValueId Name, typename CSSType>
  struct Serializer<FunctionNotation<Name, CSSType>>
  {
    template <typename... Rest>
    void operator()(CSSOMString &builder, const SerializationContext &context,
                    const FunctionNotation<Name, CSSType> &value, Rest &&...rest) noexcept
    {
      builder.append(ToLower(value.name), '(');
      Serialize(builder, context, value.parameters, std::forward<Rest>(rest)...);
      builder.append(')');
    }
  };

#pragma endregion

#pragma region ComputedStyleDependencies

  template <typename CSSType>
  struct ComputedStyleDependenciesCollector;

  struct ComputedStyleDependenciesCollectorInvoker
  {
    template <typename CSSType>
    void operator()(ComputedStyleDependencies &dependencies, const CSSType &value) const noexcept
    {
      ComputedStyleDependenciesCollector<CSSType> {}(dependencies, value);
    }

    template <typename CSSType>
    KRYS_NODISCARD ComputedStyleDependencies operator()(const CSSType &value) const noexcept
    {
      ComputedStyleDependencies dependencies;
      this->operator()(dependencies, value);
      return dependencies;
    }
  };

  constexpr ComputedStyleDependenciesCollectorInvoker CollectComputedStyleDependencies {};

  template <typename CSSType>
  void CollectComputedStyleDependenciesOnOptionalLike(ComputedStyleDependencies &dependencies,
                                                      const CSSType &value) noexcept
  {
    if (!value)
    {
      return;
    }

    CollectComputedStyleDependencies(dependencies, *value);
  }

  template <typename CSSType>
  void CollectComputedStyleDependenciesOnTupleLike(ComputedStyleDependencies &dependencies,
                                                   const CSSType &value) noexcept
  {
    krys::apply([&](const auto &...x) { (..., CollectComputedStyleDependencies(dependencies, x)); }, value);
  }

  template <typename CSSType>
  void CollectComputedStyleDependenciesOnRangeLike(ComputedStyleDependencies &dependencies,
                                                   const CSSType &value) noexcept
  {
    for (const auto &element : value)
    {
      CollectComputedStyleDependencies(dependencies, element);
    }
  }

  template <typename CSSType>
  void CollectComputedStyleDependenciesOnVariantLike(ComputedStyleDependencies &dependencies,
                                                     const CSSType &value) noexcept
  {
    krys::SwitchOn(value, [&](const auto &alternative)
                   { CollectComputedStyleDependencies(dependencies, alternative); });
  }

  // Constrained for `TreatAsEmptyLike`.
  template <EmptyLike CSSType>
  struct ComputedStyleDependenciesCollector<CSSType>
  {
    void operator()(ComputedStyleDependencies &, const CSSType &) noexcept
    {
    }
  };

  // Constrained for `TreatAsOptionalLike`.
  template <OptionalLike CSSType>
  struct ComputedStyleDependenciesCollector<CSSType>
  {
    void operator()(ComputedStyleDependencies &dependencies, const CSSType &value) noexcept
    {
      CollectComputedStyleDependenciesOnOptionalLike(dependencies, value);
    }
  };

  // Constrained for `TreatAsTupleLike`.
  template <TupleLike CSSType>
  struct ComputedStyleDependenciesCollector<CSSType>
  {
    void operator()(ComputedStyleDependencies &dependencies, const CSSType &value) noexcept
    {
      CollectComputedStyleDependenciesOnTupleLike(dependencies, value);
    }
  };

  // Constrained for `TreatAsRangeLike`.
  template <RangeLike CSSType>
  struct ComputedStyleDependenciesCollector<CSSType>
  {
    void operator()(ComputedStyleDependencies &dependencies, const CSSType &value) noexcept
    {
      CollectComputedStyleDependenciesOnRangeLike(dependencies, value);
    }
  };

  // Constrained for `TreatAsVariantLike`.
  template <VariantLike CSSType>
  struct ComputedStyleDependenciesCollector<CSSType>
  {
    void operator()(ComputedStyleDependencies &dependencies, const CSSType &value) noexcept
    {
      CollectComputedStyleDependenciesOnVariantLike(dependencies, value);
    }
  };

  // Specialization for `KeywordValueConstant`.
  template <ValueId C>
  struct ComputedStyleDependenciesCollector<KeywordValueConstant<C>>
  {
    constexpr void operator()(ComputedStyleDependencies &, const KeywordValueConstant<C> &) noexcept
    {
      // Nothing to do.
    }
  };

  // Specialization for `CustomIdentifier`.
  template <>
  struct ComputedStyleDependenciesCollector<CustomIdentifier>
  {
    constexpr void operator()(ComputedStyleDependencies &, const CustomIdentifier &) noexcept
    {
      // Nothing to do.
    }
  };

  // Specialization for `PropertyIdentifier`.
  template <>
  struct ComputedStyleDependenciesCollector<PropertyIdentifier>
  {
    constexpr void operator()(ComputedStyleDependencies &, const PropertyIdentifier &) noexcept
    {
      // Nothing to do.
    }
  };

  // Specialization for `CSSOMStringAtom`.
  template <>
  struct ComputedStyleDependenciesCollector<CSSOMStringAtom>
  {
    constexpr void operator()(ComputedStyleDependencies &, const CSSOMStringAtom &) noexcept
    {
      // Nothing to do.
    }
  };

  // Specialization for `CSSOMString`.
  template <>
  struct ComputedStyleDependenciesCollector<CSSOMString>
  {
    constexpr void operator()(ComputedStyleDependencies &, const CSSOMString &) noexcept
    {
      // Nothing to do.
    }
  };

  // Specialization for `url::Url`.
  template <>
  struct ComputedStyleDependenciesCollector<url::Url>
  {
    constexpr void operator()(ComputedStyleDependencies &, const url::Url &)
    {
      // Nothing to do.
    }
  };
#pragma endregion

#pragma region ValueChildrenVisitor

  template <typename CSSType>
  struct ValueChildrenVisitor;

  struct ValueChildrenVisitorInvoker
  {
    template <typename CSSType>
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const CSSType &value) const
    {
      return ValueChildrenVisitor<CSSType> {}(func, value);
    }
  };

  constexpr ValueChildrenVisitorInvoker VisitValueChildren {};

  template <typename CSSType>
  KRYS_NODISCARD IterationStatus VisitValueChildrenOnOptionalLike(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func, const CSSType &value)
  {
    return value ? VisitValueChildren(func, *value) : IterationStatus::Continue;
  }

  template <typename CSSType>
  KRYS_NODISCARD IterationStatus VisitValueChildrenOnTupleLike(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func, const CSSType &value)
  {
    // Process a single element of the tuple-like, updating result, and return true if result ==
    // IterationStatus::Done to short circuit the fold in the apply lambda.
    auto process = [&](const auto &x, IterationStatus &result) -> bool
    {
      result = VisitValueChildren(func, x);
      return result == IterationStatus::Done;
    };

    return krys::apply(
      [&](const auto &...x)
      {
        auto result = IterationStatus::Continue;
        (process(x, result) || ...);
        return result;
      },
      value);
  }

  template <typename CSSType>
  KRYS_NODISCARD IterationStatus VisitValueChildrenOnRangeLike(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func, const CSSType &value)
  {
    for (const auto &element : value)
    {
      if (VisitValueChildren(func, element) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    return IterationStatus::Continue;
  }

  template <typename CSSType>
  KRYS_NODISCARD IterationStatus VisitValueChildrenOnVariantLike(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func, const CSSType &value)
  {
    return krys::SwitchOn(value,
                          [&](const auto &alternative) { return VisitValueChildren(func, alternative); });
  }

  // Constrained for `TreatAsEmptyLike`.
  template <EmptyLike CSSType>
  struct ValueChildrenVisitor<CSSType>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &, const CSSType &)
    {
      return IterationStatus::Continue;
    }
  };

  // Constrained for `TreatAsOptionalLike`.
  template <OptionalLike CSSType>
  struct ValueChildrenVisitor<CSSType>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func, const CSSType &value)
    {
      return VisitValueChildrenOnOptionalLike(func, value);
    }
  };

  // Constrained for `TreatAsTupleLike`.
  template <TupleLike CSSType>
  struct ValueChildrenVisitor<CSSType>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func, const CSSType &value)
    {
      return VisitValueChildrenOnTupleLike(func, value);
    }
  };

  // Constrained for `TreatAsRangeLike`.
  template <RangeLike CSSType>
  struct ValueChildrenVisitor<CSSType>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func, const CSSType &value)
    {
      return VisitValueChildrenOnRangeLike(func, value);
    }
  };

  // Constrained for `TreatAsVariantLike`.
  template <VariantLike CSSType>
  struct ValueChildrenVisitor<CSSType>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func, const CSSType &value)
    {
      return VisitValueChildrenOnVariantLike(func, value);
    }
  };

  // Specialization for `Constant`.
  template <ValueId C>
  struct ValueChildrenVisitor<KeywordValueConstant<C>>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const KeywordValueConstant<C> &)
    {
      return IterationStatus::Continue;
    }
  };

  // Specialization for `CustomIdentifier`.
  template <>
  struct ValueChildrenVisitor<CustomIdentifier>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const CustomIdentifier &)
    {
      return IterationStatus::Continue;
    }
  };

  // Specialization for `PropertyIdentifier`.
  template <>
  struct ValueChildrenVisitor<PropertyIdentifier>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const PropertyIdentifier &)
    {
      return IterationStatus::Continue;
    }
  };

  // Specialization for `CSSOMStringAtom`.
  template <>
  struct ValueChildrenVisitor<CSSOMStringAtom>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const CSSOMStringAtom &)
    {
      return IterationStatus::Continue;
    }
  };

  // Specialization for `CSSOMString`.
  template <>
  struct ValueChildrenVisitor<CSSOMString>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const CSSOMString &)
    {
      return IterationStatus::Continue;
    }
  };

  // Specialization for `url::Url`.
  template <>
  struct ValueChildrenVisitor<url::Url>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const url::Url &)
    {
      return IterationStatus::Continue;
    }
  };

#pragma endregion

#pragma region ValueCreation

  template <typename CSSType>
  struct ValueCreation;

  struct ValueCreationInvoker
  {
    template <typename CSSType, typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &pool, const CSSType &value, Args &&...args) const noexcept
    {
      return ValueCreation<CSSType> {}(pool, value, std::forward<Args>(args)...);
    }
  };

  constexpr ValueCreationInvoker CreateValue {};

  KRYS_NODISCARD Ref<Value> CreatePrimitiveValue(ValueId keyword) noexcept;

  KRYS_NODISCARD Ref<Value> CreatePrimitiveValue(const CustomIdentifier &identifer) noexcept;

  KRYS_NODISCARD Ref<Value> CreatePrimitiveValue(const PropertyIdentifier &identifer) noexcept;

  KRYS_NODISCARD Ref<Value> CreatePrimitiveValue(const CSSOMStringAtom &str) noexcept;

  KRYS_NODISCARD Ref<Value> CreatePrimitiveValue(const CSSOMString &str) noexcept;

  KRYS_NODISCARD Ref<Value> CreateFunctionValue(ValueId name, Ref<Value> &&args) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<Value> CreateCoalescingPairValue(Ref<Value> &&, Ref<Value> &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<Value>
    CreateCoalescingPairValue<SerializationSeparatorType::Space>(Ref<Value> &&, Ref<Value> &&) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<Value> CreateCoalescingQuadValue(Ref<Value> &&, Ref<Value> &&, Ref<Value> &&,
                                                      Ref<Value> &&) noexcept;
  template <>
  KRYS_NODISCARD Ref<Value>
    CreateCoalescingQuadValue<SerializationSeparatorType::Space>(Ref<Value> &&, Ref<Value> &&, Ref<Value> &&,
                                                                 Ref<Value> &&) noexcept;

  template <SerializationSeparatorType>
  KRYS_NODISCARD Ref<Value> CreateListValue(ValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<Value> CreateListValue<SerializationSeparatorType::Space>(ValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<Value> CreateListValue<SerializationSeparatorType::Comma>(ValueListBuilder &&) noexcept;

  template <>
  KRYS_NODISCARD Ref<Value> CreateListValue<SerializationSeparatorType::Slash>(ValueListBuilder &&) noexcept;

  // Constrained for `TreatAsVariantLike`.
  template <VariantLike CSSType>
  struct ValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &pool, const CSSType &value, Args &&...args) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &alternative)
                            { return CreateValue(pool, alternative, std::forward<Args>(args)...); });
    }
  };

  // Constrained for `TreatAsTupleLike`
  template <TupleLike CSSType>
  struct ValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &pool, const CSSType &value, Args &&...args) noexcept
    {
      if constexpr (std::tuple_size_v<CSSType> == 1
                    && SerializationSeparator<CSSType> == SerializationSeparatorType::None)
      {
        return CreateValue(pool, get<0>(value), std::forward<Args>(args)...);
      }
      else if constexpr (std::tuple_size_v<CSSType> == 2
                         && SerializationCoalescing<CSSType> == SerializationCoalescingType::Minimal)
      {
        return CreateCoalescingPairValue<SerializationSeparator<CSSType>>(
          CreateValue(pool, get<0>(value), std::forward<Args>(args)...),
          CreateValue(pool, get<1>(value), std::forward<Args>(args)...));
      }
      else if constexpr (std::tuple_size_v<CSSType> == 4
                         && SerializationCoalescing<CSSType> == SerializationCoalescingType::Minimal)
      {
        return CreateCoalescingQuadValue<SerializationSeparator<CSSType>>(
          CreateValue(pool, get<0>(value), std::forward<Args>(args)...),
          CreateValue(pool, get<1>(value), std::forward<Args>(args)...),
          CreateValue(pool, get<2>(value), std::forward<Args>(args)...),
          CreateValue(pool, get<3>(value), std::forward<Args>(args)...));
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

            list.push_back(CreateValue(pool, *element, std::forward<Args>(args)...));
          },
          [&](const auto &element)
          { list.push_back(CreateValue(pool, element, std::forward<Args>(args)...)); });

        apply([&](const auto &...x) { (..., caller(x)); }, value);

        return CreateListValue<SerializationSeparator<CSSType>>(krys::move(list));
      }
    }
  };

  // Constrained for `TreatAsRangeLike`
  template <RangeLike CSSType>
  struct ValueCreation<CSSType>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &pool, const CSSType &value, Args &&...args) noexcept
    {
      ValueListBuilder list;
      for (const auto &element : value)
      {
        list.push_back(CreateValue(pool, element, std::forward<Args>(args)...));
      }

      return CreateListValue<SerializationSeparator<CSSType>>(krys::move(list));
    }
  };

  // Specialization for `KeywordValueConstant`.
  template <ValueId Id>
  struct ValueCreation<KeywordValueConstant<Id>>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &, const KeywordValueConstant<Id> &, Args &&...) noexcept
    {
      return CreatePrimitiveValue(Id);
    }
  };

  // Specialization for `CustomIdentifier`.
  template <>
  struct ValueCreation<CustomIdentifier>
  {
    template <typename... Args>
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &, const CustomIdentifier &customIdentifier,
                                         Args &&...) noexcept
    {
      return CreatePrimitiveValue(customIdentifier);
    }
  };

  // Specialization for `PropertyIdentifier`.
  template <>
  struct ValueCreation<PropertyIdentifier>
  {
    template <typename... Args>
    Ref<Value> operator()(ValuePool &, const PropertyIdentifier &propertyIdentifier, Args &&...) noexcept
    {
      return CreatePrimitiveValue(propertyIdentifier);
    }
  };

  // Specialization for `CSSOMStringAtom`.
  template <>
  struct ValueCreation<CSSOMStringAtom>
  {
    template <typename... Args>
    Ref<Value> operator()(ValuePool &, const CSSOMStringAtom &string, Args &&...) noexcept
    {
      return CreatePrimitiveValue(string);
    }
  };

  // Specialization for `CSSOMString`.
  template <>
  struct ValueCreation<CSSOMString>
  {
    template <typename... Args>
    Ref<Value> operator()(ValuePool &, const CSSOMString &string, Args &&...) noexcept
    {
      return CreatePrimitiveValue(string);
    }
  };

  // Specialization for `FunctionNotation`.
  template <ValueId Name, typename CSSType>
  struct ValueCreation<FunctionNotation<Name, CSSType>>
  {
    template <typename... Args>
    Ref<Value> operator()(ValuePool &pool, const FunctionNotation<Name, CSSType> &value,
                          Args &&...args) noexcept
    {
      return CreateFunctionValue(value.name,
                                 CreateValue(pool, value.parameters, std::forward<Args>(args)...));
    }
  };

#pragma endregion
}