/*
 * Copyright (C) 2016 Igalia, S.L.
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"

namespace krys::boo::ax
{
  /// @brief An object for a MathML element: `math` is a math document and every other element a math element,
  /// with the parts of fractions, roots and scripts exposed for the platform to describe.
  /// @note MathML Core's elements only: `mfenced` and the anonymous operators WebKit makes for it are
  /// dropped.
  class AccessibilityMathMLElement : public AccessibilityRenderObject
  {
  public:
    KRYS_NODISCARD static Ref<AccessibilityMathMLElement> Create(AXID id, RenderObject &renderer,
                                                                 AXObjectCache &cache) noexcept;
    ~AccessibilityMathMLElement() noexcept override;

    KRYS_NODISCARD bool IsMathElement() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD bool IsIgnoredElementWithinMathTree() const noexcept final;

    KRYS_NODISCARD bool IsMathFraction() const noexcept final;
    KRYS_NODISCARD bool IsMathSubscriptSuperscript() const noexcept final;
    KRYS_NODISCARD bool IsMathRow() const noexcept final;
    KRYS_NODISCARD bool IsMathUnderOver() const noexcept final;
    KRYS_NODISCARD bool IsMathRoot() const noexcept final;
    KRYS_NODISCARD bool IsMathSquareRoot() const noexcept final;
    KRYS_NODISCARD bool IsMathToken() const noexcept final;
    KRYS_NODISCARD bool IsMathOperator() const noexcept final;
    KRYS_NODISCARD bool IsMathFenceOperator() const noexcept final;
    KRYS_NODISCARD bool IsMathSeparatorOperator() const noexcept final;
    KRYS_NODISCARD bool IsMathText() const noexcept final;
    KRYS_NODISCARD bool IsMathNumber() const noexcept final;
    KRYS_NODISCARD bool IsMathIdentifier() const noexcept final;
    KRYS_NODISCARD bool IsMathMultiscript() const noexcept final;
    KRYS_NODISCARD bool IsMathTable() const noexcept final;
    KRYS_NODISCARD bool IsMathTableRow() const noexcept final;
    KRYS_NODISCARD bool IsMathTableCell() const noexcept final;

    KRYS_NODISCARD Maybe<AccessibilityChildrenVector> MathRadicand() noexcept final;
    KRYS_NODISCARD AccessibilityObject *MathRootIndexObject() noexcept final;
    KRYS_NODISCARD AccessibilityObject *MathNumeratorObject() noexcept final;
    KRYS_NODISCARD AccessibilityObject *MathDenominatorObject() noexcept final;
    KRYS_NODISCARD AccessibilityObject *MathUnderObject() noexcept final;
    KRYS_NODISCARD AccessibilityObject *MathOverObject() noexcept final;
    KRYS_NODISCARD AccessibilityObject *MathBaseObject() noexcept final;
    KRYS_NODISCARD AccessibilityObject *MathSubscriptObject() noexcept final;
    KRYS_NODISCARD AccessibilityObject *MathSuperscriptObject() noexcept final;
    KRYS_NODISCARD AccessibilityMathMultiscriptPairs MathPrescripts() noexcept final;
    KRYS_NODISCARD AccessibilityMathMultiscriptPairs MathPostscripts() noexcept final;

  protected:
    AccessibilityMathMLElement(AXID id, RenderObject &renderer, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override;

  private:
    KRYS_NODISCARD bool HasLocalName(dom::DOMStringView localName) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityMathMLElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMathElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
