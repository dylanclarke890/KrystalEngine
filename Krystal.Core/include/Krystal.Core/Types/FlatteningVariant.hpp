#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CompactVariant.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <utility>

namespace krys
{
  // `FlatteningVariantAdaptor` adapts "Variant-Like" types and attempts to
  // minimize memory usage by *flattening* nested "Variant-Like" types.
  //
  // For example, take the following usage:
  //
  //   using Foo = FlatteningVariantAdaptor<Variant, Variant<std::string, int>, double>;
  //
  // Here, `Foo` will be internally represented by:
  //
  //   Variant<std::string, int, double>
  //
  // allowing a single index to be used for all options.
  //
  // To add support for flattening to variant-like type, clients can specialize
  // `FlatteningVariantTraits` to expose the list of the nested types.
  //
  // Standard aliases for a flattening `Variant` and `CompactVariant` are
  // provided called `FlatteningVariant` and `FlatteningCompactVariant`.

  /// @brief A list of types.
  template <typename... Ts>
  struct FlatteningTypeList
  {
    constexpr static size_t Size = sizeof...(Ts);

    template <template <typename...> typename Wrapper>
    using WrapIn = Wrapper<Ts...>;
  };

  namespace detail_flattening
  {
    template <typename... Lists>
    struct Append;

    template <>
    struct Append<>
    {
      using Type = FlatteningTypeList<>;
    };

    template <typename... Ts>
    struct Append<FlatteningTypeList<Ts...>>
    {
      using Type = FlatteningTypeList<Ts...>;
    };

    template <typename... Ts, typename... Us, typename... Rest>
    struct Append<FlatteningTypeList<Ts...>, FlatteningTypeList<Us...>, Rest...>
    {
      using Type = typename Append<FlatteningTypeList<Ts..., Us...>, Rest...>::Type;
    };
  }

#pragma region FlatteningVariantTraits

  template <typename T>
  struct FlatteningVariantTraits
  {
    using TypeList = FlatteningTypeList<T>;
  };

  template <typename... Ts>
  struct FlatteningVariantTraits<Variant<Ts...>>
  {
    using TypeList = FlatteningTypeList<Ts...>;
  };

  template <typename... Ts>
  struct FlatteningVariantTraits<CompactVariant<Ts...>>
  {
    using TypeList = FlatteningTypeList<Ts...>;
  };

#pragma endregion

#pragma region FlatteningVariantAdaptor

  template <template <typename...> typename VariantType, typename... Ts>
  class FlatteningVariantAdaptor
  {
    template <typename... Us>
    using Wrapper = VariantType<Us...>;

  public:
    using TypeList =
      typename detail_flattening::Append<typename FlatteningVariantTraits<Ts>::TypeList...>::Type;
    using Representation = typename TypeList::template WrapIn<Wrapper>;

  private:
    Representation _value;

  public:
    template <typename T>
    KRYS_NODISCARD constexpr static Representation Extract(T &&value) noexcept
    {
      if constexpr (FlatteningVariantTraits<remove_cvref_t<T>>::TypeList::Size == 1)
      {
        return std::forward<T>(value);
      }
      else
      {
        return krys::SwitchOn(std::forward<T>(value), []<typename U>(U &&alternative) -> Representation
                              { return Extract(std::forward<U>(alternative)); });
      }
    }

    template <typename T>
    constexpr FlatteningVariantAdaptor(T &&value) noexcept : _value {Extract(std::forward<T>(value))}
    {
    }

    // Checks if type `T` is included in the flattened list of types.
    template <typename T>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      return krys::HoldsAlternative<T>(_value);
    }

    // Switches on the flattened list of types.
    template <typename... F>
    decltype(auto) SwitchOn(F &&...functors) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(functors)...);
    }

    // FIXME: Add `HoldsAlternativeFromOriginalList` that checks all flattened alternatives for the provided
    // type.
    // FIXME: Add `SwitchOnOriginalList` that reconstructs extracted values.

    KRYS_NODISCARD bool operator==(const FlatteningVariantAdaptor &) const noexcept = default;
  };

  // Specialization of `FlatteningVariantTraits` for `FlatteningVariantAdaptor` itself so that
  // recursively used `FlatteningVariantAdaptor` types can flatten together.
  template <template <typename...> typename VariantType, typename... Ts>
  struct FlatteningVariantTraits<FlatteningVariantAdaptor<VariantType, Ts...>>
  {
    using TypeList = typename FlatteningVariantAdaptor<VariantType, Ts...>::TypeList;
  };

#pragma endregion

#pragma region Standard FlatteningVariantAdaptor Aliases

  template <typename... Ts>
  using FlatteningVariant = FlatteningVariantAdaptor<Variant, Ts...>;

  template <typename... Ts>
  using FlatteningCompactVariant = FlatteningVariantAdaptor<CompactVariant, Ts...>;

#pragma endregion
}
