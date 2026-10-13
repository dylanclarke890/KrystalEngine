#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace krys
{
  template <typename... Types>
  using Variant = std::variant<Types...>;

  template <typename... Types>
  using VariantOrSingle =
    conditional_t<(sizeof...(Types) == 1), std::tuple_element_t<0, std::tuple<Types...>>, Variant<Types...>>;

  // `HoldsAlternative<T/I>` are namespaced versions of `std::holds_alternative<T/I>` that work with any
  // "variant-like".

  // Default implementation expects "variant-like" to have "HoldsAlternative" member functions.
  template <typename V>
  struct HoldsAlternativeDispatcher
  {
    template <typename T>
    KRYS_NODISCARD constexpr static bool HoldsAlternative(const V &v) noexcept
    {
      return v.template HoldsAlternative<T>();
    }

    template <size_t I>
    KRYS_NODISCARD constexpr static bool HoldsAlternative(const V &v) noexcept
    {
      return v.template HoldsAlternative<I>();
    }
  };

  // Specialization for `Variant`.
  template <typename... Ts>
  struct HoldsAlternativeDispatcher<Variant<Ts...>>
  {
    template <typename T>
    KRYS_NODISCARD constexpr static bool HoldsAlternative(const Variant<Ts...> &v) noexcept
    {
      return std::holds_alternative<T>(v);
    }

    template <size_t I>
    KRYS_NODISCARD constexpr static bool HoldsAlternative(const Variant<Ts...> &v) noexcept
    {
      return std::holds_alternative<I>(v);
    }
  };

  template <typename T, typename V>
  KRYS_NODISCARD constexpr bool HoldsAlternative(const V &v) noexcept
  {
    return HoldsAlternativeDispatcher<V>::template HoldsAlternative<T>(v);
  }

  template <size_t I, typename V>
  KRYS_NODISCARD constexpr bool HoldsAlternative(const V &v) noexcept
  {
    return HoldsAlternativeDispatcher<V>::template HoldsAlternative<I>(v);
  }

  /// @brief The index of `T` in the variant `V`. `T` must appear exactly once.
  template <typename T, typename V>
  struct AlternativeIndex;

  template <typename T, typename... Ts>
  struct AlternativeIndex<T, Variant<Ts...>>
  {
  private:
    constexpr static size_t Find() noexcept
    {
      constexpr bool matches[] = {SameType<T, Ts>...};
      size_t index = sizeof...(Ts);
      size_t count = 0;
      for (size_t i = 0; i < sizeof...(Ts); ++i)
      {
        if (matches[i])
        {
          index = i;
          ++count;
        }
      }
      return count == 1 ? index : sizeof...(Ts);
    }

  public:
    constexpr static size_t Value = Find();
    static_assert(Value < sizeof...(Ts), "T must appear exactly once in the variant.");
  };

  template <typename T, typename V>
  constexpr size_t AlternativeIndexV = AlternativeIndex<T, V>::Value;

  namespace detail_variant
  {
    template <typename V, size_t I, typename F>
    constexpr decltype(auto) TypeForIndexImpl(size_t index, F &f)
    {
      if constexpr (I + 1 == std::variant_size_v<V>)
      {
        return f.template operator()<std::variant_alternative_t<I, V>>();
      }
      else
      {
        if (index == I)
        {
          return f.template operator()<std::variant_alternative_t<I, V>>();
        }

        return TypeForIndexImpl<V, I + 1>(index, f);
      }
    }
  }

  /// @brief Calls a zero argument functor with a template argument corresponding to the index's alternative.
  ///
  /// e.g.
  ///   using V = Variant<int, float>;
  ///   TypeForIndex<V>(1, []<typename T>() { /* T is float */ });
  template <typename V, typename F>
  constexpr decltype(auto) TypeForIndex(size_t index, F &&f)
  {
    return detail_variant::TypeForIndexImpl<remove_cvref_t<V>, 0>(index, f);
  }

  // `VariantBestMatch` picks the type `T` from `Ts...` in `Variant<Ts...>` that will be used when the
  // `Variant<Ts...>` is constructed from type `Arg`. Implementation based off of libc++.
  namespace detail_variant
  {
    struct NoNarrowingCheck
    {
      template <typename D, typename S>
      using Apply = std::type_identity_t<D>;
    };

    struct NarrowingCheck
    {
      template <typename D>
      static auto Test(D (&&)[1]) -> std::type_identity_t<D>;

      template <typename D, typename S>
      using Apply = decltype(Test<D>({std::declval<S>()}));
    };

    template <typename D, typename S>
    using CheckForNarrowing =
      typename conditional_t<std::is_arithmetic_v<D>, NarrowingCheck, NoNarrowingCheck>::template Apply<D, S>;

    template <typename T, size_t I>
    struct Overload
    {
      template <typename U>
      auto operator()(T, U &&) const -> CheckForNarrowing<T, U>;
    };

    template <typename... Bases>
    struct AllOverloads : Bases...
    {
      void operator()() const;
      using Bases::operator()...;
    };

    template <typename Seq>
    struct MakeOverloadsImpl;

    template <size_t... I>
    struct MakeOverloadsImpl<std::index_sequence<I...>>
    {
      template <typename... Ts>
      using Apply = AllOverloads<Overload<Ts, I>...>;
    };

    template <typename... Ts>
    using MakeOverloads =
      typename MakeOverloadsImpl<std::make_index_sequence<sizeof...(Ts)>>::template Apply<Ts...>;

    template <typename T, typename... Ts>
    using BestMatchImpl = std::invoke_result_t<MakeOverloads<Ts...>, T, T>;
  }

  template <typename V, typename Arg>
  struct VariantBestMatch;

  template <typename Arg, typename... Ts>
  struct VariantBestMatch<Variant<Ts...>, Arg>
  {
    using type = detail_variant::BestMatchImpl<Arg, Ts...>;
  };

  template <typename T>
  constexpr bool IsInPlaceType = false;

  template <typename T>
  constexpr bool IsInPlaceType<std::in_place_type_t<T>> = true;

  template <typename T>
  constexpr bool IsInPlaceIndex = false;

  template <size_t I>
  constexpr bool IsInPlaceIndex<std::in_place_index_t<I>> = true;

  /// @brief Utility macro for wrapping a variant in a struct
#define KRYS_FORWARD_VARIANT_FUNCTIONS(Self, name)                                                           \
  KRYS_NODISCARD size_t index() const noexcept                                                               \
  {                                                                                                          \
    return name.index();                                                                                     \
  }                                                                                                          \
                                                                                                             \
  template <typename... F>                                                                                   \
  KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept                                            \
  {                                                                                                          \
    return krys::SwitchOn(name, std::forward<F>(f)...);                                                      \
  }                                                                                                          \
                                                                                                             \
  template <typename... F>                                                                                   \
  KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) noexcept                                                  \
  {                                                                                                          \
    return krys::SwitchOn(name, std::forward<F>(f)...);                                                      \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD bool HoldsAlternative() const noexcept                                                      \
  {                                                                                                          \
    return krys::HoldsAlternative<T>(name);                                                                  \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD friend T &get(Self &self) noexcept                                                          \
  {                                                                                                          \
    return std::get<T>(self.name);                                                                           \
  }                                                                                                          \
  template <typename T>                                                                                      \
  KRYS_NODISCARD friend T &&get(Self &&self) noexcept                                                        \
  {                                                                                                          \
    return std::get<T>(krys::move(self.name));                                                               \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD friend const T &get(const Self &self) noexcept                                              \
  {                                                                                                          \
    return std::get<T>(self.name);                                                                           \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD friend const T &&get(const Self &&self) noexcept                                            \
  {                                                                                                          \
    return std::get<T>(krys::move(self.name));                                                               \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD friend add_ptr_t<T> get_if(Self *self) noexcept                                             \
  {                                                                                                          \
    return std::get_if<T>(&self->name);                                                                      \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD friend add_ptr_t<const T> get_if(const Self *self) noexcept                                 \
  {                                                                                                          \
    return std::get_if<T>(&self->name);                                                                      \
  }
}