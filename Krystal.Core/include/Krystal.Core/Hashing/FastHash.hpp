#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Hashing/HashAlgorithm.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <bit>
#include <functional>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>

namespace krys
{
  template <typename T>
  struct FastHash;

  /// @brief The fast hash of each of @p values, folded into one, for hashing a type by its members.
  template <typename... Ts>
  KRYS_NODISCARD size_t CombineFastHashes(const Ts &...values) noexcept;

  namespace detail
  {
    template <typename T>
    concept HasFastHashMember = requires(const T &value) {
      { value.Hash() } -> ConvertibleTo<size_t>;
    };

    template <typename T, typename TChar>
    concept ViewableAs = requires(const T &value) { std::basic_string_view<TChar>(value); };

    template <typename T>
    concept StringLike = ViewableAs<T, char> || ViewableAs<T, char8_t> || ViewableAs<T, char16_t>
                         || ViewableAs<T, char32_t> || ViewableAs<T, wchar_t>;

    template <typename T>
    concept StdHashable = requires(const T &value) {
      { std::hash<T> {}(value) } -> ConvertibleTo<size_t>;
    };

    template <typename TChar>
    KRYS_NODISCARD KRYS_ALWAYS_INLINE uint64 HashCharacters(std::basic_string_view<TChar> view) noexcept
    {
      return FastHashAlgorithm::HashBytes(std::as_bytes(Span<const TChar>(view.data(), view.size())));
    }

    template <typename T>
    KRYS_NODISCARD KRYS_ALWAYS_INLINE uint64 HashString(const T &value) noexcept
    {
      if constexpr (ViewableAs<T, char>)
      {
        return HashCharacters(std::string_view(value));
      }
      else if constexpr (ViewableAs<T, char8_t>)
      {
        return HashCharacters(std::u8string_view(value));
      }
      else if constexpr (ViewableAs<T, char16_t>)
      {
        return HashCharacters(std::u16string_view(value));
      }
      else if constexpr (ViewableAs<T, char32_t>)
      {
        return HashCharacters(std::u32string_view(value));
      }
      else
      {
        return HashCharacters(std::wstring_view(value));
      }
    }
  }

  /// @brief The engine's fast in-memory hash of a `T`, for hash tables and caches, and the hasher `HashMap`,
  /// `HashSet` and `ListHashSet` use by default.
  ///
  /// It covers integers, enumerations, floating-point numbers, pointers, smart pointers (by address), strings
  /// and string views (by content), pairs, tuples, `Maybe` and `Variant`, types with a `size_t Hash() const`
  /// member, and types `std::hash` knows. Specialise it for other types, usually with `CombineFastHashes`
  /// over their members.
  /// @note It's built from `FastHashAlgorithm`, which may change between builds: never store its results.
  template <typename T>
  struct FastHash
  {
    /// Tells hash tables that the result is already well mixed, so they needn't mix it again.
    using is_avalanching = std::true_type;

    KRYS_NODISCARD size_t operator()(const T &value) const noexcept
    {
      if constexpr (detail::HasFastHashMember<T>)
      {
        // Mixed again, since a member may return something simple, such as an identifier.
        return static_cast<size_t>(FastHashAlgorithm::MixInteger(static_cast<uint64>(value.Hash())));
      }
      else if constexpr (IsEnum<T>)
      {
        return static_cast<size_t>(FastHashAlgorithm::MixInteger(static_cast<uint64>(ToUnderlying(value))));
      }
      else if constexpr (Integral<T>)
      {
        return static_cast<size_t>(FastHashAlgorithm::MixInteger(static_cast<uint64>(value)));
      }
      else if constexpr (FloatingPoint<T>)
      {
        // Zero and negative zero are equal, so they hash the same.
        if (value == T(0))
        {
          return static_cast<size_t>(FastHashAlgorithm::MixInteger(0));
        }

        if constexpr (sizeof(T) == sizeof(uint32))
        {
          return static_cast<size_t>(FastHashAlgorithm::MixInteger(std::bit_cast<uint32>(value)));
        }
        else
        {
          static_assert(sizeof(T) == sizeof(uint64), "FastHash covers 32- and 64-bit floating-point types.");
          return static_cast<size_t>(FastHashAlgorithm::MixInteger(std::bit_cast<uint64>(value)));
        }
      }
      else if constexpr (IsPointer<T>)
      {
        return static_cast<size_t>(FastHashAlgorithm::MixInteger(reinterpret_cast<uintptr_t>(value)));
      }
      else if constexpr (SameType<T, std::nullptr_t>)
      {
        return static_cast<size_t>(FastHashAlgorithm::MixInteger(0));
      }
      else if constexpr (IsSmartPtr<T>)
      {
        return FastHash<decltype(value.get())> {}(value.get());
      }
      else if constexpr (detail::StringLike<T>)
      {
        return static_cast<size_t>(detail::HashString(value));
      }
      else if constexpr (detail::StdHashable<T>)
      {
        return static_cast<size_t>(
          FastHashAlgorithm::MixInteger(static_cast<uint64>(std::hash<T> {}(value))));
      }
      else
      {
        static_assert(DependentFalse<T>,
                      "FastHash isn't implemented for this type: specialise it, or give the "
                      "type a `size_t Hash() const` member.");
        return 0;
      }
    }
  };

  template <typename T, typename U>
  struct FastHash<std::pair<T, U>>
  {
    using is_avalanching = std::true_type;

    KRYS_NODISCARD size_t operator()(const std::pair<T, U> &pair) const noexcept
    {
      return CombineFastHashes(pair.first, pair.second);
    }
  };

  template <typename... Ts>
  struct FastHash<std::tuple<Ts...>>
  {
    using is_avalanching = std::true_type;

    KRYS_NODISCARD size_t operator()(const std::tuple<Ts...> &tuple) const noexcept
    {
      return std::apply([](const auto &...elements) noexcept { return CombineFastHashes(elements...); },
                        tuple);
    }
  };

  template <typename T>
  struct FastHash<std::optional<T>>
  {
    using is_avalanching = std::true_type;

    KRYS_NODISCARD size_t operator()(const std::optional<T> &maybe) const noexcept
    {
      // An empty one hashes as `false`, so it can't share a hash with a held value as easily.
      return maybe ? CombineFastHashes(true, *maybe) : CombineFastHashes(false);
    }
  };

  template <typename... Ts>
  struct FastHash<std::variant<Ts...>>
  {
    using is_avalanching = std::true_type;

    KRYS_NODISCARD size_t operator()(const std::variant<Ts...> &variant) const noexcept
    {
      return std::visit([&](const auto &held) noexcept { return CombineFastHashes(variant.index(), held); },
                        variant);
    }
  };

  template <typename... Ts>
  KRYS_NODISCARD size_t CombineFastHashes(const Ts &...values) noexcept
  {
    uint64 hash = 0;
    ((hash = FastHashAlgorithm::Combine(hash, static_cast<uint64>(FastHash<Ts> {}(values)))), ...);
    return static_cast<size_t>(hash);
  }
}
