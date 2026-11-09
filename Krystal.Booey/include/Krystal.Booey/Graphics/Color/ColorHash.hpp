/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Utils/Tags.hpp"

namespace krys
{
  template <>
  struct HashTraits<krys::boo::Color> : public GenericHashTraits<krys::boo::Color>
  {
    constexpr static bool HasIsEmptyValueFunction = true;
    constexpr static bool EmptyValueIsZero = false;

    KRYS_NODISCARD static krys::boo::Color EmptyValue() noexcept
    {
      return krys::boo::Color(HashTableEmptyValueTag {});
    }

    KRYS_NODISCARD static bool IsEmptyValue(const krys::boo::Color &value) noexcept
    {
      return value.IsHashTableEmptyValue();
    }

    static void ConstructDeletedValue(krys::boo::Color &slot) noexcept
    {
      new (NotNullTag {}, &slot) krys::boo::Color(HashTableDeletedValueTag {});
    }

    KRYS_NODISCARD static bool IsDeletedValue(const krys::boo::Color &value) noexcept
    {
      return value.IsHashTableDeletedValue();
    }
  };
}
