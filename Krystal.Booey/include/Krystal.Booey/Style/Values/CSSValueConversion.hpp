#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // Specialization for `css::CSSOMString`.
  template <>
  struct CSSValueConversion<css::CSSOMString>
  {
    KRYS_NODISCARD css::CSSOMString operator()(BuilderState &state, const css::PrimitiveValue &value) noexcept
    {
      if (!value.IsString()) KRYS_UNLIKELY
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return {};
      }
      return value.StringValue();
    }

    KRYS_NODISCARD css::CSSOMString operator()(BuilderState &state, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue) KRYS_UNLIKELY
      {
        return {};
      }
      return (*this)(state, *primitiveValue);
    }
  };

  // Specialization for `css::CSSOMStringAtom`.
  template <>
  struct CSSValueConversion<css::CSSOMStringAtom>
  {
    KRYS_NODISCARD css::CSSOMStringAtom operator()(BuilderState &state,
                                                   const css::PrimitiveValue &value) noexcept
    {
      if (!value.IsString()) KRYS_UNLIKELY
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::CSSOMStringAtom::Empty();
      }
      return css::CSSOMStringAtom {value.StringValue()};
    }

    KRYS_NODISCARD css::CSSOMStringAtom operator()(BuilderState &state, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue) KRYS_UNLIKELY
      {
        return css::CSSOMStringAtom::Empty();
      }
      return (*this)(state, *primitiveValue);
    }
  };

  // Specialization for `css::CustomIdentifier`.
  template <>
  struct CSSValueConversion<css::CustomIdentifier>
  {
    KRYS_NODISCARD css::CustomIdentifier operator()(BuilderState &state,
                                                    const css::PrimitiveValue &value) noexcept
    {
      if (!value.IsCustomIdent()) KRYS_UNLIKELY
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return {.Value = css::CSSOMStringAtom::Empty()};
      }
      return {.Value = css::CSSOMStringAtom {value.CustomIdent()}};
    }

    KRYS_NODISCARD css::CustomIdentifier operator()(BuilderState &state, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue) KRYS_UNLIKELY
      {
        return {.Value = css::CSSOMStringAtom::Empty()};
      }
      return (*this)(state, *primitiveValue);
    }
  };

  // Specialization for `css::PropertyIdentifier`.
  template <>
  struct CSSValueConversion<css::PropertyIdentifier>
  {
    KRYS_NODISCARD css::PropertyIdentifier operator()(BuilderState &state,
                                                      const css::PrimitiveValue &value) noexcept
    {
      if (!value.IsPropertyId()) KRYS_UNLIKELY
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return {.Value = css::PropertyId::Invalid};
      }
      return {.Value = value.PropertyId()};
    }

    KRYS_NODISCARD css::PropertyIdentifier operator()(BuilderState &state, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue) KRYS_UNLIKELY
      {
        return {.Value = css::PropertyId::Invalid};
      }
      return (*this)(state, *primitiveValue);
    }
  };

  // Specialization for `TupleLike` (wrapper).
  template <css::TupleLike StyleType>
  requires(std::tuple_size_v<StyleType> == 1)
  struct CSSValueConversion<StyleType>
  {
    KRYS_NODISCARD StyleType operator()(BuilderState &state, const css::Value &value) noexcept
    {
      return {ConvertCSSValueToStyle<remove_cvref_t<std::tuple_element_t<0, StyleType>>>(state, value)};
    }
  };

  // Specialization for `css::SpaceSeparatedEnumSet`.
  template <typename T>
  struct CSSValueConversion<css::SpaceSeparatedEnumSet<T>>
  {
    KRYS_NODISCARD css::SpaceSeparatedEnumSet<T> operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
    {
      if (auto *list = DynamicDowncast<css::ValueList>(value);
          list && list->Separator() == css::ValueSeparator::SpaceSeparator)
      {
        return css::SpaceSeparatedEnumSet<T>::map(*list, [&](const css::Value &element)
                                                  { return ConvertCSSValueToStyle<T>(state, element); });
      }
      return {ConvertCSSValueToStyle<T>(state, value)};
    }
  };

  // Specialization for `css::CommaSeparatedEnumSet`.
  template <typename T>
  struct CSSValueConversion<css::CommaSeparatedEnumSet<T>>
  {
    KRYS_NODISCARD css::CommaSeparatedEnumSet<T> operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
    {
      if (auto *list = DynamicDowncast<css::ValueList>(value);
          list && list->Separator() == css::ValueSeparator::CommaSeparator)
      {
        return css::CommaSeparatedEnumSet<T>::map(*list, [&](const css::Value &element)
                                                  { return ConvertCSSValueToStyle<T>(state, element); });
      }
      return {ConvertCSSValueToStyle<T>(state, value)};
    }
  };

  // Specialization for `css::SpaceSeparatedListHashSet`.
  template <typename T>
  struct CSSValueConversion<css::SpaceSeparatedListHashSet<T>>
  {
    KRYS_NODISCARD css::SpaceSeparatedListHashSet<T> operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
    {
      if (auto *list = DynamicDowncast<css::ValueList>(value))
      {
        return css::SpaceSeparatedListHashSet<T>::map(*list, [&](const css::Value &element)
                                                      { return ConvertCSSValueToStyle<T>(state, element); });
      }
      return {ConvertCSSValueToStyle<T>(state, value)};
    }
  };

  // Specialization for `css::CommaSeparatedListHashSet`.
  template <typename T>
  struct CSSValueConversion<css::CommaSeparatedListHashSet<T>>
  {
    KRYS_NODISCARD css::CommaSeparatedListHashSet<T> operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
    {
      if (auto *list = DynamicDowncast<css::ValueList>(value))
      {
        return css::CommaSeparatedListHashSet<T>::map(*list, [&](const css::Value &element)
                                                      { return ConvertCSSValueToStyle<T>(state, element); });
      }
      return {ConvertCSSValueToStyle<T>(state, value)};
    }
  };

  // Specialization for `css::SpaceSeparatedFixedList`.
  template <typename StyleType>
  struct CSSValueConversion<css::SpaceSeparatedFixedList<StyleType>>
  {
    KRYS_NODISCARD css::SpaceSeparatedFixedList<StyleType> operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
    {
      if (auto *list = DynamicDowncast<css::ValueList>(value);
          list && list->Separator() == css::ValueSeparator::SpaceSeparator)
      {
        return css::SpaceSeparatedFixedList<StyleType>::map(
          *list,
          [&](const css::Value &element) { return ConvertCSSValueToStyle<StyleType>(state, element); });
      }
      return {ConvertCSSValueToStyle<StyleType>(state, value)};
    }
  };

  // Specialization for `css::CommaSeparatedFixedList`.
  template <typename StyleType>
  struct CSSValueConversion<css::CommaSeparatedFixedList<StyleType>>
  {
    KRYS_NODISCARD css::CommaSeparatedFixedList<StyleType> operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
    {
      if (auto *list = DynamicDowncast<css::ValueList>(value);
          list && list->Separator() == css::ValueSeparator::CommaSeparator)
      {
        return css::CommaSeparatedFixedList<StyleType>::map(
          *list,
          [&](const css::Value &element) { return ConvertCSSValueToStyle<StyleType>(state, element); });
      }
      return {ConvertCSSValueToStyle<StyleType>(state, value)};
    }
  };

  // Specialization for `css::ValueOrKeyword`.
  template <typename StyleType, typename Keyword, typename StyleTypeMarkableTraits>
  struct CSSValueConversion<css::ValueOrKeyword<StyleType, Keyword, StyleTypeMarkableTraits>>
  {
    KRYS_NODISCARD css::ValueOrKeyword<StyleType, Keyword, StyleTypeMarkableTraits>
      operator()(BuilderState &state, const css::Value &value) noexcept
    {
      if (value.ValueId() == Keyword::Value)
      {
        return Keyword {};
      }
      return ConvertCSSValueToStyle<StyleType>(state, value);
    }
  };

  // Specialization for types derived from `css::ValueOrKeyword`.
  template <css::ValueOrKeywordDerived T>
  struct CSSValueConversion<T>
  {
    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      if (value.ValueId() == T::Keyword::Value)
      {
        return typename T::Keyword {};
      }
      return ConvertCSSValueToStyle<typename T::Value>(state, value);
    }
  };

  // Specialization for `css::ListOrNone`.
  template <typename ListType>
  struct CSSValueConversion<css::ListOrNone<ListType>>
  {
    KRYS_NODISCARD css::ListOrNone<ListType> operator()(BuilderState &state, const css::Value &value) noexcept
    {
      if (value.ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }
      return ConvertCSSValueToStyle<ListType>(state, value);
    }
  };

  // Specialization for types derived from `css::ListOrNone`.
  template <css::ListOrNoneDerived T>
  struct CSSValueConversion<T>
  {
    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      if (value.ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }
      return ConvertCSSValueToStyle<typename T::List>(state, value);
    }
  };

  // Specialization for `css::ListOrDefault`.
  template <typename ListType, typename Defaulter>
  struct CSSValueConversion<css::ListOrDefault<ListType, Defaulter>>
  {
    KRYS_NODISCARD css::ListOrDefault<ListType, Defaulter> operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
    {
      return ConvertCSSValueToStyle<ListType>(state, value);
    }
  };

  // Specialization for types derived from `css::ListOrDefault`.
  template <css::ListOrDefaultDerived T>
  struct CSSValueConversion<T>
  {
    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      return ConvertCSSValueToStyle<typename T::List>(state, value);
    }
  };

  // Specialization for types derived from `css::EnumSetOrKeywordBase`.
  template <css::EnumSetOrKeywordBaseDerived T>
  struct CSSValueConversion<T>
  {
    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      if (value.ValueId() == T::Keyword::Value)
      {
        return typename T::Keyword {};
      }
      return ConvertCSSValueToStyle<typename T::EnumSet>(state, value);
    }
  };
}
