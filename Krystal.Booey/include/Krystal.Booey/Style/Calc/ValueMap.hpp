#pragma once

#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::style::calc
{
  /// @brief Maps integer handles to calculation values, so that a calculated length fits in the 32 bits of a
  /// float. Each handle carries its own reference count.
  class ValueMap
  {
    friend class NeverDestroyed<ValueMap>;

  private:
    struct Entry
    {
      uint64 ReferenceCountMinusOne {0};
      RefPtr<Value> Value;

      Entry() noexcept = default;
      explicit Entry(Ref<calc::Value> &&value) noexcept;
    };

    uint32 _nextAvailableHandle {1};
    HashMap<uint32, Entry> _map;

  public:
    KRYS_NODISCARD static ValueMap &CalculationValues() noexcept;

    KRYS_NODISCARD uint32 Insert(Ref<Value> &&) noexcept;
    void AddRef(uint32 handle) noexcept;
    void SubRef(uint32 handle) noexcept;

    KRYS_NODISCARD Value &Get(uint32 handle) const noexcept;

  private:
    ValueMap() noexcept;

    // Handles 0 and the maximum value are reserved, matching a hash table's empty and deleted keys.
    KRYS_NODISCARD constexpr static bool IsValidHandle(uint32 handle) noexcept
    {
      return handle != 0 && handle != Limits<uint32>::max();
    }
  };

  inline ValueMap::Entry::Entry(Ref<calc::Value> &&value) noexcept : Value(krys::move(value))
  {
  }

  inline uint32 ValueMap::Insert(Ref<Value> &&value) noexcept
  {
    krys_debug_assert(_nextAvailableHandle);

    Entry entry(krys::move(value));

    // FIXME: This monotonically increasing handle generation scheme is potentially wasteful
    // of the handle space. Consider reusing empty handles. https://bugs.webkit.org/show_bug.cgi?id=80489
    while (!IsValidHandle(_nextAvailableHandle) || !_map.try_emplace(_nextAvailableHandle, entry).second)
    {
      ++_nextAvailableHandle;
    }

    return _nextAvailableHandle++;
  }

  inline Value &ValueMap::Get(uint32 handle) const noexcept
  {
    krys_debug_assert(_map.contains(handle));

    return *_map.find(handle)->second.Value;
  }

  inline void ValueMap::AddRef(uint32 handle) noexcept
  {
    krys_debug_assert(_map.contains(handle));

    ++_map.find(handle)->second.ReferenceCountMinusOne;
  }

  inline void ValueMap::SubRef(uint32 handle) noexcept
  {
    krys_debug_assert(_map.contains(handle));

    auto it = _map.find(handle);
    if (it->second.ReferenceCountMinusOne)
    {
      --it->second.ReferenceCountMinusOne;
      return;
    }

    _map.erase(it);
  }
}
