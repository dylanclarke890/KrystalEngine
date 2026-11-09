/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Algorithms/FindRevealAlgorithms.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDetailsElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  namespace
  {
    enum class RevealType : bool
    {
      ClosedDetails,
      HiddenUntilFound
    };

    KRYS_NODISCARD RefPtr<html::HTMLDetailsElement> ClosedDetailsElementAncestor(const Node &node) noexcept
    {
      auto *slot = SlotAlgorithms::GetAssignedSlot(node);
      if (slot && slot->UserAgentPart() == css::UserAgentParts::DetailsContent())
      {
        if (auto *details = DynamicDowncast<html::HTMLDetailsElement>(TreeQueries::ShadowHost(*slot)))
        {
          if (!details->HasAttribute(u8"open"))
          {
            return ShareRefPtr(details);
          }
        }
      }
      return nullptr;
    }
  }

  // https://html.spec.whatwg.org/#ancestor-revealing-algorithm
  bool RevealClosedDetailsAndHiddenUntilFoundAncestors(Node &node) noexcept
  {
    (void)node.NodeDocument().UpdateStyleIfNeeded();

    // Bail out if there is neither a hidden=until-found or details ancestor.
    if (auto *style = node.GetRenderStyle(); style && !style->AutoRevealsWhenFound())
    {
      return false;
    }

    // 1. Let ancestorsToReveal be « ».
    List<std::pair<Ref<html::HTMLElement>, RevealType>> ancestors;

    // 2. Let ancestor be target.
    // 3. While ancestor has a parent node within the flat tree:
    // (The document can't be revealed, so this stops at the document element.)
    for (Node *ancestor = &node; ancestor->ParentElementInComposedTree();
         ancestor = ancestor->ParentElementInComposedTree())
    {
      // 3.1. If ancestor has a hidden attribute in the Hidden Until Found state, then append
      //      (ancestor, "until-found") to ancestorsToReveal.
      if (auto *element = DynamicDowncast<html::HTMLElement>(*ancestor);
          element && element->IsHiddenUntilFound())
      {
        ancestors.emplace_back(ShareRef(*element), RevealType::HiddenUntilFound);
      }
      // 3.2. If ancestor is slotted into the second slot of a details element which does not have an open
      //      attribute, then append (ancestor's parent node, "details") to ancestorsToReveal.
      if (auto details = ClosedDetailsElementAncestor(*ancestor))
      {
        ancestors.emplace_back(ShareRef<html::HTMLElement>(*details), RevealType::ClosedDetails);
      }
      // 3.3. Set ancestor to the parent node of ancestor within the flat tree.
    }

    // 4. For each (ancestorToReveal, revealType) of ancestorsToReveal:
    auto revealed = false;
    for (auto &[element, revealType] : ancestors)
    {
      // 4.1. If ancestorToReveal is not connected, then return.
      if (!element->IsConnected())
      {
        return revealed;
      }
      switch (revealType)
      {
        // 4.3. Otherwise:
        case RevealType::ClosedDetails:
          // 4.3.1. Assert: revealType is "details".
          // 4.3.2. If ancestorToReveal has an open attribute, then return.
          if (element->HasAttribute(u8"open"))
          {
            return revealed;
          }
          // 4.3.3. Set ancestorToReveal's open attribute to the empty string.
          Downcast<html::HTMLDetailsElement>(*element).Open(true);
          revealed = true;
          break;
        // 4.2. If revealType is "until-found":
        case RevealType::HiddenUntilFound:
          // 4.2.1. If ancestorToReveal's hidden attribute is not in the Hidden Until Found state, then
          // return.
          if (!element->IsHiddenUntilFound())
          {
            return revealed;
          }
          // 4.2.2. Fire an event named beforematch at ancestorToReveal with the bubbles attribute initialized
          //        to true.
          {
            auto event = Event::Create(DOMStringAtom(EventNames::BeforeMatch), true, false);
            (void)element->DispatchEvent(*event);
          }
          // 4.2.3. If ancestorToReveal is not connected, then return.
          // 4.2.4. If ancestorToReveal's hidden attribute is not in the Hidden Until Found state, then
          // return.
          if (!element->IsConnected() || !element->IsHiddenUntilFound())
          {
            return revealed;
          }
          // 4.2.5. Remove the hidden attribute from ancestorToReveal.
          element->Hidden(false);
          revealed = true;
          break;
      }
    }
    return revealed;
  }
}
