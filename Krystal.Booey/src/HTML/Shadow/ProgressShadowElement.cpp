/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
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

#include "Krystal.Booey/HTML/Shadow/ProgressShadowElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderProgress.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::html
{
  ProgressShadowElement::ProgressShadowElement(dom::Document &document) noexcept : HTMLDivElement(document)
  {
  }

  HTMLProgressElement *ProgressShadowElement::ProgressElement() const noexcept
  {
    return Downcast<HTMLProgressElement>(dom::TreeQueries::ShadowHost(*this));
  }

  bool ProgressShadowElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    if (auto *progressRenderer = ProgressElement()->Renderer())
    {
      return !progressRenderer->Style().HasUsedAppearance() && HTMLDivElement::RendererIsNeeded(style);
    }
    return false;
  }

  ProgressInnerElement::ProgressInnerElement(dom::Document &document) noexcept
      : ProgressShadowElement(document)
  {
  }

  RenderPtr<RenderElement> ProgressInnerElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderProgress>(*this, krys::move(style));
  }

  bool ProgressInnerElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    auto *progressRenderer = ProgressElement()->Renderer();
    return progressRenderer && !progressRenderer->Style().HasUsedAppearance()
           && HTMLDivElement::RendererIsNeeded(style);
  }

  ProgressBarElement::ProgressBarElement(dom::Document &document) noexcept : ProgressShadowElement(document)
  {
  }

  ProgressValueElement::ProgressValueElement(dom::Document &document) noexcept
      : ProgressShadowElement(document)
  {
  }

  void ProgressValueElement::SetInlineSizePercentage(double size) noexcept
  {
    (void)SetInlineStyleProperty(css::PropertyId::InlineSize, std::max(0.0, size), css::UnitType::Percentage);
  }

  Ref<ProgressInnerElement> ProgressInnerElement::Create(dom::Document &document) noexcept
  {
    auto result = CreateRef<ProgressInnerElement>(document);
    result->SetUserAgentPart(css::UserAgentParts::WebkitProgressInnerElement());
    return result;
  }

  Ref<ProgressBarElement> ProgressBarElement::Create(dom::Document &document) noexcept
  {
    auto result = CreateRef<ProgressBarElement>(document);
    result->SetUserAgentPart(css::UserAgentParts::WebkitProgressBar());
    return result;
  }

  Ref<ProgressValueElement> ProgressValueElement::Create(dom::Document &document) noexcept
  {
    auto result = CreateRef<ProgressValueElement>(document);
    result->SetUserAgentPart(css::UserAgentParts::WebkitProgressValue());
    return result;
  }
}
