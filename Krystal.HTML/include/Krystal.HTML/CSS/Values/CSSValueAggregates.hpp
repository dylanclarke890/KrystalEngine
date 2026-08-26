#pragma once

#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueConcepts.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Variant.hpp"
#include <tuple>

namespace Krys::HTML
{
  template <class F, class T, size_t... I>
  constexpr decltype(auto) apply_impl(F &&functor, T &&tupleLike, std::index_sequence<I...>) noexcept
  {
    using std::get;
    return std::invoke(std::forward<F>(functor), get<I>(std::forward<T>(tupleLike))...);
  }

  template <class F, class T>
  constexpr decltype(auto) apply(F &&functor, T &&tupleLike) noexcept
  {
    return apply_impl(std::forward<F>(functor), std::forward<T>(tupleLike),
                      std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>> {});
  }

#pragma region SerializationSeparatorType

  enum class SerializationSeparatorType : uint8
  {
    None,
    Space,
    Comma,
    Slash
  };

  /// @brief Types that specialize TreatAsTupleLike or TreatAsRangeLike can specialize this to indicate how to
  /// serialize the gaps between elements.
  template <typename>
  constexpr SerializationSeparatorType SerializationSeparator = SerializationSeparatorType::None;

#define DEFINE_SERIALIZATION_SEPARATOR_FOR_TYPE(T, separator)                                                \
  namespace Krys::HTML                                                                                       \
  {                                                                                                          \
    template <>                                                                                              \
    constexpr SerializationSeparatorType SerializationSeparator<T> = separator;                              \
  }

#pragma endregion

#pragma region SerializationCoalescingType

  enum class SerializationCoalescingType : uint8
  {
    None,
    Minimal
  };

  /// @brief Types that specialize TreatAsTupleLike and have size 2 or 4 can specialize this to indicate how
  /// to serialize identical elements.
  template <typename>
  constexpr SerializationCoalescingType SerializationCoalescing = SerializationCoalescingType::None;

#define DEFINE_SERIALIZATION_COALESCING_FOR_TYPE(T, coalescing)                                              \
  namespace Krys::HTML                                                                                       \
  {                                                                                                          \
    template <>                                                                                              \
    constexpr SerializationCoalescingType SerializationCoalescing<T> = coalescing;                           \
  }

#pragma endregion

#pragma region SerializationSeparatorStringForType

  template <SerializationSeparatorType>
  constexpr CSSOMStringView SerializationSeparatorStringForType = u8"";

  template <>
  constexpr CSSOMStringView SerializationSeparatorStringForType<SerializationSeparatorType::Space> = u8" ";

  template <>
  constexpr CSSOMStringView SerializationSeparatorStringForType<SerializationSeparatorType::Comma> = u8", ";

  template <>
  constexpr CSSOMStringView SerializationSeparatorStringForType<SerializationSeparatorType::Slash> = u8" / ";

  template <typename T>
  constexpr CSSOMStringView SerializationSeparatorString =
    SerializationSeparatorStringForType<SerializationSeparator<T>>;

#pragma endregion

#pragma region Helper Macros

  /// @brief Defines a simple `get()` implementation for a single value `name`.
#define DEFINE_TYPE_WRAPPER_GET(T, name)                                                                     \
  template <size_t>                                                                                          \
  KRYS_NODISCARD const auto &get(const T &value) noexcept                                                    \
  {                                                                                                          \
    return value.name;                                                                                       \
  }

/// @brief Defines a type by extending another type via inheritance.
#define DEFINE_TYPE_EXTENDER(wrapper, wrapped)                                                               \
  struct wrapper : wrapped                                                                                   \
  {                                                                                                          \
    using Wrapped = wrapped;                                                                                 \
    using Wrapped::Wrapped;                                                                                  \
                                                                                                             \
    template <size_t I>                                                                                      \
    friend KRYS_NODISCARD const auto &get(const wrapper &self) noexcept                                      \
    {                                                                                                        \
      return get<I>(static_cast<const wrapped &>(self));                                                     \
    }                                                                                                        \
                                                                                                             \
    bool operator==(const wrapper &) const = default;                                                        \
  };

/// @brief Defines a type via direct wrapping of another type.
#define DEFINE_TYPE_WRAPPER(wrapper, wrapped)                                                                \
  struct wrapper                                                                                             \
  {                                                                                                          \
    using Wrapped = wrapped;                                                                                 \
                                                                                                             \
    wrapped value;                                                                                           \
                                                                                                             \
    template <typename... Args>                                                                              \
    wrapper(Args &&...args) noexcept                                                                         \
    requires(requires {                                                                                      \
      { wrapped(args...) };                                                                                  \
    })                                                                                                       \
        : value(std::forward<Args>(args)...)                                                                 \
    {                                                                                                        \
    }                                                                                                        \
                                                                                                             \
    const Wrapped &operator*() const noexcept                                                                \
    {                                                                                                        \
      return value;                                                                                          \
    }                                                                                                        \
                                                                                                             \
    Wrapped &operator*() noexcept                                                                            \
    {                                                                                                        \
      return value;                                                                                          \
    }                                                                                                        \
                                                                                                             \
    const Wrapped *operator->() const noexcept                                                               \
    {                                                                                                        \
      return &value;                                                                                         \
    }                                                                                                        \
                                                                                                             \
    Wrapped *operator->() noexcept                                                                           \
    {                                                                                                        \
      return &value;                                                                                         \
    }                                                                                                        \
                                                                                                             \
    template <size_t>                                                                                        \
    friend KRYS_NODISCARD const auto &get(const wrapper &self) noexcept                                      \
    {                                                                                                        \
      return self.value;                                                                                     \
    }                                                                                                        \
                                                                                                             \
    bool operator==(const wrapper &) const noexcept = default;                                               \
  };

/// @brief Defines tuple-like conformance for a type with `numberOfArguments` arguments.
#define DEFINE_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                                  \
  namespace std                                                                                              \
  {                                                                                                          \
    template <>                                                                                              \
    class tuple_size<T> : public std::integral_constant<size_t, numberOfArguments>                           \
    {                                                                                                        \
    };                                                                                                       \
                                                                                                             \
    template <size_t I>                                                                                      \
    class tuple_element<I, T>                                                                                \
    {                                                                                                        \
    public:                                                                                                  \
      using type = decltype(get<I>(std::declval<T>()));                                                      \
    };                                                                                                       \
  }                                                                                                          \
                                                                                                             \
  template <>                                                                                                \
  constexpr bool ::Krys::HTML::TreatAsTupleLike<T> = true;

/// @brief Defines tuple-like conformance and that the type should be serialized as space separated.
#define DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                  \
  DEFINE_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                                        \
  DEFINE_SERIALIZATION_SEPARATOR_FOR_TYPE(T, Krys::HTML::SerializationSeparatorType::Space)

/// @brief Defines tuple-like conformance and that the type should be serialized as coalescing and space
/// separated.
#define DEFINE_COALESCING_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                       \
  static_assert(numberOfArguments == 2 || numberOfArguments == 4);                                           \
  DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                        \
  DEFINE_SERIALIZATION_COALESCING_FOR_TYPE(T, Krys::HTML::SerializationCoalescingType::Minimal)

/// @brief Defines tuple-like conformance and that the type should be serialized as comma separated.
#define DEFINE_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                  \
  DEFINE_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                                        \
  DEFINE_SERIALIZATION_SEPARATOR_FOR_TYPE(T, Krys::HTML::SerializationSeparatorType::Comma)

/// @brief Defines tuple-like conformance and that the type should be serialized as coalescing and comma
/// separated.
#define DEFINE_COALESCING_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                       \
  static_assert(numberOfArguments == 2 || numberOfArguments == 4);                                           \
  DEFINE_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                        \
  DEFINE_SERIALIZATION_COALESCING_FOR_TYPE(T, Krys::HTML::SerializationCoalescingType::Minimal)

/// @brief Defines tuple-like conformance and that the type should be serialized as slash separated.
#define DEFINE_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                  \
  DEFINE_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                                        \
  DEFINE_SERIALIZATION_SEPARATOR_FOR_TYPE(T, Krys::HTML::SerializationSeparatorType::Slash)

/// @brief Defines tuple-like conformance and that the type should be serialized as coalescing and slash
/// separated.
#define DEFINE_COALESCING_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                       \
  static_assert(numberOfArguments == 2 || numberOfArguments == 4);                                           \
  DEFINE_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                        \
  DEFINE_SERIALIZATION_COALESCING_FOR_TYPE(T, Krys::HTML::SerializationCoalescingType::Minimal)

/// @brief Defines tuple-like conformance based on the type being extended.
#define DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_EXTENDER(T)                                                   \
  DEFINE_TUPLE_LIKE_CONFORMANCE(T, std::tuple_size_v<T::Wrapped>)                                            \
  DEFINE_SERIALIZATION_SEPARATOR_FOR_TYPE(T, Krys::HTML::SerializationSeparator<T::Wrapped>)

/// @brief Defines tuple-like conformance for a wrapper type.
#define DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(T) DEFINE_TUPLE_LIKE_CONFORMANCE(T, 1)

/// @brief Defines variant-like conformance for a type.
#define DEFINE_VARIANT_LIKE_CONFORMANCE(T)                                                                   \
  template <>                                                                                                \
  constexpr bool ::Krys::HTML::TreatAsVariantLike<T> = true;

/// @brief Defines range-like conformance for a type.
#define DEFINE_RANGE_LIKE_CONFORMANCE(T)                                                                     \
  template <>                                                                                                \
  constexpr bool ::Krys::HTML::TreatAsRangeLike<T> = true;

/// @brief Defines range-like conformance and that the type should be serialized as space separated.
#define DEFINE_SPACE_SEPARATED_RANGE_LIKE_CONFORMANCE(T)                                                     \
  DEFINE_RANGE_LIKE_CONFORMANCE(T)                                                                           \
  DEFINE_SERIALIZATION_SEPARATOR_FOR_TYPE(T, Krys::HTML::SerializationSeparatorType::Space)

/// @brief Defines a range-like conformance and that the type should be serialized as comma separated.
#define DEFINE_COMMA_SEPARATED_RANGE_LIKE_CONFORMANCE(T)                                                     \
  DEFINE_RANGE_LIKE_CONFORMANCE(T)                                                                           \
  DEFINE_SERIALIZATION_SEPARATOR_FOR_TYPE(T, Krys::HTML::SerializationSeparatorType::Comma)

/// @brief Defines a range-like conformance and that the type should be serialized as slash separated.
#define DEFINE_SLASH_SEPARATED_RANGE_LIKE_CONFORMANCE(T)                                                     \
  DEFINE_RANGE_LIKE_CONFORMANCE(T)                                                                           \
  DEFINE_SERIALIZATION_SEPARATOR_FOR_TYPE(T, Krys::HTML::SerializationSeparatorType::Slash)

/// @brief Defines an empty-like conformance for a type.
#define DEFINE_EMPTY_LIKE_CONFORMANCE(T)                                                                     \
  template <>                                                                                                \
  constexpr bool TreatAsEmptyLike<T> = true;

  template <typename T>
  constexpr bool TreatAsOptionalLike<Maybe<T>> = true;

  template <typename... Ts>
  constexpr bool TreatAsTupleLike<std::tuple<Ts...>> = true;

  template <typename... Ts>
  constexpr bool TreatAsVariantLike<Variant<Ts...>> = true;

#pragma endregion

#pragma region Identifiers

  /// @brief Used to represent an arbitrary custom identifier.
  struct CustomIdentifier
  {
    CSSOMStringAtom value;

    bool operator==(const CustomIdentifier &) const = default;
    bool operator==(const CSSOMStringAtom &other) const
    {
      return value == other;
    }
  };

  /// @brief Used to represent an arbitrary property identifier.
  struct PropertyIdentifier
  {
    CSSPropertyId value;

    bool operator==(const PropertyIdentifier &) const = default;
  };

#pragma endregion

#pragma region FunctionNotation

  template <CSSValueId C, typename T>
  struct FunctionNotation
  {
    constexpr static auto name = C;

    T parameters;

    // Forward * and -> to the parameters for convenience.
    KRYS_NODISCARD const T &operator*() const noexcept
    {
      return parameters;
    }

    KRYS_NODISCARD T &operator*() noexcept
    {
      return parameters;
    }

    KRYS_NODISCARD const T *operator->() const noexcept
    {
      return &parameters;
    }

    KRYS_NODISCARD T *operator->() noexcept
    {
      return &parameters;
    }

    KRYS_NODISCARD operator const T &() const noexcept
    {
      return parameters;
    }

    KRYS_NODISCARD operator T &() noexcept
    {
      return parameters;
    }

    bool operator==(const FunctionNotation<C, T> &) const noexcept = default;
  };

  // Deduction guide for getter/setters that return values and take r-value references.
  template <typename Keyword, typename T>
  FunctionNotation(Keyword, T) -> FunctionNotation<Keyword::value, T>;

  template <CSSValueId C, typename T>
  KRYS_NODISCARD bool operator==(const UniquePtr<FunctionNotation<C, T>> &a,
                                 const UniquePtr<FunctionNotation<C, T>> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <size_t, CSSValueId C, typename T>
  KRYS_NODISCARD const auto &get(const FunctionNotation<C, T> &function) noexcept
  {
    return function.parameters;
  }

  template <CSSValueId C, typename T>
  constexpr bool TreatAsTupleLike<FunctionNotation<C, T>> = true;

#pragma endregion

#pragma region SpaceSeparatedArray

  /// @brief Wraps a fixed size list of elements of a single type, semantically marking them as serializing as
  /// "space separated".
  template <typename T, size_t N>
  struct SpaceSeparatedArray
  {
    using Array = std::array<T, N>;
    using value_type = T;

    std::array<T, N> value;

    template <typename... Ts>
    requires(sizeof...(Ts) == N && Krys::AllTrue<ConvertibleTo<Ts, T>...>)
    constexpr SpaceSeparatedArray(Ts... values) noexcept : value(std::forward<Ts>(values)...)
    {
    }

    constexpr SpaceSeparatedArray(std::array<T, N> &&array) noexcept : value(Krys::Move(array))
    {
    }

    constexpr bool operator==(const SpaceSeparatedArray<T, N> &) const noexcept = default;
  };

  template <typename T, typename... Ts>
  requires(Krys::AllTrue<ConvertibleTo<Ts, T>...>)
  SpaceSeparatedArray(T, Ts...) -> SpaceSeparatedArray<T, 1 + sizeof...(Ts)>;

  template <size_t I, typename T, size_t N>
  KRYS_NODISCARD decltype(auto) get(const SpaceSeparatedArray<T, N> &array) noexcept
  {
    return std::get<I>(array.value);
  }

  template <typename T, size_t N>
  constexpr bool TreatAsTupleLike<SpaceSeparatedArray<T, N>> = true;

  template <typename T, size_t N>
  constexpr bool SerializationSeparator<SpaceSeparatedArray<T, N>> = SerializationSeparatorType::Space;

  /// @brief Convenience for representing a two element array.
  template <typename T>
  using SpaceSeparatedPair = SpaceSeparatedArray<T, 2uz>;

#pragma endregion

#pragma region MinimallySerializingSpaceSeparatedPair

  /// @brief Wraps a pair of elements of a single type, semantically marking them as serializing as "space
  /// separated" and "minimally serializing".
  template <typename T>
  struct MinimallySerializingSpaceSeparatedPair
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> value;

    constexpr MinimallySerializingSpaceSeparatedPair(T p1, T p2) noexcept
        : value {Krys::Move(p1), Krys::Move(p2)}
    {
    }

    constexpr MinimallySerializingSpaceSeparatedPair(SpaceSeparatedPair<T> &&array) noexcept
        : value {Krys::Move(array)}
    {
    }

    KRYS_NODISCARD constexpr bool
      operator==(const MinimallySerializingSpaceSeparatedPair<T> &) const noexcept = default;

    KRYS_NODISCARD constexpr const T &first() const noexcept
    {
      return get<0>(value);
    }

    KRYS_NODISCARD constexpr const T &second() const noexcept
    {
      return get<1>(value);
    }
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const MinimallySerializingSpaceSeparatedPair<T> &size) noexcept
  {
    return get<I>(size.value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<MinimallySerializingSpaceSeparatedPair<T>> = true;

  template <typename T>
  constexpr auto SerializationSeparator<MinimallySerializingSpaceSeparatedPair<T>> =
    SerializationSeparatorType::Space;

  template <typename T>
  constexpr auto SerializationCoalescing<MinimallySerializingSpaceSeparatedPair<T>> =
    SerializationCoalescingType::Minimal;

#pragma endregion

#pragma region CommaSeparatedArray

  /// @brief Wraps a fixed size list of elements of a single type, semantically marking them as serializing as
  /// "comma separated".
  template <typename T, size_t N>
  struct CommaSeparatedArray
  {
    using Array = std::array<T, N>;
    using value_type = T;

    std::array<T, N> value;

    template <typename... Ts>
    requires(sizeof...(Ts) == N && Krys::AllTrue<ConvertibleTo<Ts, T>...>)
    constexpr CommaSeparatedArray(Ts... values) noexcept : value(std::forward<Ts>(values)...)
    {
    }

    constexpr CommaSeparatedArray(std::array<T, N> &&array) noexcept : value(Krys::Move(array))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const CommaSeparatedArray<T, N> &) const noexcept = default;
  };

  template <typename T, typename... Ts>
  requires(Krys::AllTrue<ConvertibleTo<Ts, T>...>)
  CommaSeparatedArray(T, Ts...) -> CommaSeparatedArray<T, 1 + sizeof...(Ts)>;

  template <size_t I, typename T, size_t N>
  KRYS_NODISCARD decltype(auto) get(const CommaSeparatedArray<T, N> &array) noexcept
  {
    return std::get<I>(array.value);
  }

  template <typename T, size_t N>
  constexpr auto TreatAsTupleLike<CommaSeparatedArray<T, N>> = true;

  template <typename T, size_t N>
  constexpr auto SerializationSeparator<CommaSeparatedArray<T, N>> = SerializationSeparatorType::Comma;

  // Convenience for representing a two element array.
  template <typename T>
  using CommaSeparatedPair = CommaSeparatedArray<T, 2>;

#pragma endregion

#pragma region SpaceSeparatedTuple

  /// @brief Wraps a variadic list of types, semantically marking them as serializing as "space separated".
  template <typename... Ts>
  struct SpaceSeparatedTuple
  {
    using Tuple = std::tuple<Ts...>;

    std::tuple<Ts...> value;

    constexpr SpaceSeparatedTuple(Ts &&...values) noexcept
        : value(std::make_tuple(std::forward<Ts>(values)...))
    {
    }

    constexpr SpaceSeparatedTuple(const Ts &...values) noexcept : value(std::make_tuple(values...))
    {
    }

    constexpr SpaceSeparatedTuple(std::tuple<Ts...> &&tuple) noexcept : value(Krys::Move(tuple))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const SpaceSeparatedTuple<Ts...> &) const = default;
  };

  template <size_t I, typename... Ts>
  KRYS_NODISCARD decltype(auto) get(const SpaceSeparatedTuple<Ts...> &tuple) noexcept
  {
    return std::get<I>(tuple.value);
  }

  template <typename... Ts>
  constexpr bool TreatAsTupleLike<SpaceSeparatedTuple<Ts...>> = true;

  template <typename... Ts>
  constexpr auto SerializationSeparator<SpaceSeparatedTuple<Ts...>> = SerializationSeparatorType::Space;

#pragma endregion

#pragma region CommaSeparatedTuple

  /// @brief Wraps a variadic list of types, semantically marking them as serializing as "comma separated".
  template <typename... Ts>
  struct CommaSeparatedTuple
  {
    using Tuple = std::tuple<Ts...>;

    std::tuple<Ts...> value;

    constexpr CommaSeparatedTuple(Ts &&...values) noexcept
        : value(std::make_tuple(std::forward<Ts>(values)...))
    {
    }

    constexpr CommaSeparatedTuple(const Ts &...values) noexcept : value(std::make_tuple(values...))
    {
    }

    constexpr CommaSeparatedTuple(std::tuple<Ts...> &&tuple) noexcept : value(Krys::Move(tuple))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const CommaSeparatedTuple<Ts...> &) const noexcept = default;
  };

  template <size_t I, typename... Ts>
  KRYS_NODISCARD decltype(auto) get(const CommaSeparatedTuple<Ts...> &tuple) noexcept
  {
    return std::get<I>(tuple.value);
  }

  template <typename... Ts>
  constexpr bool TreatAsTupleLike<CommaSeparatedTuple<Ts...>> = true;

  template <typename... Ts>
  constexpr auto SerializationSeparator<CommaSeparatedTuple<Ts...>> = SerializationSeparatorType::Comma;

#pragma endregion

#pragma region SpaceSeparatedPoint

  /// @brief Wraps a pair of elements of a single type representing a point, semantically marking them as
  /// serializing as "space separated".
  template <typename T>
  struct SpaceSeparatedPoint
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> value;

    constexpr SpaceSeparatedPoint(T p1, T p2) noexcept : value(Krys::Move(p1), Krys::Move(p2))
    {
    }

    constexpr SpaceSeparatedPoint(SpaceSeparatedPair<T> &&array) noexcept : value(Krys::Move(array))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const SpaceSeparatedPoint<T> &) const noexcept = default;

    KRYS_NODISCARD const T &x() const noexcept
    {
      return get<0>(value);
    }

    KRYS_NODISCARD const T &y() const noexcept
    {
      return get<1>(value);
    }
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const SpaceSeparatedPoint<T> &point) noexcept
  {
    return get<I>(point.value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<SpaceSeparatedPoint<T>> = true;

  template <typename T>
  constexpr auto SerializationSeparator<SpaceSeparatedPoint<T>> = SerializationSeparatorType::Space;

#pragma endregion

#pragma region SpaceSeparatedSize

  /// @brief Wraps a pair of elements of a single type representing a size, semantically marking them as
  /// serializing as "space separated".
  template <typename T>
  struct SpaceSeparatedSize
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> value;

    constexpr SpaceSeparatedSize(T p1, T p2) noexcept : value(Krys::Move(p1), Krys::Move(p2))
    {
    }

    constexpr SpaceSeparatedSize(SpaceSeparatedPair<T> &&array) noexcept : value(Krys::Move(array))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const SpaceSeparatedSize<T> &) const noexcept = default;

    KRYS_NODISCARD const T &width() const noexcept
    {
      return get<0>(value);
    }

    KRYS_NODISCARD const T &height() const noexcept
    {
      return get<1>(value);
    }
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const SpaceSeparatedSize<T> &size) noexcept
  {
    return get<I>(size.value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<SpaceSeparatedSize<T>> = true;

  template <typename T>
  constexpr auto SerializationSeparator<SpaceSeparatedSize<T>> = SerializationSeparatorType::Space;

#pragma endregion

#pragma region MinimallySerializingSpaceSeparatedPoint

  /// @brief Wraps a pair of elements of a single type representing a point, semantically marking them as
  /// serializing as "space separated" and "minimally serializing".
  template <typename T>
  struct MinimallySerializingSpaceSeparatedPoint
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> value;

    template <typename U>
    constexpr MinimallySerializingSpaceSeparatedPoint(U p1) noexcept : value(p1, p1)
    {
    }

    template <typename U>
    constexpr MinimallySerializingSpaceSeparatedPoint(U p1, U p2) noexcept
        : value(Krys::Move(p1), Krys::Move(p2))
    {
    }

    constexpr MinimallySerializingSpaceSeparatedPoint(SpaceSeparatedPair<T> &&array) noexcept
        : value {Krys::Move(array)}
    {
    }

    KRYS_NODISCARD constexpr bool
      operator==(const MinimallySerializingSpaceSeparatedPoint<T> &) const noexcept = default;

    KRYS_NODISCARD const T &x() const noexcept
    {
      return get<0>(value);
    }

    KRYS_NODISCARD const T &y() const noexcept
    {
      return get<1>(value);
    }
  };

  template <size_t I, typename T>
  decltype(auto) get(const MinimallySerializingSpaceSeparatedPoint<T> &point) noexcept
  {
    return get<I>(point.value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<MinimallySerializingSpaceSeparatedPoint<T>> = true;

  template <typename T>
  constexpr auto SerializationSeparator<MinimallySerializingSpaceSeparatedPoint<T>> =
    SerializationSeparatorType::Space;

  template <typename T>
  constexpr auto SerializationCoalescing<MinimallySerializingSpaceSeparatedPoint<T>> =
    SerializationCoalescingType::Minimal;

#pragma endregion

#pragma region MinimallySerializingSpaceSeparatedSize

  // Wraps a pair of elements of a single type representing a size, semantically marking them as serializing
  // as "space separated" and "minimally serializing".
  template <typename T>
  struct MinimallySerializingSpaceSeparatedSize
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> value;

    template <typename U>
    constexpr MinimallySerializingSpaceSeparatedSize(U p1) noexcept : value(p1, p1)
    {
    }

    template <typename U>
    constexpr MinimallySerializingSpaceSeparatedSize(U p1, U p2) noexcept
        : value(Krys::Move(p1), Krys::Move(p2))
    {
    }

    constexpr MinimallySerializingSpaceSeparatedSize(SpaceSeparatedPair<T> &&array) noexcept
        : value(Krys::Move(array))
    {
    }

    KRYS_NODISCARD constexpr bool
      operator==(const MinimallySerializingSpaceSeparatedSize<T> &) const noexcept = default;

    KRYS_NODISCARD constexpr const T &width() const noexcept
    {
      return get<0>(value);
    }

    KRYS_NODISCARD constexpr const T &height() const noexcept
    {
      return get<1>(value);
    }
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const MinimallySerializingSpaceSeparatedSize<T> &size) noexcept
  {
    return get<I>(size.value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<MinimallySerializingSpaceSeparatedSize<T>> = true;

  template <typename T>
  constexpr auto SerializationSeparator<MinimallySerializingSpaceSeparatedSize<T>> =
    SerializationSeparatorType::Space;

  template <typename T>
  constexpr auto SerializationCoalescing<MinimallySerializingSpaceSeparatedSize<T>> =
    SerializationCoalescingType::Minimal;

#pragma endregion
}