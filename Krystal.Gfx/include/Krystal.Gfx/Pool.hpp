#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::gfx
{
  /// @brief Generational storage for one resource type. A removed slot is reused with the next generation, so
  /// handles to the old resource stop resolving. Nothing throws: stale or invalid handles resolve to null.
  template <typename T, typename THandle>
  class Pool
  {
    struct Slot
    {
      Maybe<T> Value {};
      uint8 Generation {0u};
    };

  private:
    List<Slot> _slots {};
    List<uint32> _freeSlots {};
    uint32 _count {0u};

  public:
    /// @brief Adds a value and returns its handle.
    KRYS_NODISCARD THandle Add(T &&value) noexcept
    {
      uint32 index;
      if (!_freeSlots.empty())
      {
        index = _freeSlots.back();
        _freeSlots.pop_back();
      }
      else
      {
        krys_assert_msg(_slots.size() <= THandle::MaxIndex, "Pool is full.");
        index = static_cast<uint32>(_slots.size());
        _slots.emplace_back();
      }

      Slot &slot = _slots[index];
      slot.Value.emplace(krys::move(value));
      ++_count;
      return THandle::Create(index, slot.Generation);
    }

    /// @brief Returns the value for a handle, or null if the handle is invalid or its value was removed.
    KRYS_NODISCARD T *TryGet(THandle handle) noexcept
    {
      if (!handle.IsValid() || handle.Index() >= _slots.size())
      {
        return nullptr;
      }

      Slot &slot = _slots[handle.Index()];
      if (slot.Generation != handle.Generation() || !slot.Value.has_value())
      {
        return nullptr;
      }
      return &*slot.Value;
    }

    KRYS_NODISCARD const T *TryGet(THandle handle) const noexcept
    {
      return const_cast<Pool *>(this)->TryGet(handle);
    }

    /// @brief Returns the value for a handle that must be live.
    KRYS_NODISCARD T &Get(THandle handle) noexcept
    {
      T *value = TryGet(handle);
      krys_assert_msg(value != nullptr, "Stale or invalid handle.");
      return *value;
    }

    KRYS_NODISCARD const T &Get(THandle handle) const noexcept
    {
      return const_cast<Pool *>(this)->Get(handle);
    }

    /// @brief Whether the handle refers to a live value.
    KRYS_NODISCARD bool Contains(THandle handle) const noexcept
    {
      return TryGet(handle) != nullptr;
    }

    /// @brief Removes the value for a handle and hands it back, or returns nothing if the handle isn't live.
    KRYS_NODISCARD Maybe<T> Remove(THandle handle) noexcept
    {
      if (TryGet(handle) == nullptr)
      {
        return {};
      }

      Slot &slot = _slots[handle.Index()];
      Maybe<T> value = krys::move(slot.Value);
      slot.Value.reset();
      ++slot.Generation;
      _freeSlots.push_back(handle.Index());
      --_count;
      return value;
    }

    /// @brief The number of live values.
    KRYS_NODISCARD uint32 Count() const noexcept
    {
      return _count;
    }

    /// @brief Calls `func(handle, value)` for every live value.
    template <typename TFunc>
    void ForEach(TFunc &&func) noexcept
    {
      for (uint32 index = 0u; index < _slots.size(); ++index)
      {
        Slot &slot = _slots[index];
        if (slot.Value.has_value())
        {
          func(THandle::Create(index, slot.Generation), *slot.Value);
        }
      }
    }

    /// @brief Removes every value.
    void Clear() noexcept
    {
      for (uint32 index = 0u; index < _slots.size(); ++index)
      {
        Slot &slot = _slots[index];
        if (slot.Value.has_value())
        {
          slot.Value.reset();
          ++slot.Generation;
          _freeSlots.push_back(index);
        }
      }
      _count = 0u;
    }
  };
}
