/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Shadow/DataListButtonElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"

namespace krys::boo::html
{
  DataListButtonElement::DataListButtonElement(dom::Document &document, DataListButtonOwner &owner) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks), _owner(&owner)
  {
  }

  Ref<DataListButtonElement> DataListButtonElement::Create(dom::Document &document,
                                                           DataListButtonOwner &owner) noexcept
  {
    auto element = CreateRef<DataListButtonElement>(document, owner);
    element->SetUserAgentPart(css::UserAgentParts::WebkitListButton());
    return element;
  }

  void DataListButtonElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (!Is<dom::MouseEvent>(event))
    {
      if (!event.DefaultHandled())
      {
        HTMLDivElement::DefaultEventHandler(event);
      }
      return;
    }

    if (dom::IsAnyClick(event))
    {
      if (_owner)
      {
        _owner->DataListButtonElementWasClicked();
      }
      event.SetDefaultHandled();
    }

    if (!event.DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(event);
    }
  }

  Maybe<style::UnadjustedStyle>
    DataListButtonElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                              const RenderStyle *) noexcept
  {
    auto elementStyle = ResolveStyle(resolutionContext);
    auto *input = DynamicDowncast<HTMLInputElement>(dom::TreeQueries::ShadowHost(*this));
    if (!input || !input->List())
    {
      elementStyle.Style->SetDisplay(DisplayType::None);
    }
    return elementStyle;
  }

  bool DataListButtonElement::IsDisabledFormControl() const noexcept
  {
    auto *host = dom::TreeQueries::ShadowHost(*this);
    return host && host->IsDisabledFormControl();
  }
}
