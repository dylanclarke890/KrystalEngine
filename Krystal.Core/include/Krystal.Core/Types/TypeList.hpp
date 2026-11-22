#pragma once

#include "Krystal.Core/TypeTraits.hpp"

namespace krys
{
  /// @brief A list of types, for computing with them at compile time.
  template <typename... Ts>
  struct TypeList
  {
  };

  namespace detail
  {
    template <typename TList, template <typename...> typename TTemplate>
    struct ApplyTypeList;

    template <typename... Ts, template <typename...> typename TTemplate>
    struct ApplyTypeList<TypeList<Ts...>, TTemplate>
    {
      using type = TTemplate<Ts...>;
    };

    template <typename TList, template <typename> typename TMetaFunction>
    struct TransformTypeList;

    template <typename... Ts, template <typename> typename TMetaFunction>
    struct TransformTypeList<TypeList<Ts...>, TMetaFunction>
    {
      using type = TypeList<typename TMetaFunction<Ts>::type...>;
    };

    template <typename... TLists>
    struct ConcatTypeLists;

    template <>
    struct ConcatTypeLists<>
    {
      using type = TypeList<>;
    };

    template <typename... Ts>
    struct ConcatTypeLists<TypeList<Ts...>>
    {
      using type = TypeList<Ts...>;
    };

    template <typename... Ts, typename... Us, typename... TRest>
    struct ConcatTypeLists<TypeList<Ts...>, TypeList<Us...>, TRest...>
    {
      using type = typename ConcatTypeLists<TypeList<Ts..., Us...>, TRest...>::type;
    };

    template <typename TList, template <typename> typename TPredicate>
    struct RemoveTypesIf;

    template <typename... Ts, template <typename> typename TPredicate>
    struct RemoveTypesIf<TypeList<Ts...>, TPredicate>
    {
      using type =
        typename ConcatTypeLists<conditional_t<TPredicate<Ts>::value, TypeList<>, TypeList<Ts>>...>::type;
    };
  }

  /// @brief @p TTemplate instantiated with the types of @p TList (`TypeList<A, B>` -> `TTemplate<A, B>`).
  template <typename TList, template <typename...> typename TTemplate>
  using ApplyTypeList = typename detail::ApplyTypeList<TList, TTemplate>::type;

  /// @brief Each type `T` of @p TList replaced with `TMetaFunction<T>::type`.
  template <typename TList, template <typename> typename TMetaFunction>
  using TransformTypeList = typename detail::TransformTypeList<TList, TMetaFunction>::type;

  /// @brief @p TList with @p Ts added at the end.
  template <typename TList, typename... Ts>
  using AppendToTypeList = typename detail::ConcatTypeLists<TList, TypeList<Ts...>>::type;

  /// @brief @p TList without the types `T` for which `TPredicate<T>::value` is true.
  template <typename TList, template <typename> typename TPredicate>
  using RemoveTypesIf = typename detail::RemoveTypesIf<TList, TPredicate>::type;
}
