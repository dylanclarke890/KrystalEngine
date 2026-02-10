#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCountDebugger.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  class KRYS_EMPTY_BASE_CLASS RefCountedWithInlineWeakPtrBase
  {
    using Debugger = RefCountDebugger<uint32>;

  private:
    mutable uint32 _strongCount {1};
    mutable uint32 _weakCount {1}; // The strong counts collectively share one weak count.
    KRYS_NO_UNIQUE_ADDRESS Debugger _refCountDebugger;

  protected:
    constexpr RefCountedWithInlineWeakPtrBase() noexcept = default;

    ~RefCountedWithInlineWeakPtrBase() noexcept
    {
      _refCountDebugger.WillDestroy(_strongCount);
      _strongCount = 0;
    }

  public:
    void AddRef() const noexcept
    {
      _refCountDebugger.WillAddRef(_strongCount);
      ++_strongCount;
    }

    void AddRefWeak() const noexcept
    {
      ++_weakCount;
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      return _strongCount == 1;
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _strongCount;
    }

    KRYS_NODISCARD Debugger &GetRefCountDebugger() const noexcept
    {
      return const_cast<Debugger &>(_refCountDebugger);
    }

  protected:
    // Returns true if the pointer should be destroyed.
    KRYS_NODISCARD bool SubRefBase() const noexcept
    {
      _refCountDebugger.WillSubRef(_strongCount);

      if (_strongCount != 1)
      {
        --_strongCount;
        return false;
      }

      _refCountDebugger.MarkDeletionHasBegun();
      return true;
    }

    // Returns true if the pointer should be freed.
    KRYS_NODISCARD bool SubRefWeakBase() const noexcept
    {
      if (_weakCount != 1)
      {
        --_weakCount;
        return false;
      }

      return true;
    }
  };

  template <typename T>
  class RefCountedWithInlineWeakPtr : public RefCountedWithInlineWeakPtrBase,
                                      public NonCopyable<RefCountedWithInlineWeakPtr>
  {
  public:
    constexpr RefCountedWithInlineWeakPtr() noexcept = default;

    void SubRef() const noexcept
    {
      if (!SubRefBase()) KRYS_LIKELY
      {
        return;
      }

      T::operator delete(const_cast<T *>(static_cast<const T *>(this)));
    }

    void SubRefWeak() const noexcept
    {
      if (!SubRefWeakBase()) KRYS_LIKELY
      {
        return;
      }

      const_cast<T *>(static_cast<const T *>(this))->~T();
      SubRefWeak();
    }
  };
}
