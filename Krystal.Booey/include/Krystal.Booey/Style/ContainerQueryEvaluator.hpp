/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryEvaluator.hpp"
#include "Krystal.Booey/Style/Scope/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Update.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  /// @brief The state container queries are evaluated in during a tree resolution: the size query containers
  /// among the ancestors of the element being resolved, and the style update so far.
  struct ContainerQueryEvaluationState
  {
    List<Ref<const dom::Element>> SizeQueryContainers;
    CheckedPtr<style::Update> StyleUpdate;
  };

  /// @brief Evaluates the container queries of the rules that apply to an element against its query
  /// containers.
  class ContainerQueryEvaluator : public css::mq::GenericMediaQueryEvaluator<ContainerQueryEvaluator>
  {
  public:
    enum class SelectionMode : uint8
    {
      Element,
      PseudoElement,
      PartPseudoElement
    };

  private:
    Ref<const dom::Element> _element;
    const SelectionMode _selectionMode;
    const ScopeOrdinal _scopeOrdinal;
    ContainerQueryEvaluationState *_evaluationState {nullptr};

  public:
    ContainerQueryEvaluator(const dom::Element &element, SelectionMode selectionMode,
                            ScopeOrdinal scopeOrdinal,
                            ContainerQueryEvaluationState *evaluationState) noexcept;

    KRYS_NODISCARD bool Evaluate(const css::cq::ContainerQuery &containerQuery) const noexcept;

    /// @brief The query container for @p element: the nearest ancestor (in the composed tree) that is a
    /// container for all of @p requiredAxes and, if @p name isn't empty, has that container name.
    KRYS_NODISCARD static RefPtr<const dom::Element>
      SelectContainer(css::cq::Axis requiredAxes, const css::CSSOMString &name, const dom::Element &element,
                      SelectionMode selectionMode = SelectionMode::Element,
                      ScopeOrdinal scopeOrdinal = ScopeOrdinal::Element,
                      const ContainerQueryEvaluationState *evaluationState = nullptr) noexcept;

  private:
    KRYS_NODISCARD Maybe<css::mq::FeatureEvaluationContext>
      FeatureEvaluationContextForQuery(const css::cq::ContainerQuery &containerQuery) const noexcept;
  };
}
