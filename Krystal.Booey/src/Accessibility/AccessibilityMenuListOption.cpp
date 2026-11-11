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

#include "Krystal.Booey/Accessibility/AccessibilityMenuListOption.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"

namespace krys::boo::ax
{
  Ref<AccessibilityMenuListOption> AccessibilityMenuListOption::Create(AXID id,
                                                                       html::HTMLOptionElement &element,
                                                                       AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityMenuListOption(id, element, cache));
  }

  AccessibilityMenuListOption::AccessibilityMenuListOption(AXID id, html::HTMLOptionElement &element,
                                                           AXObjectCache &cache) noexcept
      : AccessibilityNodeObject(id, &element, cache)
  {
  }

  html::HTMLOptionElement *AccessibilityMenuListOption::OptionElement() const noexcept
  {
    return DynamicDowncast<html::HTMLOptionElement>(GetNode());
  }

  bool AccessibilityMenuListOption::IsEnabled() const noexcept
  {
    auto *option = OptionElement();
    return option && !option->OwnElementDisabled();
  }

  bool AccessibilityMenuListOption::IsSelected() const noexcept
  {
    auto *option = OptionElement();
    return option && option->Selected();
  }

  dom::DOMString AccessibilityMenuListOption::StringValue() const noexcept
  {
    auto *option = OptionElement();
    return option ? option->Label() : dom::DOMString {};
  }

  LayoutRect AccessibilityMenuListOption::ElementRect() const noexcept
  {
    // An option is where its menu list is (the pop-up is the host's, not the page's).
    auto *popup = ParentObject();
    auto *menuList = popup ? popup->ParentObject() : nullptr;
    return menuList ? menuList->ElementRect() : LayoutRect();
  }
}
