#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CompactPtrTuple.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys
{
  template <typename T>
  using CompactRefPtr = RefPtr<T, CompactPtrTraits<T>>;

  template <typename T, typename TData>
  class CompactRefPtrTuple final
  {
    static_assert(krys::AllowsCompactPointers<T>());

  private:
    CompactPtrTuple<T *, TData> _data;

  public:
    constexpr CompactRefPtrTuple() noexcept = default;

    constexpr CompactRefPtrTuple(T *ptr, TData data) noexcept
    {
      SetPtr(ptr);
      SetData(data);
    }

    constexpr CompactRefPtrTuple(RefPtr<T> &&ptr, TData data) noexcept
    {
      SetPtr(krys::move(ptr));
      SetData(data);
    }

    constexpr ~CompactRefPtrTuple() noexcept
    {
      RefPtr<T>::ref_policy::SubRef(_data.Ptr());
    }

    constexpr CompactRefPtrTuple(const CompactRefPtrTuple &other) noexcept
    {
      SetPtr(other.Ptr());
      SetData(other.Data());
    }

    constexpr CompactRefPtrTuple(CompactRefPtrTuple &&other) noexcept
    {
      _data.SetPtr(other.Ptr());
      _data.SetData(other.Data());
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
      CompactRefPtrTuple moved(krys::move(other));
      swap(moved);
      return *this;
    }

    KRYS_NODISCARD constexpr T *Ptr() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.Ptr();
    }

    constexpr void SetPtr(T *ptr) noexcept
    {
      auto *old = _data.Ptr();
      _data.SetPtr(RefPtr<T>::ref_policy::AddRef(ptr));
      RefPtr<T>::ref_policy::SubRef(old);
    }

    constexpr void SetPtr(RefPtr<T> &&ptr) noexcept
    {
      auto *old = _data.Ptr();
      _data.SetPtr(ptr.release());
      RefPtr<T>::ref_policy::SubRef(old);
    }

    constexpr void SetPtr(Ref<T> &&ptr) noexcept
    {
      auto *old = _data.Ptr();
      _data.SetPtr(&ptr.release());
      RefPtr<T>::ref_policy::SubRef(old);
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
