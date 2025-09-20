#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Krys
{
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
  using Span = std::span<T>;

  template <typename T>
  using Func = std::function<T>;

  template <typename T, typename ErrorType = string>
  using Expected = std::expected<T, ErrorType>;

  template <typename T>
  using Unexpected = std::unexpected<T>;

  template <bool predicate, typename T, typename U>
  using ConditionalType = std::conditional_t<predicate, T, U>;
}