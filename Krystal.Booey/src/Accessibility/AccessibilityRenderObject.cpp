/*
 * Copyright (C) 2008-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo::ax
{
  namespace
  {
    KRYS_NODISCARD html::TagName RenderObjectTagName(const dom::Node *node) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      return element ? element->TagQualifiedName().TagName() : html::TagName::Unknown;
    }

    /// @brief Whether an alt text (on an image, or from CSS) includes the object or leaves it out: an empty
    /// one marks it decorative.
    KRYS_NODISCARD AccessibilityObjectInclusion
      ObjectInclusionFromAltText(const Maybe<dom::DOMString> &altText) noexcept
    {
      if (!altText)
      {
        return AccessibilityObjectInclusion::DefaultBehavior;
      }
      // Don't ignore an image that has alt text.
      if (!infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(*altText).empty())
      {
        return AccessibilityObjectInclusion::IncludeObject;
      }
      // An image with an empty alt is decorative (https://html.spec.whatwg.org/#alt).
      return AccessibilityObjectInclusion::IgnoreObject;
    }
  }

  Ref<AccessibilityRenderObject> AccessibilityRenderObject::Create(AXID id, RenderObject &renderer,
                                                                   AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityRenderObject(id, &renderer, renderer.GetNode(), cache));
  }

  Ref<AccessibilityRenderObject> AccessibilityRenderObject::Create(AXID id, dom::Node &node,
                                                                   AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityRenderObject(id, nullptr, &node, cache));
  }

  AccessibilityRenderObject::AccessibilityRenderObject(AXID id, RenderObject *renderer, dom::Node *node,
                                                       AXObjectCache &cache) noexcept
      : AccessibilityNodeObject(id, node, cache), _renderer(CreateWeakPtr(renderer))
  {
  }

  AccessibilityRenderObject::~AccessibilityRenderObject() noexcept = default;

  bool AccessibilityRenderObject::SetRendererIfNeeded(RenderObject *renderer) noexcept
  {
    if (renderer == _renderer.get())
    {
      return false;
    }
    _renderer = CreateWeakPtr(renderer);
    return true;
  }

  void AccessibilityRenderObject::DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept
  {
    AccessibilityNodeObject::DetachRemoteParts(detachmentType);
    _renderer = nullptr;
  }

  dom::Document *AccessibilityRenderObject::GetDocument() const noexcept
  {
    if (auto *renderer = _renderer.get())
    {
      return &renderer->GetDocument();
    }
    return AccessibilityNodeObject::GetDocument();
  }

  AccessibilityObject *AccessibilityRenderObject::ParentObject() const noexcept
  {
    auto *renderer = _renderer.get();
    auto *cache = AxObjectCache();
    if (!renderer || !cache)
    {
      return AccessibilityNodeObject::ParentObject();
    }

    // ::before and ::after are the first and last children of the element that generates them (see
    // AddChildren), not siblings of it as their renderers are.
    if (auto *pseudo = DynamicDowncast<dom::PseudoElement>(GetNode()))
    {
      return cache->GetOrCreate(pseudo->HostElement());
    }
    if (GetNode())
    {
      return AccessibilityNodeObject::ParentObject();
    }

    // A list marker is a child of its list item, even when it isn't the item's renderer's direct child.
    if (renderer->IsRenderListMarker())
    {
      for (auto *ancestor = renderer->Parent(); ancestor; ancestor = ancestor->Parent())
      {
        if (ancestor->IsRenderListItem())
        {
          return cache->GetOrCreate(*ancestor);
        }
      }
    }
    // Other anonymous renderers are under the nearest renderer with a node.
    for (auto *ancestor = renderer->Parent(); ancestor; ancestor = ancestor->Parent())
    {
      if (ancestor->GetNode())
      {
        return cache->GetOrCreate(*ancestor);
      }
    }
    return nullptr;
  }

  RenderObject *AccessibilityRenderObject::MarkerRenderer() const noexcept
  {
    auto *listItem = DynamicDowncast<RenderListItem>(_renderer.get());
    if (!listItem || !IsListItem() || IsIgnored())
    {
      return nullptr;
    }
    return listItem->MarkerRenderer();
  }

  void AccessibilityRenderObject::AddChildren() noexcept
  {
    if (!_renderer) KRYS_UNLIKELY
    {
      AccessibilityNodeObject::AddChildren();
      return;
    }

    // If the need to add more children in addition to existing children arises, childrenChanged should have
    // been called, leaving the object with no children.
    krys_debug_assert(!_childrenInitialized);
    _childrenInitialized = true;

    auto *cache = AxObjectCache();
    if (!cache)
    {
      return;
    }

    // FIXME: Exposed tables' rows, columns and header container (addTableChildrenAndCellSlots), with tables.

    auto *node = DynamicDowncast<dom::ContainerNode>(GetNode());
    auto *element = DynamicDowncast<dom::Element>(GetNode());

    // ::before and ::after are the first and last children of the element that generates them.
    if (auto *before = element ? element->BeforePseudoElement() : nullptr)
    {
      AddChild(cache->GetOrCreate(*before));
    }

    // A list item's marker is its first child (or second, after ::before).
    if (auto *marker = MarkerRenderer())
    {
      AddChild(cache->GetOrCreate(*marker));
    }

    if (node && !Is<dom::PseudoElement>(*node))
    {
      auto *select = DynamicDowncast<html::HTMLSelectElement>(*node);
      if (select && Role() == AccessibilityRole::ListBox)
      {
        // A list box's children are its options (and option groups), wherever they are.
        for (const auto &item : select->ListItems())
        {
          if (item)
          {
            AddChild(cache->GetOrCreate(*item), DescendIfIgnored::No);
          }
        }
      }
      else
      {
        // The tree comes from the DOM: the node's children in the composed tree.
        for (auto &child : dom::ComposedTreeChildren(*node))
        {
          AddChild(cache->GetOrCreate(child));
        }
      }
    }
    else if (auto *renderElement = DynamicDowncast<RenderElement>(_renderer.get()))
    {
      // An anonymous renderer that's in the tree (generated content, a list marker) gets its children from
      // the render tree. Anonymous blocks' children belong to their nearest ancestor with a node.
      if (!renderElement->IsAnonymousBlock())
      {
        for (auto *child = renderElement->FirstChild(); child; child = child->NextSibling())
        {
          AddChild(cache->GetOrCreate(*child));
        }
      }
    }

    if (auto *after = element ? element->AfterPseudoElement() : nullptr)
    {
      AddChild(cache->GetOrCreate(*after));
    }

    // FIXME: aria-owned children (updateOwnedChildrenIfNecessary), with relations.
    _subtreeDirty = false;
    UpdateRoleAfterChildrenCreation();
  }

  void AccessibilityRenderObject::UpdateRoleAfterChildrenCreation() noexcept
  {
    auto role = _role;
    // A menu needs a menu item (or a group of them) to be exposed as a menu
    // (https://w3c.github.io/aria/#menu).
    if (role == AccessibilityRole::Menu)
    {
      bool hasMenuItemDescendant = false;
      for (const auto &child : _children)
      {
        if (child->IsMenuItem())
        {
          hasMenuItemDescendant = true;
          break;
        }
        if (child->IsGroup())
        {
          for (const auto &grandchild : child->Children())
          {
            if (grandchild->IsMenuItem())
            {
              hasMenuItemDescendant = true;
              break;
            }
          }
        }
      }
      if (!hasMenuItemDescendant)
      {
        _role = AccessibilityRole::Generic;
      }
    }
    // FIXME: An SVG root with no children is an image, with SVG objects.

    if (IsAccessibilityList())
    {
      UpdateRole();
      return;
    }
    if (role != _role)
    {
      if (auto *cache = AxObjectCache())
      {
        cache->HandleRoleChanged(*this, role);
      }
    }
  }

  AccessibilityRole AccessibilityRenderObject::DetermineAccessibilityRole() noexcept
  {
    // A list's role is worked out the same way rendered or not.
    if (IsAccessibilityList())
    {
      if (!_childrenDirty && _childrenInitialized)
      {
        return DetermineListRoleWithCleanChildren();
      }
      return IsDescriptionList() ? AccessibilityRole::DescriptionList : AccessibilityRole::List;
    }
    // FIXME: Trees (from their items), with them.
    auto *renderer = _renderer.get();
    if (!renderer)
    {
      return AccessibilityNodeObject::DetermineAccessibilityRole();
    }
    if (renderer->IsRenderText())
    {
      return AccessibilityRole::StaticText;
    }

    if (_ariaRole != AccessibilityRole::Unknown)
    {
      return _ariaRole;
    }

    // FIXME: Exposed tables and rows, with tables.

    auto *node = renderer->GetNode();
    if (renderer->IsRenderListItem())
    {
      // The details' disclosure triangle is a list item, but it's the summary.
      auto *summary = DynamicDowncast<html::HTMLSummaryElement>(node);
      if (!summary || !summary->IsActiveSummary())
      {
        return AccessibilityRole::ListItem;
      }
    }
    if (renderer->IsRenderListMarker())
    {
      return AccessibilityRole::ListMarker;
    }
    if (renderer->IsBR())
    {
      return AccessibilityRole::LineBreak;
    }
    if (RenderObjectTagName(node) == html::TagName::img
        && DynamicDowncast<dom::Element>(node)->HasAttribute(u8"usemap"))
    {
      return AccessibilityRole::ImageMap;
    }
    if (renderer->IsRenderImage())
    {
      if (Is<html::HTMLInputElement>(node))
      {
        return HasPopup() ? AccessibilityRole::PopUpButton : AccessibilityRole::Button;
      }
      // FIXME: An image of an SVG document with accessible content is an SVG root, with SVG objects.
      return AccessibilityRole::Image;
    }
    if (renderer->IsRenderView())
    {
      return AccessibilityRole::WebArea;
    }
    if (renderer->IsRenderTextControlSingleLine())
    {
      if (auto *input = DynamicDowncast<html::HTMLInputElement>(node))
      {
        return RoleFromInputElement(*input);
      }
    }
    if (renderer->IsRenderTextControlMultiLine())
    {
      return AccessibilityRole::TextArea;
    }
    if (renderer->IsRenderMenuList())
    {
      return AccessibilityRole::PopUpButton;
    }
    if (renderer->IsRenderListBox())
    {
      return AccessibilityRole::ListBox;
    }
    if (renderer->IsLegacyRenderSVGRoot())
    {
      return AccessibilityRole::SVGRoot;
    }

    if (auto *renderElement = DynamicDowncast<RenderElement>(*renderer))
    {
      switch (renderElement->Style().EffectiveDisplay())
      {
        case DisplayType::Ruby:           return AccessibilityRole::RubyInline;
        case DisplayType::RubyAnnotation: return AccessibilityRole::RubyText;
        case DisplayType::RubyBlock:
        case DisplayType::RubyBase:       return AccessibilityRole::Group;
        default:                          break;
      }
    }

    if (renderer->IsRenderTableSection())
    {
      return AccessibilityRole::Ignored;
    }

    auto roleFromNode = DetermineAccessibilityRoleFromNode(
      renderer->IsRenderInline() ? TreatStyleFormatGroupAsInline::Yes : TreatStyleFormatGroupAsInline::No);
    if (roleFromNode != AccessibilityRole::Unknown)
    {
      return roleFromNode;
    }

    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*renderer))
    {
      return blockFlow->IsAnonymousBlock() ? AccessibilityRole::TextGroup : AccessibilityRole::Generic;
    }

    // Inline is the last fallback before Unknown: it tells a non-focusable inline element with an event
    // handler or something suggesting the platform include it apart from one that's truly unknown.
    if (renderer->IsRenderInline()
        && (HasAttributesRequiredForInclusion() || (node && !node->EventListeners().empty())
            || (RenderObjectTagName(node) == html::TagName::time && !GetAttribute(u8"datetime").empty())))
    {
      return AccessibilityRole::Inline;
    }
    return AccessibilityRole::Unknown;
  }

  bool AccessibilityRenderObject::ComputeIsIgnored() const noexcept
  {
    auto *renderer = _renderer.get();
    if (!renderer)
    {
      return AccessibilityNodeObject::ComputeIsIgnored();
    }

    // FIXME: Trees (isIgnoredByDefault), with tree objects.

    // The common reasons first, then the rules for each kind of object.
    auto decision = DefaultObjectInclusion();
    if (decision == AccessibilityObjectInclusion::IncludeObject)
    {
      return false;
    }
    if (decision == AccessibilityObjectInclusion::IgnoreObject)
    {
      return true;
    }

    if (Role() == AccessibilityRole::Ignored)
    {
      return true;
    }

    // FIXME: Table cells and rows (exposed when their table is), with tables.

    // A presentational role leaves the object out (its children stay).
    if (_ariaRole == AccessibilityRole::Presentational)
    {
      return true;
    }

    // FIXME: Children of a tree that aren't tree items (isAllowedChildOfTree), with trees.
    if (IsAccessibilityList())
    {
      return false;
    }

    // A select's pop-up menu items are its menu list's.
    for (auto *ancestor = renderer->Parent(); ancestor; ancestor = ancestor->Parent())
    {
      if (ancestor->IsRenderMenuList())
      {
        return true;
      }
    }

    if (renderer->IsBR())
    {
      return true;
    }

    if (auto *text = DynamicDowncast<RenderText>(*renderer))
    {
      // Text with no rendered text, or only whitespace, isn't in the tree.
      if (!text->HasRenderedText())
      {
        return true;
      }
      if (infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(text->Text()).empty())
      {
        return true;
      }
      if (text->Parent() && text->Parent()->IsFirstLetter())
      {
        return true;
      }
      // FIXME: Alt text from CSS on generated text (RenderTextFragment::altText), with generated content.

      bool checkForIgnored = true;
      for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
      {
        // Static text beneath text controls is reported along with the text control's text.
        if (ancestor->Role() == AccessibilityRole::TextField)
        {
          return true;
        }
        if (checkForIgnored && !ancestor->IsIgnored())
        {
          checkForIgnored = false;
          // Static text beneath menu items is reported along with the menu item.
          if (ancestor->IsMenuItem())
          {
            return true;
          }
        }
      }
      // Walking the ancestors can destroy the renderer.
      return !_renderer;
    }

    if (IsHeading() || IsLink() || IsLandmark() || IsControl() || IsSummary())
    {
      return false;
    }
    switch (Role())
    {
      case AccessibilityRole::Audio:
      case AccessibilityRole::DescriptionListTerm:
      case AccessibilityRole::DescriptionListDetail:
      case AccessibilityRole::Details:
      case AccessibilityRole::DocumentArticle:
      case AccessibilityRole::Figure:
      case AccessibilityRole::LandmarkRegion:
      case AccessibilityRole::ListItem:
      case AccessibilityRole::SectionFooter:
      case AccessibilityRole::SectionHeader:
      case AccessibilityRole::Time:
      case AccessibilityRole::Video:                 return false;
      default:                                       break;
    }

    auto *node = renderer->GetNode();
    if (IsImage())
    {
      // An image that can take focus is kept, lest the user be unable to reach something important.
      if (CanSetFocusAttribute())
      {
        return false;
      }
      // An image in a figure is kept: the figure may use it in its name
      // (https://github.com/w3c/aria/pull/2224).
      for (auto *ancestor = node ? node->ParentNode() : nullptr; ancestor; ancestor = ancestor->ParentNode())
      {
        if (RenderObjectTagName(ancestor) == html::TagName::figure)
        {
          return false;
        }
      }

      // A decorative image (alt="") is ignored even if a title, aria-label and so on is given.
      auto *element = GetElement();
      auto inclusion =
        ObjectInclusionFromAltText(element ? element->GetAttribute(u8"alt") : Maybe<dom::DOMString> {});
      if (inclusion == AccessibilityObjectInclusion::IgnoreObject)
      {
        return true;
      }
      if (inclusion == AccessibilityObjectInclusion::IncludeObject)
      {
        return false;
      }
      // An image with an accessible name from an attribute is kept (as ARIA allows).
      if (HasAccNameAttribute())
      {
        return false;
      }
      // A one-pixel image (a spacer or tracker) is ignored.
      if (auto *box = DynamicDowncast<RenderBox>(*renderer))
      {
        return box->Height() <= 1 || box->Width() <= 1;
      }
      return false;
    }

    // FIXME: Objects between a table row and its cells (ignoredByRowAncestor), with tables.

    if (_ariaRole != AccessibilityRole::Unknown)
    {
      return false;
    }
    if (Role() == AccessibilityRole::HorizontalRule)
    {
      return false;
    }

    // Labels aren't ignored: they title their controls.
    if (RenderObjectTagName(node) == html::TagName::label)
    {
      return false;
    }
    // The top of a contenteditable region isn't ignored (its editable descendants are part of it).
    if (HasContentEditableAttributeSet())
    {
      return false;
    }
    // If this element has aria attributes on it, it should not be ignored.
    if (SupportsARIAAttributes())
    {
      return false;
    }

    // FIXME: MathML elements, with MathML objects.

    // A block of inline content with no lines with content (and nothing clickable) is ignored.
    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*renderer);
        blockFlow && renderer->ChildrenInline() && !CanSetFocusAttribute()
        && !blockFlow->HasBlocksInInlineLayout())
    {
      // FIXME: An ancestor with a click handler keeps it (clickableSelfOrAncestor), with event handler
      // checks.
      return !blockFlow->HasContentfulInlineLine();
    }

    if (Role() == AccessibilityRole::Canvas)
    {
      // A canvas with an element child may have fallback content.
      if (auto *element = GetElement())
      {
        for (auto *child = element->FirstChild(); child; child = child->NextSibling())
        {
          if (Is<dom::Element>(*child))
          {
            return false;
          }
        }
      }
      if (auto *box = DynamicDowncast<RenderBox>(*renderer); box && (box->Height() <= 1 || box->Width() <= 1))
      {
        return true;
      }
      // Otherwise its name decides, below.
    }

    if (renderer->IsRenderListMarker())
    {
      auto *parent = ParentObjectUnignored();
      return parent && !parent->IsListItem();
    }

    if (IsWebArea())
    {
      return false;
    }

    // A meter's render tree is a block (the meter) holding the meter renderer: the latter is exposed.
    if (RenderObjectTagName(node) == html::TagName::meter)
    {
      return true;
    }

    // Having an accessible name is a weaker reason than the ones above, so it comes late.
    if (HasAttributesRequiredForInclusion())
    {
      return false;
    }

    // A generic focusable element (<div tabindex=0>) is kept unless it's empty.
    if (IsGenericFocusableElement() && node && node->FirstChild())
    {
      // The generic div a user agent renders within a text input is still ignored.
      auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(*node);
      if (shadowRoot && shadowRoot->Mode() == dom::ShadowRootMode::UserAgent
          && Is<html::HTMLInputElement>(shadowRoot->Host()))
      {
        return true;
      }
      return false;
    }

    // A span conveys nothing without other ARIA information, and exposing it (as a group) could make its
    // parent's text look like its own.
    auto tagName = RenderObjectTagName(node);
    if (tagName == html::TagName::span)
    {
      return true;
    }
    if (tagName == html::TagName::dfn || IsStyleFormatGroup())
    {
      return false;
    }

    if (auto *renderElement = DynamicDowncast<RenderElement>(*renderer))
    {
      switch (renderElement->Style().EffectiveDisplay())
      {
        case DisplayType::Ruby:
        case DisplayType::RubyBlock:
        case DisplayType::RubyAnnotation:
        case DisplayType::RubyBase:       return false;
        default:                          break;
      }
    }

    // FIXME: The contents of a checkbox's or radio button's label are part of the control
    // (controlForLabelElement), with relations.

    // By default, objects should be ignored so that the tree isn't filled with unnecessary items.
    return true;
  }

  dom::DOMString AccessibilityRenderObject::StringValue() const noexcept
  {
    auto *renderer = _renderer.get();
    if (!renderer)
    {
      return AccessibilityNodeObject::StringValue();
    }
    if (auto *text = DynamicDowncast<RenderText>(*renderer))
    {
      // The rendered text (after text-transform and whitespace processing).
      return dom::DOMString(text->Text());
    }
    if (auto *marker = DynamicDowncast<RenderListMarker>(*renderer))
    {
      return dom::DOMString(marker->TextWithSuffix());
    }
    return AccessibilityNodeObject::StringValue();
  }

  LayoutRect AccessibilityRenderObject::ElementRect() const noexcept
  {
    auto *renderer = _renderer.get();
    if (!renderer)
    {
      return AccessibilityNodeObject::ElementRect();
    }
    // FIXME: The union of an inline's continuations' rects, with continuations; and the boxes of the text
    // runs for text, with text ranges.
    return LayoutRect(renderer->AbsoluteBoundingBoxRect());
  }
}
