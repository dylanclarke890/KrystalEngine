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

#include "Krystal.Booey/MathML/Elements/MathMLTokenElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLToken.hpp"

namespace krys::boo::mathml
{
  void MathMLTokenElement::DidAttachRenderers() noexcept
  {
    MathMLPresentationElement::DidAttachRenderers();
    if (auto *mathmlRenderer = DynamicDowncast<RenderMathMLToken>(Renderer()))
    {
      mathmlRenderer->UpdateTokenContent();
    }
  }

  void MathMLTokenElement::OnChildrenChanged() noexcept
  {
    MathMLPresentationElement::OnChildrenChanged();
    if (auto *mathmlRenderer = DynamicDowncast<RenderMathMLToken>(Renderer()))
    {
      mathmlRenderer->UpdateTokenContent();
    }
  }

  RenderPtr<RenderElement> MathMLTokenElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(HasTagName(html::TagName::mi) || HasTagName(html::TagName::mn)
                      || HasTagName(html::TagName::ms) || HasTagName(html::TagName::mtext));

    return CreateRenderer<RenderMathMLToken>(RenderObject::Type::MathMLToken, *this, krys::move(style));
  }

  bool MathMLTokenElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    // The HTML specification defines <mi>, <mo>, <mn>, <ms> and <mtext> as insertion points.
    return StyledElement::ChildShouldCreateRenderer(child);
  }

  bool MathMLTokenElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                              dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == MathMLNames::MathvariantAttr().LocalName()
        && HasTagName(html::TagName::mi))
    {
      return true;
    }
    return MathMLPresentationElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void MathMLTokenElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                  dom::DOMStringAtom namespaceURI,
                                                                  dom::DOMStringView value,
                                                                  css::MutableStyleProperties &style) noexcept
  {
    // https://w3c.github.io/mathml-core/#the-mathvariant-attribute
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == MathMLNames::MathvariantAttr().LocalName()
        && HasTagName(html::TagName::mi))
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"normal"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextTransform, css::ValueId::None);
      }
      return;
    }
    MathMLPresentationElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}
