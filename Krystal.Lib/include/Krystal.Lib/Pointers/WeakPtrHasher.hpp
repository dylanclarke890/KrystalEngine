#pragma once

#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include <utility>

namespace std
{
  template <typename T, typename Impl, typename PtrTraits, ::Krys::IsNullable Nullable>
  struct hash<Krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, Nullable>>
  {
    size_t operator()(const Krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, Nullable> &ptr) const noexcept
    {
      return hash<::Krys::RawPtr<T>>()(ptr.get());
    }
  };
}