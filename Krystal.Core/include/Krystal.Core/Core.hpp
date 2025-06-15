#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <initializer_list>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <xutility>

namespace Krys
{
#pragma region Attributes

  /// @brief Convenience macro for the attribute equivalent.
#define NO_DISCARD [[nodiscard]]

/// @brief Convenience macro for the attribute equivalent.
#define NO_RETURN [[noreturn]]

/// @brief Convenience macro for the attribute equivalent.
#define ALLOW_CASE_FALLTHROUGH [[fallthrough]]

/// @brief Convenience macro for the attribute equivalent.
#define BRANCH_LIKELY [[likely]]

/// @brief Convenience macro for the attribute equivalent.
#define BRANCH_UNLIKELY [[unlikely]]

/// @brief Convenience macro for the attribute equivalent.
#define MAYBE_UNUSED [[maybe_unused]]

#pragma endregion

#pragma region Types

  using uchar = unsigned char;
  using ushort = unsigned short;
  using uint = unsigned int;
  using ulong = unsigned long;

  using int8 = int8_t;
  using int16 = int16_t;
  using int32 = int32_t;
  using int64 = int64_t;
  using fast_int8 = int_fast8_t;
  using fast_int16 = int_fast16_t;
  using fast_int32 = int_fast32_t;
  using fast_int64 = int_fast64_t;

  using uint8 = uint8_t;
  using uint16 = uint16_t;
  using uint32 = uint32_t;
  using uint64 = uint64_t;
  using fast_uint8 = uint_fast8_t;
  using fast_uint16 = uint_fast16_t;
  using fast_uint32 = uint_fast32_t;
  using fast_uint64 = uint_fast64_t;

  using byte = std::byte;
  using float32 = float;
  using float64 = double;

  using string = std::string;
  using wstring = std::wstring;
  using stringview = std::string_view;

  using std::declval;
  using std::initializer_list;

  template <typename T>
  using Nullable = std::optional<T>;

  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using Map = std::unordered_map<TKey, TValue, TKeyHasher>;

  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using MultiMap = std::unordered_multimap<TKey, TValue, TKeyHasher>;

  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using OrderedMap = std::map<TKey, TValue, TKeyHasher>;

  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using OrderedMultiMap = std::multimap<TKey, TValue, TKeyHasher>;

  template <typename T, typename U>
  using Pair = std::pair<T, U>;

  template <typename T, size_t size>
  using Array = std::array<T, size>;

  template <typename T>
  using List = std::vector<T>;

  template <typename T>
  using LinkedList = std::list<T>;

  template <typename T>
  using Queue = std::queue<T>;

  template <typename T>
  using Set = std::set<T>;

  template <typename T>
  using Func = std::function<T>;

  template <typename T, typename ErrorType = string>
  using Expected = std::expected<T, ErrorType>;

  template <typename T>
  using Unexpected = std::unexpected<T>;

  template <bool predicate, typename T, typename U>
  using ConditionalType = std::conditional_t<predicate, T, U>;

#pragma endregion

#pragma region Concepts

  template <typename T>
  concept Integral = std::is_integral_v<T>;

  template <typename T>
  concept FloatingPoint = std::is_floating_point_v<T>;

  template <typename T>
  concept SignedIntegral = std::is_signed_v<T> && Integral<T>;

  template <typename T>
  concept UnsignedIntegral = std::is_unsigned_v<T> && Integral<T>;

  template <typename T>
  concept Arithmetic = std::is_arithmetic_v<T>;

  template <typename T>
  concept StringLike = std::is_same_v<T, string> || std::is_same_v<T, stringview>;

  template <typename T>
  concept Pointer = std::is_pointer_v<T>;

  template <typename F, class... Args>
  concept Callable = std::invocable<F, Args...>;

  /// @brief Checks type `T` is callable type that does not cause side effects.
  template <typename T, class... Args>
  concept RegularCallable = std::regular_invocable<T, Args...>;

  template <typename T, class... Args>
  concept Predicate = std::predicate<T, Args...>;

  template <typename T, typename U>
  concept ConvertibleTo = std::convertible_to<T, U>;

  template <typename T>
  concept DefaultConstructible = std::is_default_constructible_v<T>;

  template <typename D, typename B>
  concept DerivedFrom = std::derived_from<D, B>;

  template <typename T, typename U>
  concept SameType = std::same_as<T, U>;

  template <typename T>
  concept IsEnum = std::is_enum_v<T>;

  template <typename T>
  concept IsScopedEnum = std::is_scoped_enum_v<T>;

#pragma endregion

#pragma region Smart Pointers

  /// @brief Owns the memory it points to.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T, typename TDeleter = std::default_delete<T>>
  using Unique = std::unique_ptr<T, TDeleter>;

  /// @brief Shares the memory it points to via reference counting.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using Ref = std::shared_ptr<T>;

  /// @brief Weakly references a resource.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using WeakRef = std::weak_ptr<T>;

  /// @brief Non-owning pointer to a resource.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using Ptr = T *;

  template <typename T, typename... Args>
  constexpr Unique<T> CreateUnique(Args &&...args)
  {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  constexpr Ref<T> CreateRef(Args &&...args)
  {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }

#pragma endregion

#pragma region Function Signature

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000))                                     \
  || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
  #define KRYS_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
  #define KRYS_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__) || defined(_MSC_VER)
  #define KRYS_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
  #define KRYS_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
  #define KRYS_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199'901)
  #define KRYS_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201'103)
  #define KRYS_FUNC_SIG __func__
#else
  #error "KRYS_FUNC_SIG unknown!"
#endif

#pragma endregion

#pragma region Compiler Warnings

  // Compiler-specific macros for disabling and restoring warnings
#if defined(_MSC_VER) // Microsoft Visual C++
  #define KRYS_DISABLE_WARNING_PUSH() __pragma(warning(push))
  #define KRYS_DISABLE_WARNING_POP() __pragma(warning(pop))
  #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName) __pragma(warning(disable : msvcWarningCode))
#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang
  #define KRYS_DISABLE_WARNING_PUSH() _Pragma("GCC diagnostic push")
  #define KRYS_DISABLE_WARNING_POP() _Pragma("GCC diagnostic pop")
  #if defined(__clang__)
    #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)                                            \
      _Pragma("clang diagnostic ignored \"" gccWarningName "\"")
  #else
    #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)                                            \
      _Pragma("GCC diagnostic ignored \"" gccWarningName "\"")
  #endif
#else
  #define KRYS_DISABLE_WARNING_PUSH()
  #define KRYS_DISABLE_WARNING_POP()
  #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)
#endif

#pragma endregion

#pragma region Macros

#define KRYS_UNUSED(x) (void)(x) // Suppress unused variable warnings

#define NO_COPY(ClassName)                                                                                   \
  ClassName(const ClassName &) = delete;                                                                     \
  ClassName &operator=(const ClassName &) = delete;

#define NO_MOVE(ClassName)                                                                                   \
  ClassName(ClassName &&) = delete;                                                                          \
  ClassName &operator=(ClassName &&) = delete;

#define NO_COPY_MOVE(ClassName)                                                                              \
  NO_COPY(ClassName)                                                                                         \
  NO_MOVE(ClassName)

#define STATIC_CLASS(ClassName)                                                                              \
  ClassName() = delete;                                                                                      \
  ~ClassName() = delete;                                                                                     \
  NO_COPY_MOVE(ClassName)

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define IS_COMPILE_TIME std::is_constant_evaluated()

/// @brief Defines bitwise operators for an enum class.
/// @param EnumType The enum class to define the operators for.
#define ENUM_BITWISE_OPERATORS(EnumType)                                                                     \
  inline constexpr EnumType operator|(EnumType a, EnumType b)                                                \
  {                                                                                                          \
    return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(a)                            \
                                 | static_cast<std::underlying_type_t<EnumType>>(b));                        \
  }                                                                                                          \
                                                                                                             \
  inline constexpr EnumType operator&(EnumType a, EnumType b)                                                \
  {                                                                                                          \
    return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(a)                            \
                                 & static_cast<std::underlying_type_t<EnumType>>(b));                        \
  }                                                                                                          \
                                                                                                             \
  inline constexpr EnumType operator^(EnumType a, EnumType b)                                                \
  {                                                                                                          \
    return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(a)                            \
                                 ^ static_cast<std::underlying_type_t<EnumType>>(b));                        \
  }                                                                                                          \
                                                                                                             \
  inline constexpr EnumType operator~(EnumType a)                                                            \
  {                                                                                                          \
    return static_cast<EnumType>(~static_cast<std::underlying_type_t<EnumType>>(a));                         \
  }                                                                                                          \
                                                                                                             \
  inline EnumType &operator|=(EnumType &a, EnumType b)                                                       \
  {                                                                                                          \
    return a = a | b;                                                                                        \
  }                                                                                                          \
                                                                                                             \
  inline EnumType &operator&=(EnumType &a, EnumType b)                                                       \
  {                                                                                                          \
    return a = a & b;                                                                                        \
  }                                                                                                          \
                                                                                                             \
  inline EnumType &operator^=(EnumType &a, EnumType b)                                                       \
  {                                                                                                          \
    return a = a ^ b;                                                                                        \
  }                                                                                                          \
  inline bool operator!(EnumType a)                                                                          \
  {                                                                                                          \
    return static_cast<std::underlying_type_t<EnumType>>(a) == 0;                                            \
  }

#pragma endregion

  class NativeHandle
  {
    void *_handle;

  public:
    constexpr NativeHandle() noexcept : _handle(nullptr)
    {
    }

    constexpr NativeHandle(void *handle) noexcept : _handle(handle)
    {
    }

    template <typename T>
    NO_DISCARD constexpr T As() const noexcept
    {
      return static_cast<T>(_handle);
    }
  };

  template <typename>
  struct FuncPtrTraits;

  template <typename R, typename... Args>
  struct FuncPtrTraits<R (*)(Args...)>
  {
    using ReturnType = R;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t NumArgs = sizeof...(Args);
  };
}