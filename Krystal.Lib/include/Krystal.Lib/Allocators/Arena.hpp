#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <new>
#include <utility>

namespace Krys
{
  /// @brief A memory arena for fast allocations for a particular type.
  /// @tparam T The type of object to allocate.
  template <typename T>
  class Arena : NonCopyable<Page>
  {
    constexpr static size_t Alignment = alignof(T);

  private:
    List<byte> _memoryPool;
    size_t _offset = 0;
    size_t _constructedCount = 0;

  public:
    Arena(size_t initialCapacity = 32_KB)
    {
      _memoryPool.reserve(initialCapacity);
    }

    ~Arena()
    {
      if constexpr (!std::is_trivially_destructible_v<T>)
      {
        byte *ptr = _memoryPool.data();
        for (size_t i = 0; i < _constructedCount; ++i)
        {
          reinterpret_cast<T *>(ptr)->~T();
          ptr += ByteUtils::AlignNext(sizeof(T), Alignment);
        }
      }
    }

    MOVE_SWAP(Arena)

    KRYS_NODISCARD T &Emplace(Args &&...args)
    {
      // Ensure there’s enough room
      size_t alignedOffset = ByteUtils::AlignNext(_offset, Alignment);
      size_t requiredSize = alignedOffset + sizeof(T);
      if (requiredSize > _memoryPool.size())
      {
        _memoryPool.resize(requiredSize * 2); // grow exponentially
      }

      // Construct object in place
      void *ptr = _memoryPool.data() + alignedOffset;
      T *obj = new (ptr) T(std::forward<Args>(args)...);
      _offset = alignedOffset + sizeof(T);
      _constructedCount++;
      return *obj;
    }

    KRYS_NODISCARD size_t SizeInBytes() const noexcept
    {
      return _memoryPool.size();
    }

    void Clear() noexcept
    {
      if constexpr (!std::is_trivially_destructible_v<T>)
      {
        byte *ptr = _memoryPool.data();
        for (size_t i = 0; i < _constructedCount; ++i)
        {
          reinterpret_cast<T *>(ptr)->~T();
          ptr += ByteUtils::AlignNext(sizeof(T), Alignment);
        }
      }
      _offset = 0;
      _constructedCount = 0;
    }

    void Swap(Arena &other) noexcept
    {
      _memoryPool.swap(other._memoryPool);
      std::swap(_offset, other._offset);
      std::swap(_constructedCount, other._constructedCount);
    }
  };
}