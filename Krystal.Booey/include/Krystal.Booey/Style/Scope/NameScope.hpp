/*
 * Copyright (C) 2024-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/Style/Scope/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Visitor.hpp"

#include <algorithm>

namespace krys::boo::style
{
  // <name-scope> = none | all | <dashed-ident>#
  // Used by:
  // <'timeline-scope'>
  // https://drafts.csswg.org/scroll-animations-1/#propdef-timeline-scope
  // <'anchor-scope'>
  // https://drafts.csswg.org/css-anchor-position-1/#propdef-anchor-scope
  struct NameScope
  {
    enum class Type : uint8
    {
      None,
      All,
      Ident
    };

    NameScope(css::keywords::None) noexcept
    {
    }

    NameScope(css::keywords::All, style::ScopeOrdinal scopeOrdinal) noexcept
        : Type {NameScope::Type::All}, ScopeOrdinal {scopeOrdinal}
    {
    }

    NameScope(css::CommaSeparatedListHashSet<css::CustomIdentifier> &&names,
              style::ScopeOrdinal scopeOrdinal) noexcept
        : Type {NameScope::Type::Ident}, Names {krys::move(names)}, ScopeOrdinal {scopeOrdinal}
    {
    }

    NameScope::Type Type {NameScope::Type::None};
    css::CommaSeparatedListHashSet<css::CustomIdentifier> Names;
    style::ScopeOrdinal ScopeOrdinal {style::ScopeOrdinal::Element};

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (Type)
      {
        case NameScope::Type::None: return visitor(css::keywords::None {});
        case NameScope::Type::All:  return visitor(css::keywords::All {});
        case NameScope::Type::Ident:
          if (Names.isEmpty())
          {
            return visitor(css::keywords::None {});
          }
          return visitor(Names);
      }
      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const NameScope &other) const noexcept
    {
      return Type == other.Type
             && ScopeOrdinal == other.ScopeOrdinal
             // Two name lists are equal if they contain the same values in the same order.
             // FIXME: This is not symmetrical in the case that other.Names.isEmpty() is true, but
             // Names.isEmpty() is false.
             && (Names.isEmpty() || std::ranges::equal(Names, other.Names));
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<NameScope>
  {
    KRYS_NODISCARD NameScope operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::NameScope)
