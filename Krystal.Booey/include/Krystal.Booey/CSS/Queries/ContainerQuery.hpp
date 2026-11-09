/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Queries/ContainerQueryAxis.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::cq
{
  /// @brief The axes a container must support size queries on for the feature to be evaluated.
  KRYS_NODISCARD Axis RequiredAxesForFeature(const mq::Feature &feature) noexcept;

  enum class ContainsUnknownFeature : bool
  {
    No,
    Yes
  };

  /// @brief The prelude of an `@container` rule: an optional container name and a condition.
  struct ContainerQuery
  {
    CSSOMStringAtom Name {CSSOMStringAtom::Null()};
    mq::Condition Condition;
    Axis RequiredAxes {Axis::None};
    cq::ContainsUnknownFeature ContainsUnknownFeature {cq::ContainsUnknownFeature::No};
  };

  /// @brief An `@container` rule's prelude: true if any of its queries is, each against its own container.
  /// @see https://drafts.csswg.org/css-conditional-5/#typedef-container-query-list
  using ContainerQueryList = List<ContainerQuery>;

  void Serialise(StringBuilder &builder, const ContainerQuery &query) noexcept;
  void Serialise(StringBuilder &builder, const ContainerQueryList &queries) noexcept;
}
