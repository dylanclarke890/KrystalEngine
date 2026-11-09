/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::style
{
  /// @brief Identifies a pseudo-element that has its own style.
  struct PseudoElementIdentifier
  {
    PseudoElementType Type;

    /// @brief The highlight name, for `::highlight`.
    css::CSSOMStringAtom NameArgument {css::CSSOMStringAtom::Null()};

    KRYS_NODISCARD friend bool operator==(const PseudoElementIdentifier &,
                                          const PseudoElementIdentifier &) noexcept = default;
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::style::PseudoElementIdentifier>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const boo::style::PseudoElementIdentifier &identifier) noexcept
    {
      return krys::Hash::Combine(identifier.Type, identifier.NameArgument.Storage());
    }

    KRYS_NODISCARD static bool Equal(const boo::style::PseudoElementIdentifier &a,
                                     const boo::style::PseudoElementIdentifier &b) noexcept
    {
      return a == b;
    }
  };
}
