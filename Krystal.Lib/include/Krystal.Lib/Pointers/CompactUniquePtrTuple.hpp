#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/CompactPtr.hpp"
#include "Krystal.Lib/Pointers/CompactPtrTuple.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include <memory>

namespace Krys
{
  template <typename T, typename TData, typename Deleter = std::default_delete<T>>
  class CompactUniquePtrTuple;

  template <typename T, typename TData, typename... Args>
  KRYS_NODISCARD constexpr CompactUniquePtrTuple<T, TData> CreateCompactUniquePtr(Args &&...args) noexcept
  {
    return CompactUniquePtrTuple<T, TData>(CreateUnique<T>(std::forward<Args>(args)...));
  }

  template <typename T, typename TData, typename Deleter, typename... Args>
  KRYS_NODISCARD constexpr CompactUniquePtrTuple<T, TData, Deleter>
    CreateCompactUniquePtr(Args &&...args) noexcept
  {
    return CompactUniquePtrTuple<T, TData, Deleter>(CreateUnique<T>(std::forward<Args>(args)...));
  }

  template <typename T, typename TData, typename Deleter>
  class CompactUniquePtrTuple final : NonCopyable<CompactUniquePtrTuple>
  {
    static_assert(Krys::AllowsCompactPointers<T>());

    template <typename U, typename E, typename... Args>
    friend constexpr CompactUniquePtrTuple<U, E> CreateCompactUniquePtr(Args &&...args) noexcept;

    template <typename U, typename E, typename D, typename... Args>
    friend constexpr CompactUniquePtrTuple<U, E, D> CreateCompactUniquePtr(Args &&...args) noexcept;

    template <typename, typename, typename>
    friend class CompactUniquePtrTuple;

  private:
    CompactPtrTuple<RawPtr<T>, TData> _data;

  public:
    constexpr CompactUniquePtrTuple() noexcept = default;

    template <typename U, typename UDeleter>
    requires(SameType<UDeleter, Deleter> || SameType<UDeleter, std::default_delete<U>>)
    constexpr CompactUniquePtrTuple(CompactUniquePtrTuple<U, TData, UDeleter> &&other) noexcept
        : _data {std::exchange(other._data, {})}
    {
    }

    constexpr ~CompactUniquePtrTuple() noexcept
    {
      SetPtr(nullptr);
    }

    template <typename U, typename UDeleter>
    requires(SameType<UDeleter, Deleter> || SameType<UDeleter, std::default_delete<U>>)
    CompactUniquePtrTuple<T, TData, Deleter> &
      operator=(CompactUniquePtrTuple<U, TData, UDeleter> &&other) noexcept
    {
      CompactUniquePtrTuple moved {Krys::Move(other)};
      std::swap(_data, moved._data);
      return *this;
    }

    KRYS_NODISCARD RawPtr<T> Ptr() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.Ptr();
    }

    KRYS_NODISCARD UniquePtr<T, Deleter> MoveToUniquePtr() noexcept
    {
      RawPtr<T> ptr = _data.Ptr();
      _data.SetPtr(nullptr);

      return UniquePtr<T, Deleter>(ptr);
    }

    void SetPtr(std::nullptr_t) noexcept
    {
      DeletePtr();
      _data.SetPtr(nullptr);
    }

    template <typename U, typename UDeleter>
    requires(SameType<UDeleter, Deleter> || SameType<UDeleter, std::default_delete<U>>)
    void SetPtr(UniquePtr<U, UDeleter> &&ptr) noexcept
    {
      DeletePtr();
      _data.SetPtr(ptr.release());
    }

    KRYS_NODISCARD TData Data() const noexcept
    {
      return _data.Data();
    }

    void SetData(TData data) noexcept
    {
      _data.SetData(data);
    }

  private:
    constexpr CompactUniquePtrTuple(std::unique_ptr<T> &&ptr) noexcept
    {
      _data.SetPtr(ptr.release());
    }

    constexpr void DeletePtr() noexcept
    {
      if (RawPtr<T> ptr = _data.Ptr())
      {
        Deleter()(ptr);
      }
    }
  };
}