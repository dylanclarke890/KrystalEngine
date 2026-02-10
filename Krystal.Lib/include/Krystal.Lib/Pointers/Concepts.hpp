#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"

namespace Krys
{
  template <typename T>
  concept IntrusiveRefCounted = requires(T &t) {
    { t.AddRef() } noexcept;
    { t.SubRef() } noexcept;
  };

  template <typename T>
  concept ThreadSafeIntrusiveRefCounted = requires(T &t) {
    { t.AddRef() } noexcept;
    { t.SubRef() } noexcept;
    { t.AddRefWeak() } noexcept;
    { t.SubRefWeak() } noexcept;
  };
}