/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/Scope/ScopeIdentifier.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include <functional>

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  struct ScopedName;

  /// @brief A ScopedName that has been resolved to determine which specific scope it belongs to.
  class ResolvedScopedName
  {
  private:
    css::CSSOMStringAtom _name;
    ScopeIdentifier _scopeIdentifier;

  public:
    ResolvedScopedName(css::CSSOMStringAtom name, ScopeIdentifier scopeIdentifier) noexcept
        : _name(krys::move(name)), _scopeIdentifier(scopeIdentifier)
    {
    }

    /// @brief Creates a ResolvedScopedName from a ScopedName by resolving the scope ordinal into the concrete
    /// style scope.
    KRYS_NODISCARD static ResolvedScopedName CreateFromScopedName(const dom::Element &element,
                                                                  const ScopedName &scopedName) noexcept;

    KRYS_NODISCARD const css::CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }
    KRYS_NODISCARD ScopeIdentifier GetScopeIdentifier() const noexcept
    {
      return _scopeIdentifier;
    }

    KRYS_NODISCARD bool operator==(const ResolvedScopedName &) const noexcept = default;
  };
}

namespace std
{
  template <>
  struct hash<krys::boo::style::ResolvedScopedName>
  {
    size_t operator()(const krys::boo::style::ResolvedScopedName &name) const noexcept
    {
      return krys::UnsignedIntegerPairHash(
        hash<krys::boo::css::CSSOMStringAtom> {}(name.Name()),
        hash<krys::boo::style::ScopeIdentifier> {}(name.GetScopeIdentifier()));
    }
  };
}
