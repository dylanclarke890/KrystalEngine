#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include <utility>

namespace std
{
  template <typename T, typename PtrTraits, typename RefPolicy, ::Krys::IsNullable Nullable, typename TChar>
  struct formatter<::Krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>, TChar>
      : public formatter<void *, TChar>
  {
    template <typename FormatContext>
    auto format(const ::Krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &ptr, FormatContext &ctx) const
      -> decltype(ctx.out())
    {
      return formatter<void *, TChar>::format(ptr.get(), ctx);
    }
  };

  template <typename T, typename PtrTraits, typename RefPolicy, ::Krys::IsNullable Nullable>
  struct hash<::Krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>>
  {
    constexpr size_t
      operator()(const ::Krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &ptr) const noexcept
    {
      return std::hash<::Krys::RawPtr<T>>()(ptr.get());
    }
  };
}
