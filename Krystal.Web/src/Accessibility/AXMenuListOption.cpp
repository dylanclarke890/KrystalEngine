/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Accessibility/AXMenuListOption.hpp"
#include "Krystal.Web/Html/Elements/HTMLOptionElement.hpp"

namespace krys::web
{
  Ref<AXMenuListOption> AXMenuListOption::Create(AXId id, HTMLOptionElement &element,
                                                 AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXMenuListOption(id, element, cache));
  }

  AXMenuListOption::AXMenuListOption(AXId id, HTMLOptionElement &element, AXObjectCache &cache) noexcept
      : AXNodeObject(id, &element, cache)
  {
  }

  HTMLOptionElement *AXMenuListOption::OptionElement() const noexcept
  {
    return DynamicDowncast<HTMLOptionElement>(GetNode());
  }

  bool AXMenuListOption::IsEnabled() const noexcept
  {
    auto *option = OptionElement();
    return option && !option->OwnElementDisabled();
  }

  bool AXMenuListOption::IsSelected() const noexcept
  {
    auto *option = OptionElement();
    return option && option->Selected();
  }

  void AXMenuListOption::SetSelected(bool selected) noexcept
  {
    if (auto *option = OptionElement(); option && CanSetSelectedAttribute())
    {
      option->Selected(selected);
    }
  }

  DOMString AXMenuListOption::StringValue() const noexcept
  {
    auto *option = OptionElement();
    return option ? option->Label() : DOMString {};
  }

  LayoutRect AXMenuListOption::ElementRect() const noexcept
  {
    // An option is where its menu list is (the pop-up is the host's, not the page's).
    auto *popup = ParentObject();
    auto *menuList = popup ? popup->ParentObject() : nullptr;
    return menuList ? menuList->ElementRect() : LayoutRect();
  }
}
