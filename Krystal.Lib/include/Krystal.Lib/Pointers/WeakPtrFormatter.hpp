#pragma once

#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include <format>

namespace std
{
  template <typename T, typename PtrTraits, typename RefPolicy, ::Krys::IsNullable Nullable, typename TChar>
  struct formatter<::Krys::detail::IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable>, TChar>
      : public formatter<void *, TChar>
  {
    template <typename FormatContext>
    auto format(const ::Krys::detail::IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &ptr,
                FormatContext &ctx) const -> decltype(ctx.out())
    {
      return formatter<void *, TChar>::format(ptr.get(), ctx);
    }
  };
}