/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Web/Css/Calc/Tree.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"

namespace krys::web::css::calc
{
  struct RandomCachingKey
  {
    using IdentifierVariant = Variant<Random::SharingOptions::Auto, CSSOMStringAtom>;

    IdentifierVariant Identifier;

    RandomCachingKey(IdentifierVariant identifier) noexcept : Identifier {krys::move(identifier)}
    {
    }

    KRYS_NODISCARD bool operator==(const RandomCachingKey &) const noexcept = default;
  };

  struct RandomCachingKeyHash
  {
    KRYS_NODISCARD static size_t Hash(const RandomCachingKey &key) noexcept
    {
      return krys::SwitchOn(
        key.Identifier,
        [&](const Random::SharingOptions::Auto &autoValue) -> size_t
        {
          return krys::CombineFastHashes(key.Identifier.index(), ToUnderlying(autoValue.Property),
                                         autoValue.Index);
        },
        [&](const CSSOMStringAtom &string) -> size_t
        { return krys::CombineFastHashes(key.Identifier.index(), string); });
    }

    // Lets `HashMap` use this as its hasher.
    KRYS_NODISCARD size_t operator()(const RandomCachingKey &key) const noexcept
    {
      return Hash(key);
    }
  };
}

namespace krys
{
  template <>
  struct FastHash<krys::web::css::calc::RandomCachingKey> : public krys::web::css::calc::RandomCachingKeyHash
  {
  };
}
