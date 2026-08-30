#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include <tuple>
#include <variant>

namespace Krys
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
    return Krys::SwitchOn(name, std::forward<F>(f)...);                                                      \
  }                                                                                                          \
                                                                                                             \
  template <typename... F>                                                                                   \
  KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) noexcept                                                  \
  {                                                                                                          \
    return Krys::SwitchOn(name, std::forward<F>(f)...);                                                      \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD bool HoldsAlternative() const noexcept                                                      \
  {                                                                                                          \
    return Krys::HoldsAlternative<T>(name);                                                                 \
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
    return std::get<T>(Krys::Move(self.name));                                                               \
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
    return std::get<T>(std::move(self.name));                                                                \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD friend add_pointer_t<T> get_if(Self *self) noexcept                                         \
  {                                                                                                          \
    return std::get_if<T>(&self->name);                                                                      \
  }                                                                                                          \
                                                                                                             \
  template <typename T>                                                                                      \
  KRYS_NODISCARD friend add_pointer_t<const T> get_if(const Self *self) noexcept                             \
  {                                                                                                          \
    return std::get_if<T>(&self->name);                                                                      \
  }
}