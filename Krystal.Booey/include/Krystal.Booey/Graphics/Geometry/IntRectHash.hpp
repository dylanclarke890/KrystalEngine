/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Core/Hash.hpp"

namespace krys
{
  template <>
  struct DefaultHash<boo::IntRect>
  {
    KRYS_NODISCARD static size_t Hash(const boo::IntRect &key) noexcept
    {
      return krys::Hash::Combine(key.X(), key.Y(), key.Width(), key.Height());
    }

    KRYS_NODISCARD static bool Equal(const boo::IntRect &a, const boo::IntRect &b) noexcept
    {
      return a == b;
    }

    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;
  };

  template <>
  struct HashTraits<boo::IntRect> : public GenericHashTraits<boo::IntRect>
  {
    constexpr static bool EmptyValueIsZero = true;

    constexpr static void ConstructDeletedValue(boo::IntRect &slot) noexcept
    {
      slot = boo::IntRect(-1, -1, -1, -1);
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(const boo::IntRect &value) noexcept
    {
      return value.X() == -1 && value.Y() == -1 && value.Width() == -1 && value.Height() == -1;
    }
  };
}
