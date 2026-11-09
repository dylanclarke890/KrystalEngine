/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo::dom
{
  const QualifiedName &PseudoElementTagName() noexcept
  {
    static NeverDestroyed<const QualifiedName> name {DOMStringAtom::Null(), DOMStringAtom::Null(),
                                                     u8"<pseudo>"};
    return name;
  }

  PseudoElement::PseudoElement(Element &host, PseudoElementType pseudoElementType) noexcept
      : Element(host.NodeDocument(), PseudoElementTagName(), NodeFlags::IsPseudoElement),
        _hostElement(CreateWeakPtr(&host)), _pseudoElementType(pseudoElementType)
  {
    SetNodeFlag(NodeFlags::IsConnected);
    krys_debug_assert(pseudoElementType == PseudoElementType::Before
                      || pseudoElementType == PseudoElementType::After);
  }

  PseudoElement::~PseudoElement() noexcept
  {
    krys_debug_assert(!_hostElement);
  }

  Ref<PseudoElement> PseudoElement::Create(Element &host, PseudoElementType pseudoElementType) noexcept
  {
    return CreateRef<PseudoElement>(host, pseudoElementType);
  }

  void PseudoElement::ClearHostElement() noexcept
  {
    style::Styleable::FromElement(*this).ElementWasRemoved();

    _hostElement = nullptr;
  }

  bool PseudoElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    if (PseudoElementRendererIsNeeded(&style))
    {
      return true;
    }

    if (RefPtr element = ShareRefPtr(_hostElement.get()))
    {
      if (auto *stack =
            element->GetKeyframeEffectStack(style::PseudoElementIdentifier {GetPseudoElementType()}))
      {
        return stack->RequiresPseudoElement();
      }
    }

    return false;
  }
}
