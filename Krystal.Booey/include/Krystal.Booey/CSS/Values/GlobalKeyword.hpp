/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  enum class GlobalKeyword : uint8
  {
    Initial,
    Inherit,
    Unset,
    Revert,
    RevertLayer
  };

  KRYS_NODISCARD constexpr bool IsGlobalKeyword(ValueId valueId) noexcept
  {
    switch (valueId)
    {
      case ValueId::Initial:
      case ValueId::Inherit:
      case ValueId::Unset:
      case ValueId::Revert:
      case ValueId::RevertLayer:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  KRYS_NODISCARD constexpr Maybe<GlobalKeyword> ParseGlobalKeyword(ValueId valueId)
  {
    switch (valueId)
    {
      case ValueId::Initial:     return GlobalKeyword::Initial;
      case ValueId::Inherit:     return GlobalKeyword::Inherit;
      case ValueId::Unset:       return GlobalKeyword::Unset;
      case ValueId::Revert:      return GlobalKeyword::Revert;
      case ValueId::RevertLayer: return GlobalKeyword::RevertLayer;
      default:                   return {};
    }
  }

  KRYS_NODISCARD constexpr ValueId ToValueId(GlobalKeyword keyword) noexcept
  {
    switch (keyword)
    {
      case GlobalKeyword::Initial:     return ValueId::Initial;
      case GlobalKeyword::Inherit:     return ValueId::Inherit;
      case GlobalKeyword::Unset:       return ValueId::Unset;
      case GlobalKeyword::Revert:      return ValueId::Revert;
      case GlobalKeyword::RevertLayer: return ValueId::RevertLayer;
    }

    krys_unreachable();
  }
}