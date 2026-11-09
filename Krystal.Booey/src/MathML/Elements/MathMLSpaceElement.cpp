/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
 * Copyright (C) 2016 Igalia S.L. All rights reserved.
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
 *
 */

#include "Krystal.Booey/MathML/Elements/MathMLSpaceElement.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLSpace.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::mathml
{
  const MathMLElement::Length &MathMLSpaceElement::Width() noexcept
  {
    return CachedMathMLLength(MathMLNames::WidthAttr(), _width);
  }

  const MathMLElement::Length &MathMLSpaceElement::Height() noexcept
  {
    return CachedMathMLLength(MathMLNames::HeightAttr(), _height);
  }

  const MathMLElement::Length &MathMLSpaceElement::Depth() noexcept
  {
    return CachedMathMLLength(MathMLNames::DepthAttr(), _depth);
  }

  void MathMLSpaceElement::AttributeChanged(const dom::QualifiedName &name,
                                            Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value) noexcept
  {
    bool affectsLayout = false;
    if (name == MathMLNames::WidthAttr())
    {
      _width.reset();
      affectsLayout = true;
    }
    else if (name == MathMLNames::HeightAttr())
    {
      _height.reset();
      affectsLayout = true;
    }
    else if (name == MathMLNames::DepthAttr())
    {
      _depth.reset();
      affectsLayout = true;
    }

    if (affectsLayout)
    {
      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }

    MathMLPresentationElement::AttributeChanged(name, oldValue, value);
  }

  RenderPtr<RenderElement> MathMLSpaceElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(HasTagName(html::TagName::mspace));
    return CreateRenderer<RenderMathMLSpace>(*this, krys::move(style));
  }
}
