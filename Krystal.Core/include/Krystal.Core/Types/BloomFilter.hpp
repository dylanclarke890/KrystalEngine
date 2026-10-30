#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <array>
#include <limits>

namespace krys
{
  /// @brief Bloom filter with k=2. Uses 2^TKeyBits/8 bytes of memory.
  /// @note False positive rate is approximately (1-e^(-2n/m))^2, where n is the number of unique keys and m
  /// is the table size (==2^TKeyBits).
  /// @see http://en.wikipedia.org/wiki/Bloom_filter
  template <uint32 TKeyBits>
  class BloomFilter
  {
  public:
    static constexpr size_t TableSize = size_t {1} << TKeyBits;

  private:
    static constexpr uint32 BitsPerPosition = 8 * sizeof(uint32);
    static constexpr uint32 KeyMask = (uint32 {1} << TKeyBits) - 1;

    std::array<uint32, TableSize / BitsPerPosition> _bitArray {};

  public:
    constexpr BloomFilter() noexcept = default;

    constexpr void Add(uint32 hash) noexcept
    {
      SetBit(hash);
      SetBit(hash >> 16);
    }

    constexpr void Add(const BloomFilter &other) noexcept
    {
      for (size_t i = 0; i < _bitArray.size(); ++i)
      {
        _bitArray[i] |= other._bitArray[i];
      }
    }

    /// @brief Whether @p hash may have been added.
    /// @note The filter may give false positives (claim it may contain a key it doesn't) but never false
    /// negatives (claim it doesn't contain a key it does).
    KRYS_NODISCARD constexpr bool MayContain(uint32 hash) const noexcept
    {
      // The top and bottom bits of the incoming hash are treated as independent bloom filter hash functions.
      // This works well as long as the filter size is not much above 2^16.
      return IsBitSet(hash) && IsBitSet(hash >> 16);
    }

    constexpr void Clear() noexcept
    {
      _bitArray.fill(0);
    }

  private:
    KRYS_NODISCARD constexpr static uint32 ArrayIndex(uint32 key) noexcept
    {
      return key / BitsPerPosition;
    }

    KRYS_NODISCARD constexpr static uint32 BitMask(uint32 key) noexcept
    {
      return uint32 {1} << (key % BitsPerPosition);
    }

    KRYS_NODISCARD constexpr bool IsBitSet(uint32 key) const noexcept
    {
      const uint32 maskedKey = key & KeyMask;
      krys_debug_assert(ArrayIndex(maskedKey) < _bitArray.size());
      return _bitArray[ArrayIndex(maskedKey)] & BitMask(maskedKey);
    }

    constexpr void SetBit(uint32 key) noexcept
    {
      const uint32 maskedKey = key & KeyMask;
      krys_debug_assert(ArrayIndex(maskedKey) < _bitArray.size());
      _bitArray[ArrayIndex(maskedKey)] |= BitMask(maskedKey);
    }
  };

  /// @brief Counting bloom filter with 8 bit counters. Uses 2^TKeyBits bytes of memory. Error rates as
  /// `BloomFilter`.
  /// @see http://en.wikipedia.org/wiki/Bloom_filter#Counting_filters
  template <uint32 TKeyBits>
  class CountingBloomFilter
  {
  public:
    static constexpr size_t TableSize = size_t {1} << TKeyBits;

  private:
    static constexpr uint32 KeyMask = (uint32 {1} << TKeyBits) - 1;

    std::array<uint8, TableSize> _buckets {};

  public:
    constexpr CountingBloomFilter() noexcept = default;

    KRYS_NODISCARD constexpr static uint32 MaximumCount() noexcept
    {
      return std::numeric_limits<uint8>::max();
    }

    constexpr void Add(uint32 hash) noexcept
    {
      auto &first = FirstBucket(hash);
      auto &second = SecondBucket(hash);
      if (first < MaximumCount()) KRYS_LIKELY
      {
        ++first;
      }
      if (second < MaximumCount()) KRYS_LIKELY
      {
        ++second;
      }
    }

    constexpr void Remove(uint32 hash) noexcept
    {
      auto &first = FirstBucket(hash);
      auto &second = SecondBucket(hash);
      krys_debug_assert(first);
      krys_debug_assert(second);
      // In case of an overflow, the bucket sticks in the table until Clear().
      if (first < MaximumCount()) KRYS_LIKELY
      {
        --first;
      }
      if (second < MaximumCount()) KRYS_LIKELY
      {
        --second;
      }
    }

    /// @brief Whether @p hash may have been added.
    /// @note The filter may give false positives (claim it may contain a key it doesn't) but never false
    /// negatives (claim it doesn't contain a key it does).
    KRYS_NODISCARD constexpr bool MayContain(uint32 hash) const noexcept
    {
      return FirstBucket(hash) && SecondBucket(hash);
    }

    /// @brief Empties the filter.
    /// @attention The filter must be cleared before reuse even if all keys are removed. Otherwise overflowed
    /// keys will stick around.
    constexpr void Clear() noexcept
    {
      _buckets.fill(0);
    }

    /// @brief Whether every bucket is empty or overflowed. Slow.
    KRYS_NODISCARD constexpr bool LikelyEmpty() const noexcept
    {
      for (auto bucket : _buckets)
      {
        if (bucket && bucket != MaximumCount())
        {
          return false;
        }
      }
      return true;
    }

    /// @brief Whether every bucket is empty. Slow.
    KRYS_NODISCARD constexpr bool IsClear() const noexcept
    {
      for (auto bucket : _buckets)
      {
        if (bucket)
        {
          return false;
        }
      }
      return true;
    }

  private:
    KRYS_NODISCARD constexpr uint8 &FirstBucket(uint32 hash) noexcept
    {
      return _buckets[hash & KeyMask];
    }

    KRYS_NODISCARD constexpr uint8 &SecondBucket(uint32 hash) noexcept
    {
      return _buckets[(hash >> 16) & KeyMask];
    }

    KRYS_NODISCARD constexpr const uint8 &FirstBucket(uint32 hash) const noexcept
    {
      return _buckets[hash & KeyMask];
    }

    KRYS_NODISCARD constexpr const uint8 &SecondBucket(uint32 hash) const noexcept
    {
      return _buckets[(hash >> 16) & KeyMask];
    }
  };
}
