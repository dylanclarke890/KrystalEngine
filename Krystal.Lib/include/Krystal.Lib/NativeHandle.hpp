#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"

namespace Krys
{
  class NativeHandle
  {
    void *_handle;

  public:
    constexpr NativeHandle() noexcept : _handle(nullptr)
    {
    }

    constexpr NativeHandle(void *handle) noexcept : _handle(handle)
    {
    }

    template <typename T>
    KRYS_NODISCARD constexpr T As() const noexcept
    {
      return static_cast<T>(_handle);
    }

    KRYS_NODISCARD constexpr bool IsValid() const noexcept
    {
      return _handle != nullptr;
    }
  };
}