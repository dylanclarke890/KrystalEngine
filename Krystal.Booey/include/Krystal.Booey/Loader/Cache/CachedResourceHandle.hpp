#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class CachedResource;

  /// @brief Keeps a cached resource alive: a resource lives while a handle or the memory cache holds it.
  template <typename R>
  class CachedResourceHandle
  {
    template <typename>
    friend class CachedResourceHandle;

  private:
    RefPtr<R> _resource;

  public:
    CachedResourceHandle() noexcept = default;
    CachedResourceHandle(std::nullptr_t) noexcept
    {
    }
    CachedResourceHandle(R &resource) noexcept : _resource(ShareRefPtr(&resource))
    {
    }
    CachedResourceHandle(R *resource) noexcept : _resource(ShareRefPtr(resource))
    {
    }
    template <typename U>
    requires std::is_convertible_v<U *, R *>
    CachedResourceHandle(const CachedResourceHandle<U> &other) noexcept : _resource(other._resource)
    {
    }

    KRYS_NODISCARD R *get() const noexcept
    {
      return _resource.get();
    }
    R *operator->() const noexcept
    {
      return get();
    }
    R &operator*() const noexcept
    {
      return *get();
    }
    explicit operator bool() const noexcept
    {
      return static_cast<bool>(_resource);
    }

    CachedResourceHandle &operator=(R *resource) noexcept
    {
      _resource = ShareRefPtr(resource);
      return *this;
    }

    template <typename U>
    KRYS_NODISCARD bool operator==(const CachedResourceHandle<U> &other) const noexcept
    {
      return static_cast<const void *>(get()) == static_cast<const void *>(other.get());
    }
    KRYS_NODISCARD bool operator==(const R *resource) const noexcept
    {
      return get() == resource;
    }
  };
}
