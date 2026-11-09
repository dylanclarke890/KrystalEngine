/*
 * Copyright (c) 2012 Motorola Mobility, Inc. All rights reserved.
 * Copyright (C) 2014-2020 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY MOTOROLA MOBILITY, INC. AND ITS CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MOTOROLA MOBILITY, INC. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Collections/RadioNodeList.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"

namespace krys::boo::html
{
  RadioNodeList::RadioNodeList(dom::ContainerNode &ownerNode, dom::DOMStringView name,
                               RadioNodeListCandidates candidates) noexcept
      : LiveNodeList(
          ownerNode,
          [this](const dom::Node &node)
          {
            auto *element = DynamicDowncast<dom::Element>(node);
            return element && ElementMatches(*element);
          },
          dom::LiveCollectionInvalidation::FormControls,
          // A form's list is rooted at its tree, as its controls can be anywhere in it.
          Is<HTMLFormElement>(ownerNode) ? dom::LiveCollectionTraversal::OwnerTree
                                         : dom::LiveCollectionTraversal::Descendants),
        _ownerNode(ShareRef(ownerNode)), _name(name), _candidates(candidates)
  {
  }

  RadioNodeList::~RadioNodeList() noexcept
  {
    if (auto *form = DynamicDowncast<HTMLFormElement>(*_ownerNode))
    {
      form->RadioNodeListDestroyed(_name, _candidates);
    }
  }

  dom::DOMString RadioNodeList::Value() const noexcept
  {
    // 1. Let element be the first element in tree order represented by the RadioNodeList object that is an
    //    input element whose type attribute is in the Radio Button state and whose checkedness is true.
    //    Otherwise, let it be null.
    for (size_t i = 0, length = Length(); i < length; ++i)
    {
      auto element = Item(i);
      auto *input = DynamicDowncast<HTMLInputElement>(element.get());
      if (input && input->IsRadioButton() && input->Checked())
      {
        // 3. If element is an element with no value attribute, return the string "on".
        // 4. Otherwise, return the value of element's value attribute.
        return input->GetAttribute(u8"value").value_or(u8"on");
      }
    }

    // 2. If element is null, return the empty string.
    return {};
  }

  void RadioNodeList::SetValue(dom::DOMStringView value) noexcept
  {
    // 1. If the new value is the string "on": let element be the first element in tree order represented by
    //    the RadioNodeList object that is an input element whose type attribute is in the Radio Button state
    //    and whose value content attribute is either absent, or present and equal to the new value, if any.
    //    If no such element exists, then instead let element be null. Otherwise: let element be the first
    //    element in tree order represented by the RadioNodeList object that is an input element whose type
    //    attribute is in the Radio Button state and whose value content attribute is present and equal to the
    //    new value, if any. If no such element exists, then instead let element be null.
    for (size_t i = 0, length = Length(); i < length; ++i)
    {
      auto element = Item(i);
      auto *input = DynamicDowncast<HTMLInputElement>(element.get());
      if (!input || !input->IsRadioButton())
      {
        continue;
      }
      auto valueAttribute = input->GetAttribute(u8"value");
      if (valueAttribute ? *valueAttribute == value : value == u8"on")
      {
        // 2. If element is not null, then set its checkedness to true.
        input->SetChecked(true);
        return;
      }
    }
  }

  bool RadioNodeList::ElementMatches(const dom::Element &element) const noexcept
  {
    if (_candidates == RadioNodeListCandidates::Images)
    {
      auto *image = DynamicDowncast<HTMLImageElement>(element);
      return image && image->Form() == _ownerNode.get()
             && (element.GetAttribute(u8"id") == _name || element.GetAttribute(u8"name") == _name);
    }

    if (!element.IsFormListedElement())
    {
      return false;
    }

    if (auto *input = DynamicDowncast<HTMLInputElement>(element); input && input->IsImageButton())
    {
      return false;
    }

    if (Is<HTMLFormElement>(*_ownerNode))
    {
      auto *form = const_cast<HTMLElement &>(Downcast<HTMLElement>(element)).AsFormListedElement()->Form();
      if (form != _ownerNode.get())
      {
        return false;
      }
    }

    return element.GetAttribute(u8"id") == _name || element.GetAttribute(u8"name") == _name;
  }
}
