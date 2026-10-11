#pragma once

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Utils/ZipTuple.hpp"
#include <tuple>
#include <utility>

namespace krys::boo
{
  class RenderStyle;

  struct BlendingContext;

  enum class CompositeOperation : uint8;
}

namespace krys::boo::css
{
  class ToLengthConversionData;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style::lerp
{
  struct Context;
}

namespace krys::boo::style
{
  class BuilderState;

#pragma region ValueRepresentation

  /// @brief All leaf types that want to conform to ValueRepresentation must implement the following:
  ///
  /// template<>
  /// struct krys::boo::style::ValueRepresentation<StyleType>
  /// {
  ///   template<typename... F>
  ///   bool operator()(const StyleType&, F&&... f) noexcept;
  /// };
  template <typename>
  struct ValueRepresentation;

  struct ValueRepresentationInvoker
  {
    template <typename StyleType, typename... F>
    KRYS_NODISCARD decltype(auto) operator()(const StyleType &value, F &&...f) const noexcept
    {
      return ValueRepresentation<StyleType> {}(value, std::forward<F>(f)...);
    }
  };

  constexpr ValueRepresentationInvoker valueRepresentation {};

  /// @brief Types can specialize this and set the value to true to be treated as "non-converting" for css to
  /// style / style to css conversion algorithms. This means the type is identical for both css and style
  /// systems (e.g. a constant value or an enum).
  template <typename>
  constexpr bool TreatAsNonConverting = false;

  /// @brief The `NonConverting` concept can be used to filter to types that specialize
  /// `TreatAsNonConverting`.
  template <typename T>
  concept NonConverting = TreatAsNonConverting<T>;

  /// @brief Types that are treated as "tuple-like" can have their conversion operations defined automatically
  /// by just defining their type mapping.
  template <typename>
  struct ToStyleMapping;

  template <typename>
  struct ToCSSMapping;

// Macro to define two-way mapping between a CSS and Style type. This is only needed
// for "tuple-like" types, in lieu of explicit ToCSS/ToStyle specializations.
#define DEFINE_TYPE_MAPPING(css, style)                                                                      \
  template <>                                                                                                \
  struct ToStyleMapping<css>                                                                                 \
  {                                                                                                          \
    using type = style;                                                                                      \
  };                                                                                                         \
  template <>                                                                                                \
  struct ToCSSMapping<style>                                                                                 \
  {                                                                                                          \
    using type = css;                                                                                        \
  };

  /// @brief All non-converting and non-tuple-like conforming types must implement the following for
  /// conversions: template<> struct krys::boo::style::ToCSS<StyleType>
  /// {
  ///   CSSType operator()(const StyleType&, const RenderStyle&) noexcept;
  /// };
  ///
  /// template<>
  /// struct krys::boo::style::ToStyle<CSSType>
  /// {
  ///   StyleType operator()(const CSSType&, const ToLengthConversionData&) noexcept;
  ///   StyleType operator()(const CSSType&, const BuilderState&) noexcept;
  ///   StyleType operator()(const CSSType&, NoConversionDataRequiredTag) noexcept;
  /// };

  template <typename>
  struct ToCSS;

  template <typename>
  struct ToStyle;

  // MARK: Common Types.

  // Specialize `TreatAsNonConverting` for `css::KeywordValueConstant<C>`, to indicate that its type does not
  // change from the CSS representation.
  template <css::ValueId C>
  constexpr bool TreatAsNonConverting<css::KeywordValueConstant<C>> = true;

  // Specialize `TreatAsNonConverting` for `css::CustomIdentifier`, to indicate that its type does not change
  // from the CSS representation.
  template <>
  constexpr bool TreatAsNonConverting<css::CustomIdentifier> = true;

  // Specialize `TreatAsNonConverting` for `css::PropertyIdentifier`, to indicate that its type does not
  // change from the CSS representation.
  template <>
  constexpr bool TreatAsNonConverting<css::PropertyIdentifier> = true;

  // Specialize `TreatAsNonConverting` for `StringAtom`, to indicate that its type does not change from the
  // CSS representation.
  template <>
  constexpr bool TreatAsNonConverting<StringAtom> = true;

  // Specialize `TreatAsNonConverting` for `utf8_string`, to indicate that its type does not change from the
  // CSS representation.
  template <>
  constexpr bool TreatAsNonConverting<utf8_string> = true;

  // Specialize `TreatAsNonConverting` for `css::Url`, to indicate that its type does not change from the
  // CSS representation.
  // template <>
  // constexpr bool TreatAsNonConverting<css::url> = true;

#pragma endregion

#pragma region ToCSS

  struct ToCSSInvoker
  {
    template <typename StyleType, typename... Rest>
    decltype(auto) operator()(const StyleType &styleType, const RenderStyle &style, Rest &&...rest) const
    {
      return ToCSS<StyleType> {}(styleType, style, std::forward<Rest>(rest)...);
    }
  };

  constexpr ToCSSInvoker ConvertToCSS {};

  // Conversion Utility Types
  template <typename StyleType>
  using CSSType =
    decay_t<decltype(ConvertToCSS(std::declval<const StyleType &>(), std::declval<const RenderStyle &>()))>;

  template <typename To, typename From, typename... Rest>
  KRYS_NODISCARD auto ToCSSOnTupleLike(const From &tupleLike, Rest &&...rest) noexcept -> To
  {
    return krys::apply([&](const auto &...x) { return To {ConvertToCSS(x, rest...)...}; }, tupleLike);
  }

  // Standard NonConverting type mappings (identity mappings):
  template <NonConverting T>
  struct ToCSSMapping<T>
  {
    using type = T;
  };

  // Standard Optional-Like type mappings:
  template <typename T>
  struct ToCSSMapping<Maybe<T>>
  {
    using type = Maybe<CSSType<T>>;
  };

  template <typename T>
  struct ToCSSMapping<Markable<T>>
  {
    using type = Markable<CSSType<T>>;
  };

  // Standard Tuple-Like type mappings:
  template <typename... Ts>
  struct ToCSSMapping<std::tuple<Ts...>>
  {
    using type = std::tuple<CSSType<Ts>...>;
  };

  template <css::ValueId C, typename T>
  struct ToCSSMapping<css::FunctionNotation<C, T>>
  {
    using type = css::FunctionNotation<C, CSSType<T>>;
  };

  template <typename T, size_t N>
  struct ToCSSMapping<css::SpaceSeparatedArray<T, N>>
  {
    using type = css::SpaceSeparatedArray<CSSType<T>, N>;
  };

  template <typename T, size_t N>
  struct ToCSSMapping<css::CommaSeparatedArray<T, N>>
  {
    using type = css::CommaSeparatedArray<CSSType<T>, N>;
  };

  template <typename... Ts>
  struct ToCSSMapping<css::SpaceSeparatedTuple<Ts...>>
  {
    using type = css::SpaceSeparatedTuple<CSSType<Ts>...>;
  };

  template <typename... Ts>
  struct ToCSSMapping<css::CommaSeparatedTuple<Ts...>>
  {
    using type = css::CommaSeparatedTuple<CSSType<Ts>...>;
  };

  template <typename T>
  struct ToCSSMapping<css::SpaceSeparatedPoint<T>>
  {
    using type = css::SpaceSeparatedPoint<CSSType<T>>;
  };

  template <typename T>
  struct ToCSSMapping<css::SpaceSeparatedSize<T>>
  {
    using type = css::SpaceSeparatedSize<CSSType<T>>;
  };

  template <typename T>
  struct ToCSSMapping<css::SpaceSeparatedRectEdges<T>>
  {
    using type = css::SpaceSeparatedRectEdges<CSSType<T>>;
  };

  template <typename T>
  struct ToCSSMapping<css::CommaSeparatedRectEdges<T>>
  {
    using type = css::CommaSeparatedRectEdges<CSSType<T>>;
  };

  template <typename T>
  struct ToCSSMapping<css::MinimallySerializingSpaceSeparatedPair<T>>
  {
    using type = css::MinimallySerializingSpaceSeparatedPair<CSSType<T>>;
  };

  template <typename T>
  struct ToCSSMapping<css::MinimallySerializingSpaceSeparatedPoint<T>>
  {
    using type = css::MinimallySerializingSpaceSeparatedPoint<CSSType<T>>;
  };

  template <typename T>
  struct ToCSSMapping<css::MinimallySerializingSpaceSeparatedSize<T>>
  {
    using type = css::MinimallySerializingSpaceSeparatedSize<CSSType<T>>;
  };

  template <typename T>
  struct ToCSSMapping<css::MinimallySerializingSpaceSeparatedRectEdges<T>>
  {
    using type = css::MinimallySerializingSpaceSeparatedRectEdges<CSSType<T>>;
  };

  // Standard Range-Like type mappings:
  template <typename T, size_t N>
  struct ToCSSMapping<css::SpaceSeparatedList<T, N>>
  {
    using type = css::SpaceSeparatedList<CSSType<T>, N>;
  };

  template <typename T, size_t N>
  struct ToCSSMapping<css::CommaSeparatedList<T, N>>
  {
    using type = css::CommaSeparatedList<CSSType<T>, N>;
  };

  // Standard Variant-Like type mappings:
  template <typename... Ts>
  struct ToCSSMapping<Variant<Ts...>>
  {
    using type = Variant<CSSType<Ts>...>;
  };

  // Constrained for `TreatAsNonConverting`.
  template <NonConverting StyleType>
  struct ToCSS<StyleType>
  {
    template <typename... Rest>
    KRYS_NODISCARD constexpr StyleType operator()(const StyleType &value, const RenderStyle &,
                                                  Rest &&...) noexcept
    {
      return value;
    }
  };

  // Constrained for `TreatAsOptionalLike`.
  template <css::OptionalLike StyleType>
  struct ToCSS<StyleType>
  {
    using Result = typename ToCSSMapping<StyleType>::type;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const StyleType &value, const RenderStyle &style,
                                     Rest &&...rest) noexcept
    {
      if (value)
      {
        return ConvertToCSS(*value, style, std::forward<Rest>(rest)...);
      }

      return null;
    }
  };

  // Constrained for `TreatAsTupleLike`.
  template <css::TupleLike StyleType>
  struct ToCSS<StyleType>
  {
    using Result = typename ToCSSMapping<StyleType>::type;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const StyleType &value, const RenderStyle &style,
                                     Rest &&...rest) noexcept
    {
      return ToCSSOnTupleLike<Result>(value, style, std::forward<Rest>(rest)...);
    }
  };

  // Constrained for `TreatAsVariantLike`.
  template <css::VariantLike StyleType>
  struct ToCSS<StyleType>
  {
    using Result = typename ToCSSMapping<StyleType>::type;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const StyleType &value, const RenderStyle &style,
                                     Rest &&...rest) noexcept
    {
      return krys::SwitchOn(
        value, [&](const auto &alternative)
        { return Result {ConvertToCSS(alternative, style, std::forward<Rest>(rest)...)}; });
    }
  };

  // Specialization for `css::SpaceSeparatedList`.
  template <typename StyleType, size_t InlineCapacity>
  struct ToCSS<css::SpaceSeparatedList<StyleType, InlineCapacity>>
  {
    using Result = css::SpaceSeparatedList<CSSType<StyleType>, InlineCapacity>;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const css::SpaceSeparatedList<StyleType, InlineCapacity> &value,
                                     const RenderStyle &style, Rest &&...rest) noexcept
    {
      return Result {value.value.template map<typename Result::Container>(
        [&](const auto &x) { return ConvertToCSS(x, style, rest...); })};
    }
  };

  // Specialization for `css::CommaSeparatedList`.
  template <typename StyleType, size_t InlineCapacity>
  struct ToCSS<css::CommaSeparatedList<StyleType, InlineCapacity>>
  {
    using Result = css::CommaSeparatedList<CSSType<StyleType>, InlineCapacity>;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const css::CommaSeparatedList<StyleType, InlineCapacity> &value,
                                     const RenderStyle &style, Rest &&...rest) noexcept
    {
      return Result {value.value.template map<typename Result::Container>(
        [&](const auto &x) { return ConvertToCSS(x, style, rest...); })};
    }
  };

#pragma endregion

#pragma region ToStyle

  // MARK: - Conversion from "CSS" to "Style"

  struct ToStyleInvoker
  {
    template <typename CSSType, typename... Rest>
    KRYS_NODISCARD decltype(auto) operator()(const CSSType &cssType,
                                             const css::ToLengthConversionData &conversionData,
                                             Rest &&...rest) const noexcept
    {
      return ToStyle<CSSType> {}(cssType, conversionData, std::forward<Rest>(rest)...);
    }

    template <typename CSSType, typename... Rest>
    KRYS_NODISCARD decltype(auto) operator()(const CSSType &cssType, const BuilderState &builderState,
                                             Rest &&...rest) const noexcept
    {
      return ToStyle<CSSType> {}(cssType, builderState, std::forward<Rest>(rest)...);
    }

    template <typename CSSType, typename... Rest>
    KRYS_NODISCARD decltype(auto) operator()(const CSSType &cssType, css::NoConversionDataRequiredTag token,
                                             Rest &&...rest) const noexcept
    {
      return ToStyle<CSSType> {}(cssType, token, std::forward<Rest>(rest)...);
    }
  };
  constexpr ToStyleInvoker ConvertToStyle {};

  // Convenience invoker that adds a `NoConversionDataRequiredToken` argument.
  struct ToStyleNoConversionDataRequiredInvoker
  {
    template <typename CSSType, typename... Rest>
    KRYS_NODISCARD decltype(auto) operator()(const CSSType &cssType, Rest &&...rest) const noexcept
    {
      return krys::boo::style::ConvertToStyle(cssType, css::NoConversionDataRequiredTag {},
                                              std::forward<Rest>(rest)...);
    }
  };

  constexpr ToStyleNoConversionDataRequiredInvoker ConvertToStyleNoConversionDataRequired {};

  // Conversion Utilities
  template <typename To, typename From, typename... Rest>
  KRYS_NODISCARD auto ToStyleOnTupleLike(const From &tupleLike, Rest &&...rest) noexcept -> To
  {
    return krys::apply([&](const auto &...x) { return To {ConvertToStyle(x, rest...)...}; }, tupleLike);
  }

  template <typename To, typename From, typename... Rest>
  KRYS_NODISCARD auto ToStyleNoConversionDataRequiredOnTupleLike(const From &tupleLike,
                                                                 Rest &&...rest) noexcept -> To
  {
    return krys::apply([&](const auto &...x)
                       { return To {ConvertToStyleNoConversionDataRequired(x, rest...)...}; }, tupleLike);
  }

  template <typename CSSType>
  using StyleType =
    decay_t<decltype(ConvertToStyle(std::declval<const CSSType &>(), std::declval<const BuilderState &>()))>;

  // Standard NonConverting type mappings (identity mappings):
  template <NonConverting T>
  struct ToStyleMapping<T>
  {
    using type = T;
  };

  // Standard Optional-Like type mappings:
  template <typename T>
  struct ToStyleMapping<Maybe<T>>
  {
    using type = Maybe<StyleType<T>>;
  };

  template <typename T>
  struct ToStyleMapping<Markable<T>>
  {
    using type = Markable<StyleType<T>>;
  };

  // Standard Tuple-Like type mappings:
  template <typename... Ts>
  struct ToStyleMapping<std::tuple<Ts...>>
  {
    using type = std::tuple<StyleType<Ts>...>;
  };

  template <css::ValueId C, typename T>
  struct ToStyleMapping<css::FunctionNotation<C, T>>
  {
    using type = css::FunctionNotation<C, StyleType<T>>;
  };

  template <typename T, size_t N>
  struct ToStyleMapping<css::SpaceSeparatedArray<T, N>>
  {
    using type = css::SpaceSeparatedArray<StyleType<T>, N>;
  };

  template <typename T, size_t N>
  struct ToStyleMapping<css::CommaSeparatedArray<T, N>>
  {
    using type = css::CommaSeparatedArray<StyleType<T>, N>;
  };

  template <typename... Ts>
  struct ToStyleMapping<css::SpaceSeparatedTuple<Ts...>>
  {
    using type = css::SpaceSeparatedTuple<StyleType<Ts>...>;
  };

  template <typename... Ts>
  struct ToStyleMapping<css::CommaSeparatedTuple<Ts...>>
  {
    using type = css::CommaSeparatedTuple<StyleType<Ts>...>;
  };

  template <typename T>
  struct ToStyleMapping<css::SpaceSeparatedPoint<T>>
  {
    using type = css::SpaceSeparatedPoint<StyleType<T>>;
  };

  template <typename T>
  struct ToStyleMapping<css::SpaceSeparatedSize<T>>
  {
    using type = css::SpaceSeparatedSize<StyleType<T>>;
  };

  template <typename T>
  struct ToStyleMapping<css::SpaceSeparatedRectEdges<T>>
  {
    using type = css::SpaceSeparatedRectEdges<StyleType<T>>;
  };

  template <typename T>
  struct ToStyleMapping<css::CommaSeparatedRectEdges<T>>
  {
    using type = css::CommaSeparatedRectEdges<StyleType<T>>;
  };

  template <typename T>
  struct ToStyleMapping<css::MinimallySerializingSpaceSeparatedPair<T>>
  {
    using type = css::MinimallySerializingSpaceSeparatedPair<StyleType<T>>;
  };

  template <typename T>
  struct ToStyleMapping<css::MinimallySerializingSpaceSeparatedPoint<T>>
  {
    using type = css::MinimallySerializingSpaceSeparatedPoint<StyleType<T>>;
  };

  template <typename T>
  struct ToStyleMapping<css::MinimallySerializingSpaceSeparatedSize<T>>
  {
    using type = css::MinimallySerializingSpaceSeparatedSize<StyleType<T>>;
  };

  template <typename T>
  struct ToStyleMapping<css::MinimallySerializingSpaceSeparatedRectEdges<T>>
  {
    using type = css::MinimallySerializingSpaceSeparatedRectEdges<StyleType<T>>;
  };

  // Standard Range-Like type mappings:
  template <typename T, size_t N>
  struct ToStyleMapping<css::SpaceSeparatedList<T, N>>
  {
    using type = css::SpaceSeparatedList<StyleType<T>, N>;
  };

  template <typename T, size_t N>
  struct ToStyleMapping<css::CommaSeparatedList<T, N>>
  {
    using type = css::CommaSeparatedList<StyleType<T>, N>;
  };

  // Standard Variant-Like type mappings:
  template <typename... Ts>
  struct ToStyleMapping<Variant<Ts...>>
  {
    using type = Variant<StyleType<Ts>...>;
  };

  // Constrained for `TreatAsNonConverting`.
  template <NonConverting CSSType>
  struct ToStyle<CSSType>
  {
    template <typename... Rest>
    KRYS_NODISCARD constexpr CSSType operator()(const CSSType &value, Rest &&...) noexcept
    {
      return value;
    }
  };

  // Constrained for `TreatAsOptionalLike`.
  template <css::OptionalLike CSSType>
  struct ToStyle<CSSType>
  {
    using Result = typename ToStyleMapping<CSSType>::type;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const CSSType &value, Rest &&...rest) noexcept
    {
      if (value)
      {
        return ConvertToStyle(*value, std::forward<Rest>(rest)...);
      }

      return null;
    }
  };

  // Constrained for `TreatAsTupleLike`.
  template <css::TupleLike CSSType>
  struct ToStyle<CSSType>
  {
    using Result = typename ToStyleMapping<CSSType>::type;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const CSSType &value, Rest &&...rest) noexcept
    {
      return ToStyleOnTupleLike<Result>(value, std::forward<Rest>(rest)...);
    }
  };

  // Constrained for `TreatAsVariantLike`.
  template <css::VariantLike CSSType>
  struct ToStyle<CSSType>
  {
    using Result = typename ToStyleMapping<CSSType>::type;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const CSSType &value, Rest &&...rest) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &alternative)
                            { return Result {ConvertToStyle(alternative, std::forward<Rest>(rest)...)}; });
    }
  };

  // Specialization for `css::SpaceSeparatedList`.
  template <typename CSSType, size_t InlineCapacity>
  struct ToStyle<css::SpaceSeparatedList<CSSType, InlineCapacity>>
  {
    using Result = css::SpaceSeparatedList<StyleType<CSSType>, InlineCapacity>;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const css::SpaceSeparatedList<CSSType, InlineCapacity> &value,
                                     Rest &&...rest) noexcept
    {
      return Result {value.Value.template map<typename Result::Container>(
        [&](const auto &x) { return ConvertToStyle(x, rest...); })};
    }
  };

  // Specialization for `css::CommaSeparatedList`.
  template <typename CSSType, size_t InlineCapacity>
  struct ToStyle<css::CommaSeparatedList<CSSType, InlineCapacity>>
  {
    using Result = css::CommaSeparatedList<StyleType<CSSType>, InlineCapacity>;

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const css::CommaSeparatedList<CSSType, InlineCapacity> &value,
                                     Rest &&...rest) noexcept
    {
      return Result {value.value.template map<typename Result::Container>(
        [&](const auto &x) { return ConvertToStyle(x, rest...); })};
    }
  };

#pragma endregion

#pragma region CSSValueCreation

  /// @brief Conversion directly from style to "Ref<css::Value>"
  /// All leaf types must implement the following:
  ///
  /// template<>
  /// struct krys::boo::style::CSSValueCreation<StyleType>
  /// {
  ///   Ref<css::Value> operator()(css::ValuePool&, const RenderStyle&, const StyleType&) noexcept;
  /// };
  template <typename StyleType>
  struct CSSValueCreation;

  struct CSSValueCreationInvoker
  {
    template <typename StyleType, typename... Rest>
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value, Rest &&...rest) const noexcept
    {
      return CSSValueCreation<StyleType> {}(pool, style, value, std::forward<Rest>(rest)...);
    }
  };

  constexpr CSSValueCreationInvoker CreateCSSValue {};

  // Constrained for `TreatAsVariantLike`.
  template <css::VariantLike StyleType>
  struct CSSValueCreation<StyleType>
  {
    template <typename... Rest>
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value, Rest &&...rest) noexcept
    {
      return krys::SwitchOn(
        value, [&](const auto &alternative) -> Ref<css::Value>
        { return CreateCSSValue(pool, style, alternative, std::forward<Rest>(rest)...); });
    }
  };

  // Constrained for `TreatAsTupleLike`
  template <css::TupleLike StyleType>
  struct CSSValueCreation<StyleType>
  {
    template <typename... Rest>
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value, Rest &&...rest) noexcept
    {
      if constexpr (std::tuple_size_v<StyleType> == 1
                    && css::SerializationSeparator<StyleType> == css::SerializationSeparatorType::None)
      {
        return CreateCSSValue(pool, style, get<0>(value), std::forward<Rest>(rest)...);
      }
      else if constexpr (std::tuple_size_v<StyleType> == 2
                         && css::SerializationCoalescing<StyleType>
                              == css::SerializationCoalescingType::Minimal)
      {
        return css::CreateCoalescingPairValue<css::SerializationSeparator<StyleType>>(
          CreateCSSValue(pool, style, get<0>(value), rest...),
          CreateCSSValue(pool, style, get<1>(value), rest...));
      }
      else if constexpr (std::tuple_size_v<StyleType> == 4
                         && css::SerializationCoalescing<StyleType>
                              == css::SerializationCoalescingType::Minimal)
      {
        return css::CreateCoalescingQuadValue<css::SerializationSeparator<StyleType>>(
          CreateCSSValue(pool, style, get<0>(value), rest...),
          CreateCSSValue(pool, style, get<1>(value), rest...),
          CreateCSSValue(pool, style, get<2>(value), rest...),
          CreateCSSValue(pool, style, get<3>(value), rest...));
      }
      else
      {
        css::ValueListBuilder list;

        auto caller = krys::CreateVisitor(
          [&]<css::OptionalLike T>(const T &element)
          {
            if (!element)
            {
              return;
            }

            list.push_back(CreateCSSValue(pool, style, *element, rest...));
          },
          [&](const auto &element) { list.push_back(CreateCSSValue(pool, style, element, rest...)); });

        krys::apply([&](const auto &...x) { (..., caller(x)); }, value);
        return css::CreateListValue<css::SerializationSeparator<StyleType>>(krys::move(list));
      }
    }
  };

  // Constrained for `TreatAsRangeLike`
  template <css::RangeLike StyleType>
  struct CSSValueCreation<StyleType>
  {
    template <typename... Rest>
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value, Rest &&...rest) noexcept
    {
      css::ValueListBuilder list;

      for (const auto &element : value)
      {
        list.push_back(CreateCSSValue(pool, style, element, rest...));
      }

      return css::CreateListValue<css::SerializationSeparator<StyleType>>(krys::move(list));
    }
  };

  // Constrained for `TreatAsNonConverting`.
  template <NonConverting StyleType>
  struct CSSValueCreation<StyleType>
  {
    template <typename... Rest>
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &,
                                              const StyleType &value, Rest &&...rest) noexcept
    {
      return css::CreateValue(pool, value, std::forward<Rest>(rest)...);
    }
  };

  // Specialization for `css::FunctionNotation`.
  template <css::ValueId Name, typename StyleType>
  struct CSSValueCreation<css::FunctionNotation<Name, StyleType>>
  {
    template <typename... Rest>
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const css::FunctionNotation<Name, StyleType> &value,
                                              Rest &&...rest) noexcept
    {
      return css::CreateFunctionValue(
        value.name, CreateCSSValue(pool, style, value.parameters, std::forward<Rest>(rest)...));
    }
  };

#pragma endregion

#pragma region CSSValueConversion

  /// @brief Conversion directly from "Ref<css::Value>" to style
  /// All leaf types must implement the following:
  ///
  /// template<>
  /// struct krys::boo::style::CSSValueConversion<StyleType>
  /// {
  ///   StyleType operator()(BuilderState&, const css::Value&) noexcept;
  ///   [optional] StyleType operator()(BuilderState&, [DerivedFrom<css::Value>]) noexcept;
  ///   [optional] StyleType operator()(const css::ToLengthConversionData&, [DerivedFrom<css::Value>])
  ///   noexcept;
  /// };
  template <typename StyleType>
  struct CSSValueConversion;

  template <typename StyleType>
  struct CSSValueConversionInvoker
  {
    template <typename... Rest>
    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, const css::Value &value,
                                        Rest &&...rest) const noexcept
    {
      return CSSValueConversion<StyleType> {}(builderState, value, std::forward<Rest>(rest)...);
    }

    template <typename... Rest>
    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, const css::PrimitiveValue &value,
                                        Rest &&...rest) const noexcept
    {
      return CSSValueConversion<StyleType> {}(builderState, value, std::forward<Rest>(rest)...);
    }

    template <typename... Rest>
    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, DerivedFrom<css::Value> auto const &value,
                                        Rest &&...rest) const noexcept
    {
      return CSSValueConversion<StyleType> {}(builderState, value, std::forward<Rest>(rest)...);
    }

    template <typename... Rest>
    KRYS_NODISCARD StyleType operator()(const css::ToLengthConversionData &conversionData,
                                        const css::Value &value, Rest &&...rest) const noexcept
    {
      return CSSValueConversion<StyleType> {}(conversionData, value, std::forward<Rest>(rest)...);
    }

    template <typename... Rest>
    KRYS_NODISCARD StyleType operator()(const css::ToLengthConversionData &conversionData,
                                        const css::PrimitiveValue &value, Rest &&...rest) const noexcept
    {
      return CSSValueConversion<StyleType> {}(conversionData, value, std::forward<Rest>(rest)...);
    }

    template <typename... Rest>
    KRYS_NODISCARD StyleType operator()(const css::ToLengthConversionData &conversionData,
                                        DerivedFrom<css::Value> auto const &value,
                                        Rest &&...rest) const noexcept
    {
      return CSSValueConversion<StyleType> {}(conversionData, value, std::forward<Rest>(rest)...);
    }
  };

  template <typename StyleType>
  constexpr CSSValueConversionInvoker<StyleType> ConvertCSSValueToStyle {};

#pragma endregion

#pragma region DeprecatedCSSValueConversion

  /// @brief Conversion directly from "Ref<css::Value>" to "Style" when lacking BuilderState or
  /// css::ToLengthConversionData. Should not be used for new code and should be phased out. All leaf types
  /// must implement the following:
  ///
  /// template<>
  /// struct krys::boo::style::DeprecatedCSSValueConversion<StyleType>
  /// {
  ///   Maybe<StyleType> operator()(const RefPtr<dom::Element>&&, const css::Value&);
  ///   Maybe<StyleType> operator()(const RefPtr<dom::Element>&&, const css::PrimitiveValue&);
  ///   [optional] Maybe<StyleType> operator()(const RefPtr<dom::Element>&&, [DerivedFrom<css::Value>])
  ///   noexcept;
  /// };
  template <typename StyleType>
  struct DeprecatedCSSValueConversion;

  template <typename StyleType>
  struct DeprecatedCSSValueConversionInvoker
  {
    template <typename... Rest>
    KRYS_NODISCARD Maybe<StyleType> operator()(const RefPtr<dom::Element> &element, const css::Value &value,
                                               Rest &&...rest) const noexcept
    {
      return DeprecatedCSSValueConversion<StyleType> {}(element, value, std::forward<Rest>(rest)...);
    }

    template <typename... Rest>
    KRYS_NODISCARD Maybe<StyleType> operator()(const RefPtr<dom::Element> &element,
                                               const css::PrimitiveValue &value,
                                               Rest &&...rest) const noexcept
    {
      return DeprecatedCSSValueConversion<StyleType> {}(element, value, std::forward<Rest>(rest)...);
    }

    template <typename... Rest>
    KRYS_NODISCARD Maybe<StyleType> operator()(const RefPtr<dom::Element> &element,
                                               DerivedFrom<css::Value> auto const &value,
                                               Rest &&...rest) const noexcept
    {
      return DeprecatedCSSValueConversion<StyleType> {}(element, value, std::forward<Rest>(rest)...);
    }
  };

  template <typename StyleType>
  constexpr DeprecatedCSSValueConversionInvoker<StyleType> DeprecatedConvertCSSValueToStyle {};

#pragma endregion

#pragma region ToPlatform

  /// @brief All leaf types must implement the following:
  ///
  /// template<>
  /// struct krys::boo::style::ToPlatform<StyleType>
  /// {
  ///   PlatformType operator()(const StyleType&) noexcept;
  /// };
  template <typename StyleType>
  struct ToPlatform;

  struct ToPlatformInvoker
  {
    template <typename StyleType, typename... Rest>
    KRYS_NODISCARD decltype(auto) operator()(const StyleType &value, Rest &&...rest) const noexcept
    {
      return ToPlatform<StyleType> {}(value, std::forward<Rest>(rest)...);
    }
  };

  constexpr ToPlatformInvoker ConvertToPlatform {};

#pragma endregion

#pragma region Serialisation

  /// @brief All leaf types must implement the following:
  ///
  /// template<>
  /// struct krys::boo::style::Serialiser<StyleType>
  /// {
  ///   void operator()(css::CSSOMString&, const css::SerialisationContext&, const RenderStyle&, const
  ///                   StyleType&) noexcept;
  /// };
  template <typename StyleType>
  struct Serialiser;

  struct SerializeInvoker
  {
    template <typename StyleType, typename... Rest>
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value, Rest &&...rest) const noexcept
    {
      Serialiser<StyleType> {}(builder, context, style, value, std::forward<Rest>(rest)...);
    }

    template <typename StyleType, typename... Rest>
    KRYS_NODISCARD css::CSSOMString operator()(const css::SerialisationContext &context,
                                               const RenderStyle &style, const StyleType &value,
                                               Rest &&...rest) const noexcept
    {
      css::CSSOMString builder;
      this->operator()(builder, context, style, value, std::forward<Rest>(rest)...);
      return builder;
    }
  };

  constexpr SerializeInvoker Serialise {};

  template <typename StyleType, typename... Rest>
  void SerialisationForCSSOnOptionalLike(css::CSSOMString &builder, const css::SerialisationContext &context,
                                         const RenderStyle &style, const StyleType &value,
                                         Rest &&...rest) noexcept
  {
    if (!value)
    {
      return;
    }

    Serialise(builder, context, style, *value, std::forward<Rest>(rest)...);
  }

  template <typename StyleType, typename... Rest>
  void SerialisationForCSSOnTupleLike(css::CSSOMString &builder, const css::SerialisationContext &context,
                                      const RenderStyle &style, const StyleType &value, utf8_string separator,
                                      Rest &&...rest) noexcept
  {
    utf8_string swappedSeparator = "";
    auto caller = krys::CreateVisitor(
      [&]<css::OptionalLike T>(const T &element)
      {
        if (!element)
        {
          return;
        }

        builder.push_back(std::exchange(swappedSeparator, separator));
        Serialise(builder, context, style, *element, rest...);
      },
      [&](const auto &element)
      {
        builder.push_back(std::exchange(swappedSeparator, separator));
        Serialise(builder, context, style, element, rest...);
      });

    krys::apply([&](const auto &...x) { (..., caller(x)); }, value);
  }

  template <typename StyleType, typename... Rest>
  void SerialisationForCSSOnTupleLikeCoalescing(css::CSSOMString &builder,
                                                const css::SerialisationContext &context,
                                                const RenderStyle &style, const StyleType &value,
                                                utf8_string separator, Rest &&...rest) noexcept
  {
    if constexpr (std::tuple_size_v<StyleType> == 2)
    {
      if (get<0>(value) != get<1>(value))
      {
        SerialisationForCSSOnTupleLike(builder, context, style, std::tuple {get<0>(value), get<1>(value)},
                                       separator, std::forward<Rest>(rest)...);
        return;
      }

      Serialise(builder, context, style, get<0>(value), std::forward<Rest>(rest)...);
    }
    else if constexpr (std::tuple_size_v<StyleType> == 4)
    {
      if (get<3>(value) != get<1>(value))
      {
        SerialisationForCSSOnTupleLike(
          builder, context, style, std::tuple {get<0>(value), get<1>(value), get<2>(value), get<3>(value)},
          separator, std::forward<Rest>(rest)...);
        return;
      }

      if (get<2>(value) != get<0>(value))
      {
        SerialisationForCSSOnTupleLike(builder, context, style,
                                       std::tuple {get<0>(value), get<1>(value), get<2>(value)}, separator,
                                       std::forward<Rest>(rest)...);
        return;
      }

      if (get<1>(value) != get<0>(value))
      {
        SerialisationForCSSOnTupleLike(builder, context, style, std::tuple {get<0>(value), get<1>(value)},
                                       separator, std::forward<Rest>(rest)...);
        return;
      }

      Serialise(builder, context, style, get<0>(value), std::forward<Rest>(rest)...);
    }
  }

  template <typename StyleType, typename... Rest>
  void SerialisationForCSSOnRangeLike(css::CSSOMString &builder, const css::SerialisationContext &context,
                                      const RenderStyle &style, const StyleType &value, utf8_string separator,
                                      Rest &&...rest) noexcept
  {
    utf8_string swappedSeparator = "";
    for (const auto &element : value)
    {
      builder.push_back(std::exchange(swappedSeparator, separator));
      Serialise(builder, context, style, element, rest...);
    }
  }

  template <typename StyleType, typename... Rest>
  void SerialisationForCSSOnVariantLike(css::CSSOMString &builder, const css::SerialisationContext &context,
                                        const RenderStyle &style, const StyleType &value,
                                        Rest &&...rest) noexcept
  {
    krys::SwitchOn(value, [&](const auto &alternative)
                   { Serialise(builder, context, style, alternative, std::forward<Rest>(rest)...); });
  }

  // Constrained for `TreatAsEmptyLike`.
  template <css::EmptyLike StyleType>
  struct Serialiser<StyleType>
  {
    template <typename... Rest>
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const StyleType &, Rest &&...) noexcept
    {
    }
  };

  // Constrained for `TreatAsOptionalLike`.
  template <css::OptionalLike StyleType>
  struct Serialiser<StyleType>
  {
    template <typename... Rest>
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value, Rest &&...rest) noexcept
    {
      SerialisationForCSSOnOptionalLike(builder, context, style, value, std::forward<Rest>(rest)...);
    }
  };

  // Constrained for `TreatAsTupleLike`.
  template <css::TupleLike StyleType>
  struct Serialiser<StyleType>
  {
    template <typename... Rest>
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value, Rest &&...rest) noexcept
    {
      if constexpr (css::SerializationCoalescing<StyleType> == css::SerializationCoalescingType::Minimal)
      {
        SerialisationForCSSOnTupleLikeCoalescing(builder, context, style, value,
                                                 css::SerializationSeparatorString<StyleType>,
                                                 std::forward<Rest>(rest)...);
      }
      else
      {
        SerialisationForCSSOnTupleLike(builder, context, style, value,
                                       css::SerializationSeparatorString<StyleType>,
                                       std::forward<Rest>(rest)...);
      }
    }
  };

  // Constrained for `TreatAsRangeLike`.
  template <css::RangeLike StyleType>
  struct Serialiser<StyleType>
  {
    template <typename... Rest>
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value, Rest &&...rest) noexcept
    {
      SerialisationForCSSOnRangeLike(builder, context, style, value,
                                     css::SerializationSeparatorString<StyleType>,
                                     std::forward<Rest>(rest)...);
    }
  };

  // Constrained for `TreatAsVariantLike`.
  template <css::VariantLike StyleType>
  struct Serialiser<StyleType>
  {
    template <typename... Rest>
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value, Rest &&...rest) noexcept
    {
      SerialisationForCSSOnVariantLike(builder, context, style, value, std::forward<Rest>(rest)...);
    }
  };

  // Constrained for `TreatAsNonConverting`.
  template <NonConverting StyleType>
  struct Serialiser<StyleType>
  {
    template <typename... Rest>
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context, const RenderStyle &,
                    const StyleType &value, Rest &&...rest) noexcept
    {
      css::Serialise(builder, context, value, std::forward<Rest>(rest)...);
    }
  };

  // Specialization for `css::FunctionNotation`.
  template <css::ValueId Name, typename StyleType>
  struct Serialiser<css::FunctionNotation<Name, StyleType>>
  {
    template <typename... Rest>
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const css::FunctionNotation<Name, StyleType> &value,
                    Rest &&...rest) noexcept
    {
      builder.append(css::ToString(value.name));
      builder.push_back(u8'(');
      Serialise(builder, context, style, value.parameters, std::forward<Rest>(rest)...);
      builder.push_back(u8')');
    }
  };

#pragma endregion

#pragma region Evaluation

  /// @brief Types that want to participate in evaluation overloading must specialize the following interface:
  ///
  /// template<>
  /// struct krys::boo::style::Evaluation<StyleType>
  /// {
  ///   decltype(auto) operator()(const StyleType&, ...) noexcept;
  /// };
  template <typename, typename>
  struct Evaluation;

  template <typename StyleType, typename Result, typename T1>
  concept HasTwoParameterEvaluate =
    requires { Evaluation<StyleType, Result> {}(std::declval<const StyleType &>(), std::declval<T1>()); };

  template <typename StyleType, typename Result, typename T1, typename T2>
  concept HasThreeParameterEvaluate = requires {
    Evaluation<StyleType, Result> {}(std::declval<const StyleType &>(), std::declval<T1>(),
                                     std::declval<T2>());
  };

  template <typename Result>
  struct EvaluationInvoker
  {
    template <typename StyleType>
    KRYS_NODISCARD Result operator()(const StyleType &value) const noexcept
    {
      return Evaluation<StyleType, Result> {}(value);
    }

    template <typename StyleType, typename T1>
    KRYS_NODISCARD Result operator()(const StyleType &value, T1 &&t1) const noexcept
    {
      if constexpr (HasTwoParameterEvaluate<StyleType, Result, T1>)
      {
        return Evaluation<StyleType, Result> {}(value, std::forward<T1>(t1));
      }
      else
      {
        return operator()(value);
      }
    }

    template <typename StyleType, typename T1, typename T2>
    KRYS_NODISCARD Result operator()(const StyleType &value, T1 &&t1, T2 &&t2) const noexcept
    {
      if constexpr (HasThreeParameterEvaluate<StyleType, Result, T1, T2>)
      {
        return Evaluation<StyleType, Result> {}(value, std::forward<T1>(t1), std::forward<T2>(t2));
      }
      else
      {
        return operator()(value, std::forward<T1>(t1));
      }
    }
  };

  template <typename Result>
  constexpr EvaluationInvoker<Result> Eval {};

  // Constrained for `TreatAsVariantLike`.
  template <css::VariantLike StyleType, typename Result>
  struct Evaluation<StyleType, Result>
  {
    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const StyleType &value, Rest &&...rest) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &alternative)
                            { return Eval<Result>(alternative, std::forward<Rest>(rest)...); });
    }
  };

  // Specialization for `css::TupleLike` (wrapper).
  template <css::TupleLike StyleType, typename Result>
  requires(std::tuple_size_v<StyleType> == 1)
  struct Evaluation<StyleType, Result>
  {
    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const StyleType &value, Rest &&...rest) noexcept
    {
      return Eval<Result>(get<0>(value), std::forward<Rest>(rest)...);
    }
  };

#pragma endregion

#pragma region Blending

  /// @brief All non-tuple-like leaf types must specialize `Blending` with the following member functions:
  ///
  /// template<>
  /// struct krys::boo::style::Blending<StyleType>
  /// {
  ///   [optional default=(a==b)] bool Equals(const StyleType&, const StyleType&) noexcept;
  ///
  ///   [optional default=true] bool CanBlend(const StyleType&, const StyleType&) noexcept;
  ///
  ///   [optional default=false] bool RequiresInterpolationForAccumulativeIteration(const StyleType&, const
  ///                                                                               StyleType&) noexcept;
  ///
  ///   StyleType Blend(const StyleType&, const StyleType&, const [BlendingContext|lerp::Context]&) noexcept;
  /// };
  ///
  /// or, if a RenderStyle is needed for blending:
  ///
  /// template<>
  /// struct krys::boo::style::Blending<StyleType>
  /// {
  ///   [optional default=(a==b)] bool Equals(const StyleType&, const StyleType&, const RenderStyle&, const
  ///                                         RenderStyle&) noexcept;
  ///
  ///   [optional default=true] bool CanBlend(const StyleType&, const StyleType&,  const RenderStyle&, const
  ///                                         RenderStyle&) noexcept;
  ///
  ///   [optional default=false] bool RequiresInterpolationForAccumulativeIteration(
  ///       const StyleType&, const StyleType&, const RenderStyle&, const RenderStyle&) noexcept;
  ///
  ///   StyleType Blend(
  ///       const StyleType&, const StyleType&, const RenderStyle&, const RenderStyle&, const
  ///       [BlendingContext|lerp::Context]&
  ///   ) noexcept;
  /// };
  template <typename>
  struct Blending;

  template <typename StyleType>
  concept HasEqualsForBlendingWithoutRenderStyle = requires {
    {
      Blending<StyleType> {}.Equals(std::declval<const StyleType &>(), std::declval<const StyleType &>())
    } -> SameType<bool>;
  };

  template <typename StyleType>
  concept HasEqualsForBlendingWithRenderStyle = requires {
    {
      Blending<StyleType> {}.Equals(std::declval<const StyleType &>(), std::declval<const StyleType &>(),
                                    std::declval<const RenderStyle &>(), std::declval<const RenderStyle &>())
    } -> SameType<bool>;
  };

  template <typename StyleType>
  concept HasCanBlendWithoutRenderStyleAndWithoutCompositeOperation = requires {
    {
      Blending<StyleType> {}.CanBlend(std::declval<const StyleType &>(), std::declval<const StyleType &>())
    } -> SameType<bool>;
  };

  template <typename StyleType>
  concept HasCanBlendWithRenderStyleAndWithoutCompositeOperation = requires {
    {
      Blending<StyleType> {}.CanBlend(std::declval<const StyleType &>(), std::declval<const StyleType &>(),
                                      std::declval<const RenderStyle &>(),
                                      std::declval<const RenderStyle &>())
    } -> SameType<bool>;
  };

  template <typename StyleType>
  concept HasCanBlendWithoutRenderStyleAndWithCompositeOperation = requires {
    {
      Blending<StyleType> {}.CanBlend(std::declval<const StyleType &>(), std::declval<const StyleType &>(),
                                      std::declval<CompositeOperation>())
    } -> SameType<bool>;
  };

  template <typename StyleType>
  concept HasCanBlendWithRenderStyleAndWithCompositeOperation = requires {
    {
      Blending<StyleType> {}.CanBlend(std::declval<const StyleType &>(), std::declval<const StyleType &>(),
                                      std::declval<const RenderStyle &>(),
                                      std::declval<const RenderStyle &>(), std::declval<CompositeOperation>())
    } -> SameType<bool>;
  };

  template <typename StyleType>
  concept HasRequiresInterpolationForAccumulativeIterationWithoutRenderStyle = requires {
    {
      Blending<StyleType> {}.RequiresInterpolationForAccumulativeIteration(std::declval<const StyleType &>(),
                                                                           std::declval<const StyleType &>())
    } -> SameType<bool>;
  };

  template <typename StyleType>
  concept HasRequiresInterpolationForAccumulativeIterationWithRenderStyle = requires {
    {
      Blending<StyleType> {}.RequiresInterpolationForAccumulativeIteration(
        std::declval<const StyleType &>(), std::declval<const StyleType &>(),
        std::declval<const RenderStyle &>(), std::declval<const RenderStyle &>())
    } -> SameType<bool>;
  };

  template <typename StyleType>
  concept HasBlendWithoutRenderStyleAndWithInterpolationContext = requires {
    {
      Blending<StyleType> {}.Blend(std::declval<const StyleType &>(), std::declval<const StyleType &>(),
                                   std::declval<const lerp::Context &>())
    } -> SameType<StyleType>;
  };

  template <typename StyleType>
  concept HasBlendWithoutRenderStyleAndWithBlendingContext = requires {
    {
      Blending<StyleType> {}.Blend(std::declval<const StyleType &>(), std::declval<const StyleType &>(),
                                   std::declval<const BlendingContext &>())
    } -> SameType<StyleType>;
  };

  template <typename StyleType>
  concept HasBlendWithRenderStyleAndWithInterpolationContext = requires {
    {
      Blending<StyleType> {}.Blend(std::declval<const StyleType &>(), std::declval<const StyleType &>(),
                                   std::declval<const RenderStyle &>(), std::declval<const RenderStyle &>(),
                                   std::declval<const lerp::Context &>())
    } -> SameType<StyleType>;
  };

  template <typename StyleType>
  concept HasBlendWithRenderStyleAndWithBlendingContext = requires {
    {
      Blending<StyleType> {}.Blend(std::declval<const StyleType &>(), std::declval<const StyleType &>(),
                                   std::declval<const RenderStyle &>(), std::declval<const RenderStyle &>(),
                                   std::declval<const BlendingContext &>())
    } -> SameType<StyleType>;
  };

  struct EqualsForBlendingInvoker
  {
    template <typename StyleType>
    KRYS_NODISCARD bool operator()(const StyleType &a, const StyleType &b) const noexcept
    {
      if constexpr (HasEqualsForBlendingWithoutRenderStyle<StyleType>)
      {
        return Blending<StyleType> {}.Equals(a, b);
      }
      else
      {
        return a == b;
      }
    }

    template <typename StyleType>
    KRYS_NODISCARD bool operator()(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                   const RenderStyle &bStyle) const noexcept
    {
      if constexpr (HasEqualsForBlendingWithRenderStyle<StyleType>)
      {
        return Blending<StyleType> {}.Equals(a, b, aStyle, bStyle);
      }
      else
      {
        return this->operator()(a, b);
      }
    }
  };

  constexpr EqualsForBlendingInvoker EqualsForBlending {};

  struct CanBlendInvoker
  {
    template <typename StyleType>
    KRYS_NODISCARD bool operator()(const StyleType &a, const StyleType &b) const noexcept
    {
      if constexpr (HasCanBlendWithoutRenderStyleAndWithoutCompositeOperation<StyleType>)
      {
        return Blending<StyleType> {}.CanBlend(a, b);
      }
      else
      {
        return true;
      }
    }

    template <typename StyleType>
    KRYS_NODISCARD bool operator()(const StyleType &a, const StyleType &b,
                                   CompositeOperation compositeOperation) const noexcept
    {
      if constexpr (HasCanBlendWithoutRenderStyleAndWithCompositeOperation<StyleType>)
      {
        return Blending<StyleType> {}.CanBlend(a, b, compositeOperation);
      }
      else
      {
        return this->operator()(a, b);
      }
    }

    template <typename StyleType>
    KRYS_NODISCARD bool operator()(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                   const RenderStyle &bStyle) const noexcept
    {
      if constexpr (HasCanBlendWithRenderStyleAndWithoutCompositeOperation<StyleType>)
      {
        return Blending<StyleType> {}.CanBlend(a, b, aStyle, bStyle);
      }
      else
      {
        return this->operator()(a, b);
      }
    }

    template <typename StyleType>
    KRYS_NODISCARD bool operator()(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                   const RenderStyle &bStyle,
                                   CompositeOperation compositeOperation) const noexcept
    {
      if constexpr (HasCanBlendWithRenderStyleAndWithCompositeOperation<StyleType>)
      {
        return Blending<StyleType> {}.CanBlend(a, b, aStyle, bStyle, compositeOperation);
      }
      else if constexpr (HasCanBlendWithRenderStyleAndWithoutCompositeOperation<StyleType>)
      {
        return Blending<StyleType> {}.CanBlend(a, b, aStyle, bStyle);
      }
      else if constexpr (HasCanBlendWithoutRenderStyleAndWithCompositeOperation<StyleType>)
      {
        return Blending<StyleType> {}.CanBlend(a, b, compositeOperation);
      }
      else
      {
        return this->operator()(a, b);
      }
    }
  };

  constexpr CanBlendInvoker CanBlend {};

  struct RequiresInterpolationForAccumulativeIterationInvoker
  {
    template <typename StyleType>
    KRYS_NODISCARD bool operator()(const StyleType &a, const StyleType &b) const noexcept
    {
      if constexpr (HasRequiresInterpolationForAccumulativeIterationWithoutRenderStyle<StyleType>)
      {
        return Blending<StyleType> {}.RequiresInterpolationForAccumulativeIteration(a, b);
      }
      else
      {
        return false;
      }
    }

    template <typename StyleType>
    KRYS_NODISCARD bool operator()(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                   const RenderStyle &bStyle) const noexcept
    {
      if constexpr (HasRequiresInterpolationForAccumulativeIterationWithRenderStyle<StyleType>)
      {
        return Blending<StyleType> {}.RequiresInterpolationForAccumulativeIteration(a, b, aStyle, bStyle);
      }
      else
      {
        return this->operator()(a, b);
      }
    }
  };

  constexpr RequiresInterpolationForAccumulativeIterationInvoker
    RequiresInterpolationForAccumulativeIteration {};

  struct BlendInvoker
  {
    template <typename StyleType>
    KRYS_NODISCARD StyleType operator()(const StyleType &a, const StyleType &b,
                                        const lerp::Context &context) const noexcept
    {
      return Blending<StyleType> {}.Blend(a, b, context);
    }

    template <typename StyleType>
    KRYS_NODISCARD StyleType operator()(const StyleType &a, const StyleType &b,
                                        const BlendingContext &context) const noexcept
    {
      return Blending<StyleType> {}.Blend(a, b, context);
    }

    template <typename StyleType>
    KRYS_NODISCARD StyleType operator()(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                        const RenderStyle &bStyle,
                                        const lerp::Context &context) const noexcept
    {
      if constexpr (HasBlendWithRenderStyleAndWithInterpolationContext<StyleType>
                    || HasBlendWithRenderStyleAndWithBlendingContext<StyleType>)
      {
        return Blending<StyleType> {}.Blend(a, b, aStyle, bStyle, context);
      }
      else
      {
        return this->operator()(a, b, context);
      }
    }

    template <typename StyleType>
    KRYS_NODISCARD StyleType operator()(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                        const RenderStyle &bStyle,
                                        const BlendingContext &context) const noexcept
    {
      if constexpr (HasBlendWithRenderStyleAndWithBlendingContext<StyleType>)
      {
        return Blending<StyleType> {}.Blend(a, b, aStyle, bStyle, context);
      }
      else
      {
        return this->operator()(a, b, context);
      }
    }
  };
  constexpr BlendInvoker Blend {};

  // Utilities for blending

  template <typename StyleType>
  KRYS_NODISCARD bool EqualsForBlendingOnOptionalLike(const StyleType &a, const StyleType &b) noexcept
  {
    if (a && b)
    {
      return krys::boo::style::EqualsForBlending(*a, *b);
    }

    return !a && !b;
  }

  template <typename StyleType>
  KRYS_NODISCARD bool EqualsForBlendingOnOptionalLike(const StyleType &a, const StyleType &b,
                                                      const RenderStyle &aStyle,
                                                      const RenderStyle &bStyle) noexcept
  {
    if (a && b)
    {
      return krys::boo::style::EqualsForBlending(*a, *b, aStyle, bStyle);
    }

    return !a && !b;
  }

  template <typename StyleType>
  KRYS_NODISCARD bool CanBlendOnOptionalLike(const StyleType &a, const StyleType &b) noexcept
  {
    if (a && b)
    {
      return krys::boo::style::CanBlend(*a, *b);
    }

    return !a && !b;
  }

  template <typename StyleType>
  KRYS_NODISCARD bool CanBlendOnOptionalLike(const StyleType &a, const StyleType &b,
                                             const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
  {
    if (a && b)
    {
      return krys::boo::style::CanBlend(*a, *b, aStyle, bStyle);
    }

    return !a && !b;
  }

  template <typename StyleType>
  KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIterationOnOptionalLike(const StyleType &a,
                                                                                  const StyleType &b) noexcept
  {
    if (a && b)
    {
      return krys::boo::style::RequiresInterpolationForAccumulativeIteration(*a, *b);
    }

    return false;
  }

  template <typename StyleType>
  KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIterationOnOptionalLike(
    const StyleType &a, const StyleType &b, const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
  {
    if (a && b)
    {
      return krys::boo::style::RequiresInterpolationForAccumulativeIteration(*a, *b, aStyle, bStyle);
    }

    return false;
  }

  template <typename StyleType>
  KRYS_NODISCARD StyleType BlendOnOptionalLike(const StyleType &a, const StyleType &b,
                                               const auto &context) noexcept
  {
    if (a && b)
    {
      return krys::boo::style::Blend(*a, *b, context);
    }

    return null;
  }

  template <typename StyleType>
  KRYS_NODISCARD StyleType BlendOnOptionalLike(const StyleType &a, const StyleType &b,
                                               const RenderStyle &aStyle, const RenderStyle &bStyle,
                                               const auto &context) noexcept
  {
    if (a && b)
    {
      return krys::boo::style::Blend(*a, *b, aStyle, bStyle, context);
    }

    return null;
  }

  template <typename StyleType>
  KRYS_NODISCARD auto EqualsForBlendingOnTupleLike(const StyleType &a, const StyleType &b) noexcept -> bool
  {
    return krys::apply(
      [&](const auto &...pair)
      { return (krys::boo::style::EqualsForBlending(std::get<0>(pair), std::get<1>(pair)) && ...); },
      ZipTuple(a, b));
  }

  template <typename StyleType>
  KRYS_NODISCARD auto EqualsForBlendingOnTupleLike(const StyleType &a, const StyleType &b,
                                                   const RenderStyle &aStyle,
                                                   const RenderStyle &bStyle) noexcept -> bool
  {
    return krys::apply(
      [&](const auto &...pair)
      {
        return (krys::boo::style::EqualsForBlending(std::get<0>(pair), std::get<1>(pair), aStyle, bStyle)
                && ...);
      },
      ZipTuple(a, b));
  }

  template <typename StyleType>
  KRYS_NODISCARD auto CanBlendOnTupleLike(const StyleType &a, const StyleType &b) noexcept -> bool
  {
    return krys::apply([&](const auto &...pair)
                       { return (krys::boo::style::CanBlend(std::get<0>(pair), std::get<1>(pair)) && ...); },
                       ZipTuple(a, b));
  }

  template <typename StyleType>
  KRYS_NODISCARD auto CanBlendOnTupleLike(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                          const RenderStyle &bStyle) noexcept -> bool
  {
    return krys::apply(
      [&](const auto &...pair)
      { return (krys::boo::style::CanBlend(std::get<0>(pair), std::get<1>(pair), aStyle, bStyle) && ...); },
      ZipTuple(a, b));
  }

  template <typename StyleType>
  KRYS_NODISCARD auto RequiresInterpolationForAccumulativeIterationOnTupleLike(const StyleType &a,
                                                                               const StyleType &b) noexcept
    -> bool
  {
    return krys::apply(
      [&](const auto &...pair)
      {
        return (krys::boo::style::RequiresInterpolationForAccumulativeIteration(std::get<0>(pair),
                                                                                std::get<1>(pair))
                || ...);
      },
      ZipTuple(a, b));
  }

  template <typename StyleType>
  KRYS_NODISCARD auto
    RequiresInterpolationForAccumulativeIterationOnTupleLike(const StyleType &a, const StyleType &b,
                                                             const RenderStyle &aStyle,
                                                             const RenderStyle &bStyle) noexcept -> bool
  {
    return krys::apply(
      [&](const auto &...pair)
      {
        return (krys::boo::style::RequiresInterpolationForAccumulativeIteration(
                  std::get<0>(pair), std::get<1>(pair), aStyle, bStyle)
                || ...);
      },
      ZipTuple(a, b));
  }

  template <typename StyleType>
  KRYS_NODISCARD auto BlendOnTupleLike(const StyleType &a, const StyleType &b, const auto &context) noexcept
    -> StyleType
  {
    return krys::apply(
      [&](const auto &...pair)
      { return StyleType {krys::boo::style::Blend(std::get<0>(pair), std::get<1>(pair), context)...}; },
      ZipTuple(a, b));
  }

  template <typename StyleType>
  KRYS_NODISCARD auto BlendOnTupleLike(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                       const RenderStyle &bStyle, const auto &context) noexcept -> StyleType
  {
    return krys::apply(
      [&](const auto &...pair)
      {
        return StyleType {
          krys::boo::style::Blend(std::get<0>(pair), std::get<1>(pair), aStyle, bStyle, context)...};
      },
      ZipTuple(a, b));
  }

  // Constrained for `TreatAsOptionalLike`.
  template <css::OptionalLike StyleType>
  struct Blending<StyleType>
  {
    KRYS_NODISCARD constexpr bool Equals(const StyleType &a, const StyleType &b) noexcept
    {
      return EqualsForBlendingOnOptionalLike(a, b);
    }

    KRYS_NODISCARD constexpr bool Equals(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                         const RenderStyle &bStyle) noexcept
    {
      return EqualsForBlendingOnOptionalLike(a, b, aStyle, bStyle);
    }

    KRYS_NODISCARD constexpr bool CanBlend(const StyleType &a, const StyleType &b) noexcept
    {
      return CanBlendOnOptionalLike(a, b);
    }

    KRYS_NODISCARD constexpr bool CanBlend(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                           const RenderStyle &bStyle) noexcept
    {
      return CanBlendOnOptionalLike(a, b, aStyle, bStyle);
    }

    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const StyleType &a,
                                                                                const StyleType &b) noexcept
    {
      return RequiresInterpolationForAccumulativeIterationOnOptionalLike(a, b);
    }

    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(
      const StyleType &a, const StyleType &b, const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
    {
      return RequiresInterpolationForAccumulativeIterationOnOptionalLike(a, b, aStyle, bStyle);
    }

    KRYS_NODISCARD StyleType Blend(const StyleType &a, const StyleType &b, const auto &context) noexcept
    {
      return BlendOnOptionalLike(a, b, context);
    }

    KRYS_NODISCARD StyleType Blend(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                   const RenderStyle &bStyle, const auto &context) noexcept
    {
      return BlendOnOptionalLike(a, b, aStyle, bStyle, context);
    }
  };

  // Constrained for `TreatAsTupleLike`.
  template <css::TupleLike StyleType>
  struct Blending<StyleType>
  {
    KRYS_NODISCARD constexpr bool Equals(const StyleType &a, const StyleType &b) noexcept
    {
      return EqualsForBlendingOnTupleLike(a, b);
    }

    KRYS_NODISCARD constexpr bool Equals(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                         const RenderStyle &bStyle) noexcept
    {
      return EqualsForBlendingOnTupleLike(a, b, aStyle, bStyle);
    }

    KRYS_NODISCARD constexpr bool CanBlend(const StyleType &a, const StyleType &b) noexcept
    {
      return CanBlendOnTupleLike(a, b);
    }

    KRYS_NODISCARD constexpr bool CanBlend(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                           const RenderStyle &bStyle) noexcept
    {
      return CanBlendOnTupleLike(a, b, aStyle, bStyle);
    }

    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const StyleType &a,
                                                                                const StyleType &b) noexcept
    {
      return RequiresInterpolationForAccumulativeIterationOnTupleLike(a, b);
    }

    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(
      const StyleType &a, const StyleType &b, const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
    {
      return RequiresInterpolationForAccumulativeIterationOnTupleLike(a, b, aStyle, bStyle);
    }

    KRYS_NODISCARD StyleType Blend(const StyleType &a, const StyleType &b, const auto &context) noexcept
    {
      return BlendOnTupleLike(a, b, context);
    }

    KRYS_NODISCARD StyleType Blend(const StyleType &a, const StyleType &b, const RenderStyle &aStyle,
                                   const RenderStyle &bStyle, const auto &context) noexcept
    {
      return BlendOnTupleLike(a, b, aStyle, bStyle, context);
    }
  };

  // Specialization for `css::KeywordValueConstant`.
  template <css::ValueId C>
  struct Blending<css::KeywordValueConstant<C>>
  {
    KRYS_NODISCARD css::KeywordValueConstant<C> Blend(const css::KeywordValueConstant<C> &,
                                                      const css::KeywordValueConstant<C> &,
    {
      return {};
    }

    KRYS_NODISCARD css::KeywordValueConstant<C> Blend(const css::KeywordValueConstant<C> &,
                                                      const css::KeywordValueConstant<C> &,
                                                      const RenderStyle &, const RenderStyle &,
                                                      const auto &) noexcept
    {
      return {};
    }
  };

  // Specialization for `Variant`.
  template <typename... StyleTypes>
  struct Blending<Variant<StyleTypes...>>
  {
    KRYS_NODISCARD bool Equals(const Variant<StyleTypes...> &a, const Variant<StyleTypes...> &b) noexcept
    {
      return std::visit(krys::CreateVisitor([]<typename T>(const T &a, const T &b) -> bool
                                            { return krys::boo::style::EqualsForBlending(a, b); },
                                            [](const auto &, const auto &) -> bool { return false; }),
                        a, b);
    }

    KRYS_NODISCARD bool Equals(const Variant<StyleTypes...> &a, const Variant<StyleTypes...> &b,
                               const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
    {
      return std::visit(
        krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> bool
                            { return krys::boo::style::EqualsForBlending(a, b, aStyle, bStyle); },
                            [](const auto &, const auto &) -> bool { return false; }),
        a, b);
    }

    KRYS_NODISCARD bool CanBlend(const Variant<StyleTypes...> &a, const Variant<StyleTypes...> &b) noexcept
    {
      return std::visit(krys::CreateVisitor([]<typename T>(const T &a, const T &b) -> bool
                                            { return krys::boo::style::CanBlend(a, b); },
                                            [](const auto &, const auto &) -> bool { return false; }),
                        a, b);
    }

    KRYS_NODISCARD bool CanBlend(const Variant<StyleTypes...> &a, const Variant<StyleTypes...> &b,
                                 const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
    {
      return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> bool
                                            { return krys::boo::style::CanBlend(a, b, aStyle, bStyle); },
                                            [](const auto &, const auto &) -> bool { return false; }),
                        a, b);
    }

    KRYS_NODISCARD bool
      RequiresInterpolationForAccumulativeIteration(const Variant<StyleTypes...> &a,
                                                    const Variant<StyleTypes...> &b) noexcept
    {
      return std::visit(
        krys::CreateVisitor([]<typename T>(const T &a, const T &b) -> bool
                            { return krys::boo::style::RequiresInterpolationForAccumulativeIteration(a, b); },
                            [](const auto &, const auto &) -> bool { return false; }),
        a, b);
    }

    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const Variant<StyleTypes...> &a,
                                                                      const Variant<StyleTypes...> &b,
                                                                      const RenderStyle &aStyle,
                                                                      const RenderStyle &bStyle) noexcept
    {
      return std::visit(
        krys::CreateVisitor(
          [&]<typename T>(const T &a, const T &b) -> bool
          { return krys::boo::style::RequiresInterpolationForAccumulativeIteration(a, b, aStyle, bStyle); },
          [](const auto &, const auto &) -> bool { return false; }),
        a, b);
    }

    KRYS_NODISCARD Variant<StyleTypes...> Blend(const Variant<StyleTypes...> &a,
                                                const Variant<StyleTypes...> &b, const auto &context) noexcept
    {
      return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> Variant<StyleTypes...>
                                            { return krys::boo::style::Blend(a, b, context); },
                                            [](const auto &, const auto &) -> Variant<StyleTypes...>
                                            { krys_unreachable(); }),
                        a, b);
    }

    KRYS_NODISCARD Variant<StyleTypes...> Blend(const Variant<StyleTypes...> &a,
                                                const Variant<StyleTypes...> &b, const RenderStyle &aStyle,
                                                const RenderStyle &bStyle, const auto &context) noexcept
    {
      return std::visit(
        krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> Variant<StyleTypes...>
                            { return krys::boo::style::Blend(a, b, aStyle, bStyle, context); },
                            [](const auto &, const auto &) -> Variant<StyleTypes...> { krys_unreachable(); }),
        a, b);
    }
  };

  // Specialization for `css::SpaceSeparatedList`.
  template <typename StyleType, size_t InlineCapacity>
  struct Blending<css::SpaceSeparatedList<StyleType, InlineCapacity>>
  {
    KRYS_NODISCARD bool Equals(const css::SpaceSeparatedList<StyleType, InlineCapacity> &a,
                               const css::SpaceSeparatedList<StyleType, InlineCapacity> &b) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (!krys::boo::style::EqualsForBlending(a[i], b[i]))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool Equals(const css::SpaceSeparatedList<StyleType, InlineCapacity> &a,
                               const css::SpaceSeparatedList<StyleType, InlineCapacity> &b,
                               const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (!krys::boo::style::EqualsForBlending(a[i], b[i], aStyle, bStyle))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool CanBlend(const css::SpaceSeparatedList<StyleType, InlineCapacity> &a,
                                 const css::SpaceSeparatedList<StyleType, InlineCapacity> &b) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (!krys::boo::style::CanBlend(a[i], b[i]))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool CanBlend(const css::SpaceSeparatedList<StyleType, InlineCapacity> &a,
                                 const css::SpaceSeparatedList<StyleType, InlineCapacity> &b,
                                 const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (!krys::boo::style::CanBlend(a[i], b[i], aStyle, bStyle))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(
      const css::SpaceSeparatedList<StyleType, InlineCapacity> &a,
      const css::SpaceSeparatedList<StyleType, InlineCapacity> &b) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (krys::boo::style::RequiresInterpolationForAccumulativeIteration(a[i], b[i]))
        {
          return true;
        }
      }

      return false;
    }

    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(
      const css::SpaceSeparatedList<StyleType, InlineCapacity> &a,
      const css::SpaceSeparatedList<StyleType, InlineCapacity> &b, const RenderStyle &aStyle,
      const RenderStyle &bStyle) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (krys::boo::style::RequiresInterpolationForAccumulativeIteration(a[i], b[i], aStyle, bStyle))
        {
          return true;
        }
      }

      return false;
    }

    KRYS_NODISCARD css::SpaceSeparatedList<StyleType, InlineCapacity>
      Blend(const css::SpaceSeparatedList<StyleType, InlineCapacity> &a,
            const css::SpaceSeparatedList<StyleType, InlineCapacity> &b, const auto &context) noexcept
    {
      auto size = a.size();
      typename css::SpaceSeparatedList<StyleType, InlineCapacity>::Container result;
      result.reserve(size);
      for (size_t i = 0uz; i < size; ++i)
        result.push_back(krys::boo::style::Blend(a[i], b[i], context));
      return {krys::move(result)};
    }

    KRYS_NODISCARD css::SpaceSeparatedList<StyleType, InlineCapacity>
      Blend(const css::SpaceSeparatedList<StyleType, InlineCapacity> &a,
            const css::SpaceSeparatedList<StyleType, InlineCapacity> &b, const RenderStyle &aStyle,
            const RenderStyle &bStyle, const auto &context) noexcept
    {
      auto size = a.size();
      typename css::SpaceSeparatedList<StyleType, InlineCapacity>::Container result;

      result.reserve(size);
      for (size_t i = 0uz; i < size; ++i)
      {
        result.push_back(krys::boo::style::Blend(a[i], b[i], aStyle, bStyle, context));
      }

      return {krys::move(result)};
    }
  };

  // Specialization for `css::CommaSeparatedList`.
  template <typename StyleType, size_t InlineCapacity>
  struct Blending<css::CommaSeparatedList<StyleType, InlineCapacity>>
  {
    KRYS_NODISCARD bool Equals(const css::CommaSeparatedList<StyleType, InlineCapacity> &a,
                               const css::CommaSeparatedList<StyleType, InlineCapacity> &b) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (!krys::boo::style::EqualsForBlending(a[i], b[i]))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool Equals(const css::CommaSeparatedList<StyleType, InlineCapacity> &a,
                               const css::CommaSeparatedList<StyleType, InlineCapacity> &b,
                               const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (!krys::boo::style::EqualsForBlending(a[i], b[i], aStyle, bStyle))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool CanBlend(const css::CommaSeparatedList<StyleType, InlineCapacity> &a,
                                 const css::CommaSeparatedList<StyleType, InlineCapacity> &b) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (!krys::boo::style::CanBlend(a[i], b[i]))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool CanBlend(const css::CommaSeparatedList<StyleType, InlineCapacity> &a,
                                 const css::CommaSeparatedList<StyleType, InlineCapacity> &b,
                                 const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (!krys::boo::style::CanBlend(a[i], b[i], aStyle, bStyle))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(
      const css::CommaSeparatedList<StyleType, InlineCapacity> &a,
      const css::CommaSeparatedList<StyleType, InlineCapacity> &b) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (krys::boo::style::RequiresInterpolationForAccumulativeIteration(a[i], b[i]))
        {
          return true;
        }
      }

      return false;
    }

    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(
      const css::CommaSeparatedList<StyleType, InlineCapacity> &a,
      const css::CommaSeparatedList<StyleType, InlineCapacity> &b, const RenderStyle &aStyle,
      const RenderStyle &bStyle) noexcept
    {
      if (a.size() != b.size())
      {
        return false;
      }

      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (krys::boo::style::RequiresInterpolationForAccumulativeIteration(a[i], b[i], aStyle, bStyle))
        {
          return true;
        }
      }

      return false;
    }

    KRYS_NODISCARD css::CommaSeparatedList<StyleType, InlineCapacity>
      Blend(const css::CommaSeparatedList<StyleType, InlineCapacity> &a,
            const css::CommaSeparatedList<StyleType, InlineCapacity> &b, const auto &context) noexcept
    {
      auto size = a.size();
      typename css::CommaSeparatedList<StyleType, InlineCapacity>::Container result;

      result.reserve(size);
      for (size_t i = 0uz; i < size; ++i)
      {
        result.push_back(krys::boo::style::Blend(a[i], b[i], context));
      }

      return {krys::move(result)};
    }

    KRYS_NODISCARD css::CommaSeparatedList<StyleType, InlineCapacity>
      Blend(const css::CommaSeparatedList<StyleType, InlineCapacity> &a,
            const css::CommaSeparatedList<StyleType, InlineCapacity> &b, const RenderStyle &aStyle,
            const RenderStyle &bStyle, const auto &context) noexcept
    {
      auto size = a.size();
      typename css::CommaSeparatedList<StyleType, InlineCapacity>::Container result {};

      result.reserve(size);
      for (size_t i = 0uz; i < size; ++i)
      {
        result.push_back(krys::boo::style::Blend(a[i], b[i], aStyle, bStyle, context));
      }

      return {krys::move(result)};
    }
  };

#pragma endregion

#pragma region IsZero

  /// @brief All leaf types that want to conform to IsZero must implement the following (or have a member
  /// function such that the type matches the `HasIsZero` concept):
  ///
  /// template<>
  /// struct krys::boo::style::IsZero<StyleType>
  /// {
  ///   bool operator()(const StyleType&) noexcept;
  /// };
  template <typename>
  struct IsZero;

  struct IsZeroInvoker
  {
    template <typename T>
    KRYS_NODISCARD bool operator()(const T &value) const noexcept
    {
      if constexpr (HasIsZero<T>)
      {
        return value.IsZero();
      }
      else
      {
        return IsZero<T> {}(value);
      }
    }
  };

  constexpr IsZeroInvoker isZero {};

  // Constrained for `TreatAsTupleLike`.
  template <css::TupleLike T>
  struct IsZero<T>
  {
    KRYS_NODISCARD bool operator()(const T &value) noexcept
    {
      return krys::apply([&](const auto &...x) { return (isZero(x) && ...); }, value);
    }
  };

  // Constrained for `TreatAsVariantLike`.
  template <css::VariantLike T>
  struct IsZero<T>
  {
    KRYS_NODISCARD bool operator()(const T &value) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &alternative) { return isZero(alternative); });
    }
  };

#pragma endregion

#pragma region IsKnownZero

  // All leaf types that want to conform to IsKnownZero must implement
  // the following:
  //
  //    template<> struct krys::boo::style::IsKnownZero<StyleType> {
  //        bool operator()(const StyleType&);
  //    };
  //
  // or have a member function such that the type matches the
  // `HasIsKnownZero` concept.

  template <typename>
  struct IsKnownZero;

  struct IsKnownZeroInvoker
  {
    template <typename T>
    KRYS_NODISCARD bool operator()(const T &value) const noexcept
    {
      if constexpr (HasIsKnownZero<T>)
      {
        return value.IsKnownZero();
      }
      else if constexpr (HasIsZero<T>)
      {
        return !value.IsZero();
      }
      else
      {
        return IsKnownZero<T> {}(value);
      }
    }
  };

  constexpr IsKnownZeroInvoker isKnownZero {};

  // Constrained for `TreatAsTupleLike`.
  template <css::TupleLike T>
  struct IsKnownZero<T>
  {
    KRYS_NODISCARD bool operator()(const T &value) noexcept
    {
      return krys::apply([&](const auto &...x) { return (isKnownZero(x) && ...); }, value);
    }
  };

  // Constrained for `TreatAsVariantLike`.
  template <css::VariantLike T>
  struct IsKnownZero<T>
  {
    KRYS_NODISCARD bool operator()(const T &value) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &alternative) { return isKnownZero(alternative); });
    }
  };

#pragma endregion

#pragma region IsEmpty

  /// @brief All leaf types that want to conform to IsEmpty must implement the following (or have a member
  /// function such that the type matches the `HasIsEmpty` concept):
  ///
  /// template<>
  /// struct krys::boo::style::IsEmpty<StyleType>
  /// {
  ///   bool operator()(const StyleType&) noexcept;
  /// };
  template <typename>
  struct IsEmpty;

  struct IsEmptyInvoker
  {
    template <typename T>
    KRYS_NODISCARD bool operator()(const T &value) const noexcept
    {
      if constexpr (HasIsEmpty<T>)
      {
        return value.IsEmpty();
      }
      else
      {
        return IsEmpty<T> {}(value);
      }
    }
  };

  constexpr IsEmptyInvoker isEmpty {};

  // Specialization for `css::SpaceSeparatedSize`.
  template <typename T>
  struct IsEmpty<css::SpaceSeparatedSize<T>>
  {
    KRYS_NODISCARD bool operator()(const auto &value) noexcept
    {
      return isZero(value.Width()) || isZero(value.Height());
    }
  };

  // Specialization for `css::MinimallySerializingSpaceSeparatedPoint`.
  template <typename T>
  struct IsEmpty<css::MinimallySerializingSpaceSeparatedPoint<T>>
  {
    KRYS_NODISCARD bool operator()(const auto &value) noexcept
    {
      return isZero(value.x()) || isZero(value.y());
    }
  };

  // Specialization for `css::MinimallySerializingSpaceSeparatedSize`.
  template <typename T>
  struct IsEmpty<css::MinimallySerializingSpaceSeparatedSize<T>>
  {
    KRYS_NODISCARD bool operator()(const auto &value) noexcept
    {
      return isZero(value.Width()) || isZero(value.Height());
    }
  };

#pragma endregion

#pragma region IsKnownEmpty

  /// @brief All leaf types that want to conform to IsKnownEmpty must implement the following (or have a
  /// member function such that the type matches the `HasIsKnownEmpty` concept):
  ///
  /// template<>
  /// struct krys::boo::style::IsKnownEmpty<StyleType>
  /// {
  ///   bool operator()(const StyleType&) noexcept;
  /// };
  template <typename>
  struct IsKnownEmpty;

  struct IsKnownEmptyInvoker
  {
    template <typename T>
    KRYS_NODISCARD bool operator()(const T &value) const noexcept
    {
      if constexpr (HasIsKnownEmpty<T>)
      {
        return value.isKnownEmpty();
      }
      else if constexpr (HasIsEmpty<T>)
      {
        return value.isEmpty();
      }
      else
      {
        return IsKnownEmpty<T> {}(value);
      }
    }
  };

  constexpr IsKnownEmptyInvoker isKnownEmpty {};

  // Specialization for `css::SpaceSeparatedSize`.
  template <typename T>
  struct IsKnownEmpty<css::SpaceSeparatedSize<T>>
  {
    KRYS_NODISCARD bool operator()(const auto &value) noexcept
    {
      return isKnownZero(value.Width()) || isKnownZero(value.Height());
    }
  };

  // Specialization for `css::MinimallySerializingSpaceSeparatedPoint`.
  template <typename T>
  struct IsKnownEmpty<css::MinimallySerializingSpaceSeparatedPoint<T>>
  {
    KRYS_NODISCARD bool operator()(const auto &value) noexcept
    {
      return isKnownZero(value.x()) || isKnownZero(value.y());
    }
  };

  // Specialization for `css::MinimallySerializingSpaceSeparatedSize`.
  template <typename T>
  struct IsKnownEmpty<css::MinimallySerializingSpaceSeparatedSize<T>>
  {
    KRYS_NODISCARD bool operator()(const auto &value) noexcept
    {
      return isKnownZero(value.width()) || isKnownZero(value.height());
    }
  };

#pragma endregion
}