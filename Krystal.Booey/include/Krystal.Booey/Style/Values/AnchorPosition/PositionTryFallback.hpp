/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/Style/Scope/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbackTactic.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::style
{
  // <position-try-fallback> = [ [<dashed-ident> || <try-tactic>] | <position-area> ]
  // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks
  struct PositionTryFallback
  {
    using Tactic = PositionTryFallbackTactic;

    struct RuleAndTactics
    {
      Markable<ScopedName> Rule {};
      css::ListOrNull<css::SpaceSeparatedList<Tactic>> Tactics {std::nullopt};

      KRYS_NODISCARD bool operator==(const RuleAndTactics &) const noexcept = default;
    };

    struct PositionArea
    {
      RefPtr<const css::ImmutableStyleProperties> Properties {};

      KRYS_NODISCARD bool operator==(const PositionArea &) const noexcept;
    };

    // Only one of these is valid at a time.
    // FIXME: Use a Variant<PositionTryFallback::RuleAndTactics, PositionTryFallback::PositionArea> to enforce
    // this invariant, and then make PositionArea use a Ref instead of a RefPtr.
    RuleAndTactics RuleAndTacticsFallback {};
    PositionArea PositionAreaFallback {};

    KRYS_NODISCARD bool IsPositionArea() const noexcept
    {
      return !!PositionAreaFallback.Properties;
    }
    KRYS_NODISCARD bool IsRuleAndTactics() const noexcept
    {
      return !PositionAreaFallback.Properties;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsPositionArea())
      {
        return visitor(PositionAreaFallback);
      }
      return visitor(RuleAndTacticsFallback);
    }

    KRYS_NODISCARD bool operator==(const PositionTryFallback &) const noexcept;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const PositionTryFallback::RuleAndTactics &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Rule;
    }
    else if constexpr (I == 1)
    {
      return value.Tactics;
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<PositionTryFallback>
  {
    KRYS_NODISCARD PositionTryFallback operator()(BuilderState &, const css::Value &) noexcept;
  };
  // `PositionTryFallback::PositionArea` is special-cased to handle `ImmutableStyleProperties`.
  template <>
  struct CSSValueCreation<PositionTryFallback::PositionArea>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const PositionTryFallback::PositionArea &) noexcept;
  };

  // MARK: - Serialization

  // `PositionTryFallback::PositionArea` is special-cased to handle `ImmutableStyleProperties`.
  template <>
  struct Serialiser<PositionTryFallback::PositionArea>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const PositionTryFallback::PositionArea &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionTryFallback)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::PositionTryFallback::RuleAndTactics, 2)
