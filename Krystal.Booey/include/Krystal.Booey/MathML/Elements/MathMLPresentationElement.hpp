/*
 * Copyright (C) 2009 Alex Milowski (alex@milowski.com). All rights reserved.
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2016 Igalia S.L.
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

#include "Krystal.Booey/MathML/Elements/MathMLElement.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::mathml
{
  /// @brief A presentation MathML element: one that lays out its children (as `<mtable>`, `<mtr>`,
  /// `<mtd>`, `<mprescripts>` and `<none>` do) and the base of the other presentation elements.
  class MathMLPresentationElement : public MathMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLPresentationElement);

  public:
    explicit MathMLPresentationElement(
      dom::Document &document, MathMLElementInterface interface = MathMLElementInterface::Presentation,
      dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;

    /// @brief Parses a MathML length, as MathML Core does: a number and an optional unit, with a unitless
    /// value only allowed for "0".
    /// @return A length of type `ParsingFailed` if @p string isn't one.
    KRYS_NODISCARD static Length ParseMathMLLength(dom::DOMStringView string) noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;

  protected:
    KRYS_NODISCARD static Maybe<bool> ToOptionalBool(BooleanValue value) noexcept
    {
      return value == BooleanValue::Default ? Maybe<bool> {} : Maybe<bool> {value == BooleanValue::True};
    }

    KRYS_NODISCARD BooleanValue CachedBooleanAttribute(const dom::QualifiedName &name,
                                                       Maybe<BooleanValue> &attribute) const noexcept;
    KRYS_NODISCARD const Length &CachedMathMLLength(const dom::QualifiedName &name,
                                                    Maybe<Length> &length) const noexcept;

  private:
    KRYS_NODISCARD bool IsPresentationMathML() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD static Length ParseNumberAndUnit(dom::DOMStringView string) noexcept;
  };
}
