#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <utility>

namespace krys::boo
{
  class RenderObject;

  /// @brief Destroys a renderer (`RenderObject::Destroy`) instead of deleting it.
  struct RenderObjectDeleter
  {
    void operator()(RenderObject *renderer) const noexcept;
  };

  /// @brief Owns a renderer that isn't in the render tree.
  template <typename T>
  using RenderPtr = UniquePtr<T, RenderObjectDeleter>;

  template <typename T, typename... Args>
  KRYS_NODISCARD inline RenderPtr<T> CreateRenderer(Args &&...args) noexcept
  {
    return RenderPtr<T>(new T(std::forward<Args>(args)...));
  }
}
