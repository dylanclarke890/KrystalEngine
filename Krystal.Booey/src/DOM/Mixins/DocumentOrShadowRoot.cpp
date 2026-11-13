/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/DOM/Mixins/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheetObservableArray.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/DOM/Internals/TreeScopeOrderedMap.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/Forms/RadioButtonGroups.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  using WeakSVGElementSet = List<WeakPtr<svg::SVGElement>>;

  struct SVGResourcesMap
  {
    KRYS_NON_COPYABLE(SVGResourcesMap);

    HashMap<DOMStringAtom, WeakSVGElementSet> PendingResources;
    HashMap<DOMStringAtom, WeakSVGElementSet> PendingResourcesForRemoval;
    HashMap<DOMStringAtom, WeakPtr<LegacyRenderSVGResourceContainer>> LegacyResources;

    SVGResourcesMap() noexcept = default;
  };

  namespace
  {
    // The sets are lists: an element is in one at most once, and elements that have gone are ignored.
    void AddToSet(WeakSVGElementSet &set, svg::SVGElement &element) noexcept
    {
      std::erase_if(set, [](const WeakPtr<svg::SVGElement> &weakElement) { return !weakElement; });
      if (std::ranges::none_of(set, [&](const WeakPtr<svg::SVGElement> &weakElement)
                               { return weakElement.get() == &element; }))
      {
        set.push_back(CreateWeakPtr(&element));
      }
    }

    void RemoveFromSet(WeakSVGElementSet &set, svg::SVGElement &element) noexcept
    {
      std::erase_if(set, [&](const WeakPtr<svg::SVGElement> &weakElement)
                    { return !weakElement || weakElement.get() == &element; });
    }

    KRYS_NODISCARD bool SetContains(const WeakSVGElementSet &set, svg::SVGElement &element) noexcept
    {
      return std::ranges::any_of(set, [&](const WeakPtr<svg::SVGElement> &weakElement)
                                 { return weakElement.get() == &element; });
    }

    KRYS_NODISCARD bool IsEmptyIgnoringNullReferences(const WeakSVGElementSet &set) noexcept
    {
      return std::ranges::none_of(set, [](const WeakPtr<svg::SVGElement> &weakElement)
                                  { return static_cast<bool>(weakElement); });
    }
  }

  SVGResourcesMap &DocumentOrShadowRoot::GetSVGResourcesMap() const noexcept
  {
    if (!_svgResourcesMap)
    {
      _svgResourcesMap = CreateUnique<SVGResourcesMap>();
    }
    return *_svgResourcesMap;
  }

  void DocumentOrShadowRoot::AddSVGResource(const DOMStringAtom &id,
                                            LegacyRenderSVGResourceContainer &resource) noexcept
  {
    if (id.View().empty())
    {
      return;
    }

    // Replaces resource if already present, to handle potential id changes
    GetSVGResourcesMap().LegacyResources.insert_or_assign(id, CreateWeakPtr(&resource));
  }

  void DocumentOrShadowRoot::RemoveSVGResource(const DOMStringAtom &id) noexcept
  {
    if (id.View().empty())
    {
      return;
    }

    GetSVGResourcesMap().LegacyResources.erase(id);
  }

  LegacyRenderSVGResourceContainer *
    DocumentOrShadowRoot::LookupLegacySVGResourceById(const DOMStringAtom &id) const noexcept
  {
    if (id.View().empty())
    {
      return nullptr;
    }

    auto &resources = GetSVGResourcesMap().LegacyResources;
    if (auto it = resources.find(id); it != resources.end())
    {
      return it->second.get();
    }

    return nullptr;
  }

  void DocumentOrShadowRoot::AddPendingSVGResource(const DOMStringAtom &id, svg::SVGElement &element) noexcept
  {
    if (id.View().empty())
    {
      return;
    }

    AddToSet(GetSVGResourcesMap().PendingResources[id], element);

    element.SetHasPendingResources();
  }

  bool DocumentOrShadowRoot::IsIdOfPendingSVGResource(const DOMStringAtom &id) const noexcept
  {
    if (id.View().empty())
    {
      return false;
    }

    return GetSVGResourcesMap().PendingResources.contains(id);
  }

  bool DocumentOrShadowRoot::IsElementWithPendingSVGResources(svg::SVGElement &element) const noexcept
  {
    // This algorithm takes time proportional to the number of pending resources and need not.
    // If performance becomes an issue we can keep a counted set of elements and answer the question
    // efficiently.
    return std::ranges::any_of(GetSVGResourcesMap().PendingResources,
                               [&](const auto &entry) { return SetContains(entry.second, element); });
  }

  bool DocumentOrShadowRoot::IsPendingSVGResource(svg::SVGElement &element,
                                                  const DOMStringAtom &id) const noexcept
  {
    if (id.View().empty())
    {
      return false;
    }

    auto &pendingResources = GetSVGResourcesMap().PendingResources;
    auto it = pendingResources.find(id);
    if (it == pendingResources.end())
    {
      return false;
    }

    return SetContains(it->second, element);
  }

  void DocumentOrShadowRoot::ClearHasPendingSVGResourcesIfPossible(svg::SVGElement &element) noexcept
  {
    if (!IsElementWithPendingSVGResources(element))
    {
      element.ClearHasPendingResources();
    }
  }

  void DocumentOrShadowRoot::RemoveElementFromPendingSVGResources(svg::SVGElement &element) noexcept
  {
    auto &map = GetSVGResourcesMap();
    if (!map.PendingResources.empty() && element.HasPendingResources())
    {
      List<DOMStringAtom> toBeRemoved;
      for (auto &[id, elements] : map.PendingResources)
      {
        RemoveFromSet(elements, element);
        if (IsEmptyIgnoringNullReferences(elements))
        {
          toBeRemoved.push_back(id);
        }
      }

      ClearHasPendingSVGResourcesIfPossible(element);

      // We use RemovePendingSVGResource here because it deals with set lifetime correctly.
      for (auto &id : toBeRemoved)
      {
        std::ignore = RemovePendingSVGResource(id);
      }
    }

    if (!map.PendingResourcesForRemoval.empty())
    {
      List<DOMStringAtom> toBeRemoved;
      for (auto &[id, elements] : map.PendingResourcesForRemoval)
      {
        RemoveFromSet(elements, element);
        if (IsEmptyIgnoringNullReferences(elements))
        {
          toBeRemoved.push_back(id);
        }
      }

      // We erase from PendingResourcesForRemoval here because it deals with set lifetime correctly.
      for (auto &id : toBeRemoved)
      {
        map.PendingResourcesForRemoval.erase(id);
      }
    }
  }

  List<WeakPtr<svg::SVGElement>>
    DocumentOrShadowRoot::RemovePendingSVGResource(const DOMStringAtom &id) noexcept
  {
    auto &pendingResources = GetSVGResourcesMap().PendingResources;
    auto node = pendingResources.extract(id);
    return node ? krys::move(node.mapped()) : WeakSVGElementSet {};
  }

  void DocumentOrShadowRoot::MarkPendingSVGResourcesForRemoval(const DOMStringAtom &id) noexcept
  {
    if (id.View().empty())
    {
      return;
    }

    krys_debug_assert(!GetSVGResourcesMap().PendingResourcesForRemoval.contains(id));

    auto existing = RemovePendingSVGResource(id);
    if (!IsEmptyIgnoringNullReferences(existing))
    {
      GetSVGResourcesMap().PendingResourcesForRemoval.emplace(id, krys::move(existing));
    }
  }

  RefPtr<svg::SVGElement>
    DocumentOrShadowRoot::TakeElementFromPendingSVGResourcesForRemovalMap(const DOMStringAtom &id) noexcept
  {
    if (id.View().empty())
    {
      return nullptr;
    }

    auto &pendingResourcesForRemoval = GetSVGResourcesMap().PendingResourcesForRemoval;
    auto it = pendingResourcesForRemoval.find(id);
    if (it == pendingResourcesForRemoval.end())
    {
      return nullptr;
    }

    auto &resourceSet = it->second;
    std::erase_if(resourceSet, [](const WeakPtr<svg::SVGElement> &weakElement) { return !weakElement; });
    if (resourceSet.empty())
    {
      return nullptr;
    }
    RefPtr<svg::SVGElement> firstElement = ShareRefPtr(resourceSet.front().get());

    RemoveFromSet(resourceSet, *firstElement);

    if (IsEmptyIgnoringNullReferences(resourceSet))
    {
      pendingResourcesForRemoval.erase(it);
    }

    return firstElement;
  }

  DocumentOrShadowRoot::DocumentOrShadowRoot(ContainerNode &rootNode,
                                             RefPtr<html::CustomElementRegistry> &&registry) noexcept
      : _rootNode(rootNode), _customElementRegistry(krys::move(registry)),
        _idTargetObserverRegistry(CreateUnique<dom::IdTargetObserverRegistry>()),
        _radioButtonGroups(CreateUnique<html::RadioButtonGroups>())
  {
  }

  DocumentOrShadowRoot::~DocumentOrShadowRoot() noexcept = default;

  namespace
  {
    KRYS_NODISCARD bool ElementHasId(const Element &element, DOMStringView id) noexcept
    {
      return element.Id() == id;
    }

    KRYS_NODISCARD bool ElementIsImageMapNamed(const Element &element, DOMStringView name) noexcept
    {
      auto *map = DynamicDowncast<html::HTMLMapElement>(element);
      return map && (map->Id() == name || map->Name() == name);
    }
  }

  void DocumentOrShadowRoot::AddElementById(DOMStringAtom id, Element &element) noexcept
  {
    if (!_elementsById)
    {
      _elementsById = CreateUnique<TreeScopeOrderedMap>();
    }
    _elementsById->Add(id, element);
  }

  void DocumentOrShadowRoot::RemoveElementById(DOMStringAtom id, Element &element) noexcept
  {
    krys_debug_assert(_elementsById);
    if (_elementsById)
    {
      _elementsById->Remove(id, element);
    }
  }

  void DocumentOrShadowRoot::NotifyIdTargetObservers(Element &element, DOMStringAtom id) noexcept
  {
    IdTargetObserverRegistry().NotifyObservers(element, id);

    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(RootNode());
        shadowRoot && shadowRoot->ReferenceTarget() && id.View() == *shadowRoot->ReferenceTarget())
    {
      shadowRoot->NotifyHostIdTargetObservers();
    }
  }

  void DocumentOrShadowRoot::ElementsWithIdChangedOrder(DOMStringAtom id) noexcept
  {
    if (_elementsById)
    {
      _elementsById->OrderChanged(id);
    }
  }

  uint32 DocumentOrShadowRoot::ElementCountWithId(DOMStringView id) const noexcept
  {
    return _elementsById && !id.empty() ? _elementsById->Count(DOMStringAtom(id)) : 0;
  }

  Element *DocumentOrShadowRoot::FirstElementWithId(DOMStringView id) const noexcept
  {
    if (!_elementsById || id.empty())
    {
      return nullptr;
    }
    return _elementsById->First(DOMStringAtom(id), _rootNode, ElementHasId);
  }

  void DocumentOrShadowRoot::AddImageMap(DOMStringAtom name, html::HTMLMapElement &map) noexcept
  {
    if (!_imageMapsByName)
    {
      _imageMapsByName = CreateUnique<TreeScopeOrderedMap>();
    }
    _imageMapsByName->Add(name, map);
  }

  void DocumentOrShadowRoot::RemoveImageMap(DOMStringAtom name, html::HTMLMapElement &map) noexcept
  {
    krys_debug_assert(_imageMapsByName);
    if (_imageMapsByName)
    {
      _imageMapsByName->Remove(name, map);
    }
  }

  void DocumentOrShadowRoot::ImageMapsChangedOrder(DOMStringAtom name) noexcept
  {
    if (_imageMapsByName)
    {
      _imageMapsByName->OrderChanged(name);
    }
  }

  html::HTMLMapElement *DocumentOrShadowRoot::FirstImageMap(DOMStringView name) const noexcept
  {
    if (!_imageMapsByName || name.empty())
    {
      return nullptr;
    }
    return static_cast<html::HTMLMapElement *>(
      _imageMapsByName->First(DOMStringAtom(name), _rootNode, ElementIsImageMapNamed));
  }

  Span<Element *const> DocumentOrShadowRoot::AllElementsWithId(DOMStringView id) const noexcept
  {
    if (!_elementsById || id.empty())
    {
      return {};
    }
    return _elementsById->All(DOMStringAtom(id), _rootNode, ElementHasId);
  }

#pragma region DocumentOrShadowRoot Mixin (DOM)

  RefPtr<html::CustomElementRegistry> DocumentOrShadowRoot::CustomElementRegistry() const noexcept
  {
    // 1. If this is a document, then return this's custom element registry.
    // 2. Assert: this is a ShadowRoot node.
    // 3. Return this's custom element registry.
    // Documents and shadow roots keep their registry here alike.
    return _customElementRegistry;
  }

  void DocumentOrShadowRoot::SetCustomElementRegistry(RefPtr<html::CustomElementRegistry> &&registry) noexcept
  {
    _customElementRegistry = krys::move(registry);
  }

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (HTML)

  RefPtr<Element> DocumentOrShadowRoot::PointerLockElement() const noexcept
  {
    auto &document = _rootNode.NodeDocument();
    auto *page = document.GetPage();
    if (!page || page->GetPointerLockController().LockPending())
    {
      return nullptr;
    }
    auto *element = page->GetPointerLockController().GetElement();
    if (!element || &element->NodeDocument() != &document)
    {
      return nullptr;
    }

    // The element retargeted against this, if that's in this tree.
    auto *retargeted = DynamicDowncast<Node>(EventTargetAlgorithms::Retarget(element, _rootNode));
    if (!retargeted || &TreeQueries::Root(*retargeted) != &_rootNode)
    {
      return nullptr;
    }
    return ShareRefPtr(DynamicDowncast<Element>(*retargeted));
  }

  // https://html.spec.whatwg.org/#dom-documentorshadowroot-activeelement
  RefPtr<Element> DocumentOrShadowRoot::ActiveElement() const noexcept
  {
    // 1. Let candidate be this's node document's focused area's DOM anchor.
    auto &document = _rootNode.NodeDocument();
    Node *candidate = document.FocusedElement();
    if (!candidate)
    {
      candidate = &document;
    }

    // 2. Set candidate to the result of retargeting candidate against this.
    auto *retargeted = DynamicDowncast<Node>(EventTargetAlgorithms::Retarget(candidate, _rootNode));
    if (!retargeted)
    {
      return nullptr;
    }

    // 3. If candidate's root is not this, then return null.
    if (&TreeQueries::Root(*retargeted) != &_rootNode)
    {
      return nullptr;
    }

    // 4. If candidate is not a Document object, then return candidate.
    if (!Is<Document>(*retargeted))
    {
      return ShareRefPtr(Downcast<Element>(retargeted));
    }

    // 5. If candidate has a body element, then return that body element.
    if (auto body = document.Body())
    {
      return body;
    }

    // 6. If candidate's document element is non-null, then return that document element.
    return document.DocumentElement();
  }

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (CSSOM)

  css::CSSStyleSheetObservableArray &DocumentOrShadowRoot::AdoptedStyleSheets() noexcept
  {
    if (!_adoptedStyleSheets) KRYS_UNLIKELY
    {
      _adoptedStyleSheets = CreateUnique<css::CSSStyleSheetObservableArray>(_rootNode);
    }
    return *_adoptedStyleSheets;
  }

  ExceptionOr<void>
    DocumentOrShadowRoot::SetAdoptedStyleSheets(List<Ref<css::CSSStyleSheet>> &&sheets) noexcept
  {
    if (!_adoptedStyleSheets && sheets.empty())
    {
      return {};
    }
    return AdoptedStyleSheets().SetSheets(krys::move(sheets));
  }

#pragma endregion

  Span<const Ref<css::CSSStyleSheet>> DocumentOrShadowRoot::GetAdoptedStyleSheets() const noexcept
  {
    if (!_adoptedStyleSheets)
    {
      return {};
    }
    return _adoptedStyleSheets->Sheets();
  }

  namespace
  {
    KRYS_NODISCARD Maybe<LayoutPoint> AbsolutePointIfNotClipped(Document &document,
                                                                const LayoutPoint &clientPoint) noexcept
    {
      if (!document.Frame() || !document.View())
      {
        return null;
      }

      // FIXME: The visual viewport (clientCoordinatesRelativeToLayoutViewport) comes with visual viewports.

      auto *frame = document.Frame();
      auto *view = document.View();
      float scaleFactor = frame->PageZoomFactor() * frame->FrameScaleFactor();

      LayoutPoint absolutePoint = clientPoint;
      absolutePoint.Scale(scaleFactor);
      absolutePoint.MoveBy(view->ContentsScrollPosition());

      LayoutRect visibleRect = view->VisibleContentRect();
      if (visibleRect.Contains(absolutePoint))
      {
        return absolutePoint;
      }
      return null;
    }

    /// @brief @p node retargeted against @p scope, or null if it isn't a node.
    KRYS_NODISCARD RefPtr<Node> RetargetToScope(Node &node, ContainerNode &scope) noexcept
    {
      return ShareRefPtr(DynamicDowncast<Node>(EventTargetAlgorithms::Retarget(&node, scope)));
    }
  }

  RefPtr<Node> DocumentOrShadowRoot::NodeFromPoint(const LayoutPoint &clientPoint, LayoutPoint *localPoint,
                                                   HitTestSource source) noexcept
  {
    Ref document = ShareRef(_rootNode.NodeDocument());
    auto absolutePoint = AbsolutePointIfNotClipped(*document, clientPoint);
    if (!absolutePoint)
    {
      return nullptr;
    }

    HitTestResult result(absolutePoint.value());
    document->HitTest(HitTestRequest(source, HitTestRequest::DefaultTypes), result);
    if (localPoint)
    {
      *localPoint = result.LocalPoint();
    }
    return ShareRefPtr(result.InnerNode());
  }

  RefPtr<Element> DocumentOrShadowRoot::ElementFromPoint(double clientX, double clientY,
                                                         HitTestSource source) noexcept
  {
    if (!_rootNode.NodeDocument().HasLivingRenderTree())
    {
      return nullptr;
    }

    auto node = NodeFromPoint(LayoutPoint(clientX, clientY), nullptr, source);
    if (!node)
    {
      return nullptr;
    }

    node = RetargetToScope(*node, _rootNode);
    while (node && !Is<Element>(*node))
    {
      node = ShareRefPtr(node->ParentInComposedTree());
      if (!node)
      {
        break;
      }
      node = RetargetToScope(*node, _rootNode);
    }

    return DynamicDowncast<Element>(krys::move(node));
  }

  List<RefPtr<Element>> DocumentOrShadowRoot::ElementsFromPoint(double clientX, double clientY,
                                                                HitTestSource source) noexcept
  {
    List<RefPtr<Element>> elements;

    Ref document = ShareRef(_rootNode.NodeDocument());
    if (!document->HasLivingRenderTree())
    {
      return elements;
    }

    auto absolutePoint = AbsolutePointIfNotClipped(*document, LayoutPoint(clientX, clientY));
    if (!absolutePoint)
    {
      return elements;
    }

    static constexpr EnumSet<HitTestRequest::Type> hitTypes {
      HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
      HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::CollectMultipleElements,
      HitTestRequest::Type::IncludeAllElementsUnderPoint};

    HitTestResult result {absolutePoint.value()};
    document->HitTest(HitTestRequest(source, hitTypes), result);

    RefPtr<Node> lastNode;
    auto &nodeSet = result.ListBasedTestResult();
    elements.reserve(nodeSet.size());
    for (auto &listBasedNode : nodeSet)
    {
      RefPtr<Node> node = RetargetToScope(*listBasedNode, _rootNode);
      while (node && !Is<Element>(*node))
      {
        node = ShareRefPtr(node->ParentInComposedTree());
        if (!node)
        {
          break;
        }
        node = RetargetToScope(*node, _rootNode);
      }

      if (!node)
      {
        continue;
      }

      if (auto *pseudoElement = DynamicDowncast<PseudoElement>(*node))
      {
        node = ShareRefPtr(pseudoElement->HostElement());
      }

      // Prune duplicate entries. A pseudo ::before content above its parent
      // node should only result in one entry.
      if (node == lastNode)
      {
        continue;
      }

      elements.push_back(DynamicDowncast<Element>(node));
      lastNode = node;
    }

    if (auto *rootDocument = DynamicDowncast<Document>(_rootNode))
    {
      if (RefPtr rootElement = rootDocument->DocumentElement())
      {
        if (elements.empty() || elements.back() != rootElement)
        {
          elements.push_back(rootElement);
        }
      }
    }

    return elements;
  }
}
