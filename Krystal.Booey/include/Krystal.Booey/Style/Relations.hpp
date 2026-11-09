/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  class Update;

  /// @brief A dependency of an element's style on its tree position, recorded while matching selectors.
  struct Relation
  {
    enum Type : uint8
    {
      AffectedByEmpty,
      AffectedByPreviousSibling,
      DescendantsAffectedByPreviousSibling,
      // For AffectsNextSibling 'value' tells how many element siblings to mark starting with 'element'.
      AffectsNextSibling,
      ChildrenAffectedByForwardPositionalRules,
      DescendantsAffectedByForwardPositionalRules,
      ChildrenAffectedByBackwardPositionalRules,
      DescendantsAffectedByBackwardPositionalRules,
      ChildrenAffectedByFirstChildRules,
      ChildrenAffectedByLastChildRules,
      FirstChild,
      LastChild,
      NthChildIndex,
      AffectedByHasWithPositionalPseudoClass,
    };

    const dom::Element *Element;
    Type RelationType;
    uint32 Value;

    Relation(const dom::Element &element, Type type, uint32 value = 1) noexcept
        : Element(&element), RelationType(type), Value(value)
    {
    }
  };

  using Relations = SmallList<Relation, 8>;

  /// @brief Records the relations of @p element that its own style holds (`:empty`, `:first-child`,
  /// `:last-child`) on @p style.
  /// @return The relations left to commit to the elements once the style is applied, or null if there are
  /// none.
  KRYS_NODISCARD UniquePtr<Relations> CommitRelationsToRenderStyle(RenderStyle &style,
                                                                   const dom::Element &element,
                                                                   const Relations &relations) noexcept;

  /// @brief Copies the relation state (`:empty`, `:first-child`, `:last-child`) of @p from to @p to.
  void CopyRelations(RenderStyle &to, const RenderStyle &from) noexcept;

  /// @brief Sets the style flags of the elements @p relations are about, and the relation state of their new
  /// styles in @p update.
  void CommitRelations(UniquePtr<Relations> relations, Update &update) noexcept;
}
