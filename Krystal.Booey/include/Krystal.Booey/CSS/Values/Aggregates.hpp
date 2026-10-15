#pragma once

#include "Krystal.Booey/Core/RectCorners.hpp"
#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefCountedFixedVector.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Utils/Apply.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <concepts>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <tuple>
#include <type_traits>

namespace krys::boo::css
{
#pragma region SerialisationSeparatorType

  enum class SerialisationSeparatorType : uint8
  {
    None,
    Space,
    Comma,
    Slash
  };

  /// @brief Types that specialize TreatAsTupleLike or TreatAsRangeLike can specialize this to indicate how to
  /// serialize the gaps between elements.
  template <typename>
  constexpr SerialisationSeparatorType SerialisationSeparator = SerialisationSeparatorType::None;

#define DEFINE_SERIALISATION_SEPARATOR_FOR_TYPE(T, separator)                                                \
  namespace krys::boo::css                                                                                   \
  {                                                                                                          \
    template <>                                                                                              \
    constexpr SerialisationSeparatorType SerialisationSeparator<T> = separator;                              \
  }

#pragma endregion

#pragma region SerialisationCoalescingType

  enum class SerialisationCoalescingType : uint8
  {
    None,
    Minimal
  };

  /// @brief Types that specialize TreatAsTupleLike and have size 2 or 4 can specialize this to indicate how
  /// to serialize identical elements.
  template <typename>
  constexpr SerialisationCoalescingType SerialisationCoalescing = SerialisationCoalescingType::None;

#define DEFINE_SERIALISATION_COALESCING_FOR_TYPE(T, coalescing)                                              \
  namespace krys::boo::css                                                                                   \
  {                                                                                                          \
    template <>                                                                                              \
    constexpr SerialisationCoalescingType SerialisationCoalescing<T> = coalescing;                           \
  }

#pragma endregion

#pragma region SerialisationSeparatorStringForType

  template <SerialisationSeparatorType>
  constexpr CSSOMStringView SerialisationSeparatorStringForType = u8"";

  template <>
  constexpr CSSOMStringView SerialisationSeparatorStringForType<SerialisationSeparatorType::Space> = u8" ";

  template <>
  constexpr CSSOMStringView SerialisationSeparatorStringForType<SerialisationSeparatorType::Comma> = u8", ";

  template <>
  constexpr CSSOMStringView SerialisationSeparatorStringForType<SerialisationSeparatorType::Slash> = u8" / ";

  template <typename T>
  constexpr CSSOMStringView SerialisationSeparatorString =
    SerialisationSeparatorStringForType<SerialisationSeparator<T>>;

#pragma endregion

#pragma region Helper Macros

  /// @brief Defines a simple `get()` implementation for a single value `name`.
#define DEFINE_TYPE_WRAPPER_GET(T, name)                                                                     \
  template <size_t>                                                                                          \
  KRYS_NODISCARD const auto &get(const T &wrapper) noexcept                                                  \
  {                                                                                                          \
    return wrapper.name;                                                                                     \
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
    wrapped Value;                                                                                           \
                                                                                                             \
    template <typename... Args>                                                                              \
    wrapper(Args &&...args) noexcept                                                                         \
    requires(requires {                                                                                      \
      { wrapped(args...) };                                                                                  \
    })                                                                                                       \
        : Value(std::forward<Args>(args)...)                                                                 \
    {                                                                                                        \
    }                                                                                                        \
                                                                                                             \
    const Wrapped &operator*() const noexcept                                                                \
    {                                                                                                        \
      return Value;                                                                                          \
    }                                                                                                        \
                                                                                                             \
    Wrapped &operator*() noexcept                                                                            \
    {                                                                                                        \
      return Value;                                                                                          \
    }                                                                                                        \
                                                                                                             \
    const Wrapped *operator->() const noexcept                                                               \
    {                                                                                                        \
      return &Value;                                                                                         \
    }                                                                                                        \
                                                                                                             \
    Wrapped *operator->() noexcept                                                                           \
    {                                                                                                        \
      return &Value;                                                                                         \
    }                                                                                                        \
                                                                                                             \
    template <size_t>                                                                                        \
    friend KRYS_NODISCARD const auto &get(const wrapper &self) noexcept                                      \
    {                                                                                                        \
      return self.Value;                                                                                     \
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
  constexpr bool ::krys::boo::css::TreatAsTupleLike<T> = true;

/// @brief Defines tuple-like conformance and that the type should be serialized as space separated.
#define DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                  \
  DEFINE_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                                        \
  DEFINE_SERIALISATION_SEPARATOR_FOR_TYPE(T, krys::boo::css::SerialisationSeparatorType::Space)

/// @brief Defines tuple-like conformance and that the type should be serialized as coalescing and space
/// separated.
#define DEFINE_COALESCING_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                       \
  static_assert(numberOfArguments == 2 || numberOfArguments == 4);                                           \
  DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                        \
  DEFINE_SERIALISATION_COALESCING_FOR_TYPE(T, krys::boo::css::SerialisationCoalescingType::Minimal)

/// @brief Defines tuple-like conformance and that the type should be serialized as comma separated.
#define DEFINE_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                  \
  DEFINE_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                                        \
  DEFINE_SERIALISATION_SEPARATOR_FOR_TYPE(T, krys::boo::css::SerialisationSeparatorType::Comma)

/// @brief Defines tuple-like conformance and that the type should be serialized as coalescing and comma
/// separated.
#define DEFINE_COALESCING_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                       \
  static_assert(numberOfArguments == 2 || numberOfArguments == 4);                                           \
  DEFINE_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                        \
  DEFINE_SERIALISATION_COALESCING_FOR_TYPE(T, krys::boo::css::SerialisationCoalescingType::Minimal)

/// @brief Defines tuple-like conformance and that the type should be serialized as slash separated.
#define DEFINE_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                  \
  DEFINE_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                                        \
  DEFINE_SERIALISATION_SEPARATOR_FOR_TYPE(T, krys::boo::css::SerialisationSeparatorType::Slash)

/// @brief Defines tuple-like conformance and that the type should be serialized as coalescing and slash
/// separated.
#define DEFINE_COALESCING_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                       \
  static_assert(numberOfArguments == 2 || numberOfArguments == 4);                                           \
  DEFINE_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(T, numberOfArguments)                                        \
  DEFINE_SERIALISATION_COALESCING_FOR_TYPE(T, krys::boo::css::SerialisationCoalescingType::Minimal)

/// @brief Defines tuple-like conformance based on the type being extended.
#define DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_EXTENDER(T)                                                   \
  DEFINE_TUPLE_LIKE_CONFORMANCE(T, std::tuple_size_v<T::Wrapped>)                                            \
  DEFINE_SERIALISATION_SEPARATOR_FOR_TYPE(T, krys::boo::css::SerialisationSeparator<T::Wrapped>)

/// @brief Defines tuple-like conformance for a wrapper type.
#define DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(T) DEFINE_TUPLE_LIKE_CONFORMANCE(T, 1)

/// @brief Defines variant-like conformance for a type.
#define DEFINE_VARIANT_LIKE_CONFORMANCE(T)                                                                   \
  template <>                                                                                                \
  constexpr bool ::krys::boo::css::TreatAsVariantLike<T> = true;

/// @brief Defines tokens-like conformance for a type.
#define DEFINE_RANGE_LIKE_CONFORMANCE(T)                                                                     \
  template <>                                                                                                \
  constexpr bool ::krys::boo::css::TreatAsRangeLike<T> = true;

/// @brief Defines tokens-like conformance and that the type should be serialized as space separated.
#define DEFINE_SPACE_SEPARATED_RANGE_LIKE_CONFORMANCE(T)                                                     \
  DEFINE_RANGE_LIKE_CONFORMANCE(T)                                                                           \
  DEFINE_SERIALISATION_SEPARATOR_FOR_TYPE(T, krys::boo::css::SerialisationSeparatorType::Space)

/// @brief Defines a tokens-like conformance and that the type should be serialized as comma separated.
#define DEFINE_COMMA_SEPARATED_RANGE_LIKE_CONFORMANCE(T)                                                     \
  DEFINE_RANGE_LIKE_CONFORMANCE(T)                                                                           \
  DEFINE_SERIALISATION_SEPARATOR_FOR_TYPE(T, krys::boo::css::SerialisationSeparatorType::Comma)

/// @brief Defines a tokens-like conformance and that the type should be serialized as slash separated.
#define DEFINE_SLASH_SEPARATED_RANGE_LIKE_CONFORMANCE(T)                                                     \
  DEFINE_RANGE_LIKE_CONFORMANCE(T)                                                                           \
  DEFINE_SERIALISATION_SEPARATOR_FOR_TYPE(T, krys::boo::css::SerialisationSeparatorType::Slash)

/// @brief Defines an empty-like conformance for a type.
#define DEFINE_EMPTY_LIKE_CONFORMANCE(T)                                                                     \
  template <>                                                                                                \
  constexpr bool ::krys::boo::css::TreatAsEmptyLike<T> = true;

  template <typename T>
  constexpr bool TreatAsOptionalLike<Maybe<T>> = true;

  template <typename T, typename TTraits>
  constexpr bool TreatAsOptionalLike<Markable<T, TTraits>> = true;

  template <typename... Ts>
  constexpr bool TreatAsTupleLike<std::tuple<Ts...>> = true;

  template <typename... Ts>
  constexpr bool TreatAsVariantLike<Variant<Ts...>> = true;

#pragma endregion

#pragma region Identifiers

  /// @brief Used to represent an arbitrary custom identifier.
  struct CustomIdentifier
  {
    CSSOMStringAtom Value;

    bool operator==(const CustomIdentifier &) const noexcept = default;
    bool operator==(const CSSOMStringAtom &other) const noexcept
    {
      return Value == other;
    }
  };

  /// @brief Used to represent an arbitrary property identifier.
  struct PropertyIdentifier
  {
    PropertyId Value;

    constexpr bool operator==(const PropertyIdentifier &) const = default;
  };

#pragma endregion

#pragma region FunctionNotation

  template <ValueId C, typename T>
  struct FunctionNotation
  {
    constexpr static auto Name = C;

    T Parameters;

    // Forward * and -> to the parameters for convenience.
    KRYS_NODISCARD constexpr const T &operator*() const noexcept
    {
      return Parameters;
    }

    KRYS_NODISCARD constexpr T &operator*() noexcept
    {
      return Parameters;
    }

    KRYS_NODISCARD constexpr const T *operator->() const noexcept
    {
      return &Parameters;
    }

    KRYS_NODISCARD constexpr T *operator->() noexcept
    {
      return &Parameters;
    }

    KRYS_NODISCARD constexpr operator const T &() const noexcept
    {
      return Parameters;
    }

    KRYS_NODISCARD constexpr operator T &() noexcept
    {
      return Parameters;
    }

    constexpr bool operator==(const FunctionNotation<C, T> &) const noexcept = default;
  };

  // Deduction guide for getter/setters that return values and take r-value references.
  template <typename Keyword, typename T>
  FunctionNotation(Keyword, T) -> FunctionNotation<Keyword::Value, T>;

  template <ValueId C, typename T>
  KRYS_NODISCARD constexpr bool operator==(const UniquePtr<FunctionNotation<C, T>> &a,
                                           const UniquePtr<FunctionNotation<C, T>> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <size_t, ValueId C, typename T>
  KRYS_NODISCARD constexpr const auto &get(const FunctionNotation<C, T> &function) noexcept
  {
    return function.Parameters;
  }

  template <ValueId C, typename T>
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

    std::array<T, N> Value;

    template <typename... Ts>
    requires(sizeof...(Ts) == N && krys::AllTrue<ConvertibleTo<Ts, T>...>)
    constexpr SpaceSeparatedArray(Ts... values) noexcept : Value(std::forward<Ts>(values)...)
    {
    }

    constexpr SpaceSeparatedArray(std::array<T, N> &&array) noexcept : Value(krys::move(array))
    {
    }

    constexpr bool operator==(const SpaceSeparatedArray<T, N> &) const noexcept = default;
  };

  template <typename T, typename... Ts>
  requires(krys::AllTrue<ConvertibleTo<Ts, T>...>)
  SpaceSeparatedArray(T, Ts...) -> SpaceSeparatedArray<T, 1 + sizeof...(Ts)>;

  template <size_t I, typename T, size_t N>
  KRYS_NODISCARD decltype(auto) get(const SpaceSeparatedArray<T, N> &array) noexcept
  {
    return std::get<I>(array.Value);
  }

  template <typename T, size_t N>
  constexpr bool TreatAsTupleLike<SpaceSeparatedArray<T, N>> = true;

  template <typename T, size_t N>
  constexpr auto SerialisationSeparator<SpaceSeparatedArray<T, N>> = SerialisationSeparatorType::Space;

  /// @brief Convenience for representing a two element array.
  template <typename T>
  using SpaceSeparatedPair = SpaceSeparatedArray<T, 2uz>;

#pragma endregion

#pragma region MinimallySerialisingSpaceSeparatedPair

  /// @brief Wraps a pair of elements of a single type, semantically marking them as serializing as "space
  /// separated" and "minimally serializing".
  template <typename T>
  struct MinimallySerialisingSpaceSeparatedPair
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> Value;

    constexpr MinimallySerialisingSpaceSeparatedPair(T p1, T p2) noexcept
        : Value {krys::move(p1), krys::move(p2)}
    {
    }

    constexpr MinimallySerialisingSpaceSeparatedPair(SpaceSeparatedPair<T> &&array) noexcept
        : Value {krys::move(array)}
    {
    }

    KRYS_NODISCARD constexpr bool
      operator==(const MinimallySerialisingSpaceSeparatedPair<T> &) const noexcept = default;

    KRYS_NODISCARD constexpr const T &first() const noexcept
    {
      return get<0>(Value);
    }

    KRYS_NODISCARD constexpr const T &second() const noexcept
    {
      return get<1>(Value);
    }
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const MinimallySerialisingSpaceSeparatedPair<T> &size) noexcept
  {
    return get<I>(size.Value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<MinimallySerialisingSpaceSeparatedPair<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<MinimallySerialisingSpaceSeparatedPair<T>> =
    SerialisationSeparatorType::Space;

  template <typename T>
  constexpr auto SerialisationCoalescing<MinimallySerialisingSpaceSeparatedPair<T>> =
    SerialisationCoalescingType::Minimal;

#pragma endregion

#pragma region CommaSeparatedArray

  /// @brief Wraps a fixed size list of elements of a single type, semantically marking them as serializing as
  /// "comma separated".
  template <typename T, size_t N>
  struct CommaSeparatedArray
  {
    using Array = std::array<T, N>;
    using value_type = T;

    std::array<T, N> Value;

    template <typename... Ts>
    requires(sizeof...(Ts) == N && krys::AllTrue<ConvertibleTo<Ts, T>...>)
    constexpr CommaSeparatedArray(Ts... values) noexcept : Value(std::forward<Ts>(values)...)
    {
    }

    constexpr CommaSeparatedArray(std::array<T, N> &&array) noexcept : Value(krys::move(array))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const CommaSeparatedArray<T, N> &) const noexcept = default;
  };

  template <typename T, typename... Ts>
  requires(krys::AllTrue<ConvertibleTo<Ts, T>...>)
  CommaSeparatedArray(T, Ts...) -> CommaSeparatedArray<T, 1 + sizeof...(Ts)>;

  template <size_t I, typename T, size_t N>
  KRYS_NODISCARD constexpr decltype(auto) get(const CommaSeparatedArray<T, N> &array) noexcept
  {
    return std::get<I>(array.Value);
  }

  template <typename T, size_t N>
  constexpr auto TreatAsTupleLike<CommaSeparatedArray<T, N>> = true;

  template <typename T, size_t N>
  constexpr auto SerialisationSeparator<CommaSeparatedArray<T, N>> = SerialisationSeparatorType::Comma;

  // Convenience for representing a two element array.
  template <typename T>
  using CommaSeparatedPair = CommaSeparatedArray<T, 2>;

#pragma endregion

#pragma region SpaceSeparatedList

  /// @brief Wraps a variable number of elements of a single type, semantically marking them as serializing as
  /// "space separated".
  template <typename T, size_t inlineCapacity = 0>
  struct SpaceSeparatedList
  {
    using Container = krys::SmallList<T, inlineCapacity>;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;
    using value_type = typename Container::value_type;

    Container Value;

    SpaceSeparatedList() noexcept = default;

    SpaceSeparatedList(std::initializer_list<T> initializerList) noexcept : Value {initializerList}
    {
    }

    SpaceSeparatedList(Container &&container) noexcept : Value {krys::move(container)}
    {
    }

    const_iterator begin() const noexcept
    {
      return Value.begin();
    }

    const_iterator end() const noexcept
    {
      return Value.end();
    }

    const_reverse_iterator rbegin() const noexcept
    {
      return Value.rbegin();
    }

    const_reverse_iterator rend() const noexcept
    {
      return Value.rend();
    }

    bool empty() const noexcept
    {
      return Value.empty();
    }

    size_t size() const noexcept
    {
      return Value.size();
    }

    const T &operator[](size_t i) const noexcept
    {
      return Value[i];
    }

    template <typename SizedRange, typename Mapper>
    static SpaceSeparatedList map(SizedRange &&range, Mapper &&mapper)
    {
      Container result;
      if constexpr (std::ranges::sized_range<SizedRange>)
      {
        result.reserve(std::ranges::size(range));
      }
      for (auto &&item : range)
      {
        result.emplace_back(std::invoke(mapper, item));
      }
      return SpaceSeparatedList {krys::move(result)};
    }

    bool contains(const auto &item) const
    {
      return std::ranges::find(Value, item) != Value.end();
    }

    bool containsIf(const auto &predicate) const
    {
      return std::ranges::find_if(Value, predicate) != Value.end();
    }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
      using Result = std::remove_cvref_t<std::invoke_result_t<Mapper, const T &>>;
      SmallList<Result, inlineCapacity> result;
      result.reserve(Value.size());
      for (const auto &item : Value)
      {
        result.emplace_back(std::invoke(mapper, item));
      }
      return result;
    }

    bool isEmpty() const noexcept
    {
      return empty();
    }

    bool operator==(const SpaceSeparatedList &) const noexcept = default;
  };

  template <typename T, size_t N>
  constexpr auto TreatAsRangeLike<SpaceSeparatedList<T, N>> = true;

  template <typename T, size_t N>
  constexpr auto SerialisationSeparator<SpaceSeparatedList<T, N>> = SerialisationSeparatorType::Space;

#pragma endregion

#pragma region CommaSeparatedList

  /// @brief Wraps a variable number of elements of a single type, semantically marking them as serializing as
  /// "comma separated".
  template <typename T, size_t inlineCapacity = 0>
  struct CommaSeparatedList
  {
    using Container = krys::SmallList<T, inlineCapacity>;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;
    using value_type = typename Container::value_type;

    Container Value;

    CommaSeparatedList() noexcept = default;

    CommaSeparatedList(std::initializer_list<T> initializerList) noexcept : Value {initializerList}
    {
    }

    CommaSeparatedList(Container &&container) noexcept : Value {krys::move(container)}
    {
    }

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return Value.begin();
    }

    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return Value.end();
    }

    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept
    {
      return Value.rbegin();
    }

    KRYS_NODISCARD const_reverse_iterator rend() const noexcept
    {
      return Value.rend();
    }

    KRYS_NODISCARD bool empty() const noexcept
    {
      return Value.empty();
    }

    KRYS_NODISCARD size_t size() const noexcept
    {
      return Value.size();
    }

    KRYS_NODISCARD const T &operator[](size_t i) const noexcept
    {
      return Value[i];
    }

    template <typename SizedRange, typename Mapper>
    static CommaSeparatedList map(SizedRange &&range, Mapper &&mapper)
    {
      Container result;
      if constexpr (std::ranges::sized_range<SizedRange>)
      {
        result.reserve(std::ranges::size(range));
      }
      for (auto &&item : range)
      {
        result.emplace_back(std::invoke(mapper, item));
      }
      return CommaSeparatedList {krys::move(result)};
    }

    bool contains(const auto &item) const
    {
      return std::ranges::find(Value, item) != Value.end();
    }

    bool containsIf(const auto &predicate) const
    {
      return std::ranges::find_if(Value, predicate) != Value.end();
    }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
      using Result = std::remove_cvref_t<std::invoke_result_t<Mapper, const T &>>;
      SmallList<Result, inlineCapacity> result;
      result.reserve(Value.size());
      for (const auto &item : Value)
      {
        result.emplace_back(std::invoke(mapper, item));
      }
      return result;
    }

    bool isEmpty() const noexcept
    {
      return empty();
    }

    KRYS_NODISCARD bool operator==(const CommaSeparatedList &) const noexcept = default;
  };

  template <typename T, size_t N>
  constexpr auto TreatAsRangeLike<CommaSeparatedList<T, N>> = true;

  template <typename T, size_t N>
  constexpr auto SerialisationSeparator<CommaSeparatedList<T, N>> = SerialisationSeparatorType::Comma;

#pragma endregion

#pragma region SpaceSeparatedListHashSet

  /// @brief Wraps an insertion-ordered set, semantically marking it as serializing as "space separated".
  template <typename T>
  struct SpaceSeparatedListHashSet
  {
    using Container = krys::ListHashSet<T>;
    using const_iterator = typename Container::const_iterator;
    using value_type = typename Container::value_type;

    Container Value;

    SpaceSeparatedListHashSet() = default;

    SpaceSeparatedListHashSet(std::initializer_list<T> initializerList) : Value {initializerList}
    {
    }

    SpaceSeparatedListHashSet(Container &&container) : Value {krys::move(container)}
    {
    }

    template <typename SizedRange, typename Mapper>
    static SpaceSeparatedListHashSet map(SizedRange &&range, Mapper &&mapper)
    {
      Container result;
      for (auto &&item : range)
      {
        result.add(std::invoke(mapper, item));
      }
      return SpaceSeparatedListHashSet {krys::move(result)};
    }

    const_iterator begin() const noexcept
    {
      return Value.begin();
    }

    const_iterator end() const noexcept
    {
      return Value.end();
    }

    bool isEmpty() const noexcept
    {
      return Value.isEmpty();
    }

    size_t size() const noexcept
    {
      return Value.size();
    }

    bool contains(const T &item) const
    {
      return Value.contains(item);
    }

    bool operator==(const SpaceSeparatedListHashSet &) const = default;
  };

  template <typename T>
  constexpr auto TreatAsRangeLike<SpaceSeparatedListHashSet<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<SpaceSeparatedListHashSet<T>> = SerialisationSeparatorType::Space;

#pragma endregion

#pragma region CommaSeparatedListHashSet

  /// @brief Wraps an insertion-ordered set, semantically marking it as serializing as "comma separated".
  template <typename T>
  struct CommaSeparatedListHashSet
  {
    using Container = krys::ListHashSet<T>;
    using const_iterator = typename Container::const_iterator;
    using value_type = typename Container::value_type;

    Container Value;

    CommaSeparatedListHashSet() = default;

    CommaSeparatedListHashSet(std::initializer_list<T> initializerList) : Value {initializerList}
    {
    }

    CommaSeparatedListHashSet(Container &&container) : Value {krys::move(container)}
    {
    }

    template <typename SizedRange, typename Mapper>
    static CommaSeparatedListHashSet map(SizedRange &&range, Mapper &&mapper)
    {
      Container result;
      for (auto &&item : range)
      {
        result.add(std::invoke(mapper, item));
      }
      return CommaSeparatedListHashSet {krys::move(result)};
    }

    const_iterator begin() const noexcept
    {
      return Value.begin();
    }

    const_iterator end() const noexcept
    {
      return Value.end();
    }

    bool isEmpty() const noexcept
    {
      return Value.isEmpty();
    }

    size_t size() const noexcept
    {
      return Value.size();
    }

    bool contains(const T &item) const
    {
      return Value.contains(item);
    }

    bool operator==(const CommaSeparatedListHashSet &) const = default;
  };

  template <typename T>
  constexpr auto TreatAsRangeLike<CommaSeparatedListHashSet<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<CommaSeparatedListHashSet<T>> = SerialisationSeparatorType::Comma;

#pragma endregion

  // Wraps a variable (though known at construction) number of elements of a single type, semantically marking
  // them as serializing as "space separated".
  template <typename T>
  struct SpaceSeparatedFixedList
  {
    using Container = krys::FixedVector<T>;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;
    using value_type = typename Container::value_type;

    SpaceSeparatedFixedList() = default;

    SpaceSeparatedFixedList(std::initializer_list<T> initializerList) : Value {initializerList}
    {
    }

    SpaceSeparatedFixedList(Container &&container) : Value {krys::move(container)}
    {
    }

    SpaceSeparatedFixedList(T &&element) : Value {krys::move(element)}
    {
    }

    template <typename SizedRange, typename Mapper>
    static SpaceSeparatedFixedList map(SizedRange &&range, KRYS_NOESCAPE Mapper &&mapper)
    {
      return Container::map(std::forward<SizedRange>(range), std::forward<Mapper>(mapper));
    }

    template <typename SizedRange, typename Mapper>
    static SpaceSeparatedFixedList Map(SizedRange &&range, Mapper &&mapper)
    {
      return map(std::forward<SizedRange>(range), std::forward<Mapper>(mapper));
    }

    template <std::invocable<size_t> Generator>
    static SpaceSeparatedFixedList createWithSizeFromGenerator(size_t size,
                                                               KRYS_NOESCAPE Generator &&generator)
    {
      return Container::createWithSizeFromGenerator(size, std::forward<Generator>(generator));
    }

    const_iterator begin() const KRYS_LIFETIME_BOUND
    {
      return Value.begin();
    }
    const_iterator end() const KRYS_LIFETIME_BOUND
    {
      return Value.end();
    }
    const_reverse_iterator rbegin() const KRYS_LIFETIME_BOUND
    {
      return Value.rbegin();
    }
    const_reverse_iterator rend() const KRYS_LIFETIME_BOUND
    {
      return Value.rend();
    }

    bool isEmpty() const
    {
      return Value.isEmpty();
    }
    size_t size() const
    {
      return Value.size();
    }
    const T &operator[](size_t i) const KRYS_LIFETIME_BOUND
    {
      return Value[i];
    }

    const T &first() const KRYS_LIFETIME_BOUND
    {
      return Value.first();
    }
    const T &last() const KRYS_LIFETIME_BOUND
    {
      return Value.last();
    }

    bool contains(const auto &x) const
    {
      return Value.contains(x);
    }
    template <typename Predicate>
    requires std::predicate<Predicate, const value_type &>
    bool containsIf(KRYS_NOESCAPE const Predicate &f) const
    {
      return Value.containsIf(f);
    }

    template <typename F>
    decltype(auto) map(F &&functor) const
    {
      return Value.map(std::forward<F>(functor));
    }

    bool operator==(const SpaceSeparatedFixedList &) const = default;

    Container Value;
  };

  template <typename T>
  constexpr auto TreatAsRangeLike<SpaceSeparatedFixedList<T>> = true;
  template <typename T>
  constexpr auto SerialisationSeparator<SpaceSeparatedFixedList<T>> = SerialisationSeparatorType::Space;

  // Wraps a variable (though known at construction) number of elements of a single type, semantically marking
  // them as serializing as "comma separated".
  template <typename T>
  struct CommaSeparatedFixedList
  {
    using Container = krys::FixedVector<T>;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;
    using value_type = typename Container::value_type;

    CommaSeparatedFixedList() = default;

    CommaSeparatedFixedList(std::initializer_list<T> initializerList) : Value {initializerList}
    {
    }

    CommaSeparatedFixedList(Container &&container) : Value {krys::move(container)}
    {
    }

    CommaSeparatedFixedList(T &&element) : Value {krys::move(element)}
    {
    }

    template <typename SizedRange, typename Mapper>
    static CommaSeparatedFixedList map(SizedRange &&range, KRYS_NOESCAPE Mapper &&mapper)
    {
      return Container::map(std::forward<SizedRange>(range), std::forward<Mapper>(mapper));
    }

    template <typename SizedRange, typename Mapper>
    static CommaSeparatedFixedList Map(SizedRange &&range, Mapper &&mapper)
    {
      return map(std::forward<SizedRange>(range), std::forward<Mapper>(mapper));
    }

    template <std::invocable<size_t> Generator>
    static CommaSeparatedFixedList createWithSizeFromGenerator(size_t size,
                                                               KRYS_NOESCAPE Generator &&generator)
    {
      return Container::createWithSizeFromGenerator(size, std::forward<Generator>(generator));
    }

    const_iterator begin() const KRYS_LIFETIME_BOUND
    {
      return Value.begin();
    }
    const_iterator end() const KRYS_LIFETIME_BOUND
    {
      return Value.end();
    }
    const_reverse_iterator rbegin() const KRYS_LIFETIME_BOUND
    {
      return Value.rbegin();
    }
    const_reverse_iterator rend() const KRYS_LIFETIME_BOUND
    {
      return Value.rend();
    }

    bool isEmpty() const
    {
      return Value.isEmpty();
    }
    size_t size() const
    {
      return Value.size();
    }
    const T &operator[](size_t i) const KRYS_LIFETIME_BOUND
    {
      return Value[i];
    }

    const T &first() const KRYS_LIFETIME_BOUND
    {
      return Value.first();
    }
    const T &last() const KRYS_LIFETIME_BOUND
    {
      return Value.last();
    }

    bool contains(const auto &x) const
    {
      return Value.contains(x);
    }
    template <typename Predicate>
    requires std::predicate<Predicate, const value_type &>
    bool containsIf(KRYS_NOESCAPE const Predicate &f) const
    {
      return Value.containsIf(f);
    }

    template <typename F>
    decltype(auto) map(F &&functor) const
    {
      return Value.map(std::forward<F>(functor));
    }

    bool operator==(const CommaSeparatedFixedList &) const = default;

    Container Value;
  };

  template <typename T>
  constexpr auto TreatAsRangeLike<CommaSeparatedFixedList<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<CommaSeparatedFixedList<T>> = SerialisationSeparatorType::Comma;

  // Wraps a variable (though known at construction) number of elements of a single type in a reference
  // counted container, semantically marking them as serializing as "space separated".
  template <typename T>
  struct SpaceSeparatedRefCountedFixedVector
  {
    using Container = krys::RefCountedFixedVector<T>;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;
    using value_type = typename Container::value_type;

    SpaceSeparatedRefCountedFixedVector(T &&value) : Value {Container::create(krys::move(value))}
    {
    }

    SpaceSeparatedRefCountedFixedVector(std::initializer_list<T> initializerList)
        : Value {Container::create(initializerList)}
    {
    }

    SpaceSeparatedRefCountedFixedVector(Ref<Container> &&value) : Value {krys::move(value)}
    {
    }

    template <typename SizedRange, typename Mapper>
    static SpaceSeparatedRefCountedFixedVector map(SizedRange &&range, KRYS_NOESCAPE Mapper &&mapper)
    {
      return Container::map(std::forward<SizedRange>(range), std::forward<Mapper>(mapper));
    }

    template <std::invocable<size_t> Generator>
    static SpaceSeparatedRefCountedFixedVector
      createWithSizeFromGenerator(size_t size, KRYS_NOESCAPE Generator &&generator)
    {
      return Container::createWithSizeFromGenerator(size, std::forward<Generator>(generator));
    }

    const_iterator begin() const KRYS_LIFETIME_BOUND
    {
      return Value->begin();
    }
    const_iterator end() const KRYS_LIFETIME_BOUND
    {
      return Value->end();
    }
    const_reverse_iterator rbegin() const KRYS_LIFETIME_BOUND
    {
      return Value->rbegin();
    }
    const_reverse_iterator rend() const KRYS_LIFETIME_BOUND
    {
      return Value->rend();
    }

    bool isEmpty() const
    {
      return Value->isEmpty();
    }
    size_t size() const
    {
      return Value->size();
    }
    const T &operator[](size_t i) const KRYS_LIFETIME_BOUND
    {
      return (*Value)[i];
    }

    const T &first() const KRYS_LIFETIME_BOUND
    {
      return Value->first();
    }
    const T &last() const KRYS_LIFETIME_BOUND
    {
      return Value->last();
    }

    bool operator==(const SpaceSeparatedRefCountedFixedVector &other) const
    {
      return ArePointingToEqualData(Value, other.Value);
    }

    Ref<Container> Value;
  };

  template <typename T>
  constexpr auto TreatAsRangeLike<SpaceSeparatedRefCountedFixedVector<T>> = true;
  template <typename T>
  constexpr auto SerialisationSeparator<SpaceSeparatedRefCountedFixedVector<T>> =
    SerialisationSeparatorType::Space;

  // Wraps a variable (though known at construction) number of elements of a single type in a reference
  // counted container, semantically marking them as serializing as "comma separated".
  template <typename T>
  struct CommaSeparatedRefCountedFixedVector
  {
    using Container = krys::RefCountedFixedVector<T>;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;
    using value_type = typename Container::value_type;

    CommaSeparatedRefCountedFixedVector(T &&value) : Value {Container::create(krys::move(value))}
    {
    }

    CommaSeparatedRefCountedFixedVector(std::initializer_list<T> initializerList)
        : Value {Container::create(initializerList)}
    {
    }

    CommaSeparatedRefCountedFixedVector(Ref<Container> &&value) : Value {krys::move(value)}
    {
    }

    template <typename SizedRange, typename Mapper>
    static CommaSeparatedRefCountedFixedVector map(SizedRange &&range, KRYS_NOESCAPE Mapper &&mapper)
    {
      return Container::map(std::forward<SizedRange>(range), std::forward<Mapper>(mapper));
    }

    template <std::invocable<size_t> Generator>
    static CommaSeparatedRefCountedFixedVector
      createWithSizeFromGenerator(size_t size, KRYS_NOESCAPE Generator &&generator)
    {
      return Container::createWithSizeFromGenerator(size, std::forward<Generator>(generator));
    }

    const_iterator begin() const KRYS_LIFETIME_BOUND
    {
      return Value->begin();
    }
    const_iterator end() const KRYS_LIFETIME_BOUND
    {
      return Value->end();
    }
    const_reverse_iterator rbegin() const KRYS_LIFETIME_BOUND
    {
      return Value->rbegin();
    }
    const_reverse_iterator rend() const KRYS_LIFETIME_BOUND
    {
      return Value->rend();
    }

    bool isEmpty() const
    {
      return Value->isEmpty();
    }
    size_t size() const
    {
      return Value->size();
    }
    const T &operator[](size_t i) const KRYS_LIFETIME_BOUND
    {
      return (*Value)[i];
    }

    const T &first() const KRYS_LIFETIME_BOUND
    {
      return Value->first();
    }
    const T &last() const KRYS_LIFETIME_BOUND
    {
      return Value->last();
    }

    bool operator==(const CommaSeparatedRefCountedFixedVector &other) const
    {
      return ArePointingToEqualData(Value, other.Value);
    }

    Ref<Container> Value;
  };

  template <typename T>
  constexpr auto TreatAsRangeLike<CommaSeparatedRefCountedFixedVector<T>> = true;
  template <typename T>
  constexpr auto SerialisationSeparator<CommaSeparatedRefCountedFixedVector<T>> =
    SerialisationSeparatorType::Comma;

#pragma region SpaceSeparatedTuple

  /// @brief Wraps a variadic list of types, semantically marking them as serializing as "space separated".
  template <typename... Ts>
  struct SpaceSeparatedTuple
  {
    using Tuple = std::tuple<Ts...>;

    std::tuple<Ts...> Value;

    constexpr SpaceSeparatedTuple(Ts &&...values) noexcept
        : Value(std::make_tuple(std::forward<Ts>(values)...))
    {
    }

    constexpr SpaceSeparatedTuple(const Ts &...values) noexcept : Value(std::make_tuple(values...))
    {
    }

    constexpr SpaceSeparatedTuple(std::tuple<Ts...> &&tuple) noexcept : Value(krys::move(tuple))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const SpaceSeparatedTuple<Ts...> &) const = default;
  };

  template <size_t I, typename... Ts>
  KRYS_NODISCARD decltype(auto) get(const SpaceSeparatedTuple<Ts...> &tuple) noexcept
  {
    return std::get<I>(tuple.Value);
  }

  template <typename... Ts>
  constexpr bool TreatAsTupleLike<SpaceSeparatedTuple<Ts...>> = true;

  template <typename... Ts>
  constexpr auto SerialisationSeparator<SpaceSeparatedTuple<Ts...>> = SerialisationSeparatorType::Space;

#pragma endregion

#pragma region CommaSeparatedTuple

  /// @brief Wraps a variadic list of types, semantically marking them as serializing as "comma separated".
  template <typename... Ts>
  struct CommaSeparatedTuple
  {
    using Tuple = std::tuple<Ts...>;

    std::tuple<Ts...> Value;

    constexpr CommaSeparatedTuple(Ts &&...values) noexcept
        : Value(std::make_tuple(std::forward<Ts>(values)...))
    {
    }

    constexpr CommaSeparatedTuple(const Ts &...values) noexcept : Value(std::make_tuple(values...))
    {
    }

    constexpr CommaSeparatedTuple(std::tuple<Ts...> &&tuple) noexcept : Value(krys::move(tuple))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const CommaSeparatedTuple<Ts...> &) const noexcept = default;
  };

  template <size_t I, typename... Ts>
  KRYS_NODISCARD decltype(auto) get(const CommaSeparatedTuple<Ts...> &tuple) noexcept
  {
    return std::get<I>(tuple.Value);
  }

  template <typename... Ts>
  constexpr bool TreatAsTupleLike<CommaSeparatedTuple<Ts...>> = true;

  template <typename... Ts>
  constexpr auto SerialisationSeparator<CommaSeparatedTuple<Ts...>> = SerialisationSeparatorType::Comma;

#pragma endregion

#pragma region SpaceSeparatedPoint

  /// @brief Wraps a pair of elements of a single type representing a point, semantically marking them as
  /// serializing as "space separated".
  template <typename T>
  struct SpaceSeparatedPoint
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> Value;

    constexpr SpaceSeparatedPoint(T p1, T p2) noexcept : Value(krys::move(p1), krys::move(p2))
    {
    }

    constexpr SpaceSeparatedPoint(SpaceSeparatedPair<T> &&array) noexcept : Value(krys::move(array))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const SpaceSeparatedPoint<T> &) const noexcept = default;

    KRYS_NODISCARD const T &x() const noexcept
    {
      return get<0>(Value);
    }

    KRYS_NODISCARD const T &y() const noexcept
    {
      return get<1>(Value);
    }
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const SpaceSeparatedPoint<T> &point) noexcept
  {
    return get<I>(point.Value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<SpaceSeparatedPoint<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<SpaceSeparatedPoint<T>> = SerialisationSeparatorType::Space;

#pragma endregion

#pragma region SpaceSeparatedSize

  /// @brief Wraps a pair of elements of a single type representing a size, semantically marking them as
  /// serializing as "space separated".
  template <typename T>
  struct SpaceSeparatedSize
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> Value;

    constexpr SpaceSeparatedSize(T p1, T p2) noexcept : Value(krys::move(p1), krys::move(p2))
    {
    }

    constexpr SpaceSeparatedSize(SpaceSeparatedPair<T> &&array) noexcept : Value(krys::move(array))
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const SpaceSeparatedSize<T> &) const noexcept = default;

    KRYS_NODISCARD const T &Width() const noexcept
    {
      return get<0>(Value);
    }

    KRYS_NODISCARD const T &Height() const noexcept
    {
      return get<1>(Value);
    }
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const SpaceSeparatedSize<T> &size) noexcept
  {
    return get<I>(size.Value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<SpaceSeparatedSize<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<SpaceSeparatedSize<T>> = SerialisationSeparatorType::Space;

#pragma endregion

#pragma region MinimallySerialisingSpaceSeparatedPoint

  /// @brief Wraps a pair of elements of a single type representing a point, semantically marking them as
  /// serializing as "space separated" and "minimally serializing".
  template <typename T>
  struct MinimallySerialisingSpaceSeparatedPoint
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> Value;

    template <typename U>
    constexpr MinimallySerialisingSpaceSeparatedPoint(U p1) noexcept : Value(p1, p1)
    {
    }

    template <typename U>
    constexpr MinimallySerialisingSpaceSeparatedPoint(U p1, U p2) noexcept
        : Value(krys::move(p1), krys::move(p2))
    {
    }

    constexpr MinimallySerialisingSpaceSeparatedPoint(SpaceSeparatedPair<T> &&array) noexcept
        : Value {krys::move(array)}
    {
    }

    KRYS_NODISCARD constexpr bool
      operator==(const MinimallySerialisingSpaceSeparatedPoint<T> &) const noexcept = default;

    KRYS_NODISCARD const T &x() const noexcept
    {
      return get<0>(Value);
    }

    KRYS_NODISCARD const T &y() const noexcept
    {
      return get<1>(Value);
    }
  };

  template <size_t I, typename T>
  decltype(auto) get(const MinimallySerialisingSpaceSeparatedPoint<T> &point) noexcept
  {
    return get<I>(point.Value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<MinimallySerialisingSpaceSeparatedPoint<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<MinimallySerialisingSpaceSeparatedPoint<T>> =
    SerialisationSeparatorType::Space;

  template <typename T>
  constexpr auto SerialisationCoalescing<MinimallySerialisingSpaceSeparatedPoint<T>> =
    SerialisationCoalescingType::Minimal;

#pragma endregion

#pragma region MinimallySerialisingSpaceSeparatedSize

  // Wraps a pair of elements of a single type representing a size, semantically marking them as serializing
  // as "space separated" and "minimally serializing".
  template <typename T>
  struct MinimallySerialisingSpaceSeparatedSize
  {
    using Array = SpaceSeparatedPair<T>;
    using value_type = T;

    SpaceSeparatedPair<T> Value;

    template <typename U>
    constexpr MinimallySerialisingSpaceSeparatedSize(U p1) noexcept : Value(p1, p1)
    {
    }

    template <typename U>
    constexpr MinimallySerialisingSpaceSeparatedSize(U p1, U p2) noexcept
        : Value(krys::move(p1), krys::move(p2))
    {
    }

    constexpr MinimallySerialisingSpaceSeparatedSize(SpaceSeparatedPair<T> &&array) noexcept
        : Value(krys::move(array))
    {
    }

    KRYS_NODISCARD constexpr bool
      operator==(const MinimallySerialisingSpaceSeparatedSize<T> &) const noexcept = default;

    KRYS_NODISCARD constexpr const T &Width() const noexcept
    {
      return get<0>(Value);
    }

    KRYS_NODISCARD constexpr const T &Height() const noexcept
    {
      return get<1>(Value);
    }
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const MinimallySerialisingSpaceSeparatedSize<T> &size) noexcept
  {
    return get<I>(size.Value);
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<MinimallySerialisingSpaceSeparatedSize<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<MinimallySerialisingSpaceSeparatedSize<T>> =
    SerialisationSeparatorType::Space;

  template <typename T>
  constexpr auto SerialisationCoalescing<MinimallySerialisingSpaceSeparatedSize<T>> =
    SerialisationCoalescingType::Minimal;

#pragma endregion

#pragma region SpaceSeparatedRectEdges

  /// @brief Wraps a quad of elements of a single type representing the edges of a rect, semantically marking
  /// them as serializing as "space separated".
  template <typename T>
  struct SpaceSeparatedRectEdges : public RectEdges<T>
  {
    using value_type = T;

    constexpr SpaceSeparatedRectEdges(T repeat) noexcept : RectEdges<T> {repeat, repeat, repeat, repeat}
    {
    }

    constexpr SpaceSeparatedRectEdges(T top, T right, T bottom, T left) noexcept
        : RectEdges<T> {krys::move(top), krys::move(right), krys::move(bottom), krys::move(left)}
    {
    }

    constexpr SpaceSeparatedRectEdges(RectEdges<T> &&rectEdges) noexcept
        : RectEdges<T> {krys::move(rectEdges)}
    {
    }

    constexpr bool operator==(const SpaceSeparatedRectEdges<T> &) const noexcept = default;
  };

  template <size_t I, typename T>
  KRYS_NODISCARD const auto &get(const SpaceSeparatedRectEdges<T> &rectEdges) noexcept
  {
    if constexpr (!I)
    {
      return rectEdges.Top();
    }
    else if constexpr (I == 1)
    {
      return rectEdges.Right();
    }
    else if constexpr (I == 2)
    {
      return rectEdges.Bottom();
    }
    else if constexpr (I == 3)
    {
      return rectEdges.Left();
    }
  }

  template <typename T>
  constexpr auto TreatAsTupleLike<SpaceSeparatedRectEdges<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<SpaceSeparatedRectEdges<T>> = SerialisationSeparatorType::Space;

#pragma endregion

#pragma region CommaSeparatedRectEdges

  /// @brief Wraps a quad of elements of a single type representing the edges of a rect, semantically marking
  /// them as serializing as "comma separated".
  template <typename T>
  struct CommaSeparatedRectEdges : public RectEdges<T>
  {
    using value_type = T;

    constexpr CommaSeparatedRectEdges(T repeat) noexcept : RectEdges<T> {repeat, repeat, repeat, repeat}
    {
    }

    constexpr CommaSeparatedRectEdges(T top, T right, T bottom, T left) noexcept
        : RectEdges<T> {krys::move(top), krys::move(right), krys::move(bottom), krys::move(left)}
    {
    }

    constexpr CommaSeparatedRectEdges(RectEdges<T> &&rectEdges) noexcept
        : RectEdges<T> {krys::move(rectEdges)}
    {
    }

    constexpr bool operator==(const CommaSeparatedRectEdges<T> &) const noexcept = default;
  };

  template <size_t I, typename T>
  KRYS_NODISCARD const auto &get(const CommaSeparatedRectEdges<T> &rectEdges) noexcept
  {
    if constexpr (!I)
    {
      return rectEdges.Top();
    }
    else if constexpr (I == 1)
    {
      return rectEdges.Right();
    }
    else if constexpr (I == 2)
    {
      return rectEdges.Bottom();
    }
    else if constexpr (I == 3)
    {
      return rectEdges.Left();
    }
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<CommaSeparatedRectEdges<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<CommaSeparatedRectEdges<T>> = SerialisationSeparatorType::Comma;

#pragma endregion

#pragma region MinimallySerialisingSpaceSeparatedRectEdges

  /// @brief A set of 4 values parsed and interpreted in the same manner as defined for the margin shorthand.
  ///
  /// <minimally-serializing-rect-edges> = <type>{1,4}
  ///
  /// - if only 1 value, `a`, is provided, set top, bottom, right & left to `a`.
  /// - if only 2 values, `a` and `b` are provided, set top & bottom to `a`, right & left to `b`.
  /// - if only 3 values, `a`, `b`, and `c` are provided, set top to `a`, right to `b`, bottom to `c`, & left
  /// to `b`.
  ///
  /// As the name implies, the benefit of using this over `SpaceSeparatedRectEdges` directly
  /// is that this will serialize in its minimal form, checking for element equality and only
  /// serializing what is necessary.
  template <typename T>
  struct MinimallySerialisingSpaceSeparatedRectEdges : public RectEdges<T>
  {
    using value_type = T;

    constexpr MinimallySerialisingSpaceSeparatedRectEdges(T all) noexcept : RectEdges<T> {all, all, all, all}
    {
    }

    constexpr MinimallySerialisingSpaceSeparatedRectEdges(T top, T right, T bottom, T left) noexcept
        : RectEdges<T> {krys::move(top), krys::move(right), krys::move(bottom), krys::move(left)}
    {
    }

    constexpr MinimallySerialisingSpaceSeparatedRectEdges(RectEdges<T> &&rectEdges) noexcept
        : RectEdges<T> {krys::move(rectEdges)}
    {
    }

    constexpr bool
      operator==(const MinimallySerialisingSpaceSeparatedRectEdges<T> &) const noexcept = default;
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const MinimallySerialisingSpaceSeparatedRectEdges<T> &edges) noexcept
  {
    if constexpr (!I)
    {
      return edges.Top();
    }
    else if constexpr (I == 1)
    {
      return edges.Right();
    }
    else if constexpr (I == 2)
    {
      return edges.Bottom();
    }
    else if constexpr (I == 3)
    {
      return edges.Left();
    }
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<MinimallySerialisingSpaceSeparatedRectEdges<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<MinimallySerialisingSpaceSeparatedRectEdges<T>> =
    SerialisationSeparatorType::Space;

  template <typename T>
  constexpr auto SerialisationCoalescing<MinimallySerialisingSpaceSeparatedRectEdges<T>> =
    SerialisationCoalescingType::Minimal;

#pragma endregion

#pragma region MinimallySerialisingSpaceSeparatedRectCorners

  template <typename T>
  struct MinimallySerialisingSpaceSeparatedRectCorners : public RectCorners<T>
  {
    using value_type = T;

    constexpr MinimallySerialisingSpaceSeparatedRectCorners(T all) noexcept
        : RectCorners<T> {all, all, all, all}
    {
    }

    constexpr MinimallySerialisingSpaceSeparatedRectCorners(T topLeft, T topRight, T bottomLeft,
                                                            T bottomRight) noexcept
        : RectCorners<T> {krys::move(topLeft), krys::move(topRight), krys::move(bottomLeft),
                          krys::move(bottomRight)}
    {
    }

    constexpr MinimallySerialisingSpaceSeparatedRectCorners(RectCorners<T> &&rectCorners) noexcept
        : RectCorners<T> {krys::move(rectCorners)}
    {
    }

    constexpr bool
      operator==(const MinimallySerialisingSpaceSeparatedRectCorners<T> &) const noexcept = default;
  };

  template <size_t I, typename T>
  KRYS_NODISCARD decltype(auto) get(const MinimallySerialisingSpaceSeparatedRectCorners<T> &corners) noexcept
  {
    if constexpr (!I)
    {
      return corners.TopLeft();
    }
    else if constexpr (I == 1)
    {
      return corners.TopRight();
    }
    else if constexpr (I == 2)
    {
      return corners.BottomLeft();
    }
    else if constexpr (I == 3)
    {
      return corners.BottomRight();
    }
  }

  template <typename T>
  constexpr bool TreatAsTupleLike<MinimallySerialisingSpaceSeparatedRectCorners<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<MinimallySerialisingSpaceSeparatedRectCorners<T>> =
    SerialisationSeparatorType::Space;

  template <typename T>
  constexpr auto SerialisationCoalescing<MinimallySerialisingSpaceSeparatedRectCorners<T>> =
    SerialisationCoalescingType::Minimal;

#pragma endregion

#pragma region ListOrNone

  /// @brief Wraps a list and enforces the invariant that it is either created with a non-empty value or
  /// `keywords::None`.
  template <typename T>
  struct ListOrNone
  {
    using List = T;
    using const_iterator = typename List::const_iterator;
    using const_reverse_iterator = typename List::const_reverse_iterator;
    using value_type = typename List::value_type;

  protected:
    // An empty list indicates the value `none`. This invariant is ensured
    // with a release assert in the constructor.
    List _value;

  public:
    ListOrNone(List &&list) : _value {krys::move(list)}
    {
      krys_assert(!_value.isEmpty());
    }

    ListOrNone(keywords::None) : _value {}
    {
    }

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return _value.begin();
    }

    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return _value.end();
    }

    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept
    {
      return _value.rbegin();
    }
    KRYS_NODISCARD const_reverse_iterator rend() const noexcept
    {
      return _value.rend();
    }

    KRYS_NODISCARD const value_type &first() const noexcept
    {
      return _value.first();
    }
    KRYS_NODISCARD const value_type &last() const noexcept
    {
      return _value.last();
    }

    KRYS_NODISCARD size_t size() const noexcept
    {
      return _value.size();
    }
    KRYS_NODISCARD const value_type &operator[](size_t i) const noexcept
    {
      return _value[i];
    }

    bool operator==(const ListOrNone &) const = default;

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _value.isEmpty();
    }
    KRYS_NODISCARD bool IsList() const noexcept
    {
      return !_value.isEmpty();
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(keywords::None {});
      }

      return visitor(_value);
    }
  };

  template <typename T>
  constexpr auto TreatAsVariantLike<ListOrNone<T>> = true;

  // Concept to constrain types to only those that derive from `ListOrNone`.
  template <typename T>
  concept ListOrNoneDerived = IsBaseOfTemplate<ListOrNone, T>;

#pragma endregion

#pragma region ListOrDefault

  // Wraps a list and makes it so that when the list is empty, it looks to clients like it has a single
  // "default" item in instead.
  template <typename T, typename Defaulter>
  struct ListOrDefault
  {
    // Special value to construct the empty (e.g. list with just the default value) list.
    struct DefaultValueToken
    {
    };
    constexpr static DefaultValueToken DefaultValue {};

    using List = T;
    using value_type = typename List::value_type;

    // Iterator that iterates a fictitious single item list, [default value], if the underlying list is empty,
    // or the underlying list.
    struct const_iterator
    {
      typename List::const_iterator it;
      bool atEndForDefault;
      const ListOrDefault<List, Defaulter> *owner;

      using iterator_category = std::forward_iterator_tag;
      using value_type = typename List::value_type;
      using difference_type = std::ptrdiff_t;
      using pointer = const value_type *;
      using reference = const value_type &;

      const value_type &operator*() const
      {
        if (owner->IsDefault())
          return owner->defaulter();
        return *it;
      }

      const_iterator &operator++()
      {
        if (owner->IsDefault())
        {
          atEndForDefault = true;
        }
        else
        {
          ++it;
        }
        return *this;
      }

      const_iterator operator++(int)
      {
        auto result = *this;
        ++*this;
        return result;
      }

      bool operator==(const const_iterator &other) const = default;
    };

    ListOrDefault(List &&list, Defaulter &&defaulter = Defaulter()) noexcept
        : Value {krys::move(list)}, defaulter {krys::move(defaulter)}
    {
    }

    ListOrDefault(DefaultValueToken, Defaulter &&defaulter = Defaulter()) noexcept
        : Value {}, defaulter {krys::move(defaulter)}
    {
    }

    const_iterator begin() const noexcept
    {
      return {.it = Value.begin(), .atEndForDefault = !IsDefault(), .owner = this};
    }

    const_iterator end() const noexcept
    {
      return {.it = Value.end(), .atEndForDefault = true, .owner = this};
    }

    size_t size() const noexcept
    {
      return IsDefault() ? 1 : Value.size();
    }

    const value_type &operator[](size_t i) const noexcept
    {
      return IsDefault() ? defaulter() : Value[i];
    }

    bool contains(const auto &x) const noexcept
    {
      return IsDefault() ? (x == defaulter()) : Value.contains(x);
    }
    template <typename Predicate>
    requires std::predicate<Predicate, const value_type &>
    bool containsIf(const Predicate &f) const noexcept
    {
      return IsDefault() ? f(defaulter()) : Value.containsIf(f);
    }

    bool IsDefault() const noexcept
    {
      return Value.isEmpty();
    }

    bool operator==(const ListOrDefault &) const noexcept = default;

  private:
    friend struct const_iterator;

    List Value;
    KRYS_NO_UNIQUE_ADDRESS Defaulter defaulter;
  };

  template <typename List, typename Defaulter>
  constexpr auto TreatAsRangeLike<ListOrDefault<List, Defaulter>> = true;
  template <typename List, typename Defaulter>
  constexpr auto SerialisationSeparator<ListOrDefault<List, Defaulter>> = SerialisationSeparator<List>;

  // Concept to constrain types to only those that derive from `ListOrDefault`.
  template <typename T>
  concept ListOrDefaultDerived = IsBaseOfTemplate<ListOrDefault, T>;

// Helper to define a range-like conformance for a type that derives from `ListOrDefault`.
#define DEFINE_RANGE_LIKE_CONFORMANCE_FOR_LIST_OR_DEFAULT_DERIVED_TYPE(t)                                    \
  DEFINE_RANGE_LIKE_CONFORMANCE(t)                                                                           \
  template <>                                                                                                \
  constexpr auto krys::boo::css::SerialisationSeparator<t> =                                                 \
    krys::boo::css::SerialisationSeparator<typename t::List>;

#pragma endregion

#pragma region ListOrNull

  // Wraps a list and behaves as `optional-like`, using the `list.IsEmpty()` as the nullopt state.
  template <typename T>
  struct ListOrNull
  {
    using List = T;
    using const_iterator = typename List::const_iterator;
    using const_reverse_iterator = typename List::const_reverse_iterator;
    using value_type = typename List::value_type;

    ListOrNull(List &&list) noexcept : _value {krys::move(list)}
    {
      krys_assert(!_value.isEmpty());
    }

    ListOrNull(std::nullopt_t) noexcept : _value {}
    {
    }

    ListOrNull() noexcept : _value {}
    {
    }

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return _value.begin();
    }

    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return _value.end();
    }

    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept
    {
      return _value.rbegin();
    }

    KRYS_NODISCARD const_reverse_iterator rend() const noexcept
    {
      return _value.rend();
    }

    KRYS_NODISCARD const value_type &first() const noexcept
    {
      return _value.first();
    }

    KRYS_NODISCARD const value_type &last() const noexcept
    {
      return _value.last();
    }

    KRYS_NODISCARD size_t size() const
    {
      return _value.size();
    }

    KRYS_NODISCARD const value_type &operator[](size_t i) const noexcept
    {
      return _value[i];
    }

    KRYS_NODISCARD bool contains(const auto &x) const
    {
      return _value.contains(x);
    }

    template <typename Predicate>
    requires std::predicate<Predicate, const value_type &>
    KRYS_NODISCARD bool containsIf(const Predicate &f) const
    {
      return _value.containsIf(f);
    }

    KRYS_NODISCARD bool operator==(const ListOrNull &) const = default;

    KRYS_NODISCARD bool IsNull() const
    {
      return _value.isEmpty();
    }

    KRYS_NODISCARD bool IsList() const
    {
      return !_value.isEmpty();
    }

    KRYS_NODISCARD const List *TryList() const noexcept
    {
      return IsList() ? &_value : nullptr;
    }

    // Returns the underlying list regardless of whether or not it is empty.
    KRYS_NODISCARD const List &GetList() const noexcept
    {
      return _value;
    }

    explicit operator bool() const
    {
      return !_value.isEmpty();
    }

    const List *operator->() const noexcept
    {
      return &_value;
    }

    const List &operator*() const noexcept
    {
      return _value;
    }

  protected:
    List _value;
  };

  template <typename List>
  constexpr auto TreatAsOptionalLike<ListOrNull<List>> = true;

#pragma endregion

#pragma region EnumSetOrKeyword

  // Wraps an `EnumSet` and enforces the invariant that it is either created with a non-empty value or
  // specified keyword. Required to be subclassed, passing the derived type as the first template parameter.
  template <typename Derived, typename T, typename K>
  struct EnumSetOrKeywordBase
  {
    using Base = EnumSetOrKeywordBase<Derived, T, K>;
    using EnumSet = T;
    using Keyword = K;
    using value_type = typename EnumSet::value_type;

    constexpr EnumSetOrKeywordBase(EnumSet &&list) noexcept : _value {krys::move(list)}
    {
      krys_assert(!_value.IsEmpty());
    }

    constexpr EnumSetOrKeywordBase(Keyword) noexcept : _value {}
    {
    }

    constexpr EnumSetOrKeywordBase(value_type element) noexcept : EnumSetOrKeywordBase {EnumSet {element}}
    {
    }

    constexpr EnumSetOrKeywordBase(std::initializer_list<value_type> initializerList) noexcept
        : EnumSetOrKeywordBase {EnumSet {initializerList}}
    {
    }

    constexpr static Derived FromRaw(typename EnumSet::StorageType rawValue) noexcept
    {
      if (!rawValue)
        return Keyword {};
      return EnumSet::FromRaw(rawValue);
    }

    constexpr typename EnumSet::StorageType ToRaw() const noexcept
    {
      return _value.ToRaw();
    }

    KRYS_NODISCARD constexpr bool Contains(value_type e) const noexcept
    {
      return _value.Contains(e);
    }

    KRYS_NODISCARD constexpr bool ContainsAny(EnumSet other) const noexcept
    {
      return _value.ContainsAny(other);
    }

    KRYS_NODISCARD constexpr bool ContainsAll(EnumSet other) const noexcept
    {
      return _value.ContainsAll(other);
    }

    KRYS_NODISCARD constexpr bool ContainsOnly(EnumSet other) const noexcept
    {
      return _value.ContainsOnly(other);
    }

    KRYS_NODISCARD constexpr decltype(auto) begin() const noexcept
    {
      return _value.begin();
    }

    KRYS_NODISCARD constexpr decltype(auto) end() const noexcept
    {
      return _value.end();
    }

    KRYS_NODISCARD constexpr size_t Size() const noexcept
    {
      return _value.Size();
    }

    KRYS_NODISCARD constexpr bool Contains(const auto &x) const noexcept
    {
      return _value.Contains(x);
    }

    constexpr bool operator==(const EnumSetOrKeywordBase &) const noexcept = default;

    KRYS_NODISCARD constexpr bool IsKeyword() const noexcept
    {
      return _value.IsEmpty();
    }

    KRYS_NODISCARD constexpr bool IsEnumSet() const noexcept
    {
      return !_value.IsEmpty();
    }

    KRYS_NODISCARD constexpr const EnumSet *TryEnumSet() const noexcept
    {
      return IsEnumSet() ? &_value : nullptr;
    }

    template <typename... F>
    KRYS_NODISCARD constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsKeyword())
      {
        return visitor(Keyword {});
      }

      return visitor(_value);
    }

  protected:
    // An empty enum set indicates the value is `Keyword`. This invariant is ensured
    // with a release assert in the constructor.
    EnumSet _value;
  };

  // Concept to constrain types to only those that derive from `EnumSetOrKeywordBase`.
  template <typename T>
  concept EnumSetOrKeywordBaseDerived = IsBaseOfTemplate<EnumSetOrKeywordBase, T>;

#pragma endregion

#pragma region ValueOrKeyword

  /// @brief Wraps a `markable` type and enforces the invariant that it is either created with a non-empty
  /// value or the provided keyword.
  template <typename T, typename K, typename Traits = MarkableTraits<T>>
  struct ValueOrKeyword
  {
    using Base = ValueOrKeyword<T, K, Traits>;
    using Value = T;
    using Keyword = K;

  protected:
    constexpr ValueOrKeyword(Maybe<Value> &&maybeValue) : _value {krys::move(maybeValue)}
    {
    }

    Markable<Value, Traits> _value {};

  public:
    constexpr ValueOrKeyword(Keyword) noexcept
    {
    }

    constexpr ValueOrKeyword(Value &&initialValue) noexcept : _value {krys::move(initialValue)}
    {
    }

    KRYS_NODISCARD constexpr bool IsKeyword() const noexcept
    {
      return !_value;
    }

    KRYS_NODISCARD constexpr bool IsValue() const noexcept
    {
      return !!_value;
    }

    KRYS_NODISCARD constexpr Maybe<Value> TryValue() const noexcept
    {
      return _value;
    }

    template <typename U>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      if constexpr (SameType<U, Keyword>)
      {
        return IsKeyword();
      }
      else if constexpr (SameType<U, Value>)
      {
        return IsValue();
      }
    }

    template <typename... F>
    KRYS_NODISCARD constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsKeyword())
      {
        return visitor(Keyword {});
      }

      return visitor(*_value);
    }

    constexpr bool operator==(const ValueOrKeyword &) const noexcept = default;
  };

  template <typename T, typename K, typename Traits>
  constexpr auto TreatAsVariantLike<ValueOrKeyword<T, K, Traits>> = true;

  // Concept to constrain types to only those that derive from `ValueOrKeyword`.
  template <typename T>
  concept ValueOrKeywordDerived = IsBaseOfTemplate<ValueOrKeyword, T>;

#pragma endregion

#pragma region SpaceSeparatedEnumSet

  // Wraps an enum set, semantically marking it as serializing as "space separated".
  template <IsEnum T>
  struct SpaceSeparatedEnumSet
  {
    using Container = EnumSet<T>;
    using StorageType = typename Container::StorageType;
    using const_iterator = typename Container::iterator;
    using value_type = T;

    Container Value;

    constexpr SpaceSeparatedEnumSet() noexcept = default;

    constexpr SpaceSeparatedEnumSet(std::initializer_list<T> initializerList) noexcept
        : Value {initializerList}
    {
    }

    constexpr SpaceSeparatedEnumSet(Container &&container) noexcept : Value {krys::move(container)}
    {
    }

    template <typename SizedRange, typename Mapper>
    KRYS_NODISCARD static SpaceSeparatedEnumSet Map(SizedRange &&range, Mapper &&mapper) noexcept
    {
      Container result;
      for (auto &&Value : range)
      {
        result.Add(mapper(Value));
      }

      return result;
    }

    KRYS_NODISCARD constexpr static SpaceSeparatedEnumSet FromRaw(StorageType rawValue) noexcept
    {
      return Container::FromRaw(rawValue);
    }

    KRYS_NODISCARD constexpr StorageType ToRaw() const noexcept
    {
      return Value.ToRaw();
    }

    KRYS_NODISCARD constexpr const_iterator begin() const noexcept
    {
      return Value.begin();
    }

    KRYS_NODISCARD constexpr const_iterator end() const noexcept
    {
      return Value.end();
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return Value.IsEmpty();
    }

    KRYS_NODISCARD constexpr size_t Size() const noexcept
    {
      return Value.Size();
    }

    KRYS_NODISCARD constexpr bool Contains(T e) const noexcept
    {
      return Value.Contains(e);
    }

    KRYS_NODISCARD constexpr bool ContainsAny(Container other) const noexcept
    {
      return Value.ContainsAny(other);
    }

    KRYS_NODISCARD constexpr bool ContainsAll(Container other) const noexcept
    {
      return Value.ContainsAll(other);
    }

    KRYS_NODISCARD constexpr bool ContainsOnly(Container other) const noexcept
    {
      return Value.ContainsOnly(other);
    }

    constexpr friend SpaceSeparatedEnumSet operator&(SpaceSeparatedEnumSet lhs,
                                                     SpaceSeparatedEnumSet rhs) noexcept
    {
      return lhs.Value & rhs.Value;
    }

    constexpr friend SpaceSeparatedEnumSet operator-(SpaceSeparatedEnumSet lhs,
                                                     SpaceSeparatedEnumSet rhs) noexcept
    {
      return lhs.Value - rhs.Value;
    }

    constexpr friend SpaceSeparatedEnumSet operator^(SpaceSeparatedEnumSet lhs,
                                                     SpaceSeparatedEnumSet rhs) noexcept
    {
      return lhs.Value ^ rhs.Value;
    }

    constexpr bool operator==(const SpaceSeparatedEnumSet &) const noexcept = default;
  };

  template <typename T>
  constexpr auto TreatAsRangeLike<SpaceSeparatedEnumSet<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<SpaceSeparatedEnumSet<T>> = SerialisationSeparatorType::Space;

#pragma endregion

#pragma region CommaSeparatedEnumSet

  // Wraps an enum set, semantically marking it as serializing as "comma separated".
  template <IsEnum T>
  struct CommaSeparatedEnumSet
  {
    using Container = EnumSet<T>;
    using StorageType = typename Container::StorageType;
    using const_iterator = typename Container::iterator;
    using value_type = T;

    Container Value;

    constexpr CommaSeparatedEnumSet() noexcept = default;

    constexpr CommaSeparatedEnumSet(std::initializer_list<T> initializerList) noexcept
        : Value {initializerList}
    {
    }

    constexpr CommaSeparatedEnumSet(Container &&container) noexcept : Value {krys::move(container)}
    {
    }

    template <typename SizedRange, typename Mapper>
    KRYS_NODISCARD constexpr static CommaSeparatedEnumSet Map(SizedRange &&range, Mapper &&mapper) noexcept
    {
      Container result;
      for (auto &&Value : range)
      {
        result.Add(mapper(Value));
      }

      return result;
    }

    KRYS_NODISCARD constexpr static CommaSeparatedEnumSet FromRaw(StorageType rawValue) noexcept
    {
      return Container::FromRaw(rawValue);
    }

    KRYS_NODISCARD constexpr StorageType ToRaw() const noexcept
    {
      return Value.ToRaw();
    }

    KRYS_NODISCARD constexpr const_iterator begin() const noexcept
    {
      return Value.begin();
    }

    KRYS_NODISCARD constexpr const_iterator end() const noexcept
    {
      return Value.end();
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return Value.IsEmpty();
    }

    KRYS_NODISCARD constexpr size_t Size() const noexcept
    {
      return Value.Size();
    }

    KRYS_NODISCARD constexpr bool Contains(T e) const noexcept
    {
      return Value.Contains(e);
    }

    KRYS_NODISCARD constexpr bool ContainsAny(Container other) const noexcept
    {
      return Value.ContainsAny(other);
    }

    KRYS_NODISCARD constexpr bool ContainsAll(Container other) const noexcept
    {
      return Value.ContainsAll(other);
    }

    KRYS_NODISCARD constexpr bool ContainsOnly(Container other) const noexcept
    {
      return Value.ContainsOnly(other);
    }

    constexpr friend CommaSeparatedEnumSet operator&(CommaSeparatedEnumSet lhs,
                                                     CommaSeparatedEnumSet rhs) noexcept
    {
      return lhs.Value & rhs.Value;
    }

    constexpr friend CommaSeparatedEnumSet operator-(CommaSeparatedEnumSet lhs,
                                                     CommaSeparatedEnumSet rhs) noexcept
    {
      return lhs.Value - rhs.Value;
    }

    constexpr friend CommaSeparatedEnumSet operator^(CommaSeparatedEnumSet lhs,
                                                     CommaSeparatedEnumSet rhs) noexcept
    {
      return lhs.Value ^ rhs.Value;
    }

    bool operator==(const CommaSeparatedEnumSet &) const noexcept = default;
  };

  template <typename T>
  constexpr auto TreatAsRangeLike<CommaSeparatedEnumSet<T>> = true;

  template <typename T>
  constexpr auto SerialisationSeparator<CommaSeparatedEnumSet<T>> = SerialisationSeparatorType::Comma;

#pragma endregion

  template <typename RangeLike>
  struct NonEmptyRangeLikeMarkableTraits
  {
    static bool IsEmptyValue(const RangeLike &range)
    {
      return std::ranges::empty(range);
    }

    static RangeLike EmptyValue()
    {
      return {};
    }
  };
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::css::CustomIdentifier>
  {
    static bool IsEmptyValue(const boo::css::CustomIdentifier &identifier)
    {
      return identifier.Value == boo::css::CSSOMStringAtom::Null();
    }

    static boo::css::CustomIdentifier EmptyValue()
    {
      return {boo::css::CSSOMStringAtom::Null()};
    }
  };
}

namespace std
{
  template <krys::boo::css::ValueId C, typename T>
  class tuple_size<krys::boo::css::FunctionNotation<C, T>> : public std::integral_constant<size_t, 1>
  {
  };

  template <size_t I, krys::boo::css::ValueId C, typename T>
  class tuple_element<I, krys::boo::css::FunctionNotation<C, T>>
  {
  public:
    using type = T;
  };

  template <typename T, size_t N>
  class tuple_size<krys::boo::css::SpaceSeparatedArray<T, N>> : public std::integral_constant<size_t, N>
  {
  };

  template <size_t I, typename T, size_t N>
  class tuple_element<I, krys::boo::css::SpaceSeparatedArray<T, N>>
  {
  public:
    using type = T;
  };

  template <typename T, size_t N>
  class tuple_size<krys::boo::css::CommaSeparatedArray<T, N>> : public std::integral_constant<size_t, N>
  {
  };

  template <size_t I, typename T, size_t N>
  class tuple_element<I, krys::boo::css::CommaSeparatedArray<T, N>>
  {
  public:
    using type = T;
  };

  template <typename... Ts>
  class tuple_size<krys::boo::css::SpaceSeparatedTuple<Ts...>>
      : public std::integral_constant<size_t, sizeof...(Ts)>
  {
  };

  template <size_t I, typename... Ts>
  class tuple_element<I, krys::boo::css::SpaceSeparatedTuple<Ts...>>
  {
  public:
    using type = tuple_element_t<I, tuple<Ts...>>;
  };

  template <typename... Ts>
  class tuple_size<krys::boo::css::CommaSeparatedTuple<Ts...>>
      : public std::integral_constant<size_t, sizeof...(Ts)>
  {
  };

  template <size_t I, typename... Ts>
  class tuple_element<I, krys::boo::css::CommaSeparatedTuple<Ts...>>
  {
  public:
    using type = tuple_element_t<I, tuple<Ts...>>;
  };

  template <typename T>
  class tuple_size<krys::boo::css::MinimallySerialisingSpaceSeparatedPair<T>>
      : public std::integral_constant<size_t, 2>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::MinimallySerialisingSpaceSeparatedPair<T>>
  {
  public:
    using type = T;
  };

  template <typename T>
  class tuple_size<krys::boo::css::SpaceSeparatedPoint<T>> : public std::integral_constant<size_t, 2>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::SpaceSeparatedPoint<T>>
  {
  public:
    using type = T;
  };

  template <typename T>
  class tuple_size<krys::boo::css::SpaceSeparatedSize<T>> : public std::integral_constant<size_t, 2>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::SpaceSeparatedSize<T>>
  {
  public:
    using type = T;
  };

  template <typename T>
  class tuple_size<krys::boo::css::MinimallySerialisingSpaceSeparatedPoint<T>>
      : public std::integral_constant<size_t, 2>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::MinimallySerialisingSpaceSeparatedPoint<T>>
  {
  public:
    using type = T;
  };

  template <typename T>
  class tuple_size<krys::boo::css::MinimallySerialisingSpaceSeparatedSize<T>>
      : public std::integral_constant<size_t, 2>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::MinimallySerialisingSpaceSeparatedSize<T>>
  {
  public:
    using type = T;
  };

  template <typename T>
  class tuple_size<krys::boo::css::SpaceSeparatedRectEdges<T>> : public std::integral_constant<size_t, 4>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::SpaceSeparatedRectEdges<T>>
  {
  public:
    using type = T;
  };

  template <typename T>
  class tuple_size<krys::boo::css::CommaSeparatedRectEdges<T>> : public std::integral_constant<size_t, 4>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::CommaSeparatedRectEdges<T>>
  {
  public:
    using type = T;
  };

  template <typename T>
  class tuple_size<krys::boo::css::MinimallySerialisingSpaceSeparatedRectEdges<T>>
      : public std::integral_constant<size_t, 4>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::MinimallySerialisingSpaceSeparatedRectEdges<T>>
  {
  public:
    using type = T;
  };

  template <typename T>
  class tuple_size<krys::boo::css::MinimallySerialisingSpaceSeparatedRectCorners<T>>
      : public std::integral_constant<size_t, 4>
  {
  };

  template <size_t I, typename T>
  class tuple_element<I, krys::boo::css::MinimallySerialisingSpaceSeparatedRectCorners<T>>
  {
  public:
    using type = T;
  };
}
