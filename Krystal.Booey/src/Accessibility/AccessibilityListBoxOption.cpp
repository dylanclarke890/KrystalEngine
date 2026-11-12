/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Accessibility/AccessibilityListBoxOption.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderListBox.hpp"

namespace krys::boo::ax
{
  Ref<AccessibilityListBoxOption> AccessibilityListBoxOption::Create(AXID id, html::HTMLElement &element,
                                                                     AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityListBoxOption(id, element, cache));
  }

  AccessibilityListBoxOption::AccessibilityListBoxOption(AXID id, html::HTMLElement &element,
                                                         AXObjectCache &cache) noexcept
      : AccessibilityNodeObject(id, &element, cache)
  {
  }

  AccessibilityListBoxOption::~AccessibilityListBoxOption() noexcept = default;

  html::HTMLSelectElement *AccessibilityListBoxOption::ListBoxOptionParentNode() const noexcept
  {
    auto *node = GetNode();
    if (auto *option = DynamicDowncast<html::HTMLOptionElement>(node))
    {
      return option->OwnerSelectElement();
    }
    if (auto *group = DynamicDowncast<html::HTMLOptGroupElement>(node))
    {
      return group->OwnerSelectElement();
    }
    return nullptr;
  }

  int32 AccessibilityListBoxOption::ListBoxOptionIndex() const noexcept
  {
    auto *select = ListBoxOptionParentNode();
    if (!select)
    {
      return -1;
    }
    const auto &items = select->ListItems();
    for (size_t index = 0; index < items.size(); ++index)
    {
      if (items[index].get() == GetNode())
      {
        return static_cast<int32>(index);
      }
    }
    return -1;
  }

  AccessibilityObject *AccessibilityListBoxOption::ParentObject() const noexcept
  {
    auto *select = ListBoxOptionParentNode();
    auto *cache = AxObjectCache();
    return select && cache ? cache->GetOrCreate(*select) : nullptr;
  }

  bool AccessibilityListBoxOption::ComputeIsIgnored() const noexcept
  {
    if (!GetNode() || IsIgnoredByDefault())
    {
      return true;
    }
    auto *parent = ParentObject();
    return !parent || parent->IsIgnored();
  }

  bool AccessibilityListBoxOption::IsEnabled() const noexcept
  {
    return !(Is<html::HTMLOptGroupElement>(GetNode())
             || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(GetAttribute(u8"aria-disabled"), u8"true")
             || HasAttribute(u8"disabled"));
  }

  bool AccessibilityListBoxOption::IsSelected() const noexcept
  {
    auto *option = DynamicDowncast<html::HTMLOptionElement>(GetNode());
    return option && option->Selected();
  }

  void AccessibilityListBoxOption::SetSelected(bool selected) noexcept
  {
    auto *select = ListBoxOptionParentNode();
    if (!select || !CanSetSelectedAttribute() || IsSelected() == selected)
    {
      return;
    }
    select->AccessKeySetSelectedIndex(select->ListToOptionIndex(ListBoxOptionIndex()));
  }

  bool AccessibilityListBoxOption::CanSetSelectedAttribute() const noexcept
  {
    auto *option = DynamicDowncast<html::HTMLOptionElement>(GetNode());
    if (!option || option->IsDisabledFormControl())
    {
      return false;
    }
    auto *select = ListBoxOptionParentNode();
    return !select || !select->IsDisabledFormControl();
  }

  dom::DOMString AccessibilityListBoxOption::StringValue() const noexcept
  {
    if (auto label = GetAttributeTrimmed(u8"aria-label"); !label.empty())
    {
      return label;
    }
    auto *node = GetNode();
    if (auto *option = DynamicDowncast<html::HTMLOptionElement>(node))
    {
      return option->Label();
    }
    if (auto *group = DynamicDowncast<html::HTMLOptGroupElement>(node))
    {
      return group->GroupLabelText();
    }
    return {};
  }

  LayoutRect AccessibilityListBoxOption::ElementRect() const noexcept
  {
    // The option's row in the list box.
    auto *select = ListBoxOptionParentNode();
    auto *listBox = select ? DynamicDowncast<RenderListBox>(select->Renderer()) : nullptr;
    auto *parent = ParentObject();
    auto index = ListBoxOptionIndex();
    if (!listBox || !parent || index < 0)
    {
      return {};
    }
    return listBox->ItemBoundingBoxRect(parent->ElementRect().Location(), index);
  }
}
