/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Update.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeAncestorIterator.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"

namespace krys::boo::style
{
  Update::Update(dom::Document &document) noexcept : _document(ShareRef(document))
  {
  }

  Update::~Update() noexcept = default;

  const ElementUpdate *Update::GetElementUpdate(const dom::Element &element) const noexcept
  {
    auto it = _elements.find(ShareRefPtr(&element));
    if (it == _elements.end())
    {
      return nullptr;
    }
    return &it->second;
  }

  ElementUpdate *Update::GetElementUpdate(const dom::Element &element) noexcept
  {
    auto it = _elements.find(ShareRefPtr(&element));
    if (it == _elements.end())
    {
      return nullptr;
    }
    return &it->second;
  }

  const TextUpdate *Update::GetTextUpdate(const dom::Text &text) const noexcept
  {
    auto it = _texts.find(ShareRefPtr(&text));
    if (it == _texts.end())
    {
      return nullptr;
    }
    return &it->second;
  }

  const RenderStyle *Update::ElementStyle(const dom::Element &element) const noexcept
  {
    if (auto *update = GetElementUpdate(element))
    {
      return update->Style.get();
    }
    return element.GetRenderStyle();
  }

  RenderStyle *Update::ElementStyle(const dom::Element &element) noexcept
  {
    if (auto *update = GetElementUpdate(element))
    {
      return update->Style.get();
    }
    auto *renderer = element.Renderer();
    if (!renderer)
    {
      return nullptr;
    }
    return &renderer->MutableStyle();
  }

  void Update::AddElement(dom::Element &element, dom::Element *parent, ElementUpdate &&elementUpdate) noexcept
  {
    krys_debug_assert(dom::ComposedTreeAncestors(element).First() == parent);
    krys_debug_assert(!_elements.contains(ShareRefPtr<const dom::Element>(&element)));

    _roots.remove(ShareRefPtr<dom::ContainerNode>(&element));
    AddPossibleRoot(parent);

    if (elementUpdate.MayNeedRebuildRoot)
    {
      AddPossibleRebuildRoot(element, parent);
    }

    if (elementUpdate.Style && Scope::UsesViewportUnits(*elementUpdate.Style))
    {
      Scope::ForNode(element).AddElementWithViewportUnits(element);
    }

    _elements.emplace(ShareRefPtr<const dom::Element>(&element), krys::move(elementUpdate));
  }

  void Update::AddText(dom::Text &text, dom::Element *parent, TextUpdate &&textUpdate) noexcept
  {
    krys_debug_assert(dom::ComposedTreeAncestors(text).First() == parent);

    AddPossibleRoot(parent);

    auto [it, isNewEntry] = _texts.try_emplace(ShareRefPtr<const dom::Text>(&text), krys::move(textUpdate));

    if (!isNewEntry)
    {
      auto &entry = it->second;
      auto startOffset = std::min(entry.Offset, textUpdate.Offset);
      auto endOffset = std::max(entry.Offset + entry.Length, textUpdate.Offset + textUpdate.Length);
      entry.Offset = startOffset;
      entry.Length = endOffset - startOffset;

      krys_debug_assert(!entry.InheritedDisplayContentsStyle || !textUpdate.InheritedDisplayContentsStyle);
      if (!entry.InheritedDisplayContentsStyle)
      {
        entry.InheritedDisplayContentsStyle = krys::move(textUpdate.InheritedDisplayContentsStyle);
      }
    }
  }

  void Update::AddText(dom::Text &text, TextUpdate &&textUpdate) noexcept
  {
    AddText(text, dom::ComposedTreeAncestors(text).First(), krys::move(textUpdate));
  }

  void Update::AddInitialContainingBlockUpdate(UniquePtr<RenderStyle> style) noexcept
  {
    _initialContainingBlockUpdate = krys::move(style);
  }

  void Update::AddSVGRendererUpdate(svg::SVGElement &element) noexcept
  {
    auto *parent = dom::ComposedTreeAncestors(element).First();
    _roots.remove(ShareRefPtr<dom::ContainerNode>(&element));
    AddPossibleRoot(parent);
    element.SetNeedsSVGRendererUpdate(true);
  }

  void Update::AddPossibleRoot(dom::Element *element) noexcept
  {
    if (!element)
    {
      _roots.add(ShareRefPtr<dom::ContainerNode>(_document.get()));
      return;
    }
    if (element->NeedsSVGRendererUpdate() || _elements.contains(ShareRefPtr<const dom::Element>(element)))
    {
      return;
    }
    _roots.add(ShareRefPtr<dom::ContainerNode>(element));
  }

  void Update::AddPossibleRebuildRoot(dom::Element &element, dom::Element *parent) noexcept
  {
    if (parent && _rebuildRoots.contains(ShareRefPtr(parent)))
    {
      return;
    }

    _rebuildRoots.add(ShareRefPtr(&element));
  }
}
