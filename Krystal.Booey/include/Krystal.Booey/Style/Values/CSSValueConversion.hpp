#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // Specialization for `css::CSSOMString`.
  template <>
  struct CSSValueConversion<css::CSSOMString>
  {
    css::CSSOMString operator()(BuilderState &state, const css::PrimitiveValue &value)
    {
      if (!value.IsString()) [[unlikely]]
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return {};
      }

      return value.String();
    }

    css::CSSOMString operator()(BuilderState &state, const css::Value &value)
    {
      auto primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue) [[unlikely]]
      {
        return {};
      }

      return this->operator()(state, *primitiveValue);
    }
  };

  // Specialization for `css::CSSOMStringAtom`.
  template <>
  struct CSSValueConversion<css::CSSOMStringAtom>
  {
    css::CSSOMStringAtom operator()(BuilderState &state, const css::PrimitiveValue &value)
    {
      if (!value.IsString()) [[unlikely]]
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::CSSOMStringAtom::Empty();
      }

      return css::CSSOMStringAtom {value.String()};
    }

    css::CSSOMStringAtom operator()(BuilderState &state, const css::Value &value)
    {
      auto primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue) [[unlikely]]
      {
        return css::CSSOMStringAtom::Empty();
      }

      return this->operator()(state, *primitiveValue);
    }
  };

  // Specialization for `CustomIdentifier`.
  template <>
  struct CSSValueConversion<css::CustomIdentifier>
  {
    css::CustomIdentifier operator()(BuilderState &state, const css::PrimitiveValue &value)
    {
      if (!value.IsCustomIdent()) [[unlikely]]
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return {.value = css::CSSOMStringAtom::Empty()};
      }
      return {.value = css::CSSOMStringAtom {value.CustomIdent()}};
    }

    css::CustomIdentifier operator()(BuilderState &state, const css::Value &value)
    {
      RefPtr primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue) [[unlikely]]
      {
        return {.value = css::CSSOMStringAtom::Empty()};
      }

      return this->operator()(state, *primitiveValue);
    }
  };

  // Specialization for `PropertyIdentifier`.
  template <>
  struct CSSValueConversion<css::PropertyIdentifier>
  {
    css::PropertyIdentifier operator()(BuilderState &state, const css::PrimitiveValue &value)
    {
      if (!value.IsPropertyId()) [[unlikely]]
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return {.value = css::PropertyId::Invalid};
      }

      return {.value = value.PropertyId()};
    }

    css::PropertyIdentifier operator()(BuilderState &state, const css::Value &value)
    {
      RefPtr primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue) [[unlikely]]
      {
        return {.value = css::PropertyId::Invalid};
      }

      return this->operator()(state, *primitiveValue);
    }
  };

  // Specialization for `TupleLike` (wrapper).
  template <css::TupleLike StyleType>
  requires(std::tuple_size_v<StyleType> == 1)
  struct CSSValueConversion<StyleType>
  {
    StyleType operator()(BuilderState &state, const css::Value &value)
    {
      return {ConvertCSSValueToStyle<std::remove_cvref_t<std::tuple_element_t<0, StyleType>>>(state, value)};
    }
  };

  // Specialization for `css::SpaceSeparatedEnumSet`.
  template <typename T>
  struct CSSValueConversion<css::SpaceSeparatedEnumSet<T>>
  {
    css::SpaceSeparatedEnumSet<T> operator()(BuilderState &state, const css::Value &value)
    {
      if (auto list = DynamicDowncast<css::ValueList>(value);
          list && list->Separator() == css::ValueList::SpaceSeparator)
      {
        return css::SpaceSeparatedEnumSet<T>::Map(*list, [&](const css::Value &element)
                                                  { return ConvertCSSValueToStyle<T>(state, element); });
      }

      return {ConvertCSSValueToStyle<T>(state, value)};
    }
  };

  // Specialization for `css::CommaSeparatedEnumSet`.
  template <typename T>
  struct CSSValueConversion<css::CommaSeparatedEnumSet<T>>
  {
    css::CommaSeparatedEnumSet<T> operator()(BuilderState &state, const css::Value &value)
    {
      if (auto list = DynamicDowncast<css::ValueList>(value);
          list && list->Separator() == css::ValueList::CommaSeparator)
      {
        return css::CommaSeparatedEnumSet<T>::Map(*list, [&](const css::Value &element)
                                                  { return ConvertCSSValueToStyle<T>(state, element); });
      }

      return {ConvertCSSValueToStyle<T>(state, value)};
    }
  };

  // Specialization for `SpaceSeparatedListHashSet`.
  template <typename T>
  struct CSSValueConversion<css::SpaceSeparatedListHashSet<T>>
  {
    css::SpaceSeparatedListHashSet<T> operator()(BuilderState &state, const css::Value &value)
    {
      if (auto list = dynamicDowncast<CSSValueList>(value))
      {
        return css::SpaceSeparatedListHashSet<T>::map(*list, [&](const css::Value &element)
                                                 { return ConvertCSSValueToStyle<T>(state, element); });
      }
      return {ConvertCSSValueToStyle<T>(state, value)};
    }
  };

  // Specialization for `CommaSeparatedListHashSet`.
  template <typename T>
  struct CSSValueConversion<css::CommaSeparatedListHashSet<T>>
  {
    css::CommaSeparatedListHashSet<T> operator()(BuilderState &state, const css::Value &value)
    {
      if (auto list = dynamicDowncast<CSSValueList>(value))
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
    css::SpaceSeparatedFixedList<StyleType> operator()(BuilderState &state, const css::Value &value)
    {
      if (auto list = DynamicDowncast<css::ValueList>(value);
          list && list->Separator() == css::ValueList::SpaceSeparator)
      {
        return css::SpaceSeparatedFixedList<StyleType>::Map(
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
    css::CommaSeparatedFixedList<StyleType> operator()(BuilderState &state, const css::Value &value)
    {
      if (auto list = DynamicDowncast<css::ValueList>(value);
          list && list->Separator() == css::ValueList::CommaSeparator)
      {
        return css::CommaSeparatedFixedList<StyleType>::Map(
          *list,
          [&](const css::Value &element) { return ConvertCSSValueToStyle<StyleType>(state, element); });
      }

      return {ConvertCSSValueToStyle<StyleType>(state, value)};
    }
  };

  // Specialization for `ValueOrKeyword`.
  template <typename StyleType, typename Keyword, typename StyleTypeMarkableTraits>
  struct CSSValueConversion<css::ValueOrKeyword<StyleType, Keyword, StyleTypeMarkableTraits>>
  {
    css::ValueOrKeyword<StyleType, Keyword, StyleTypeMarkableTraits> operator()(BuilderState &state,
                                                                                const css::Value &value)
    {
      if (value.ValueId() == Keyword {}.Value)
      {
        return Keyword {};
      }

      return ConvertCSSValueToStyle<StyleType>(state, value);
    }
  };

  // Specialization for types derived from `ValueOrKeyword`.
  template <css::ValueOrKeywordDerived T>
  struct CSSValueConversion<T>
  {
    T operator()(BuilderState &state, const css::Value &value)
    {
      if (value.ValueId() == typename T::Keyword {}.Value)
      {
        return typename T::Keyword {};
      }

      return ConvertCSSValueToStyle<typename T::Value>(state, value);
    }
  };

  // Specialization for `ListOrNone`.
  template <typename ListType>
  struct CSSValueConversion<css::ListOrNone<ListType>>
  {
    css::ListOrNone<ListType> operator()(BuilderState &state, const css::Value &value)
    {
      if (value.ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }

      return ConvertCSSValueToStyle<ListType>(state, value);
    }
  };

  // Specialization for types derived from `ListOrNone`.
  template <css::ListOrNoneDerived T>
  struct CSSValueConversion<T>
  {
    T operator()(BuilderState &state, const css::Value &value)
    {
      if (value.ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }

      return ConvertCSSValueToStyle<T>(state, value);
    }
  };

  // Specialization for `ListOrDefault`.
  template <typename ListType, typename Defaulter>
  struct CSSValueConversion<css::ListOrDefault<ListType, Defaulter>>
  {
    KRYS_NODISCARD css::ListOrDefault<ListType, Defaulter> operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
    {
      return ConvertCSSValueToStyle<ListType>(state, value);
    }
  };

  // Specialization for types derived from `ListOrDefault`.
  template <css::ListOrDefaultDerived T>
  struct CSSValueConversion<T>
  {
    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      return ConvertCSSValueToStyle<typename T::List>(state, value);
    }
  };

  // Specialization for types derived from `EnumSetOrKeywordBase`.
  template <css::EnumSetOrKeywordBaseDerived T>
  struct CSSValueConversion<T>
  {
    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      if (value.ValueId() == typename T::Keyword {}.Value)
      {
        return typename T::Keyword {};
      }

      return ConvertCSSValueToStyle<typename T::EnumSet>(state, value);
    }
  };
}