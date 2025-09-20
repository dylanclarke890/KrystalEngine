#pragma once

#include "Krystal.Lib/Attributes.hpp"

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
    NO_DISCARD constexpr T As() const noexcept
    {
      return static_cast<T>(_handle);
    }
  };
}