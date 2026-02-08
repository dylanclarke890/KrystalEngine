#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/CompactPtr.hpp"
#include "Krystal.Lib/Pointers/CompactPtrTuple.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"

namespace Krys
{
  template <typename T, typename TData>
  class CompactRefPtrTuple final
  {
    static_assert(Krys::AllowsCompactPointers<T>());

  private:
    CompactPtrTuple<RawPtr<T>, TData> _data;

  public:
    constexpr CompactRefPtrTuple() noexcept = default;

    constexpr CompactRefPtrTuple(RawPtr<T> ptr, TData data) noexcept
    {
      SetPtr(ptr);
      SetData(data);
    }

    constexpr CompactRefPtrTuple(RefPtr<T> &&ptr, TData data) noexcept
    {
      SetPtr(Krys::Move(ptr));
      SetData(data);
    }

    constexpr ~CompactRefPtrTuple() noexcept
    {
      Krys::DefaultRefDerefTraits<T>::derefIfNotNull(_data.Ptr());
    }

    constexpr CompactRefPtrTuple(const CompactRefPtrTuple &other) noexcept
    {
      SetPtr(other.Ptr());
      SetData(other.data());
    }

    constexpr CompactRefPtrTuple(CompactRefPtrTuple &&other) noexcept
    {
      _data.SetPtr(other.Ptr());
      _data.SetData(other.data());
      other._data.SetPtr(nullptr);
      other._data.SetData({});
    }

    constexpr CompactRefPtrTuple &operator=(const CompactRefPtrTuple &other) noexcept
    {
      CompactRefPtrTuple copied(other);
      swap(copied);
      return *this;
    }

    constexpr CompactRefPtrTuple &operator=(CompactRefPtrTuple &&other) noexcept
    {
      CompactRefPtrTuple moved(Krys::Move(other));
      swap(moved);
      return *this;
    }

    KRYS_NODISCARD constexpr RawPtr<T> Ptr() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.Ptr();
    }

    constexpr void SetPtr(RawPtr<T> ptr) noexcept
    {
      auto *old = _data.Ptr();
      _data.SetPtr(Krys::DefaultRefDerefTraits<T>::refIfNotNull(ptr));
      Krys::DefaultRefDerefTraits<T>::derefIfNotNull(old);
    }

    constexpr void SetPtr(RefPtr<T> &&ptr) noexcept
    {
      auto willRelease = Krys::Move(ptr);
      auto *old = _data.Ptr();
      _data.SetPtr(willRelease.leakRef());
      Krys::DefaultRefDerefTraits<T>::derefIfNotNull(old);
    }

    constexpr void SetPtr(Ref<T> &&ptr) noexcept
    {
      auto willRelease = Krys::Move(ptr);
      auto *old = _data.Ptr();
      _data.SetPtr(&willRelease.leakRef());
      Krys::DefaultRefDerefTraits<T>::derefIfNotNull(old);
    }

    KRYS_NODISCARD constexpr TData Data() const noexcept
    {
      return _data.Data();
    }

    constexpr void SetData(TData data) noexcept
    {
      _data.SetData(data);
    }

    constexpr void swap(CompactRefPtrTuple<T, TData> &other) noexcept
    {
      _data.swap(other._data);
    }
  };
}
