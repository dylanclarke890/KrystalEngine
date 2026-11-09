/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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

#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  /// @brief The element generated for an element's `::before` or `::after` pseudo-element. It isn't in the
  /// tree: its host element owns it.
  class PseudoElement final : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(PseudoElement);

  private:
    WeakPtr<Element> _hostElement;
    PseudoElementType _pseudoElementType;

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool CanStartSelection() const noexcept override
    {
      return false;
    }

    PseudoElement(Element &host, PseudoElementType pseudoElementType) noexcept;
    ~PseudoElement() noexcept override;

    KRYS_NODISCARD static Ref<PseudoElement> Create(Element &host,
                                                    PseudoElementType pseudoElementType) noexcept;

    KRYS_NODISCARD Element *HostElement() const noexcept
    {
      return _hostElement.get();
    }

    void ClearHostElement() noexcept;

    KRYS_NODISCARD PseudoElementType GetPseudoElementType() const noexcept
    {
      return _pseudoElementType;
    }

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &style) noexcept override;
  };

  /// @brief The tag name of pseudo-elements.
  KRYS_NODISCARD const QualifiedName &PseudoElementTagName() noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::PseudoElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsPseudoElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
