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

#include "Krystal.Booey/Accessibility/AXRenderObject.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Accessibility/AXSpinButton.hpp"
#include "Krystal.Booey/Accessibility/AXSVGObject.hpp"
#include "Krystal.Booey/Accessibility/AXTableHelpers.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLabelElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/HTML/Shadow/SpinButtonElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderListBox.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD html::TagName RenderObjectTagName(const Node *node) noexcept
    {
      auto *element = DynamicDowncast<HTMLElement>(node);
      return element ? element->TagQualifiedName().TagName() : html::TagName::Unknown;
    }

    /// @brief Whether an alt text (on an image, or from CSS) includes the object or leaves it out: an empty
    /// one marks it decorative.
    KRYS_NODISCARD AXObjectInclusion ObjectInclusionFromAltText(const Maybe<DOMString> &altText) noexcept
    {
      if (!altText)
      {
        return AXObjectInclusion::DefaultBehavior;
      }
      // Don't ignore an image that has alt text.
      if (!infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(*altText).empty())
      {
        return AXObjectInclusion::IncludeObject;
      }
      // An image with an empty alt is decorative (https://html.spec.whatwg.org/#alt).
      return AXObjectInclusion::IgnoreObject;
    }
  }

  Ref<AXRenderObject> AXRenderObject::Create(AXId id, RenderObject &renderer, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXRenderObject(id, &renderer, renderer.GetNode(), cache));
  }

  Ref<AXRenderObject> AXRenderObject::Create(AXId id, Node &node, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXRenderObject(id, nullptr, &node, cache));
  }

  AXRenderObject::AXRenderObject(AXId id, RenderObject *renderer, Node *node, AXObjectCache &cache) noexcept
      : AXNodeObject(id, node, cache), _renderer(CreateWeakPtr(renderer))
  {
  }

  AXRenderObject::~AXRenderObject() noexcept = default;

  bool AXRenderObject::SetRendererIfNeeded(RenderObject *renderer) noexcept
  {
    if (renderer == _renderer.get())
    {
      return false;
    }
    _renderer = CreateWeakPtr(renderer);
    return true;
  }

  void AXRenderObject::DetachRemoteParts(AXDetachmentType detachmentType) noexcept
  {
    // An SVG image's root stops being this image's child.
    if (auto *root = RemoteSVGRoot(false); root && root->RemoteParent() == this)
    {
      root->SetRemoteParent(nullptr);
    }
    AXNodeObject::DetachRemoteParts(detachmentType);
    _renderer = nullptr;
  }

  AXSVGObject *AXRenderObject::RemoteSVGRoot(bool create) const noexcept
  {
    auto *renderImage = DynamicDowncast<RenderImage>(_renderer.get());
    auto *cachedImage = renderImage ? renderImage->GetCachedImage() : nullptr;
    if (!cachedImage || !cachedImage->HasImage())
    {
      return nullptr;
    }
    auto *svgImage = DynamicDowncast<SVGImage>(cachedImage->GetImage());
    auto root = svgImage ? svgImage->RootElement() : nullptr;
    auto *rootRenderer = root ? root->Renderer() : nullptr;
    if (!rootRenderer)
    {
      return nullptr;
    }
    // The SVG document's objects are in its own cache, as each document's are.
    auto &document = root->NodeDocument();
    auto *cache = create ? document.AxObjectCache() : document.ExistingAXObjectCache();
    if (!cache)
    {
      return nullptr;
    }
    return DynamicDowncast<AXSVGObject>(create ? cache->GetOrCreate(*rootRenderer)
                                               : cache->Get(*rootRenderer));
  }

  void AXRenderObject::AddRemoteSVGChildren() noexcept
  {
    auto *root = RemoteSVGRoot();
    if (!root)
    {
      return;
    }
    // An SVG image shown by more than one image is the first one's child (WebKit's rule too).
    if (auto *parent = root->RemoteParent(); parent && parent != this)
    {
      return;
    }
    root->SetRemoteParent(this);
    AddChild(root);
  }

  void AXRenderObject::OffsetForRemoteSVGElement(LayoutRect &rect) const noexcept
  {
    for (const AXObject *object = this; object; object = object->ParentObject())
    {
      auto *svgObject = DynamicDowncast<AXSVGObject>(*object);
      if (auto *image = svgObject ? svgObject->RemoteParent() : nullptr)
      {
        rect.MoveBy(image->ElementRect().Location());
        return;
      }
    }
  }

  AXObject *AXRenderObject::ElementAccessibilityHitTest(const IntPoint &point) const noexcept
  {
    // An SVG image's document has no view of its own to hit test (it's drawn at each image's size), so its
    // objects are found by their bounds, which are where the image is: the last child containing the point,
    // as it's drawn on top, then its children.
    if (auto *root = RemoteSVGRoot(); root && root->RemoteParent() == this)
    {
      AXObject *found = root;
      for (bool descended = true; descended;)
      {
        descended = false;
        const auto &children = found->Children();
        for (auto child = children.rbegin(); child != children.rend(); ++child)
        {
          if ((*child)->ElementRect().Contains(LayoutPoint(point)))
          {
            found = child->get();
            descended = true;
            break;
          }
        }
      }
      return found;
    }
    return AXNodeObject::ElementAccessibilityHitTest(point);
  }

  Document *AXRenderObject::GetDocument() const noexcept
  {
    if (auto *renderer = _renderer.get())
    {
      return &renderer->GetDocument();
    }
    return AXNodeObject::GetDocument();
  }

  AXObject *AXRenderObject::ParentObject() const noexcept
  {
    auto *renderer = _renderer.get();
    auto *cache = AxObjectCache();
    if (!renderer || !cache)
    {
      return AXNodeObject::ParentObject();
    }

    // ::before and ::after are the first and last children of the element that generates them (see
    // AddChildren), not siblings of it as their renderers are.
    if (auto *pseudo = DynamicDowncast<PseudoElement>(GetNode()))
    {
      return cache->GetOrCreate(pseudo->HostElement());
    }
    if (GetNode())
    {
      return AXNodeObject::ParentObject();
    }
    if (auto *owner = OwnerParentObject()) KRYS_UNLIKELY
    {
      return owner;
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
    // The web area's parent is the view it's in.
    if (Is<RenderView>(*renderer))
    {
      auto *view = DocumentFrameView();
      return view ? cache->GetOrCreate(*view) : nullptr;
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

  RenderObject *AXRenderObject::MarkerRenderer() const noexcept
  {
    auto *listItem = DynamicDowncast<RenderListItem>(_renderer.get());
    if (!listItem || !IsListItem() || IsIgnored())
    {
      return nullptr;
    }
    return listItem->MarkerRenderer();
  }

  void AXRenderObject::AddChildren() noexcept
  {
    if (!_renderer) KRYS_UNLIKELY
    {
      AXNodeObject::AddChildren();
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

    // An exposed table's children are its caption and rows (and its columns and header container).
    if (IsExposableTable())
    {
      AddTableChildrenAndCellSlots();
      return;
    }

    // A native text control's only child is its spin button, if it has one.
    if (IsNativeTextControl())
    {
      AddTextFieldChildren();
      _subtreeDirty = false;
      return;
    }

    auto *node = DynamicDowncast<ContainerNode>(GetNode());
    auto *element = DynamicDowncast<Element>(GetNode());

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

    if (node && !Is<PseudoElement>(*node))
    {
      auto *select = DynamicDowncast<HTMLSelectElement>(*node);
      if (select && Role() == AXRole::ListBox)
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

    AddImageMapChildren();
    AddRemoteSVGChildren();

    UpdateOwnedChildrenIfNecessary();
    _subtreeDirty = false;

    // A row's aria-colindex gives its cells theirs, counting on from it (whether or not they're owned).
    if (IsExposedTableRow())
    {
      if (auto columnIndex = AxColumnIndex())
      {
        int32 index = 0;
        for (const auto &child : _children)
        {
          if (auto *cell = DynamicDowncast<AXNodeObject>(child.get()); cell && cell->IsTableCell())
          {
            cell->SetAXColIndexFromRow(static_cast<int32>(*columnIndex) + index);
          }
          ++index;
        }
      }
    }

    UpdateRoleAfterChildrenCreation();
  }

  void AXRenderObject::UpdateRoleAfterChildrenCreation() noexcept
  {
    auto role = _role;
    // A menu needs a menu item (or a group of them) to be exposed as a menu
    // (https://w3c.github.io/aria/#menu).
    if (role == AXRole::Menu)
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
        _role = AXRole::Generic;
      }
    }
    // An SVG root with no children is an image.
    if (role == AXRole::SVGRoot && _children.empty())
    {
      _role = AXRole::Image;
    }

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

  AXRole AXRenderObject::DetermineAccessibilityRole() noexcept
  {
    // A list's role is worked out the same way rendered or not.
    if (IsAccessibilityList())
    {
      if (!_childrenDirty && _childrenInitialized)
      {
        return DetermineListRoleWithCleanChildren();
      }
      return IsDescriptionList() ? AXRole::DescriptionList : AXRole::List;
    }
    // A tree item outside a tree loses its role; a tree with other children isn't one.
    if (_ariaRole == AXRole::TreeItem)
    {
      _isValidTreeItem = ContainingTree() != nullptr;
    }
    if (_ariaRole == AXRole::Tree)
    {
      return IsValidTree() ? AXRole::Tree : AXRole::Generic;
    }
    auto *renderer = _renderer.get();
    if (!renderer)
    {
      return AXNodeObject::DetermineAccessibilityRole();
    }
    if (renderer->IsRenderText())
    {
      return AXRole::StaticText;
    }

    if (_ariaRole != AXRole::Unknown && !(_ariaRole == AXRole::TreeItem && !_isValidTreeItem))
    {
      return _ariaRole;
    }

    if (IsExposableTable())
    {
      return AXRole::Table;
    }
    if (IsExposedTableRow())
    {
      return AXRole::Row;
    }

    auto *node = renderer->GetNode();
    if (renderer->IsRenderListItem())
    {
      // The details' disclosure triangle is a list item, but it's the summary.
      auto *summary = DynamicDowncast<HTMLSummaryElement>(node);
      if (!summary || !summary->IsActiveSummary())
      {
        return AXRole::ListItem;
      }
    }
    if (renderer->IsRenderListMarker())
    {
      return AXRole::ListMarker;
    }
    if (renderer->IsBR())
    {
      return AXRole::LineBreak;
    }
    if (RenderObjectTagName(node) == html::TagName::img
        && DynamicDowncast<Element>(node)->HasAttribute(u8"usemap"))
    {
      return AXRole::ImageMap;
    }
    if (renderer->IsRenderImage())
    {
      if (Is<HTMLInputElement>(node))
      {
        return HasPopup() ? AXRole::PopUpButton : AXRole::Button;
      }
      // An image of an SVG document with something to expose is that document's root.
      if (auto *svgRoot = RemoteSVGRoot(); svgRoot && svgRoot->IsRootWithAccessibleContent())
      {
        return AXRole::SVGRoot;
      }
      return AXRole::Image;
    }
    if (renderer->IsRenderView())
    {
      return AXRole::WebArea;
    }
    if (renderer->IsRenderTextControlSingleLine())
    {
      if (auto *input = DynamicDowncast<HTMLInputElement>(node))
      {
        return RoleFromInputElement(*input);
      }
    }
    if (renderer->IsRenderTextControlMultiLine())
    {
      return AXRole::TextArea;
    }
    if (renderer->IsRenderMenuList())
    {
      return AXRole::PopUpButton;
    }
    if (renderer->IsRenderListBox())
    {
      return AXRole::ListBox;
    }
    if (renderer->IsLegacyRenderSVGRoot())
    {
      return AXRole::SVGRoot;
    }

    if (auto *renderElement = DynamicDowncast<RenderElement>(*renderer))
    {
      switch (renderElement->Style().EffectiveDisplay())
      {
        case DisplayType::Ruby:           return AXRole::RubyInline;
        case DisplayType::RubyAnnotation: return AXRole::RubyText;
        case DisplayType::RubyBlock:
        case DisplayType::RubyBase:       return AXRole::Group;
        default:                          break;
      }
    }

    if (renderer->IsRenderTableSection())
    {
      return AXRole::Ignored;
    }

    auto roleFromNode = DetermineAccessibilityRoleFromNode(
      renderer->IsRenderInline() ? TreatStyleFormatGroupAsInline::Yes : TreatStyleFormatGroupAsInline::No);

    // A cell is a text group unless its table is exposed.
    if (IsTableCell())
    {
      auto *parentTable = ParentTable();
      if (parentTable && parentTable->IsExposableTable())
      {
        return parentTable->HasGridRole() ? AXRole::GridCell : AXRole::Cell;
      }
    }

    if (roleFromNode != AXRole::Unknown)
    {
      return roleFromNode;
    }

    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*renderer))
    {
      return blockFlow->IsAnonymousBlock() ? AXRole::TextGroup : AXRole::Generic;
    }

    // Inline is the last fallback before Unknown: it tells a non-focusable inline element with an event
    // handler or something suggesting the platform include it apart from one that's truly unknown.
    if (renderer->IsRenderInline()
        && (HasAttributesRequiredForInclusion() || (node && !node->EventListeners().empty())
            || (RenderObjectTagName(node) == html::TagName::time && !GetAttribute(u8"datetime").empty())))
    {
      return AXRole::Inline;
    }
    return AXRole::Unknown;
  }

  bool AXRenderObject::ComputeIsIgnored() const noexcept
  {
    auto *renderer = _renderer.get();
    if (!renderer)
    {
      return AXNodeObject::ComputeIsIgnored();
    }

    if (IsTree())
    {
      return IsIgnoredByDefault();
    }

    // The common reasons first, then the rules for each kind of object.
    auto decision = DefaultObjectInclusion();
    if (decision == AXObjectInclusion::IncludeObject)
    {
      return false;
    }
    if (decision == AXObjectInclusion::IgnoreObject)
    {
      return true;
    }

    if (Role() == AXRole::Ignored)
    {
      return true;
    }

    // Before the presentational role check: an exposed table's cells are in the tree even with a
    // presentational role.
    if (IsTableCell())
    {
      // Anonymous cells (display: table-cell) outside a table are ignored.
      auto *parentTable = ParentTable();
      auto *parentElement = parentTable ? parentTable->GetElement() : nullptr;
      bool inTable =
        parentElement
        && (RenderObjectTagName(parentElement) == html::TagName::table || ax::HasTableRole(*parentElement));
      if (!inTable && !GetElement())
      {
        return true;
      }
      if (IsExposedTableCell())
      {
        return false;
      }
    }

    // A presentational role leaves the object out (its children stay).
    if (_ariaRole == AXRole::Presentational)
    {
      return true;
    }

    // A tree's children are its tree items (and their content) and text.
    if (!IsAllowedChildOfTree())
    {
      return true;
    }
    if (IsAccessibilityList() || IsExposableTable())
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
      // The alt text CSS content gives generated text decides (an empty one marks it decorative).
      if (auto *fragment = DynamicDowncast<RenderTextFragment>(*text))
      {
        auto inclusion = ObjectInclusionFromAltText(fragment->AltText());
        if (inclusion == AXObjectInclusion::IgnoreObject)
        {
          return true;
        }
        if (inclusion == AXObjectInclusion::IncludeObject)
        {
          return false;
        }
      }

      bool checkForIgnored = true;
      for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
      {
        // Static text beneath text controls is reported along with the text control's text.
        if (ancestor->Role() == AXRole::TextField)
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
      case AXRole::Audio:
      case AXRole::DescriptionListTerm:
      case AXRole::DescriptionListDetail:
      case AXRole::Details:
      case AXRole::DocumentArticle:
      case AXRole::Figure:
      case AXRole::LandmarkRegion:
      case AXRole::ListItem:
      case AXRole::SectionFooter:
      case AXRole::SectionHeader:
      case AXRole::Time:
      case AXRole::Video:                 return false;
      default:                            break;
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
        ObjectInclusionFromAltText(element ? element->GetAttribute(u8"alt") : Maybe<DOMString> {});
      if (inclusion == AXObjectInclusion::IgnoreObject)
      {
        return true;
      }
      if (inclusion == AXObjectInclusion::IncludeObject)
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

    // What's between an exposed row and its cells is ignored.
    if (!IsTableCell() && CanHaveChildren() && IgnoredByRowAncestor())
    {
      return true;
    }

    if (_ariaRole != AXRole::Unknown)
    {
      return false;
    }
    if (Role() == AXRole::HorizontalRule)
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

    // The math tree's anonymous boxes and unexposed elements are left out; the rest of it is in.
    if (IsIgnoredElementWithinMathTree())
    {
      return true;
    }
    if (IsMathElement())
    {
      return false;
    }

    // A block of inline content with no lines with content (and nothing clickable) is ignored.
    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*renderer);
        blockFlow && renderer->ChildrenInline() && !CanSetFocusAttribute()
        && !blockFlow->HasBlocksInInlineLayout())
    {
      return !blockFlow->HasContentfulInlineLine() && !ClickableSelfOrAncestor();
    }

    if (Role() == AXRole::Canvas)
    {
      // A canvas with an element child may have fallback content.
      if (auto *element = GetElement())
      {
        for (auto *child = element->FirstChild(); child; child = child->NextSibling())
        {
          if (Is<Element>(*child))
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
          && Is<HTMLInputElement>(shadowRoot->Host()))
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

    // Inside the label of a checkbox or radio button that has no label relation (a title element), the
    // label's contents are part of the control.
    if (auto *control = ControlForLabelElement();
        control && control->IsCheckboxOrRadio() && control->LabeledByObjects().empty())
    {
      return true;
    }

    if (IsExposedTableRow())
    {
      return IsRenderHidden() || _ariaRole == AXRole::Presentational;
    }

    // By default, objects should be ignored so that the tree isn't filled with unnecessary items.
    return true;
  }

  void AXRenderObject::AddImageMapChildren() noexcept
  {
    auto *image = DynamicDowncast<RenderImage>(_renderer.get());
    auto *map = image ? image->ImageMap() : nullptr;
    auto *cache = AxObjectCache();
    if (!map || !cache)
    {
      return;
    }
    // The map's areas that are links.
    for (auto &node : dom::DescendantRange(*map))
    {
      if (auto *area = DynamicDowncast<HTMLAreaElement>(node); area && css::IsLink(*area))
      {
        AddChild(cache->GetOrCreate(*area));
      }
    }
  }

  void AXRenderObject::AddTextFieldChildren() noexcept
  {
    auto *input = DynamicDowncast<HTMLInputElement>(GetNode());
    auto *spinButtonElement =
      input ? DynamicDowncast<html::SpinButtonElement>(input->InnerSpinButtonElement()) : nullptr;
    auto *cache = AxObjectCache();
    if (!spinButtonElement || !cache)
    {
      return;
    }
    auto *spinButton = DynamicDowncast<AXSpinButton>(cache->Create(AXRole::SpinButton));
    if (!spinButton)
    {
      return;
    }
    spinButton->SetSpinButtonElement(spinButtonElement);
    spinButton->SetParent(this);
    AddChild(spinButton);
  }

  AXObject *AXRenderObject::ControlForLabelElement() const noexcept
  {
    // The control isn't part of its own label.
    auto *cache = AxObjectCache();
    if (IsControl() || !cache)
    {
      return nullptr;
    }
    for (auto *node = GetNode(); node; node = node->ParentNode())
    {
      if (auto *label = DynamicDowncast<HTMLLabelElement>(*node))
      {
        auto control = label->Control();
        // A control that's a descendant of the label, being destroyed, doesn't count.
        if (!control || (control->Renderer() && !control->Renderer()->Parent()))
        {
          return nullptr;
        }
        return cache->GetOrCreate(*control);
      }
    }
    return nullptr;
  }

  AXObject *AXRenderObject::ContainingTree() const noexcept
  {
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->IsTree())
      {
        return ancestor;
      }
    }
    return nullptr;
  }

  bool AXRenderObject::IsAllowedChildOfTree() const noexcept
  {
    // Inside a tree, only tree items, their descendants and static text are exposed.
    bool isTreeItemDescendant = false;
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->IsTreeItem())
      {
        isTreeItemDescendant = true;
      }
      if (ancestor->IsTree())
      {
        return isTreeItemDescendant || IsTreeItem() || IsStaticText();
      }
    }
    return true;
  }

  bool AXRenderObject::IsIgnoredElementWithinMathTree() const noexcept
  {
    // Anonymous boxes in a MathML block (CSS's, https://www.w3.org/TR/css3-box/#block-level0) are left out.
    auto *renderer = _renderer.get();
    return renderer && renderer->IsAnonymous() && renderer->Parent()
           && renderer->Parent()->IsRenderMathMLBlock();
  }

  bool AXRenderObject::IgnoredByRowAncestor() const noexcept
  {
    // A cell above the object (before a row) means it's a cell's contents; a table above it (before a row)
    // means it could be a row, a row group or another container: neither is ignored.
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->IsTableCell() || ancestor->IsTable())
      {
        return false;
      }
      if (ancestor->IsExposedTableRow())
      {
        return true;
      }
    }
    return false;
  }

  DOMString AXRenderObject::StringValue() const noexcept
  {
    auto *renderer = _renderer.get();
    if (!renderer)
    {
      return AXNodeObject::StringValue();
    }
    // Generated text's CSS alt text stands in for it.
    if (auto *fragment = DynamicDowncast<RenderTextFragment>(*renderer); fragment && fragment->AltText())
    {
      return *fragment->AltText();
    }
    if (auto *text = DynamicDowncast<RenderText>(*renderer))
    {
      // The rendered text (after text-transform and whitespace processing).
      return DOMString(text->Text());
    }
    if (auto *marker = DynamicDowncast<RenderListMarker>(*renderer))
    {
      return DOMString(marker->TextWithSuffix());
    }
    return AXNodeObject::StringValue();
  }

  bool AXRenderObject::IsOffScreen() const noexcept
  {
    auto *renderer = _renderer.get();
    auto *view = renderer ? renderer->GetDocument().View() : nullptr;
    if (!view)
    {
      return true;
    }
    auto contentRect = SnappedIntRect(renderer->AbsoluteClippedOverflowRectForSpatialNavigation());
    auto viewRect = view->VisibleContentRect();
    viewRect.Intersect(contentRect);
    return viewRect.IsEmpty();
  }

  AXObject *AXRenderObject::AccessibilityHitTest(const IntPoint &point) const noexcept
  {
    auto *renderer = DynamicDowncast<RenderLayerModelObject>(_renderer.get());
    if (!renderer || !renderer->HasLayer())
    {
      return nullptr;
    }
    (void)renderer->GetDocument().UpdateLayout();
    // Layout may have destroyed the renderer or its layer.
    renderer = DynamicDowncast<RenderLayerModelObject>(_renderer.get());
    if (!renderer || !renderer->HasLayer())
    {
      return nullptr;
    }

    HitTestResult hitTestResult {LayoutPoint(point)};
    (void)renderer->Layer()->HitTest(HitTestRequest(EnumSet<HitTestRequest::Type> {
                                       HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
                                       HitTestRequest::Type::AccessibilityHitTest}),
                                     hitTestResult);
    Node *node = hitTestResult.InnerNode();
    if (!node)
    {
      return nullptr;
    }
    // An option is part of its list box.
    if (auto *option = DynamicDowncast<HTMLOptionElement>(*node))
    {
      node = option->OwnerSelectElement();
    }
    auto *cache = node ? node->NodeDocument().AxObjectCache() : nullptr;
    RefPtr<AXObject> result = ShareRefPtr(cache ? cache->GetOrCreate(*node) : nullptr);
    if (!result)
    {
      return nullptr;
    }
    result->UpdateChildrenIfNecessary();

    // A list box's options have no renderers: the hit one is found from their rows.
    if (auto *listBox = DynamicDowncast<RenderListBox>(result->Renderer()); listBox && result->IsListBox())
    {
      auto parentRect = result->ElementRect();
      const auto &children = result->Children();
      for (size_t i = 0; i < children.size(); ++i)
      {
        if (listBox->ItemBoundingBoxRect(parentRect.Location(), static_cast<int32>(i))
              .Contains(LayoutPoint(point)))
        {
          return children[i].get();
        }
      }
      return result.get();
    }

    auto *hit = result->ElementAccessibilityHitTest(point);
    return hit && hit->IsIgnored() ? hit->ParentObjectUnignored() : hit;
  }

  bool AXRenderObject::SetValue(DOMStringView value) noexcept
  {
    RefPtr<Element> element = ShareRefPtr(GetElement());
    if (!element)
    {
      return false;
    }
    // A focused editable element is typed into, replacing what's there.
    auto &document = element->NodeDocument();
    if (document.Frame() && document.FocusedElement() == element && element->ShouldUseInputMethod())
    {
      document.Selection().SelectAll();
      (void)document.GetEditor().InsertText(value, nullptr);
      return true;
    }
    if (auto *input = DynamicDowncast<HTMLInputElement>(*element); input && input->IsTextField())
    {
      (void)input->SetValue(value, html::TextFieldEventBehavior::DispatchInputAndChangeEvent);
      return true;
    }
    if (auto *textArea = DynamicDowncast<HTMLTextAreaElement>(*element))
    {
      (void)textArea->SetValue(value, html::TextFieldEventBehavior::DispatchInputAndChangeEvent);
      return true;
    }
    return false;
  }

  LayoutRect AXRenderObject::ElementRect() const noexcept
  {
    auto *renderer = _renderer.get();
    if (!renderer)
    {
      return AXNodeObject::ElementRect();
    }
    // (The quads cover each of an inline's line boxes and a text's text boxes; Booey has no continuations.)
    LayoutRect rect(renderer->AbsoluteBoundingBoxRect());
    // An SVG element's box is its bounding box, as for getBoundingClientRect (an SVG text renderer's quads
    // leave out where its text is).
    if (auto *svgElement = DynamicDowncast<SVGElement>(GetElement());
        svgElement && svgElement->HasAssociatedSVGLayoutBox())
    {
      if (auto box = svgElement->GetBoundingBox())
      {
        rect = LayoutRect(EnclosingIntRect(renderer->LocalToAbsoluteQuad(*box).BoundingBox()));
      }
    }
    if (rect.IsEmpty() && !IsWebArea())
    {
      return NonEmptyAncestorBoundingBox();
    }
    // An SVG image's objects are where the image showing it is.
    auto *document = GetDocument();
    auto documentElement = document ? document->DocumentElement() : nullptr;
    auto *svgElement = DynamicDowncast<SVGElement>(documentElement.get());
    if (svgElement && Is<SVGSvgElement>(*svgElement))
    {
      OffsetForRemoteSVGElement(rect);
    }
    return rect;
  }
}
