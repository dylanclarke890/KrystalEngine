/*
 * Copyright (C) 2008-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityListBoxOption.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMathMLElement.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuList.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuListOption.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuListPopup.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityProgressIndicator.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityScrollbar.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityScrollView.hpp"
#include "Krystal.Booey/Accessibility/AccessibilitySlider.hpp"
#include "Krystal.Booey/Accessibility/AccessibilitySpinButton.hpp"
#include "Krystal.Booey/Accessibility/AccessibilitySpinButtonPart.hpp"
#include "Krystal.Booey/Accessibility/AccessibilitySVGObject.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityTableColumn.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityTableHeaderContainer.hpp"
#include "Krystal.Booey/Accessibility/AXTableHelpers.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/ElementInternals.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLabelElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLElement.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMenuList.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMeter.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderProgress.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderSlider.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <utility>

namespace krys::boo::ax
{
  namespace
  {
    bool gAccessibilityEnabled = false;

    KRYS_NODISCARD bool IsHTMLElementNamed(const dom::Node &node, dom::DOMStringView localName) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      return element && Is<html::HTMLElement>(*element) && element->LocalName() == localName;
    }

    /// @brief Whether @p node is inside a canvas's fallback content (but not a user agent shadow tree).
    KRYS_NODISCARD bool IsInCanvasSubtree(const dom::Element &composedParent) noexcept
    {
      for (const dom::Node *ancestor = &composedParent; ancestor; ancestor = ancestor->ParentInComposedTree())
      {
        if (IsHTMLElementNamed(*ancestor, u8"canvas"))
        {
          // Descendants of the user agent shadow trees inside the canvas (such as an input's inner editor)
          // are left to their host.
          auto shadowRoot = composedParent.ShadowRoot();
          return !shadowRoot || shadowRoot->Mode() != dom::ShadowRootMode::UserAgent;
        }
      }
      return false;
    }

    KRYS_NODISCARD bool NodeRendererIsValid(const dom::Node &node) noexcept
    {
      auto *renderer = node.Renderer();
      if (!renderer)
      {
        return false;
      }
      // A node's renderer may still be one it no longer uses while it's being torn down.
      return !renderer->BeingDestroyed();
    }

    /// @brief Whether @p renderer is out of date (laid out or styled again soon), so a change to it is better
    /// handled once it's been brought up to date.
    KRYS_NODISCARD bool RendererNeedsDeferredUpdate(const RenderObject &renderer) noexcept
    {
      auto &document = renderer.GetDocument();
      auto *view = document.View();
      return renderer.NeedsLayout() || document.NeedsStyleRecalc() || document.InRenderTreeUpdate()
             || (view && view->LayoutContext().IsInRenderTreeLayout());
    }

    /// @brief Whether @p element is part of an HTML table (a row, cell, section, column or caption).
    KRYS_NODISCARD bool IsHTMLTablePart(const dom::Element &element) noexcept
    {
      for (auto localName :
           {u8"tr", u8"td", u8"th", u8"thead", u8"tbody", u8"tfoot", u8"col", u8"colgroup", u8"caption"})
      {
        if (IsHTMLElementNamed(element, localName))
        {
          return true;
        }
      }
      return false;
    }

    /// @brief Whether @p element is named by ARIA (which takes precedence over a label element).
    KRYS_NODISCARD bool HasAnyARIALabelling(const dom::Element &element) noexcept
    {
      return element.HasAttribute(dom::DOMStringAtom(u8"aria-label"))
             || element.HasAttribute(dom::DOMStringAtom(u8"aria-labelledby"));
    }

    /// @brief A label's control, unless it's a descendant of the label that's being torn down.
    KRYS_NODISCARD RefPtr<html::HTMLElement>
      ControlForLabelElement(const html::HTMLLabelElement &label) noexcept
    {
      auto control = label.Control();
      if (!control || (control->Renderer() && !control->Renderer()->Parent()))
      {
        return nullptr;
      }
      return control;
    }

    /// @brief The attributes that relate an element to others.
    constexpr dom::DOMStringView RelationAttributes[] = {
      u8"aria-activedescendant",
      u8"aria-controls",
      u8"aria-describedby",
      u8"aria-details",
      u8"aria-errormessage",
      u8"aria-flowto",
      u8"aria-labelledby",
      u8"aria-owns",
      u8"commandfor",
      u8"headers",
      u8"popovertarget",
    };

    KRYS_NODISCARD bool HasAnyRole(const dom::Element &element,
                                   std::initializer_list<dom::DOMStringView> roles) noexcept
    {
      for (auto role : roles)
      {
        if (table_helpers::HasRole(element, role))
        {
          return true;
        }
      }
      return false;
    }
  }

  AXObjectCache::AXObjectCache(dom::Document &document) noexcept
      : _document(document), _notificationPostTimer(*this, &AXObjectCache::NotificationPostTimerFired),
        _performCacheUpdateTimer(*this, &AXObjectCache::PerformCacheUpdateTimerFired)
  {
  }

  AXObjectCache::~AXObjectCache() noexcept
  {
    _notificationPostTimer.Stop();
    _performCacheUpdateTimer.Stop();

    // Moved out: detaching an object can remove the objects it made (a table's columns) from the cache.
    auto objects = std::exchange(_objects, {});
    for (const auto &[id, object] : objects)
    {
      object->Detach(AccessibilityDetachmentType::CacheDestroyed);
    }
  }

  void AXObjectCache::EnableAccessibility() noexcept
  {
    gAccessibilityEnabled = true;
  }

  void AXObjectCache::DisableAccessibility() noexcept
  {
    gAccessibilityEnabled = false;
  }

  bool AXObjectCache::AccessibilityEnabled() noexcept
  {
    return gAccessibilityEnabled;
  }

  AccessibilityObject *AXObjectCache::RootObject() noexcept
  {
    if (!gAccessibilityEnabled)
    {
      return nullptr;
    }
    if (auto *view = _document.View())
    {
      return GetOrCreate(*view);
    }
    return GetOrCreate(_document.GetRenderView());
  }

  AccessibilityObject *AXObjectCache::GetOrCreate(LocalFrameView &view) noexcept
  {
    if (_scrollViewID)
    {
      auto *existing = DynamicDowncast<AccessibilityScrollView>(ObjectForID(*_scrollViewID));
      if (existing && existing->ScrollView() == &view)
      {
        return existing;
      }
      // A view that's gone, or another view the document has now.
      Remove(*_scrollViewID);
    }
    auto object = AccessibilityScrollView::Create(GenerateID(), view, *this);
    CacheAndInitializeWrapper(*object, nullptr, nullptr);
    _scrollViewID = object->ObjectID();
    return object.get();
  }

  AccessibilityObject *AXObjectCache::Create(Scrollbar &scrollbar) noexcept
  {
    auto object = AccessibilityScrollbar::Create(GenerateID(), scrollbar, *this);
    CacheAndInitializeWrapper(*object, nullptr, nullptr);
    return object.get();
  }

  AccessibilityObject *AXObjectCache::FocusedObject() noexcept
  {
    if (!gAccessibilityEnabled)
    {
      return nullptr;
    }

    _document.UpdateStyleIfNeeded();
    if (auto *focusedElement = _document.FocusedElement())
    {
      return FocusedObjectForNode(*focusedElement);
    }
    return FocusedObjectForNode(_document);
  }

  AccessibilityObject *AXObjectCache::FocusedImageMapUIElement(html::HTMLAreaElement &area) noexcept
  {
    auto image = area.ImageElement();
    auto *imageObject = image ? GetOrCreate(*image) : nullptr;
    if (!imageObject)
    {
      return nullptr;
    }
    for (const auto &child : imageObject->Children())
    {
      if (child->IsImageMapLink() && child->GetNode() == &area)
      {
        return child.get();
      }
    }
    return nullptr;
  }

  AccessibilityObject *AXObjectCache::FocusedObjectForNode(dom::Node &node) noexcept
  {
    // A focused area's object is its image's child.
    if (auto *area = DynamicDowncast<html::HTMLAreaElement>(node))
    {
      return FocusedImageMapUIElement(*area);
    }
    auto *focus = GetOrCreate(node);
    if (!focus)
    {
      return nullptr;
    }
    // The focus is on the active descendant of an object that has one.
    if (focus->ShouldFocusActiveDescendant())
    {
      if (auto *descendant = focus->ActiveDescendant())
      {
        return descendant;
      }
    }
    if (focus->IsIgnored())
    {
      return focus->ParentObjectUnignored();
    }
    return focus;
  }

  AccessibilityObject *AXObjectCache::Get(const dom::Node &node) const noexcept
  {
    if (auto *document = DynamicDowncast<dom::Document>(node)) KRYS_UNLIKELY
    {
      auto *renderView = document->GetRenderView();
      return renderView ? Get(*renderView) : nullptr;
    }
    auto found = _nodeIdMapping.find(&node);
    return found == _nodeIdMapping.end() ? nullptr : ObjectForID(found->second);
  }

  AccessibilityObject *AXObjectCache::Get(const RenderObject &renderer) const noexcept
  {
    if (auto *node = renderer.GetNode())
    {
      auto found = _nodeIdMapping.find(node);
      return found == _nodeIdMapping.end() ? nullptr : ObjectForID(found->second);
    }
    auto found = _renderObjectIdMapping.find(&renderer);
    return found == _renderObjectIdMapping.end() ? nullptr : ObjectForID(found->second);
  }

  AccessibilityObject *AXObjectCache::ObjectForID(AXID id) const noexcept
  {
    auto found = _objects.find(id);
    return found == _objects.end() ? nullptr : found->second.get();
  }

  Ref<AccessibilityRenderObject> AXObjectCache::CreateObjectFromRenderer(RenderObject &renderer) noexcept
  {
    auto *node = renderer.GetNode();
    if (renderer.IsLegacyRenderSVGRoot())
    {
      return AccessibilitySVGObject::Create(GenerateID(), renderer, *this, true);
    }
    if (Is<svg::SVGElement>(node) || renderer.IsRenderSVGInlineText())
    {
      return AccessibilitySVGObject::Create(GenerateID(), renderer, *this);
    }
    if (auto *element = DynamicDowncast<dom::Element>(node); element && Is<mathml::MathMLElement>(*element))
    {
      return AccessibilityMathMLElement::Create(GenerateID(), renderer, *this);
    }
    if (auto *menuList = DynamicDowncast<RenderMenuList>(renderer))
    {
      return AccessibilityMenuList::Create(GenerateID(), *menuList, *this);
    }
    if (Is<RenderProgress>(renderer) || Is<RenderMeter>(renderer) || Is<html::HTMLProgressElement>(node)
        || Is<html::HTMLMeterElement>(node))
    {
      return AccessibilityProgressIndicator::Create(GenerateID(), renderer, *this);
    }
    if (Is<RenderSlider>(renderer))
    {
      return AccessibilitySlider::Create(GenerateID(), renderer, *this);
    }
    return AccessibilityRenderObject::Create(GenerateID(), renderer, *this);
  }

  Ref<AccessibilityNodeObject> AXObjectCache::CreateFromNode(dom::Node &node) noexcept
  {
    if (auto *element = DynamicDowncast<dom::Element>(node);
        element && (Is<html::HTMLProgressElement>(*element) || Is<html::HTMLMeterElement>(*element)))
    {
      return AccessibilityProgressIndicator::Create(GenerateID(), *element, *this);
    }
    if (auto *element = DynamicDowncast<dom::Element>(node); element && Is<svg::SVGElement>(*element))
    {
      return AccessibilitySVGObject::Create(GenerateID(), *element, *this);
    }
    return AccessibilityRenderObject::Create(GenerateID(), node, *this);
  }

  AccessibilityObject *AXObjectCache::Create(AccessibilityRole role) noexcept
  {
    RefPtr<AccessibilityObject> object;
    switch (role)
    {
      case AccessibilityRole::SliderThumb:
        object = AccessibilitySliderThumb::Create(GenerateID(), *this);
        break;
      case AccessibilityRole::MenuListPopup:
        object = AccessibilityMenuListPopup::Create(GenerateID(), *this);
        break;
      case AccessibilityRole::Column: object = AccessibilityTableColumn::Create(GenerateID(), *this); break;
      case AccessibilityRole::TableHeaderContainer:
        object = AccessibilityTableHeaderContainer::Create(GenerateID(), *this);
        break;
      case AccessibilityRole::SpinButton:
        object = AccessibilitySpinButton::Create(GenerateID(), *this);
        break;
      case AccessibilityRole::SpinButtonPart:
        object = AccessibilitySpinButtonPart::Create(GenerateID(), *this);
        break;
      default: break;
    }
    if (!object)
    {
      return nullptr;
    }
    CacheAndInitializeWrapper(*object, nullptr, nullptr);
    return object.get();
  }

  void AXObjectCache::CacheAndInitializeWrapper(AccessibilityObject &object, const dom::Node *node,
                                                const RenderObject *renderer) noexcept
  {
    auto id = object.ObjectID();
    if (node)
    {
      _nodeIdMapping.insert_or_assign(node, id);
    }
    else if (renderer)
    {
      _renderObjectIdMapping.insert_or_assign(renderer, id);
    }

    _objects.insert_or_assign(id, ShareRef(object));
    object.Init();
  }

  AccessibilityObject *AXObjectCache::GetOrCreate(dom::Node &node, IsPartOfRelation isPartOfRelation) noexcept
  {
    if (auto *object = Get(node))
    {
      return object;
    }
    return GetOrCreateSlow(node, isPartOfRelation);
  }

  AccessibilityObject *AXObjectCache::GetOrCreateSlow(dom::Node &node,
                                                      IsPartOfRelation isPartOfRelation) noexcept
  {
    krys_debug_assert(&node.NodeDocument() == &_document);

    // An option or option group is its select's, rendered or not: a list box's child, or (for an option) a
    // child of a menu list's pop-up, as long as the menu list is rendered.
    auto *option = DynamicDowncast<html::HTMLOptionElement>(node);
    auto *group = DynamicDowncast<html::HTMLOptGroupElement>(node);
    if (option || group)
    {
      auto *select = option ? option->OwnerSelectElement() : group->OwnerSelectElement();
      if (!select)
      {
        return nullptr;
      }
      RefPtr<AccessibilityObject> object;
      if (select->UsesMenuList())
      {
        if (!option || !select->Renderer())
        {
          return nullptr;
        }
        object = AccessibilityMenuListOption::Create(GenerateID(), *option, *this);
      }
      else
      {
        object = AccessibilityListBoxOption::Create(GenerateID(), Downcast<html::HTMLElement>(node), *this);
      }
      CacheAndInitializeWrapper(*object, &node, nullptr);
      return object.get();
    }

    if (NodeRendererIsValid(node))
    {
      return GetOrCreate(*node.Renderer());
    }

    if (auto *document = DynamicDowncast<dom::Document>(node)) KRYS_UNLIKELY
    {
      return GetOrCreate(document->GetRenderView());
    }

    auto *composedParent = node.ParentElementInComposedTree();
    if (!composedParent)
    {
      return nullptr;
    }

    bool inCanvasSubtree = IsInCanvasSubtree(*composedParent);
    // A relation's target, or a descendant of one, needs an object whatever it is.
    if (isPartOfRelation == IsPartOfRelation::No && !IsDescendantOfRelatedNode(node))
    {
      bool insideMeterElement = IsHTMLElementNamed(*composedParent, u8"meter");
      auto *element = DynamicDowncast<dom::Element>(node);
      bool hasDisplayContents = element && element->HasDisplayContents();
      bool isPopover = element && Is<html::HTMLElement>(*element) && element->HasAttribute(u8"popover");
      bool isFocused = element && _document.FocusedElement() == element;
      bool isAreaElement = IsHTMLElementNamed(node, u8"area");
      if (!inCanvasSubtree && !insideMeterElement && !hasDisplayContents && !isPopover && !isFocused
          && !isAreaElement)
      {
        return nullptr;
      }
    }

    // Fallback content is only focusable as long as the canvas is displayed and visible. Update the style
    // before focusability gets checked.
    if (inCanvasSubtree)
    {
      _document.UpdateStyleIfNeeded();
    }

    auto object = CreateFromNode(node);
    krys_debug_assert(!Get(node));
    CacheAndInitializeWrapper(*object, &node, nullptr);
    // Compute the object's initial ignored state.
    object->RecomputeIsIgnored();
    // Asking whether it's ignored can detach it.
    if (object->IsDetached())
    {
      return nullptr;
    }
    return object.get();
  }

  AccessibilityObject *AXObjectCache::GetOrCreate(RenderObject &renderer) noexcept
  {
    if (auto *object = Get(renderer))
    {
      return object;
    }

    // Don't create an object for this renderer if it's being destroyed.
    if (renderer.BeingDestroyed())
    {
      return nullptr;
    }

    auto object = CreateObjectFromRenderer(renderer);
    krys_debug_assert(!Get(renderer));
    CacheAndInitializeWrapper(*object, renderer.GetNode(), &renderer);
    // Compute the object's initial ignored state.
    object->RecomputeIsIgnored();
    // Asking whether it's ignored can detach it.
    if (object->IsDetached())
    {
      return nullptr;
    }
    return object.get();
  }

  void AXObjectCache::Remove(AXID id) noexcept
  {
    StopCachingComputedObjectAttributes();
    auto found = _objects.find(id);
    if (found == _objects.end())
    {
      return;
    }
    auto object = krys::move(found->second);
    _objects.erase(found);

    RemoveAllRelations(id);
    object->Detach(AccessibilityDetachmentType::ElementDestroyed);
    if (_scrollViewID == id)
    {
      _scrollViewID = null;
    }
    _deferredChildrenChangedList.remove(id);
    _changedLiveRegions.remove(id);
    _deferredRendererChangedList.remove(id);
    _deferredRecomputeTableIsExposedList.remove(id);
    _deferredRecomputeTableCellSlotsList.remove(id);
  }

  void AXObjectCache::Remove(RenderObject &renderer) noexcept
  {
    if (auto *node = renderer.GetNode())
    {
      // Objects with nodes are removed when their node is, not their renderer.
      if (auto *object = DynamicDowncast<AccessibilityRenderObject>(Get(*node)))
      {
        if (object->SetRendererIfNeeded(nullptr))
        {
          // The object still needs updating for the renderer it lost.
          _deferredRendererChangedList.add(object->ObjectID());
        }
      }
      return;
    }

    auto found = _renderObjectIdMapping.find(&renderer);
    if (found == _renderObjectIdMapping.end())
    {
      return;
    }
    auto id = found->second;
    _renderObjectIdMapping.erase(found);
    Remove(id);
  }

  void AXObjectCache::Remove(dom::Node &node) noexcept
  {
    auto found = _nodeIdMapping.find(&node);
    if (found != _nodeIdMapping.end())
    {
      auto id = found->second;
      _nodeIdMapping.erase(found);
      Remove(id);
    }

    if (auto *renderer = node.Renderer())
    {
      Remove(*renderer);
    }

    _deferredTextChangedList.remove(&node);
    if (auto *element = DynamicDowncast<dom::Element>(node))
    {
      _deferredRecomputeIsIgnoredList.remove(element);
      _deferredSelectedChildrenChangedList.remove(element);
      _deferredElementAddedOrRemovedList.remove(element);
      _deferredModalChangedList.remove(element);
      std::erase(_modalElements, element);
      if (_currentModalElement == element)
      {
        _currentModalElement = nullptr;
      }
      std::erase_if(_deferredAttributeChanges,
                    [element](const auto &change) { return change.Element == element; });
    }
  }

  void AXObjectCache::ChildrenChanged(dom::Node &node) noexcept
  {
    // A rendered node's renderer reports its children changing.
    if (!node.Renderer())
    {
      ChildrenChanged(Get(node));
    }
  }

  void AXObjectCache::ChildrenChanged(RenderObject &renderer, RenderObject *newChild) noexcept
  {
    if (renderer.IsAnonymous())
    {
      // The tree is built from the DOM, so an anonymous renderer rarely has an object, and its subtree can
      // change with no other renderer reporting it: tell the nearest ancestor that has one.
      for (auto *ancestor = &renderer; ancestor; ancestor = ancestor->Parent())
      {
        if (auto *object = Get(*ancestor))
        {
          ChildrenChanged(object);
          break;
        }
      }
    }
    else
    {
      ChildrenChanged(Get(renderer));
    }

    if (newChild)
    {
      auto *node = newChild->GetNode();
      DeferElementAddedOrRemoved(DynamicDowncast<dom::Element>(node));
      // The display: contents elements between the child and its parent renderer's element have no renderers
      // of their own, so were added with it.
      for (auto *ancestor = node ? node->ParentElement() : nullptr;
           ancestor && ancestor != renderer.GetNode() && ancestor->HasDisplayContents();
           ancestor = ancestor->ParentElement())
      {
        DeferElementAddedOrRemoved(ancestor);
      }
    }
  }

  void AXObjectCache::DeferElementAddedOrRemoved(dom::Element *element) noexcept
  {
    if (!element)
    {
      return;
    }
    _deferredElementAddedOrRemovedList.add(element);
    if (IsModalElement(*element))
    {
      DeferModalChange(*element);
    }
    StartPerformCacheUpdateTimer();
  }

  bool AXObjectCache::IsModalElement(const dom::Element &element) noexcept
  {
    if ((table_helpers::HasRole(element, u8"dialog") || table_helpers::HasRole(element, u8"alertdialog"))
        && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(
          AccessibilityObject::AttributeWithDefaultARIA(element, u8"aria-modal"), u8"true"))
    {
      return true;
    }
    auto *dialog = DynamicDowncast<html::HTMLDialogElement>(element);
    return dialog && dialog->IsModal();
  }

  void AXObjectCache::FindModalNodes() noexcept
  {
    for (auto *node = dom::TreeQueries::Next(_document, nullptr); node;
         node = dom::TreeQueries::Next(*node, nullptr))
    {
      if (auto *element = DynamicDowncast<dom::Element>(node); element && IsModalElement(*element))
      {
        _modalElements.push_back(element);
      }
    }
    _modalNodesInitialized = true;
  }

  bool AXObjectCache::ModalElementHasAccessibleContent(dom::Element &element) noexcept
  {
    // Whether an object is ignored depends on the modal dialog, which is being worked out: the DOM is walked
    // for objects that are unignored in themselves.
    List<dom::Node *> nodeStack {element.FirstChild()};
    while (!nodeStack.empty())
    {
      auto *node = nodeStack.back();
      nodeStack.pop_back();
      for (; node; node = node->NextSibling())
      {
        if (auto *object = GetOrCreate(*node); object && !object->ComputeIsIgnored())
        {
          return true;
        }
        // Hidden subtrees have nothing to expose.
        if (IsNodeVisible(node))
        {
          nodeStack.push_back(node->FirstChild());
        }
      }
    }
    return false;
  }

  void AXObjectCache::UpdateCurrentModalNode() noexcept
  {
    auto recomputeModalElement = [&]() -> dom::Element *
    {
      if (_modalElements.empty())
      {
        return nullptr;
      }
      if (auto *activeModalDialog = _document.ActiveModalDialog())
      {
        return activeModalDialog;
      }

      SetForScope retrieving(_isRetrievingCurrentModalNode, true);
      // The deepest of the modal dialogs with the focus in it; there's none if the focus isn't in one.
      RefPtr<dom::Element> focusedElement = ShareRefPtr(_document.FocusedElement());
      dom::Element *modalElementToReturn = nullptr;
      bool foundModalWithFocusInside = false;
      for (auto *element : _modalElements)
      {
        // An element may have stopped being modal without the change being handled yet. Empty or hidden
        // dialogs are skipped, so the user isn't trapped in nothing.
        if (!IsModalElement(*element) || !IsNodeVisible(element)
            || !ModalElementHasAccessibleContent(*element))
        {
          continue;
        }
        bool focusIsInsideElement = focusedElement && focusedElement->IsInclusiveDescendantOf(*element);
        if (modalElementToReturn && foundModalWithFocusInside
            && (modalElementToReturn->IsDescendantOf(*element) || !focusIsInsideElement))
        {
          continue;
        }
        modalElementToReturn = element;
        foundModalWithFocusInside |= focusIsInsideElement;
      }
      if (!focusedElement || !foundModalWithFocusInside)
      {
        return nullptr;
      }
      auto *object = GetOrCreate(*modalElementToReturn);
      return object && !object->IsAXHidden() ? modalElementToReturn : nullptr;
    };

    auto *previousModal = _currentModalElement;
    _currentModalElement = recomputeModalElement();
    if (previousModal != _currentModalElement)
    {
      // Whether everything outside the dialog is ignored changed.
      ChildrenChanged(Get(_document));
    }
  }

  bool AXObjectCache::IsNodeVisible(const dom::Node *node) noexcept
  {
    auto *element = DynamicDowncast<dom::Element>(node);
    auto *renderer = element ? element->Renderer() : nullptr;
    if (!renderer)
    {
      return false;
    }
    const auto &style = renderer->Style();
    if (style.EffectiveDisplay() == DisplayType::None)
    {
      return false;
    }
    if (auto *layer = renderer->EnclosingLayer();
        style.UsedVisibility() != Visibility::Visible && layer && !layer->HasVisibleContent())
    {
      return false;
    }
    // An element is transparent if it or any of its ancestors is.
    for (auto *ancestor = renderer; ancestor; ancestor = ancestor->Parent())
    {
      if (ancestor->Style().GetOpacity().IsTransparent())
      {
        return false;
      }
    }
    return !infra::StringAlgorithms::ASCIICaseInsensitiveMatch(
             AccessibilityObject::AttributeWithDefaultARIA(*element, u8"aria-hidden"), u8"true")
           || element->NodeDocument().FocusedElement() == element;
  }

  dom::Node *AXObjectCache::ModalNode() noexcept
  {
    if (!_modalNodesInitialized)
    {
      FindModalNodes();
    }
    if (_modalElements.empty())
    {
      return nullptr;
    }
    // Usually the current dialog is still the one; it's worked out again when it's gone or hidden.
    if (!IsNodeVisible(_currentModalElement))
    {
      UpdateCurrentModalNode();
    }
    return _currentModalElement;
  }

  void AXObjectCache::DeferModalChange(dom::Element &element) noexcept
  {
    _deferredModalChangedList.add(&element);
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::HandleMenuOpened(dom::Element &element) noexcept
  {
    if (element.Renderer() && table_helpers::HasRole(element, u8"menu"))
    {
      PostNotification(GetOrCreate(element), AXNotification::MenuOpened);
    }
  }

  void AXObjectCache::HandleLiveRegionCreated(dom::Element &element) noexcept
  {
    // (A display: contents live region has no renderer, but is in the tree.)
    if (!element.Renderer() && !element.HasDisplayContents())
    {
      return;
    }
    auto status = AccessibilityObject::AttributeWithDefaultARIA(element, u8"aria-live");
    if (status.empty())
    {
      auto role = AccessibilityObject::AttributeWithDefaultARIA(element, u8"role");
      if (!role.empty())
      {
        status =
          AccessibilityObject::DefaultLiveRegionStatusForRole(AccessibilityObject::AriaRoleToRole(role));
      }
    }
    if (AccessibilityObject::LiveRegionStatusIsEnabled(status))
    {
      PostNotification(GetOrCreate(element), AXNotification::LiveRegionCreated);
    }
  }

  void AXObjectCache::HandleLabelChanged(AccessibilityObject *object) noexcept
  {
    if (!object)
    {
      return;
    }
    auto *element = object->GetElement();
    if (!element || !UpdateLabelFor(*element))
    {
      for (const auto &labeledObject : object->LabelForObjects())
      {
        PostNotification(labeledObject.get(), AXNotification::ValueChanged);
      }
    }
    PostNotification(object, AXNotification::LabelChanged);
  }

  void AXObjectCache::PostLiveRegionChangeNotification(AccessibilityObject &object) noexcept
  {
    // A region can change many times in one task (each child added, say): it's told once.
    _changedLiveRegions.add(object.ObjectID());
    if (!_notificationPostTimer.IsActive())
    {
      _notificationPostTimer.StartOneShot({});
    }
  }

  Maybe<bool> AXObjectCache::CachedIgnoredState(AXID id) const noexcept
  {
    if (!_computedIgnoredStates)
    {
      return null;
    }
    auto found = _computedIgnoredStates->find(id);
    return found != _computedIgnoredStates->end() ? Maybe<bool>(found->second) : null;
  }

  void AXObjectCache::CacheIgnoredState(AXID id, bool ignored) noexcept
  {
    if (_computedIgnoredStates)
    {
      (*_computedIgnoredStates)[id] = ignored;
    }
  }

  void AXObjectCache::ChildrenChanged(AccessibilityObject *object) noexcept
  {
    // The tree is changing: what's cached may be out of date.
    StopCachingComputedObjectAttributes();
    if (!object)
    {
      return;
    }
    _deferredChildrenChangedList.add(object->ObjectID());

    // Adding or removing rows can make a layout table a data table or the other way round.
    auto *element = object->GetElement();
    if (Is<html::HTMLTableElement>(element))
    {
      DeferRecomputeTableIsExposed(element);
    }
    else if (auto *section = DynamicDowncast<html::HTMLTableSectionElement>(element))
    {
      DeferRecomputeTableIsExposed(const_cast<html::HTMLTableElement *>(section->FindParentTable()));
    }
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::DeferRecomputeTableIsExposed(dom::Element *element) noexcept
  {
    // A table with no object yet works it out when it gets one.
    auto *table = DynamicDowncast<html::HTMLTableElement>(element);
    auto *object = table ? Get(*table) : nullptr;
    if (!object)
    {
      return;
    }
    _deferredRecomputeTableIsExposedList.add(object->ObjectID());
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::DeferRecomputeTableCellSlots(AccessibilityNodeObject &table) noexcept
  {
    _deferredRecomputeTableCellSlotsList.add(table.ObjectID());
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::StartPerformCacheUpdateTimer() noexcept
  {
    if (!_performCacheUpdateTimer.IsActive())
    {
      _performCacheUpdateTimer.StartOneShot({});
    }
  }

  void AXObjectCache::HandleChildrenChanged(AccessibilityObject &object) noexcept
  {
    // A menu list's children are its pop-up's.
    if (auto *menuList = DynamicDowncast<AccessibilityMenuList>(object))
    {
      HandleChildrenChanged(menuList->Popup());
    }
    else if (auto *popup = DynamicDowncast<AccessibilityMenuListPopup>(object))
    {
      popup->HandleChildrenChanged();
      return;
    }
    else if (auto *nodeObject = DynamicDowncast<AccessibilityNodeObject>(object);
             nodeObject && nodeObject->IsTable())
    {
      DeferRecomputeTableCellSlots(*nodeObject);
    }
    else if (auto *parentTable = DynamicDowncast<AccessibilityNodeObject>(
               nodeObject ? nodeObject->ParentTableIfExposedTableRow() : nullptr))
    {
      DeferRecomputeTableCellSlots(*parentTable);
    }
    if (!object.GetNode() && !object.Renderer())
    {
      return;
    }

    // Make the subtree dirty so everything below is updated.
    object.SetNeedsToUpdateSubtree();
    object.RecomputeIsIgnored();

    // Go up the existing ancestors, marking their children out of date and telling the ones the change
    // affects.
    bool shouldUpdateParent = true;
    bool foundTableCaption = false;
    for (auto *parent = &object; parent; parent = parent->ParentObject())
    {
      if (shouldUpdateParent)
      {
        parent->SetNeedsToUpdateChildren();
      }
      if (parent->SupportsLiveRegion())
      {
        PostLiveRegionChangeNotification(*parent);
      }
      if (parent->IsNonNativeTextControl())
      {
        // An editable region's value changed; the ancestors of what's being edited keep their children.
        PostNotification(parent, AXNotification::ValueChanged);
        shouldUpdateParent = false;
      }
      if (parent->Role() == AccessibilityRole::Label)
      {
        HandleLabelChanged(parent);
      }
      for (const auto &describedObject : parent->DescriptionForObjects())
      {
        PostNotification(describedObject.get(), AXNotification::ExtendedDescriptionChanged);
      }
      // A table's caption names it.
      if (auto *element = parent->GetElement(); element && IsHTMLElementNamed(*element, u8"caption"))
      {
        foundTableCaption = true;
      }
      else if (foundTableCaption && parent->IsTable())
      {
        PostNotification(parent, AXNotification::TextChanged);
        foundTableCaption = false;
      }
    }

    // A list's role depends on its children.
    if (auto *nodeObject = DynamicDowncast<AccessibilityNodeObject>(object);
        nodeObject && nodeObject->IsAccessibilityList())
    {
      object.UpdateRole();
    }
  }

  void AXObjectCache::HandleAllDeferredChildrenChanged() noexcept
  {
    // Handling a change can defer another (an object's ignored state changing), so repeat until none are
    // left.
    while (!_deferredChildrenChangedList.isEmpty())
    {
      auto deferred = std::exchange(_deferredChildrenChangedList, {});
      List<Ref<AccessibilityObject>> objects;
      for (auto id : deferred)
      {
        if (auto *object = ObjectForID(id))
        {
          objects.push_back(ShareRef(*object));
        }
      }
      for (const auto &object : objects)
      {
        HandleChildrenChanged(*object);
      }
      for (const auto &object : objects)
      {
        PostPlatformNotification(*object, AXNotification::ChildrenChanged);
      }
    }
  }

  void AXObjectCache::HandleRoleChanged(AccessibilityObject &object, AccessibilityRole previousRole) noexcept
  {
    StopCachingComputedObjectAttributes();
    // The new role may change whether the object is ignored and what its parent's children are.
    (void)previousRole;
    object.RecomputeIsIgnored();
    PostNotification(&object, AXNotification::RoleChanged);
  }

  void AXObjectCache::RecomputeIsIgnored(dom::Node &node) noexcept
  {
    if (auto *object = Get(node))
    {
      object->RecomputeIsIgnored();
    }
  }

  void AXObjectCache::RecomputeIsIgnored(RenderObject &renderer) noexcept
  {
    if (auto *object = Get(renderer))
    {
      object->RecomputeIsIgnored();
    }
  }

  void AXObjectCache::DeferRecomputeIsIgnored(dom::Element *element) noexcept
  {
    if (!element || !NodeRendererIsValid(*element))
    {
      return;
    }
    _deferredRecomputeIsIgnoredList.add(element);
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::DeferRecomputeIsIgnoredIfNeeded(dom::Element *element) noexcept
  {
    if (!element || !NodeRendererIsValid(*element))
    {
      return;
    }
    if (RendererNeedsDeferredUpdate(*element->Renderer()))
    {
      DeferRecomputeIsIgnored(element);
      return;
    }
    RecomputeIsIgnored(*element->Renderer());
  }

  void AXObjectCache::DeferAttributeChangeIfNeeded(dom::Element &element, dom::DOMStringAtom name,
                                                   Maybe<dom::DOMStringView> oldValue,
                                                   Maybe<dom::DOMStringView> newValue) noexcept
  {
    if (oldValue == newValue)
    {
      return;
    }
    // Attributes may refer to other elements by the ID.
    if (name.View() == u8"id")
    {
      RelationsNeedUpdate();
    }
    auto oldString = oldValue.value_or(dom::DOMStringView {});
    auto newString = newValue.value_or(dom::DOMStringView {});
    if (NodeRendererIsValid(element) && RendererNeedsDeferredUpdate(*element.Renderer()))
    {
      _deferredAttributeChanges.push_back(
        {&element, name, dom::DOMString(oldString), dom::DOMString(newString)});
      StartPerformCacheUpdateTimer();
      return;
    }
    Ref protectedElement = ShareRef(element);
    HandleAttributeChange(element, name.View(), oldString, newString);
  }

  bool AXObjectCache::ShouldProcessAttributeChange(dom::Element &element,
                                                   dom::DOMStringView name) const noexcept
  {
    // aria-modal changes subtrees that are being shown or hidden, which assistive technology may not have
    // asked about yet.
    if (name == u8"aria-modal")
    {
      return true;
    }
    // Changes to elements with no object are of no interest, unless their parent has one (some are handled
    // there).
    auto *parent = element.ParentNode();
    return Get(element) || (parent && Get(*parent));
  }

  void AXObjectCache::HandleAttributeChange(dom::Element &element, dom::DOMStringView name,
                                            dom::DOMStringView oldValue, dom::DOMStringView newValue) noexcept
  {
    enum class TableProperty : uint8
    {
      Exposed,
      CellSlots,
    };
    auto recomputeParentTableProperties = [this, &element](EnumSet<TableProperty> properties)
    {
      if (!properties.Contains(TableProperty::CellSlots))
      {
        // Only non-ARIA tables (ARIA ones are always exposed) can change whether they're exposed.
        if (IsHTMLTablePart(element))
        {
          for (auto *ancestor = element.ParentElement(); ancestor; ancestor = ancestor->ParentElement())
          {
            if (Is<html::HTMLTableElement>(*ancestor))
            {
              DeferRecomputeTableIsExposed(ancestor);
              break;
            }
          }
        }
        return;
      }
      auto *object = GetOrCreate(element);
      if (!object || !object->IsTableCell())
      {
        return;
      }
      if (auto *parentTable = DynamicDowncast<AccessibilityNodeObject>(object->ParentTable()))
      {
        if (properties.Contains(TableProperty::Exposed) && !parentTable->IsAriaTable())
        {
          DeferRecomputeTableIsExposed(parentTable->GetElement());
        }
        DeferRecomputeTableCellSlots(*parentTable);
      }
    };

    if (!ShouldProcessAttributeChange(element, name))
    {
      return;
    }

    if (IsRelationAttribute(name))
    {
      UpdateRelations(element, name);
    }

    if (name == u8"role")
    {
      HandleRoleChanged(element, oldValue, newValue);
    }
    else if (name == u8"alt" || name == u8"title")
    {
      HandleTextChanged(GetOrCreate(element));
    }
    else if (name == u8"contenteditable")
    {
      if (auto object = ShareRefPtr(Get(element)))
      {
        object->UpdateRole();
      }
    }
    else if (name == u8"disabled")
    {
      PostNotification(&element, AXNotification::DisabledStateChanged);
    }
    else if (name == u8"for")
    {
      // The old and new controls' names change.
      if (UpdateLabelFor(element))
      {
        for (auto id : {oldValue, newValue})
        {
          auto target = id.empty() ? nullptr : element.ElementInTreeWithId(id);
          if (auto control = target ? target->ResolveReferenceTarget() : nullptr)
          {
            PostNotification(control.get(), AXNotification::TextChanged);
          }
        }
      }
    }
    else if (name == u8"required")
    {
      PostNotification(&element, AXNotification::RequiredStatusChanged);
    }
    else if (name == u8"tabindex")
    {
      // Becoming focusable or not can change whether the object is ignored.
      if (oldValue.empty() || newValue.empty())
      {
        if (auto object = ShareRefPtr(Get(element)))
        {
          object->RecomputeIsIgnored();
        }
        PostNotification(&element, AXNotification::FocusableStateChanged);
      }
    }
    else if (name == u8"open")
    {
      if (IsHTMLElementNamed(element, u8"dialog"))
      {
        DeferModalChange(element);
        RecomputeIsIgnored(element.ParentNode());
      }
      else if (IsHTMLElementNamed(element, u8"details"))
      {
        if (auto object = ShareRefPtr(Get(element)))
        {
          PostNotification(object.get(), AXNotification::ExpandedChanged);
          ChildrenChanged(object.get());
          object->RecomputeIsIgnoredForDescendants();
        }
      }
    }
    else if (name == u8"rowspan")
    {
      PostNotification(&element, AXNotification::RowSpanChanged);
      recomputeParentTableProperties({TableProperty::CellSlots});
    }
    else if (name == u8"colspan")
    {
      PostNotification(&element, AXNotification::ColumnSpanChanged);
      recomputeParentTableProperties({TableProperty::CellSlots});
    }
    else if (name == u8"command")
    {
      PostNotification(&element, AXNotification::CommandChanged);
    }
    else if (name == u8"commandfor")
    {
      PostNotification(&element, AXNotification::CommandForChanged);
    }
    else if (name == u8"popovertarget")
    {
      PostNotification(&element, AXNotification::PopoverTargetChanged);
    }
    else if (name == u8"scope")
    {
      PostNotification(&element, AXNotification::CellScopeChanged);
    }
    else if (name == u8"datetime")
    {
      PostNotification(&element, AXNotification::DatetimeChanged);
    }
    else if (name == u8"abbr")
    {
      PostNotification(&element, AXNotification::AbbreviationChanged);
    }
    else if (name == u8"hidden")
    {
      PostNotification(&element, AXNotification::HiddenStateChanged);
    }
    else if (name == u8"draggable")
    {
      PostNotification(&element, AXNotification::DraggableStateChanged);
    }
    else if (name == u8"lang")
    {
      PostNotification(&element, AXNotification::LanguageChanged);
    }
    else if (name == u8"name")
    {
      PostNotification(&element, AXNotification::NameChanged);
    }
    else if (name == u8"placeholder")
    {
      PostNotification(&element, AXNotification::PlaceholderChanged);
    }
    else if (name == u8"href" || name == u8"src")
    {
      // An area is a link only while it has a URL.
      if (name == u8"href" && IsHTMLElementNamed(element, u8"area"))
      {
        if (auto object = ShareRefPtr(Get(element)))
        {
          object->UpdateRole();
        }
      }
      PostNotification(&element, AXNotification::URLChanged);
    }
    else if (name == u8"accesskey")
    {
      PostNotification(&element, AXNotification::AccessKeyChanged);
    }
    else if (name == u8"type")
    {
      PostNotification(&element, AXNotification::InputTypeChanged);
    }

    if (!name.starts_with(u8"aria-"))
    {
      return;
    }

    if (name == u8"aria-activedescendant")
    {
      HandleActiveDescendantChange(element);
    }
    else if (name == u8"aria-atomic")
    {
      PostNotification(&element, AXNotification::IsAtomicChanged);
    }
    else if (name == u8"aria-busy")
    {
      PostNotification(&element, AXNotification::ElementBusyChanged);
    }
    else if (name == u8"aria-controls")
    {
      PostNotification(&element, AXNotification::ControlledObjectsChanged);
    }
    else if (name == u8"aria-valuenow" || name == u8"aria-valuetext")
    {
      PostNotification(&element, AXNotification::ValueChanged);
    }
    else if (name == u8"aria-label" && element.LocalName() == u8"html")
    {
      // The web area's name is the root element's label.
      HandleTextChanged(Get(_document));
    }
    else if (name == u8"aria-label" || name == u8"aria-labelledby")
    {
      auto object = ShareRefPtr(Get(element));
      if (!object)
      {
        return;
      }
      // A form's or region's role depends on whether it's labelled
      // (https://w3c.github.io/aria/#document-handling_author-errors_roles).
      if (HasAnyRole(element, {u8"form", u8"region"}) && (oldValue.empty() || newValue.empty()))
      {
        object->UpdateRole();
      }
      HandleTextChanged(object.get());
    }
    else if (name == u8"aria-checked")
    {
      CheckedStateChanged(element);
    }
    else if (name == u8"aria-colcount")
    {
      PostNotification(&element, AXNotification::ColumnCountChanged);
      DeferRecomputeTableIsExposed(&element);
    }
    else if (name == u8"aria-colindex")
    {
      PostNotification(&element, AXNotification::ARIAColumnIndexChanged);
      recomputeParentTableProperties({TableProperty::Exposed});
    }
    else if (name == u8"aria-colindextext")
    {
      PostNotification(&element, AXNotification::ARIAColumnIndexTextChanged);
    }
    else if (name == u8"aria-colspan")
    {
      PostNotification(&element, AXNotification::ColumnSpanChanged);
      recomputeParentTableProperties({TableProperty::CellSlots, TableProperty::Exposed});
    }
    else if (name == u8"aria-describedby")
    {
      PostNotification(&element, AXNotification::DescribedByChanged);
    }
    else if (name == u8"aria-description")
    {
      PostNotification(&element, AXNotification::ExtendedDescriptionChanged);
    }
    else if (name == u8"aria-flowto")
    {
      PostNotification(&element, AXNotification::FlowToChanged);
    }
    else if (name == u8"aria-keyshortcuts")
    {
      PostNotification(&element, AXNotification::KeyShortcutsChanged);
    }
    else if (name == u8"aria-level")
    {
      PostNotification(&element, AXNotification::LevelChanged);
    }
    else if (name == u8"aria-live")
    {
      PostNotification(&element, AXNotification::LiveRegionStatusChanged);
    }
    else if (name == u8"aria-placeholder")
    {
      PostNotification(&element, AXNotification::PlaceholderChanged);
    }
    else if (name == u8"aria-rowindex")
    {
      PostNotification(&element, AXNotification::ARIARowIndexChanged);
      recomputeParentTableProperties({TableProperty::CellSlots, TableProperty::Exposed});
    }
    else if (name == u8"aria-rowindextext")
    {
      PostNotification(&element, AXNotification::ARIARowIndexTextChanged);
    }
    else if (name == u8"aria-valuemax")
    {
      PostNotification(&element, AXNotification::MaximumValueChanged);
    }
    else if (name == u8"aria-valuemin")
    {
      PostNotification(&element, AXNotification::MinimumValueChanged);
    }
    else if (name == u8"aria-multiline")
    {
      // Whether a text control is a text area or a text field depends on it.
      if (auto object = ShareRefPtr(Get(element)); object
                                                   && (object->Role() == AccessibilityRole::TextArea
                                                       || object->Role() == AccessibilityRole::TextField))
      {
        object->UpdateRole();
      }
    }
    else if (name == u8"aria-multiselectable")
    {
      PostNotification(&element, AXNotification::MultiSelectableStateChanged);
    }
    else if (name == u8"aria-orientation")
    {
      PostNotification(&element, AXNotification::OrientationChanged);
    }
    else if (name == u8"aria-posinset")
    {
      PostNotification(&element, AXNotification::PositionInSetChanged);
    }
    else if (name == u8"aria-relevant")
    {
      PostNotification(&element, AXNotification::LiveRegionRelevantChanged);
    }
    else if (name == u8"aria-selected")
    {
      OnSelectedOptionChanged(element);
    }
    else if (name == u8"aria-setsize")
    {
      PostNotification(&element, AXNotification::SetSizeChanged);
    }
    else if (name == u8"aria-expanded")
    {
      HandleAriaExpandedChange(element);
    }
    else if (name == u8"aria-haspopup")
    {
      PostNotification(&element, AXNotification::HasPopupChanged);
    }
    else if (name == u8"aria-hidden")
    {
      if (auto object = ShareRefPtr(GetOrCreate(element)))
      {
        object->RecomputeIsIgnoredForDescendants(true);
      }
      if (_currentModalElement && _currentModalElement->IsDescendantOf(element))
      {
        DeferModalChange(*_currentModalElement);
      }
    }
    else if (name == u8"aria-invalid")
    {
      PostNotification(&element, AXNotification::InvalidStatusChanged);
    }
    else if (name == u8"aria-modal")
    {
      // The element may have become modal or stopped being.
      if (IsModalElement(element))
      {
        if (std::ranges::find(_modalElements, &element) == _modalElements.end())
        {
          _modalElements.push_back(&element);
        }
      }
      else
      {
        std::erase(_modalElements, &element);
      }
      DeferModalChange(element);
    }
    else if (name == u8"aria-current")
    {
      PostNotification(&element, AXNotification::CurrentStateChanged);
    }
    else if (name == u8"aria-disabled")
    {
      PostNotification(&element, AXNotification::DisabledStateChanged);
    }
    else if (name == u8"aria-pressed")
    {
      PostNotification(&element, AXNotification::PressedStateChanged);
    }
    else if (name == u8"aria-readonly")
    {
      PostNotification(&element, AXNotification::ReadOnlyStatusChanged);
    }
    else if (name == u8"aria-required")
    {
      PostNotification(&element, AXNotification::RequiredStatusChanged);
    }
    else if (name == u8"aria-roledescription")
    {
      PostNotification(&element, AXNotification::ARIARoleDescriptionChanged);
    }
    else if (name == u8"aria-rowcount")
    {
      HandleRowCountChanged(Get(element));
    }
    else if (name == u8"aria-rowspan")
    {
      PostNotification(&element, AXNotification::RowSpanChanged);
      recomputeParentTableProperties({TableProperty::CellSlots, TableProperty::Exposed});
    }
    else if (name == u8"aria-sort")
    {
      PostNotification(&element, AXNotification::SortDirectionChanged);
    }
    else if (name == u8"aria-owns")
    {
      // (The owned children move with the relation.)
      // FIXME: The groups of text runs read as one around the owned elements (setDirtyStitchGroups), with
      // accessibility text ranges.
    }
    else if (name == u8"aria-braillelabel")
    {
      PostNotification(&element, AXNotification::BrailleLabelChanged);
    }
    else if (name == u8"aria-brailleroledescription")
    {
      PostNotification(&element, AXNotification::BrailleRoleDescriptionChanged);
    }
  }

  // Relations.

  AXRelation AXObjectCache::AttributeToRelationType(dom::DOMStringView attribute) noexcept
  {
    if (attribute == u8"aria-activedescendant")
    {
      return AXRelation::ActiveDescendant;
    }
    if (attribute == u8"aria-controls" || attribute == u8"commandfor" || attribute == u8"popovertarget")
    {
      return AXRelation::ControllerFor;
    }
    if (attribute == u8"aria-describedby")
    {
      return AXRelation::DescribedBy;
    }
    if (attribute == u8"aria-details")
    {
      return AXRelation::Details;
    }
    if (attribute == u8"aria-errormessage")
    {
      return AXRelation::ErrorMessage;
    }
    if (attribute == u8"aria-flowto")
    {
      return AXRelation::FlowsTo;
    }
    if (attribute == u8"aria-labelledby")
    {
      return AXRelation::LabeledBy;
    }
    if (attribute == u8"aria-owns")
    {
      return AXRelation::OwnerFor;
    }
    if (attribute == u8"headers")
    {
      return AXRelation::Headers;
    }
    return AXRelation::None;
  }

  bool AXObjectCache::IsRelationAttribute(dom::DOMStringView attribute) noexcept
  {
    return AttributeToRelationType(attribute) != AXRelation::None;
  }

  AXRelation AXObjectCache::SymmetricRelation(AXRelation relation) noexcept
  {
    switch (relation)
    {
      case AXRelation::ActiveDescendant:   return AXRelation::ActiveDescendantOf;
      case AXRelation::ActiveDescendantOf: return AXRelation::ActiveDescendant;
      case AXRelation::ControlledBy:       return AXRelation::ControllerFor;
      case AXRelation::ControllerFor:      return AXRelation::ControlledBy;
      case AXRelation::DescribedBy:        return AXRelation::DescriptionFor;
      case AXRelation::DescriptionFor:     return AXRelation::DescribedBy;
      case AXRelation::Details:            return AXRelation::DetailsFor;
      case AXRelation::DetailsFor:         return AXRelation::Details;
      case AXRelation::ErrorMessage:       return AXRelation::ErrorMessageFor;
      case AXRelation::ErrorMessageFor:    return AXRelation::ErrorMessage;
      case AXRelation::FlowsFrom:          return AXRelation::FlowsTo;
      case AXRelation::FlowsTo:            return AXRelation::FlowsFrom;
      case AXRelation::Headers:            return AXRelation::HeaderFor;
      case AXRelation::HeaderFor:          return AXRelation::Headers;
      case AXRelation::LabeledBy:          return AXRelation::LabelFor;
      case AXRelation::LabelFor:           return AXRelation::LabeledBy;
      case AXRelation::OwnedBy:            return AXRelation::OwnerFor;
      case AXRelation::OwnerFor:           return AXRelation::OwnedBy;
      case AXRelation::None:               return AXRelation::None;
    }
    return AXRelation::None;
  }

  bool AXObjectCache::AddRelation(dom::Element &origin, dom::Element &target, AXRelation relation) noexcept
  {
    // Only an element's labelled-by relation can be to itself.
    if (relation == AXRelation::None || (&origin == &target && relation != AXRelation::LabeledBy))
    {
      return false;
    }
    // ARIA labelling takes precedence over a label element.
    if (relation == AXRelation::LabelFor && HasAnyARIALabelling(target))
    {
      return false;
    }
    auto originObject = ShareRefPtr(GetOrCreate(origin, IsPartOfRelation::Yes));
    auto targetObject = ShareRefPtr(GetOrCreate(target, IsPartOfRelation::Yes));
    return AddRelation(originObject.get(), targetObject.get(), relation);
  }

  bool AXObjectCache::AddRelation(AccessibilityObject *origin, AccessibilityObject *target,
                                  AXRelation relation, bool addSymmetricRelation) noexcept
  {
    if (!origin || !target || relation == AXRelation::None
        || (origin == target && relation != AXRelation::LabeledBy))
    {
      return false;
    }
    // aria-owns mustn't make a cycle: an object can't own one of its ancestors.
    if (relation == AXRelation::OwnerFor || relation == AXRelation::OwnedBy)
    {
      auto *descendant = relation == AXRelation::OwnerFor ? origin : target;
      auto *ancestor = relation == AXRelation::OwnerFor ? target : origin;
      for (auto *object = descendant; object; object = object->ParentObject())
      {
        if (object == ancestor)
        {
          return false;
        }
      }
    }

    if (relation == AXRelation::OwnerFor)
    {
      // The target leaves its current parent's children (unless the relations are being worked out again and
      // it already had this owner).
      auto previousOwner = _previousOwners.find(target->ObjectID());
      bool hadOwner = previousOwner != _previousOwners.end() && previousOwner->second == origin->ObjectID();
      if (auto *targetParent = target->ParentObject(); !hadOwner && targetParent && targetParent != origin)
      {
        ChildrenChanged(targetParent);
      }
    }

    auto originID = origin->ObjectID();
    auto targetID = target->ObjectID();
    auto &targets = _relations[originID][relation];
    // There's only one active descendant, and only one owner.
    if (relation == AXRelation::ActiveDescendant || relation == AXRelation::OwnedBy)
    {
      targets.clear();
    }
    targets.add(targetID);
    _relationTargets.insert(targetID);

    if (relation == AXRelation::OwnerFor)
    {
      // Any other owner loses the target.
      for (auto &[ownerID, ownerRelations] : _relations)
      {
        if (ownerID != originID)
        {
          RemoveRelationByID(ownerID, targetID, AXRelation::OwnerFor);
        }
      }
      ChildrenChanged(origin);
    }
    else if (relation == AXRelation::OwnedBy)
    {
      if (auto *parent = origin->ParentObjectUnignored())
      {
        ChildrenChanged(parent);
      }
    }

    // The reverse relation, while both objects are still in the cache.
    if (addSymmetricRelation && _objects.contains(originID) && _objects.contains(targetID))
    {
      if (auto symmetric = SymmetricRelation(relation); symmetric != AXRelation::None)
      {
        AddRelation(target, origin, symmetric, false);
      }
    }
    return true;
  }

  bool AXObjectCache::AddRelation(dom::Element &origin, dom::DOMStringView attribute) noexcept
  {
    auto relation = AttributeToRelationType(attribute);
    auto name = dom::DOMStringAtom(attribute);
    bool addedRelation = false;
    // The relations go to the resolved target elements: through a shadow host's reference target to the
    // element it names, leaving out a host whose reference target names nothing.
    auto addResolvedRelation = [&](dom::Element &unresolvedTarget)
    {
      if (auto target = unresolvedTarget.ResolveReferenceTarget())
      {
        addedRelation |= AddRelation(origin, *target, relation);
      }
    };
    auto addRelations = [&](const List<Ref<dom::Element>> &targets)
    {
      for (const auto &target : targets)
      {
        addResolvedRelation(*target);
      }
    };

    // The elements the attribute refers to (set through its IDL attribute, or by ID).
    if (dom::Element::IsElementReflectionAttribute(name))
    {
      if (auto target = origin.ElementForAttributeInternal(name))
      {
        addResolvedRelation(*target);
        return addedRelation;
      }
    }
    else if (dom::Element::IsElementsArrayReflectionAttribute(name))
    {
      if (auto targets = origin.ElementsForAttributeInternal(name))
      {
        addRelations(*targets);
        return addedRelation;
      }
    }

    auto value = origin.GetAttribute(name);
    if (!value)
    {
      // A custom element's default ARIA relations.
      auto *queue = origin.ReactionQueue();
      auto *internals = queue ? queue->AttachedInternals() : nullptr;
      if (!internals)
      {
        return false;
      }
      if (auto target = internals->ElementForAttributeInternal(name))
      {
        addResolvedRelation(*target);
      }
      else if (auto targets = internals->ElementsForAttributeInternal(name))
      {
        addRelations(*targets);
      }
      return addedRelation;
    }

    // Attributes that aren't reflected (headers) are lists of IDs.
    for (const auto &id : infra::StringAlgorithms::SplitOnWhitespace(*value))
    {
      auto target = origin.ElementInTreeWithId(id);
      if (target && target.get() != &origin)
      {
        addResolvedRelation(*target);
      }
    }
    return addedRelation;
  }

  bool AXObjectCache::AddLabelForRelation(dom::Element &origin) noexcept
  {
    auto *label = DynamicDowncast<html::HTMLLabelElement>(origin);
    auto control = label ? ControlForLabelElement(*label) : nullptr;
    return control && AddRelation(origin, *control, AXRelation::LabelFor);
  }

  bool AXObjectCache::RemoveRelation(dom::Element &origin, AXRelation relation) noexcept
  {
    auto object = ShareRefPtr(Get(origin));
    if (!object)
    {
      return false;
    }
    auto relations = _relations.find(object->ObjectID());
    if (relations == _relations.end())
    {
      return false;
    }
    auto targets = relations->second.find(relation);
    if (targets == relations->second.end())
    {
      return false;
    }
    auto targetIDs = krys::move(targets->second);
    relations->second.erase(targets);
    bool removedRelation = !targetIDs.isEmpty();

    if (auto symmetric = SymmetricRelation(relation); symmetric != AXRelation::None)
    {
      for (auto targetID : targetIDs)
      {
        RemoveRelationByID(targetID, object->ObjectID(), symmetric);
      }
    }

    if (removedRelation && relation == AXRelation::OwnerFor)
    {
      // The owned objects go back to their natural parents.
      ChildrenChanged(object.get());
      auto *node = object->GetNode();
      if (auto *parentNode = node ? node->ParentInComposedTree() : nullptr)
      {
        ChildrenChanged(Get(*parentNode));
      }
      else if (auto *renderer = object->Renderer())
      {
        ChildrenChanged(renderer->Parent() ? Get(*renderer->Parent()) : nullptr);
      }
      for (auto targetID : targetIDs)
      {
        if (auto *target = ObjectForID(targetID))
        {
          if (auto *targetNode = target->GetNode(); targetNode && targetNode->ParentInComposedTree())
          {
            ChildrenChanged(Get(*targetNode->ParentInComposedTree()));
          }
        }
      }
    }
    return removedRelation;
  }

  void AXObjectCache::RemoveAllRelations(AXID id) noexcept
  {
    auto found = _relations.find(id);
    if (found == _relations.end())
    {
      return;
    }
    auto relations = krys::move(found->second);
    _relations.erase(found);
    for (const auto &[relation, targetIDs] : relations)
    {
      if (auto symmetric = SymmetricRelation(relation); symmetric != AXRelation::None)
      {
        for (auto targetID : targetIDs)
        {
          RemoveRelationByID(targetID, id, symmetric);
        }
      }
    }
  }

  void AXObjectCache::RemoveRelationByID(AXID originID, AXID targetID, AXRelation relation) noexcept
  {
    auto relations = _relations.find(originID);
    if (relations == _relations.end())
    {
      return;
    }
    if (auto targets = relations->second.find(relation); targets != relations->second.end())
    {
      targets->second.remove(targetID);
    }
  }

  void AXObjectCache::UpdateRelationsIfNeeded() noexcept
  {
    if (!_relationsNeedUpdate)
    {
      return;
    }
    _relationsNeedUpdate = false;
    for (const auto &[originID, relations] : _relations)
    {
      if (auto owned = relations.find(AXRelation::OwnerFor); owned != relations.end())
      {
        for (auto targetID : owned->second)
        {
          _previousOwners[targetID] = originID;
        }
      }
    }
    _relations.clear();
    _relationTargets.clear();
    UpdateRelationsForTree(_document);
    _previousOwners.clear();
  }

  void AXObjectCache::UpdateRelationsForTree(dom::ContainerNode &root) noexcept
  {
    for (auto &node : dom::DescendantRange(root))
    {
      auto *elementPointer = DynamicDowncast<dom::Element>(node);
      if (!elementPointer)
      {
        continue;
      }
      auto &element = *elementPointer;
      // Elements that aren't in the tree have no relations.
      if (IsHTMLElementNamed(element, u8"meta") || IsHTMLElementNamed(element, u8"head")
          || IsHTMLElementNamed(element, u8"script") || IsHTMLElementNamed(element, u8"html")
          || IsHTMLElementNamed(element, u8"style"))
      {
        continue;
      }
      if (auto shadowRoot = element.ShadowRoot();
          shadowRoot && shadowRoot->Mode() != dom::ShadowRootMode::UserAgent)
      {
        UpdateRelationsForTree(*shadowRoot);
      }
      for (auto attribute : RelationAttributes)
      {
        AddRelation(element, attribute);
      }
      // A label's relation to its control.
      AddLabelForRelation(element);
    }
  }

  void AXObjectCache::UpdateRelations(dom::Element &origin, dom::DOMStringView attribute) noexcept
  {
    if (IsHTMLElementNamed(origin, u8"meta") || IsHTMLElementNamed(origin, u8"head")
        || IsHTMLElementNamed(origin, u8"script") || IsHTMLElementNamed(origin, u8"html")
        || IsHTMLElementNamed(origin, u8"style"))
    {
      return;
    }
    auto relation = AttributeToRelationType(attribute);
    if (relation == AXRelation::None)
    {
      return;
    }
    // commandfor is only a button's; popovertarget only a button's or an input's.
    if (attribute == u8"commandfor" && !IsHTMLElementNamed(origin, u8"button")) KRYS_UNLIKELY
    {
      return;
    }
    if (attribute == u8"popovertarget" && !IsHTMLElementNamed(origin, u8"button")
        && !IsHTMLElementNamed(origin, u8"input")) KRYS_UNLIKELY
    {
      return;
    }
    // The relations are all worked out when they're next needed anyway.
    if (_relationsNeedUpdate)
    {
      return;
    }
    RemoveRelation(origin, relation);
    AddRelation(origin, attribute);
  }

  Maybe<ListHashSet<AXID>> AXObjectCache::RelatedObjectIDsFor(const AccessibilityObject &object,
                                                              AXRelation relation) noexcept
  {
    UpdateRelationsIfNeeded();
    auto relations = _relations.find(object.ObjectID());
    if (relations == _relations.end())
    {
      return null;
    }
    auto targets = relations->second.find(relation);
    if (targets == relations->second.end())
    {
      return null;
    }
    return targets->second;
  }

  bool AXObjectCache::IsDescendantOfRelatedNode(dom::Node &node) noexcept
  {
    for (auto *parent = node.ParentNode(); parent; parent = parent->ParentNode())
    {
      auto *object = Get(*parent);
      if (object
          && (_relations.contains(object->ObjectID()) || _relationTargets.contains(object->ObjectID())))
      {
        return true;
      }
    }
    return false;
  }

  bool AXObjectCache::UpdateLabelFor(dom::Element &label) noexcept
  {
    auto *labelElement = DynamicDowncast<html::HTMLLabelElement>(label);
    if (!labelElement)
    {
      return false;
    }
    if (auto control = ControlForLabelElement(*labelElement); control && HasAnyARIALabelling(*control))
    {
      return false;
    }
    RemoveRelation(label, AXRelation::LabelFor);
    AddLabelForRelation(label);
    return true;
  }

  void AXObjectCache::HandleActiveDescendantChange(dom::Element &element) noexcept
  {
    // Only a focused element's active descendant is of interest.
    auto *frame = element.NodeDocument().Frame();
    if (!frame || !element.NodeDocument().IsFocusedAndActive()
        || element.NodeDocument().FocusedElement() != &element)
    {
      return;
    }
    auto object = ShareRefPtr(GetOrCreate(element));
    auto activeDescendant = ShareRefPtr(object ? object->ActiveDescendant() : nullptr);
    if (!activeDescendant)
    {
      return;
    }
    if (object->ShouldFocusActiveDescendant())
    {
      PostPlatformNotification(*activeDescendant, AXNotification::FocusedUIElementChanged);
    }

    // The object is told, if it can have an active descendant; otherwise the object it controls that has the
    // active descendant.
    RefPtr<AccessibilityObject> target;
    if (object->SupportsActiveDescendant())
    {
      target = object;
    }
    else
    {
      auto controlledObjects = object->ControlledObjects();
      for (auto *ancestor = activeDescendant->ParentObject(); ancestor && !target;
           ancestor = ancestor->ParentObject())
      {
        if (std::ranges::any_of(controlledObjects,
                                [&](const auto &controlled) { return controlled.get() == ancestor; }))
        {
          target = ShareRefPtr(ancestor);
        }
      }
    }
    if (!target)
    {
      return;
    }
    PostPlatformNotification(*target, AXNotification::ActiveDescendantChanged);
    // A table cell becoming active changes the table's selected cells.
    if (target->IsTable() && activeDescendant->IsExposedTableCell())
    {
      PostPlatformNotification(*target, AXNotification::SelectedCellsChanged);
    }
  }

  void AXObjectCache::HandleTabPanelSelected(dom::Element *oldElement, dom::Element *newElement) noexcept
  {
    // The tabs that control the tab panels the focus left and entered change their selected state.
    auto tabPanelOf = [this](dom::Element *element) -> AccessibilityObject *
    {
      auto *object = element ? Get(*element) : nullptr;
      for (auto *ancestor = object ? object->ParentObject() : nullptr; ancestor;
           ancestor = ancestor->ParentObject())
      {
        if (ancestor->Role() == AccessibilityRole::TabPanel)
        {
          return ancestor;
        }
      }
      return nullptr;
    };
    auto updateTabs = [this](AccessibilityObject *tabPanel)
    {
      if (!tabPanel)
      {
        return;
      }
      for (const auto &controller : tabPanel->Controllers())
      {
        PostNotification(controller.get(), AXNotification::SelectedStateChanged);
      }
    };

    auto oldTabPanel = ShareRefPtr(tabPanelOf(oldElement));
    updateTabs(oldTabPanel.get());
    auto newTabPanel = ShareRefPtr(tabPanelOf(newElement));
    if (newTabPanel && newTabPanel != oldTabPanel)
    {
      updateTabs(newTabPanel.get());
    }
  }

  void AXObjectCache::OnPopoverToggle(dom::Element &popover) noexcept
  {
    // Each element whose popovertarget (or commandfor) is the popover is told.
    if (auto object = ShareRefPtr(Get(popover)))
    {
      for (const auto &invoker : object->Controllers())
      {
        PostNotification(invoker.get(), AXNotification::ExpandedChanged);
      }
    }
  }

  void AXObjectCache::HandleRoleChanged(dom::Element &element, dom::DOMStringView oldValue,
                                        dom::DOMStringView newValue) noexcept
  {
    auto object = ShareRefPtr(Get(element));
    if (!object)
    {
      return;
    }
    // Which kind of object an element gets depends on its role (a row's especially), so it's made again: its
    // parent's children are worked out again.
    if (oldValue.empty() || oldValue == u8"row" || newValue.empty() || newValue == u8"row")
    {
      if (auto *parent = object->ParentObject())
      {
        Remove(element);
        ChildrenChanged(parent);
        return;
      }
    }
    object->UpdateRole();
  }

  void AXObjectCache::HandleAriaExpandedChange(dom::Element &element) noexcept
  {
    auto object = ShareRefPtr(Get(element));
    if (!object)
    {
      return;
    }
    // The rows of the tree or table it's in change.
    for (auto *ancestor = object->ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->SupportsRowCountChange())
      {
        HandleRowCountChanged(ancestor);
        break;
      }
    }
    auto role = object->Role();
    if (role == AccessibilityRole::Row || role == AccessibilityRole::TreeItem)
    {
      PostNotification(object.get(),
                       object->IsExpanded() ? AXNotification::RowExpanded : AXNotification::RowCollapsed);
    }
    else
    {
      PostNotification(object.get(), AXNotification::ExpandedChanged);
    }
  }

  void AXObjectCache::HandleRowCountChanged(AccessibilityObject *object) noexcept
  {
    if (!object)
    {
      return;
    }
    if (auto *nodeObject = DynamicDowncast<AccessibilityNodeObject>(*object))
    {
      nodeObject->RecomputeIsExposableIfNecessary();
    }
    PostNotification(object, AXNotification::RowCountChanged);
  }

  void AXObjectCache::HandleTextChanged(AccessibilityObject *object) noexcept
  {
    if (!object)
    {
      return;
    }
    Ref protectedObject = ShareRef(*object);

    bool isText = object->IsStaticText();
    bool notifiedNonNativeTextControl = false;
    for (auto *ancestor = object; ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->SupportsLiveRegion())
      {
        PostLiveRegionChangeNotification(*ancestor);
      }
      if (!notifiedNonNativeTextControl && ancestor->IsNonNativeTextControl())
      {
        PostNotification(ancestor, AXNotification::ValueChanged);
        notifiedNonNativeTextControl = true;
      }
      if (!isText)
      {
        continue;
      }
      // A static text's text changed: so did the name of an ancestor named by the text under it.
      auto role = ancestor->Role();
      if (ancestor->DependsOnTextUnderElement() || role == AccessibilityRole::Label
          || role == AccessibilityRole::TextField)
      {
        PostNotification(ancestor, AXNotification::TextUnderElementChanged);
      }
      // The objects the ancestor labels need their names again.
      for (const auto &labeledObject : ancestor->LabelForObjects())
      {
        PostNotification(labeledObject.get(), AXNotification::TextChanged);
      }
    }

    PostNotification(object, AXNotification::TextChanged);
    object->RecomputeIsIgnored();
  }

  void AXObjectCache::DeferTextChangedIfNeeded(dom::Node *node) noexcept
  {
    if (!node || !NodeRendererIsValid(*node))
    {
      return;
    }
    if (RendererNeedsDeferredUpdate(*node->Renderer()))
    {
      _deferredTextChangedList.add(node);
      StartPerformCacheUpdateTimer();
      return;
    }
    HandleTextChanged(GetOrCreate(*node));
  }

  void AXObjectCache::CheckedStateChanged(dom::Element &element) noexcept
  {
    PostNotification(&element, AXNotification::CheckedStateChanged);
  }

  void AXObjectCache::ValueChanged(dom::Element &element) noexcept
  {
    PostNotification(&element, AXNotification::ValueChanged);
  }

  void AXObjectCache::OnSelectedOptionChanged(dom::Element &element) noexcept
  {
    if (table_helpers::HasCellARIARole(element))
    {
      PostNotification(&element, AXNotification::SelectedCellsChanged);
    }
    else if (IsHTMLElementNamed(element, u8"option"))
    {
      PostNotification(&element, AXNotification::SelectedStateChanged);
    }
    else if (auto object = ShareRefPtr(GetOrCreate(element)))
    {
      for (auto *ancestor = object->ParentObject(); ancestor; ancestor = ancestor->ParentObject())
      {
        if (ancestor->CanHaveSelectedChildren())
        {
          SelectedChildrenChanged(ancestor->GetNode());
          PostNotification(object.get(), AXNotification::SelectedStateChanged);
          break;
        }
      }
    }

    HandleMenuItemSelected(&element);
    HandleTabPanelSelected(nullptr, &element);
  }

  void AXObjectCache::HandleMenuItemSelected(dom::Element *element) noexcept
  {
    if (!element || !HasAnyRole(*element, {u8"menuitem", u8"menuitemradio", u8"menuitemcheckbox"}))
    {
      return;
    }
    if (!element->Focused()
        && !infra::StringAlgorithms::ASCIICaseInsensitiveMatch(
          element->GetAttribute(u8"aria-selected").value_or(dom::DOMString {}), u8"true"))
    {
      return;
    }
    PostNotification(GetOrCreate(*element), AXNotification::MenuListItemSelected);
  }

  void AXObjectCache::SelectedChildrenChanged(dom::Node *node) noexcept
  {
    PostNotification(node, AXNotification::SelectedChildrenChanged);
  }

  void AXObjectCache::SelectedChildrenChanged(RenderObject *renderer) noexcept
  {
    if (renderer)
    {
      SelectedChildrenChanged(renderer->GetNode());
    }
  }

  void AXObjectCache::DeferSelectedChildrenChangedIfNeeded(dom::Element &select) noexcept
  {
    if (!NodeRendererIsValid(select))
    {
      return;
    }
    if (RendererNeedsDeferredUpdate(*select.Renderer()))
    {
      _deferredSelectedChildrenChangedList.add(&select);
      StartPerformCacheUpdateTimer();
      return;
    }
    SelectedChildrenChanged(&select);
  }

  void AXObjectCache::OnSelectedOptionChanged(RenderObject &menuList, int32 optionIndex) noexcept
  {
    if (auto menuListObject = ShareRefPtr(DynamicDowncast<AccessibilityMenuList>(Get(menuList))))
    {
      menuListObject->DidUpdateActiveOption(optionIndex);
    }
  }

  void AXObjectCache::OnEditableTextValueChanged(dom::Node &node) noexcept
  {
    // The text control the node is in (an input's inner text, an editable element's text) is the one whose
    // value changed; otherwise the node's (or its nearest ancestor's) object.
    AccessibilityObject *object = nullptr;
    for (auto *ancestor = &node; ancestor && !object; ancestor = ancestor->ParentInComposedTree())
    {
      object = Get(*ancestor);
    }
    for (auto *ancestor = object; ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->IsTextControl())
      {
        object = ancestor;
        break;
      }
    }
    PostNotification(object, AXNotification::ValueChanged);
  }

  dom::Element *AXObjectCache::RootAXEditableElement(const dom::Node &node) noexcept
  {
    auto &mutableNode = const_cast<dom::Node &>(node);
    dom::Element *result = mutableNode.RootEditableElement();
    auto *element = DynamicDowncast<dom::Element>(mutableNode);
    if (!element)
    {
      element = mutableNode.ParentElement();
    }
    for (; element; element = element->ParentElement())
    {
      if (auto *object = GetOrCreate(*element); object && object->IsTextControl())
      {
        result = element;
      }
    }
    return result;
  }

  void AXObjectCache::OnTitleChange(dom::Document &document) noexcept
  {
    if (auto *object = Get(document))
    {
      PostNotification(object, AXNotification::TextChanged);
    }
  }

  void AXObjectCache::OnValidityChange(dom::Element &element) noexcept
  {
    if (auto *object = Get(element))
    {
      PostNotification(object, AXNotification::InvalidStatusChanged);
    }
  }

  void AXObjectCache::OnRadioGroupMembershipChanged(const dom::Element &radio,
                                                    const List<Ref<html::HTMLInputElement>> &group) noexcept
  {
    for (const auto &sibling : group)
    {
      if (sibling.get() == &radio)
      {
        continue;
      }
      if (auto *object = Get(*sibling))
      {
        PostNotification(object, AXNotification::RadioGroupMembershipChanged);
      }
    }
  }

  void AXObjectCache::OnFocusChange(dom::Element *oldElement, dom::Element *newElement) noexcept
  {
    // An element whose renderer is out of date (or not made yet) can't say yet whether it's ignored, which
    // the focus change depends on.
    auto shouldDefer = [](dom::Element *element)
    {
      if (!element)
      {
        return false;
      }
      auto *renderer = element->Renderer();
      if (renderer)
      {
        return RendererNeedsDeferredUpdate(*renderer);
      }
      if (const auto *style = element->ExistingComputedStyle())
      {
        return element->RendererIsNeeded(*style);
      }
      // With no style, assume the renderer just hasn't been made yet.
      return true;
    };
    if (!shouldDefer(newElement))
    {
      HandleFocusedUIElementChanged(oldElement, newElement);
      return;
    }

    if (_deferredFocusedNodeChange)
    {
      // Focus going back to where it was isn't a change.
      if (_deferredFocusedNodeChange->first.get() == newElement)
      {
        _deferredFocusedNodeChange = null;
        return;
      }
      // The element that was to be focused may have been included only because it was focused.
      RecomputeIsIgnored(_deferredFocusedNodeChange->second.get());
      _deferredFocusedNodeChange->second = ShareRefPtr(newElement);
    }
    else
    {
      _deferredFocusedNodeChange.emplace(ShareRefPtr(oldElement), ShareRefPtr(newElement));
    }

    // A pending layout updates the tree anyway.
    auto *renderer = newElement->Renderer();
    if (!renderer || !renderer->NeedsLayout())
    {
      StartPerformCacheUpdateTimer();
    }
  }

  void AXObjectCache::HandleFocusedUIElementChanged(dom::Element *oldElement, dom::Element *newElement,
                                                    UpdateModal updateModal) noexcept
  {
    if (updateModal == UpdateModal::Yes)
    {
      UpdateCurrentModalNode();
    }
    HandleMenuItemSelected(newElement);
    HandleTabPanelSelected(oldElement, newElement);
    if (!newElement)
    {
      return;
    }
    if (auto *focusedObject = FocusedObject())
    {
      PostPlatformNotification(*focusedObject, AXNotification::FocusedUIElementChanged);
    }
  }

  void AXObjectCache::PerformDeferredCacheUpdate() noexcept
  {
    if (_performingDeferredCacheUpdate)
    {
      return;
    }
    SetForScope performing(_performingDeferredCacheUpdate, true);
    _performCacheUpdateTimer.Stop();

    if (!_document.View())
    {
      return;
    }
    // The tree is only updated correctly against a clean layout.
    (void)_document.UpdateLayoutIgnorePendingStylesheets();

    auto rendererChanged = std::exchange(_deferredRendererChangedList, {});
    for (auto id : rendererChanged)
    {
      if (auto object = ShareRefPtr(ObjectForID(id)))
      {
        object->UpdateRole();
        object->RecomputeIsIgnored();
      }
    }

    auto recomputeTableIsExposed = std::exchange(_deferredRecomputeTableIsExposedList, {});
    for (auto id : recomputeTableIsExposed)
    {
      if (auto table = ShareRefPtr(DynamicDowncast<AccessibilityNodeObject>(ObjectForID(id))))
      {
        table->RecomputeIsExposableIfNecessary();
      }
    }

    HandleAllDeferredChildrenChanged();

    auto elementsAddedOrRemoved = std::exchange(_deferredElementAddedOrRemovedList, {});
    for (auto *element : elementsAddedOrRemoved)
    {
      Ref protectedElement = ShareRef(*element);
      HandleMenuOpened(*element);
      HandleLiveRegionCreated(*element);
      if (Is<html::HTMLLabelElement>(*element))
      {
        HandleLabelChanged(GetOrCreate(*element));
      }
    }

    auto recomputeTableCellSlots = std::exchange(_deferredRecomputeTableCellSlotsList, {});
    for (auto id : recomputeTableCellSlots)
    {
      if (auto table = ShareRefPtr(DynamicDowncast<AccessibilityNodeObject>(ObjectForID(id))))
      {
        table->SetCellSlotsDirty();
      }
    }

    auto textChanged = std::exchange(_deferredTextChangedList, {});
    for (auto *node : textChanged)
    {
      Ref protectedNode = ShareRef(*node);
      HandleTextChanged(GetOrCreate(*node));
    }

    auto recomputeIsIgnored = std::exchange(_deferredRecomputeIsIgnoredList, {});
    for (auto *element : recomputeIsIgnored)
    {
      if (auto *renderer = element->Renderer())
      {
        RecomputeIsIgnored(*renderer);
      }
    }

    // Handling one change can't remove another's element (that needs the DOM to change), but it can defer
    // more; those wait for the next update.
    auto attributeChanges = std::exchange(_deferredAttributeChanges, {});
    for (const auto &change : attributeChanges)
    {
      Ref protectedElement = ShareRef(*change.Element);
      HandleAttributeChange(*change.Element, change.Name.View(), change.OldValue, change.NewValue);
    }

    auto selectedChildrenChanged = std::exchange(_deferredSelectedChildrenChangedList, {});
    for (auto *select : selectedChildrenChanged)
    {
      SelectedChildrenChanged(select);
    }

    // A focus change can change the modal dialog, as can the modal changes after it: it's worked out once,
    // after both.
    bool shouldRecomputeModal = false;
    if (auto focusChange = std::exchange(_deferredFocusedNodeChange, null))
    {
      auto &[oldElement, newElement] = *focusChange;
      HandleFocusedUIElementChanged(oldElement.get(), newElement.get(), UpdateModal::No);
      // Being focused can change whether an object is ignored.
      RecomputeIsIgnored(oldElement.get());
      RecomputeIsIgnored(newElement.get());
      shouldRecomputeModal = true;
    }

    auto modalChanged = std::exchange(_deferredModalChangedList, {});
    for (auto *element : modalChanged)
    {
      if (!Is<html::HTMLDialogElement>(*element) && !table_helpers::HasRole(*element, u8"dialog")
          && !table_helpers::HasRole(*element, u8"alertdialog"))
      {
        continue;
      }
      shouldRecomputeModal = true;
      if (!_modalNodesInitialized)
      {
        FindModalNodes();
      }
      std::erase(_modalElements, element);
      if (IsModalElement(*element))
      {
        _modalElements.push_back(element);
      }
    }
    if (shouldRecomputeModal)
    {
      UpdateCurrentModalNode();
    }

    auto notifications = std::exchange(_deferredNotifications, {});
    for (auto &[node, notification] : notifications)
    {
      PostNotification(GetOrCreate(*node), notification);
    }
  }

  void AXObjectCache::OnRendererCreated(dom::Node &node) noexcept
  {
    if (auto *object = DynamicDowncast<AccessibilityRenderObject>(Get(node)))
    {
      if (object->SetRendererIfNeeded(node.Renderer()))
      {
        _deferredRendererChangedList.add(object->ObjectID());
        StartPerformCacheUpdateTimer();
      }
    }
  }

  void AXObjectCache::OnStyleChange(dom::Element &element, const RenderStyle *oldStyle,
                                    const RenderStyle *newStyle) noexcept
  {
    if (!oldStyle || !newStyle)
    {
      return;
    }
    auto *object = Get(element);
    if (!object)
    {
      return;
    }
    if (element.Renderer())
    {
      // Unlike visibility, content-visibility changes don't reach the tree through the render tree.
      auto isContentVisibilityHidden = [](const RenderStyle &style)
      {
        return style.GetContentVisibility() == ContentVisibility::Hidden;
      };
      if (isContentVisibilityHidden(*oldStyle) != isContentVisibilityHidden(*newStyle))
      {
        ChildrenChanged(object);
      }
    }
    else if ((oldStyle->UsedVisibility() == Visibility::Visible)
             != (newStyle->UsedVisibility() == Visibility::Visible))
    {
      // A rendered element would hear of it from the render tree.
      ChildrenChanged(object);
    }
  }

  void AXObjectCache::OnInertOrVisibilityChange(RenderElement &renderer) noexcept
  {
    if (auto *parent = renderer.Parent())
    {
      ChildrenChanged(*parent, &renderer);
    }
  }

  void AXObjectCache::OnLayoutComplete(RenderObject &layoutRoot) noexcept
  {
    PostNotification(&layoutRoot, AXNotification::LayoutComplete);
  }

  void AXObjectCache::OnDocumentLoaded() noexcept
  {
    // Only a page's main document reports a new document; a frame's reports a layout.
    auto *frame = _document.Frame();
    auto *page = frame ? frame->GetPage() : nullptr;
    auto notification = page && &page->MainFrame() == frame ? AXNotification::NewDocumentLoadComplete
                                                            : AXNotification::LayoutComplete;
    // The web area's object may not exist yet, and making it may not be safe now.
    _deferredNotifications.emplace_back(ShareRef<dom::Node>(_document), notification);
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::HandleScrolledToAnchor(dom::Node &anchor) noexcept
  {
    // The anchor may be ignored: the first object after it that isn't is told.
    RefPtr<dom::Node> node = ShareRefPtr(&anchor);
    auto *object = GetOrCreate(*node);
    while (object && object->IsIgnored())
    {
      node = ShareRefPtr(dom::TreeQueries::Next(*node, nullptr));
      while (node && !node->Renderer())
      {
        node = ShareRefPtr(dom::TreeQueries::NextSkippingChildren(*node));
      }
      if (!node)
      {
        return;
      }
      object = GetOrCreate(*node);
    }
    if (object)
    {
      PostPlatformNotification(*object, AXNotification::ScrolledToAnchor);
    }
  }

  void AXObjectCache::OnSlottedContentChange(dom::Element &slot) noexcept
  {
    ChildrenChanged(Get(slot));

    // Which of a details element's summaries is its summary (the first) can change: their roles depend on it.
    auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(slot);
    auto *details = shadowRoot ? shadowRoot->Host() : nullptr;
    if (!details || !IsHTMLElementNamed(*details, u8"details"))
    {
      return;
    }
    for (auto *child = details->FirstElementChild().get(); child; child = child->NextElementSibling().get())
    {
      if (auto *object = IsHTMLElementNamed(*child, u8"summary") ? Get(*child) : nullptr)
      {
        _deferredRendererChangedList.add(object->ObjectID());
        StartPerformCacheUpdateTimer();
      }
    }
  }

  void AXObjectCache::HandlePageEditabilityChanged() noexcept
  {
    PostNotification(&_document, AXNotification::IsEditableWebAreaChanged);
  }

  void AXObjectCache::OnDragElementChanged(dom::Element *oldElement, dom::Element *newElement) noexcept
  {
    if (oldElement == newElement)
    {
      return;
    }
    for (auto *element : {oldElement, newElement})
    {
      if (auto *object = element ? Get(*element) : nullptr)
      {
        PostNotification(object, AXNotification::GrabbedStateChanged);
      }
    }
  }

  void AXObjectCache::PostNotification(AccessibilityObject *object, AXNotification notification) noexcept
  {
    StopCachingComputedObjectAttributes();
    if (!object)
    {
      object = Get(_document);
    }
    if (!object)
    {
      return;
    }

    _notificationsToPost.emplace_back(ShareRef(*object), notification);
    if (!_notificationPostTimer.IsActive())
    {
      _notificationPostTimer.StartOneShot({});
    }
  }

  AccessibilityObject *AXObjectCache::ObservableObjectForNode(dom::Node *node) noexcept
  {
    auto *object = node ? GetOrCreate(*node) : nullptr;
    if (auto *observable = object ? object->ObservableObject() : nullptr)
    {
      return observable;
    }
    return GetOrCreate(_document);
  }

  void AXObjectCache::PostTextStateChangeNotification(dom::Node *node, AXTextChange change,
                                                      dom::DOMStringView text,
                                                      const VisiblePosition &position) noexcept
  {
    if (change == AXTextChange::Inserted)
    {
      PostTextReplacementNotification(node, {}, text, position);
    }
    else
    {
      PostTextReplacementNotification(node, text, {}, position);
    }
  }

  void AXObjectCache::PostTextReplacementNotification(dom::Node *node, dom::DOMStringView deletedText,
                                                      dom::DOMStringView insertedText,
                                                      const VisiblePosition &position) noexcept
  {
    if (!node || (deletedText.empty() && insertedText.empty()))
    {
      return;
    }
    auto *object = ObservableObjectForNode(node);
    if (!object)
    {
      return;
    }
    // The offset is worked out now, while the position is still where the change happened: after the inserted
    // text, which is where the deleted text was.
    auto index = static_cast<uint64>(std::max(object->IndexForVisiblePosition(position), 0));
    auto offset = index - std::min<uint64>(index, insertedText.size());
    if (!deletedText.empty())
    {
      _textChangesToPost.push_back(
        {ShareRef(*object), AXTextChange::Deleted, offset, dom::DOMString(deletedText)});
    }
    if (!insertedText.empty())
    {
      _textChangesToPost.push_back(
        {ShareRef(*object), AXTextChange::Inserted, offset, dom::DOMString(insertedText)});
    }
    if (!_notificationPostTimer.IsActive())
    {
      _notificationPostTimer.StartOneShot({});
    }
  }

  void AXObjectCache::PostTextSelectionChangeNotification(const dom::Position &start) noexcept
  {
    auto *node = start.DeprecatedNode();
    if (!node)
    {
      return;
    }
    // An ignored object (text, say) doesn't expose text: its unignored parent does.
    auto *object = GetOrCreate(*node);
    if (object && object->IsIgnored())
    {
      object = object->ParentObjectUnignored();
    }
    auto *observable = object ? object->ObservableObject() : nullptr;
    PostNotification(observable ? observable : GetOrCreate(_document), AXNotification::SelectedTextChanged);
  }

  void AXObjectCache::PostNotification(dom::Node *node, AXNotification notification) noexcept
  {
    // Get an existing object (creating one here can lay out, in the middle of a render tree update): the
    // node's or its nearest ancestor's.
    for (; node; node = node->ParentNode())
    {
      if (auto *object = Get(*node))
      {
        PostNotification(object, notification);
        return;
      }
    }
  }

  void AXObjectCache::PostNotification(RenderObject *renderer, AXNotification notification) noexcept
  {
    for (; renderer; renderer = renderer->Parent())
    {
      if (auto *object = Get(*renderer))
      {
        PostNotification(object, notification);
        return;
      }
    }
  }

  void AXObjectCache::NotificationPostTimerFired() noexcept
  {
    // Accessibility may have been turned off since the notifications were queued.
    if (!gAccessibilityEnabled || !_document.HasLivingRenderTree())
    {
      return;
    }
    // The host may ask about the objects, so bring the tree (and the layout it depends on) up to date first.
    PerformDeferredCacheUpdate();

    // Posting can queue more notifications; those wait for the next time the timer fires.
    auto notifications = std::exchange(_notificationsToPost, {});
    for (auto id : std::exchange(_changedLiveRegions, {}))
    {
      if (auto *region = ObjectForID(id))
      {
        notifications.emplace_back(ShareRef(*region), AXNotification::LiveRegionChanged);
      }
    }
    for (auto &[object, notification] : notifications)
    {
      if (object->IsDetached() || !object->AxObjectCache())
      {
        continue;
      }
      if (notification == AXNotification::MenuOpened)
      {
        // Only notify if the object is in fact a menu.
        object->UpdateChildrenIfNecessary();
        if (object->Role() != AccessibilityRole::Menu)
        {
          continue;
        }
      }
      PostPlatformNotification(*object, notification);
    }

    auto textChanges = std::exchange(_textChangesToPost, {});
    auto *page = _document.GetPage();
    auto *view = _document.View();
    for (const auto &textChange : textChanges)
    {
      if (textChange.Object->IsDetached() || !textChange.Object->AxObjectCache())
      {
        continue;
      }
      // As for the notifications, the host mustn't ask about the object mid-layout or with style out of date.
      if (!page || !view || view->LayoutContext().IsInLayout() || _document.ChildNeedsStyleRecalc())
      {
        break;
      }
      page->GetChromeClient().PostAccessibilityTextChange(*textChange.Object, textChange.Change,
                                                          textChange.Offset, textChange.Text);
    }
  }

  AccessibilityReplacedText::AccessibilityReplacedText(const VisibleSelection &selection) noexcept
  {
    if (!AXObjectCache::AccessibilityEnabled())
    {
      return;
    }
    _replacedRange.StartIndex.Value =
      IndexForVisiblePosition(selection.VisibleStart(), _replacedRange.StartIndex.Scope);
    if (selection.IsRange())
    {
      _replacedText = AccessibilityObject::StringForVisiblePositionRange(
        {selection.VisibleStart(), selection.VisibleEnd()});
      _replacedRange.EndIndex.Value =
        IndexForVisiblePosition(selection.VisibleEnd(), _replacedRange.EndIndex.Scope);
    }
    else
    {
      _replacedRange.EndIndex = _replacedRange.StartIndex;
    }
  }

  void AccessibilityReplacedText::PostTextStateChangeNotification(
    AXObjectCache *cache, dom::DOMStringView insertedText, const VisibleSelection &selection) const noexcept
  {
    if (!cache || !AXObjectCache::AccessibilityEnabled())
    {
      return;
    }
    // After the inserted text (the start of a caret selection, the end of the inserted text if it's
    // selected).
    auto position = selection.VisibleEnd();
    auto node = HighestEditableRoot(position.DeepEquivalent(), EditableType::HasEditableAXRole);
    if (!_replacedText.empty())
    {
      cache->PostTextReplacementNotification(node.get(), _replacedText, insertedText, position);
    }
    else
    {
      cache->PostTextStateChangeNotification(node.get(), AXTextChange::Inserted, insertedText, position);
    }
  }

  void AXObjectCache::DetachWrapper(AccessibilityObject &object,
                                    AccessibilityDetachmentType detachmentType) noexcept
  {
    // The host refers to objects by ID, so it has nothing to make for a new one; it's told when one goes, to
    // let go of what it made for it.
    if (auto *page = _document.GetPage())
    {
      page->GetChromeClient().AccessibilityObjectDetached(object, detachmentType);
    }
  }

  void AXObjectCache::PostPlatformNotification(AccessibilityObject &object,
                                               AXNotification notification) noexcept
  {
    // The host may ask about the object, which mustn't happen in the middle of a layout or with style out of
    // date.
    auto *page = _document.GetPage();
    auto *view = _document.View();
    if (!page || !view || view->LayoutContext().IsInLayout() || _document.ChildNeedsStyleRecalc())
    {
      return;
    }
    page->GetChromeClient().PostAccessibilityNotification(object, notification);
  }

  utf8_stringview NotificationToString(AXNotification notification) noexcept
  {
    switch (notification)
    {
#define KRYS_AX_NOTIFICATION_NAME(name)                                                                      \
  case AXNotification::name: return u8"" #name;
      KRYS_AX_NOTIFICATIONS(KRYS_AX_NOTIFICATION_NAME)
#undef KRYS_AX_NOTIFICATION_NAME
    }
    return {};
  }
}
