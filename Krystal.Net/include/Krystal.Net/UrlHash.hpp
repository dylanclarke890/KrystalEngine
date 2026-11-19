/*
 * Copyright (C) 2008-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Utils/ForbidHeapAllocation.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include "Krystal.Net/Url.hpp"
#include <memory>

namespace krys
{
  template <>
  struct DefaultHash<net::Url>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = false;

    KRYS_NODISCARD static size_t Hash(const net::Url &key) noexcept
    {
      return std::hash<utf8_string> {}(key.String());
    }

    KRYS_NODISCARD static bool Equal(const net::Url &a, const net::Url &b) noexcept
    {
      return a.String() == b.String();
    }
  };

  template <>
  struct HashTraits<net::Url> : public GenericHashTraits<net::Url>
  {
    static void ConstructDeletedValue(net::Url &slot) noexcept
    {
      new (NotNullTag {}, std::addressof(slot)) net::Url(HashTableDeletedValueTag {});
    }

    KRYS_NODISCARD static bool IsDeletedValue(const net::Url &value) noexcept
    {
      return value.IsHashTableDeletedValue();
    }
  };
}
