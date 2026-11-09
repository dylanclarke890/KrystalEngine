/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
