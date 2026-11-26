/*
 * Copyright (C) 2006-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Web/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Web/Css/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Web/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/PseudoElement.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Editing/Editing.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Html/Elements/HTMLEmbedElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLImageElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLObjectElement.hpp"
#include "Krystal.Web/Loader/Cache/CachedImage.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Platform/Scrolling/Scrollbar.hpp"
#include "Krystal.Web/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Web/Svg/Elements/SVGImageElement.hpp"
#include <algorithm>

namespace krys::web
{
  static inline void AppendToNodeSet(const HitTestResult::NodeSet &source,
                                     HitTestResult::NodeSet &destination) noexcept
  {
    for (auto &node : source)
    {
      if (std::ranges::find(destination, node) == destination.end())
      {
        destination.push_back(node);
      }
    }
  }

  HitTestResult::HitTestResult() noexcept = default;

  HitTestResult::HitTestResult(const LayoutPoint &point) noexcept
      : _hitTestLocation(point), _doublePointInInnerNodeFrame(point)
  {
  }

  HitTestResult::HitTestResult(const LayoutRect &rect) noexcept
      : _hitTestLocation {rect}, _doublePointInInnerNodeFrame {rect.Center()}
  {
  }

  HitTestResult::HitTestResult(const HitTestLocation &location) noexcept
      : _hitTestLocation(location), _doublePointInInnerNodeFrame(_hitTestLocation.Point())
  {
  }

  HitTestResult::HitTestResult(const HitTestResult &other) noexcept
      : _hitTestLocation(other._hitTestLocation), _innerNode(ShareRefPtr(other.InnerNode())),
        _innerNonSharedNode(ShareRefPtr(other.InnerNonSharedNode())),
        _doublePointInInnerNodeFrame(other._doublePointInInnerNodeFrame), _localPoint(other.LocalPoint()),
        _innerUrlElement(ShareRefPtr(other.UrlElement())), _scrollbar(ShareRefPtr(other.GetScrollbar())),
        _isOverWidget(other.IsOverWidget()), _pseudoElementIdentifier(other.GetPseudoElementIdentifier())
  {
    // Only copy the NodeSet in case of list hit test.
    if (other._listBasedTestResult)
    {
      _listBasedTestResult = CreateUnique<NodeSet>();
      AppendToNodeSet(*other._listBasedTestResult, *_listBasedTestResult);
    }
  }

  HitTestResult::~HitTestResult() noexcept = default;

  HitTestResult &HitTestResult::operator=(const HitTestResult &other) noexcept
  {
    if (this == &other)
    {
      return *this;
    }
    _hitTestLocation = other._hitTestLocation;
    _innerNode = other._innerNode;
    _innerNonSharedNode = other._innerNonSharedNode;
    _doublePointInInnerNodeFrame = other._doublePointInInnerNodeFrame;
    _localPoint = other.LocalPoint();
    _innerUrlElement = other._innerUrlElement;
    _scrollbar = other._scrollbar;
    _isOverWidget = other.IsOverWidget();
    _pseudoElementIdentifier = other.GetPseudoElementIdentifier();

    // Only copy the NodeSet in case of list hit test.
    _listBasedTestResult = nullptr;
    if (other._listBasedTestResult)
    {
      _listBasedTestResult = CreateUnique<NodeSet>();
      AppendToNodeSet(*other._listBasedTestResult, *_listBasedTestResult);
    }

    return *this;
  }

  namespace
  {
    KRYS_NODISCARD web::Node *MoveOutOfUserAgentShadowTree(web::Node &node) noexcept
    {
      if (node.IsInShadowTree())
      {
        if (auto *root = dom::TreeQueries::ContainingShadowRoot(node))
        {
          if (root->Mode() == dom::ShadowRootMode::UserAgent)
          {
            return root->Host();
          }
        }
      }
      return &node;
    }
  }

  void HitTestResult::SetToNonUserAgentShadowAncestor() noexcept
  {
    if (auto *node = InnerNode())
    {
      SetInnerNode(MoveOutOfUserAgentShadowTree(*node));
    }
    if (auto *node = InnerNonSharedNode())
    {
      SetInnerNonSharedNode(MoveOutOfUserAgentShadowTree(*node));
    }
  }

  void HitTestResult::SetInnerNode(web::Node *node) noexcept
  {
    auto *pseudoElement = DynamicDowncast<PseudoElement>(node);
    _innerNode = ShareRefPtr(pseudoElement ? static_cast<web::Node *>(pseudoElement->HostElement()) : node);
  }

  void HitTestResult::SetInnerNonSharedNode(web::Node *node) noexcept
  {
    auto *pseudoElement = DynamicDowncast<PseudoElement>(node);
    _innerNonSharedNode =
      ShareRefPtr(pseudoElement ? static_cast<web::Node *>(pseudoElement->HostElement()) : node);
  }

  void HitTestResult::SetUrlElement(web::Element *element) noexcept
  {
    _innerUrlElement = ShareRefPtr(element);
  }

  web::Frame *HitTestResult::InnerNodeFrame() const noexcept
  {
    if (_innerNonSharedNode)
    {
      return _innerNonSharedNode->NodeDocument().Frame();
    }
    if (_innerNode)
    {
      return _innerNode->NodeDocument().Frame();
    }
    return nullptr;
  }

  void HitTestResult::SetLocalPoint(const LayoutPoint &point) noexcept
  {
    _localPoint = _pseudoElementIdentifier ? LayoutPoint() : point;
  }

  web::Frame *HitTestResult::Frame() const noexcept
  {
    if (_innerNonSharedNode)
    {
      return _innerNonSharedNode->NodeDocument().Frame();
    }

    return nullptr;
  }

  bool HitTestResult::IsOverLink() const noexcept
  {
    return _innerUrlElement && css::IsLink(*_innerUrlElement);
  }

  bool HitTestResult::IsSelected() const noexcept
  {
    if (!_innerNonSharedNode)
    {
      return false;
    }

    auto *frame = _innerNonSharedNode->NodeDocument().Frame();
    if (!frame)
    {
      return false;
    }

    return _innerNonSharedNode->NodeDocument().Selection().Contains(_hitTestLocation.Point());
  }

  // Documents are UTF-8 only, so no encoding modifies the display string (see DIFFERENCES.md).
  utf8_string HitTestResult::AltDisplayString() const noexcept
  {
    if (!_innerNonSharedNode)
    {
      return {};
    }

    if (auto *image = DynamicDowncast<HTMLImageElement>(*_innerNonSharedNode))
    {
      return image->GetAttribute(u8"alt").value_or(DOMString());
    }

    if (auto *input = DynamicDowncast<HTMLInputElement>(*_innerNonSharedNode))
    {
      return input->GetAttribute(u8"alt").value_or(DOMString());
    }

    return {};
  }

  // (There are no image overlays, whose hosts would count; see DIFFERENCES.md.)
  Image *HitTestResult::GetImage() const noexcept
  {
    if (!_innerNonSharedNode)
    {
      return nullptr;
    }

    if (auto *image = DynamicDowncast<RenderImage>(_innerNonSharedNode->Renderer()))
    {
      if (image->GetCachedImage() && !image->GetCachedImage()->ErrorOccurred())
      {
        return image->GetCachedImage()->ImageForRenderer(image);
      }
    }

    return nullptr;
  }

  RectI HitTestResult::ImageRect() const noexcept
  {
    if (!GetImage())
    {
      return {};
    }

    return _innerNonSharedNode->GetRenderBox()->AbsoluteContentQuad().EnclosingBoundingBox();
  }

  // (No link decoration filtering (applyLinkDecorationFiltering): it's Cocoa's, not a standard feature.)
  net::Url HitTestResult::AbsoluteImageUrl() const noexcept
  {
    if (!_innerNonSharedNode)
    {
      return {};
    }

    auto *renderer = _innerNonSharedNode->Renderer();
    if (!renderer || !renderer->IsRenderImage())
    {
      return {};
    }

    auto *element = DynamicDowncast<web::Element>(*_innerNonSharedNode);
    auto *htmlElement = DynamicDowncast<HTMLElement>(_innerNonSharedNode.get());
    if ((htmlElement
         && (Is<HTMLEmbedElement>(*htmlElement) || Is<HTMLImageElement>(*htmlElement)
             || Is<HTMLInputElement>(*htmlElement) || Is<HTMLObjectElement>(*htmlElement)))
        || Is<SVGImageElement>(DynamicDowncast<SVGElement>(element)))
    {
      return _innerNonSharedNode->NodeDocument().CompleteUrl(element->ImageSourceUrl().value_or(DOMString()));
    }

    return {};
  }

  net::Url HitTestResult::AbsoluteLinkUrl() const noexcept
  {
    if (!_innerUrlElement)
    {
      return {};
    }

    return _innerUrlElement->AbsoluteLinkUrl();
  }

  utf8_string HitTestResult::TextContent() const noexcept
  {
    if (!_innerUrlElement)
    {
      return {};
    }
    return _innerUrlElement->TextContent().value_or(DOMString());
  }

  template <typename TRect>
  inline HitTestProgress
    HitTestResult::AddNodeToListBasedTestResultCommon(web::Node *node, const HitTestRequest &request,
                                                      const HitTestLocation &locationInContainer,
                                                      const TRect &rect) noexcept
  {
    // If it is not a list-based hit test, this method has to be no-op.
    if (!request.ResultIsElementList())
    {
      krys_debug_assert(!IsRectBasedTest());
      return HitTestProgress::Stop;
    }

    if (!node)
    {
      return HitTestProgress::Continue;
    }

    if ((request.DisallowsUserAgentShadowContent() && node->IsInUserAgentShadowTree())
        || (request.DisallowsUserAgentShadowContentExceptForImageOverlays()
            && node->IsInUserAgentShadowTree()))
    {
      // (There are no image overlays to exempt from the second check; see DIFFERENCES.md.)
      node = AncestorNodeInThisScope(node->NodeDocument(), node);
    }

    auto &nodes = MutableListBasedTestResult();
    if (std::ranges::find_if(nodes, [&](const Ref<web::Node> &existing) { return existing.get() == node; })
        == nodes.end())
    {
      nodes.push_back(ShareRef(*node));
    }

    if (request.IncludesAllElementsUnderPoint())
    {
      return HitTestProgress::Continue;
    }

    bool regionFilled = rect.Contains(TRect(locationInContainer.BoundingBox()));
    return regionFilled ? HitTestProgress::Stop : HitTestProgress::Continue;
  }

  HitTestProgress HitTestResult::AddNodeToListBasedTestResult(web::Node *node, const HitTestRequest &request,
                                                              const HitTestLocation &locationInContainer,
                                                              const LayoutRect &rect) noexcept
  {
    return AddNodeToListBasedTestResultCommon(node, request, locationInContainer, rect);
  }

  HitTestProgress HitTestResult::AddNodeToListBasedTestResult(web::Node *node, const HitTestRequest &request,
                                                              const HitTestLocation &locationInContainer,
                                                              const RectF &rect) noexcept
  {
    return AddNodeToListBasedTestResultCommon(node, request, locationInContainer, rect);
  }

  void HitTestResult::Append(const HitTestResult &other, const HitTestRequest &request) noexcept
  {
    krys_debug_assert(request.ResultIsElementList());
    (void)request;

    if (!_innerNode && other.InnerNode())
    {
      _innerNode = other._innerNode;
      _innerNonSharedNode = other._innerNonSharedNode;
      _localPoint = other.LocalPoint();
      _doublePointInInnerNodeFrame = other._doublePointInInnerNodeFrame;
      _innerUrlElement = other._innerUrlElement;
      _scrollbar = other._scrollbar;
      _isOverWidget = other.IsOverWidget();
    }

    if (other._listBasedTestResult)
    {
      AppendToNodeSet(*other._listBasedTestResult, MutableListBasedTestResult());
    }
  }

  const HitTestResult::NodeSet &HitTestResult::ListBasedTestResult() const noexcept
  {
    if (!_listBasedTestResult)
    {
      _listBasedTestResult = CreateUnique<NodeSet>();
    }
    return *_listBasedTestResult;
  }

  HitTestResult::NodeSet &HitTestResult::MutableListBasedTestResult() noexcept
  {
    if (!_listBasedTestResult)
    {
      _listBasedTestResult = CreateUnique<NodeSet>();
    }
    return *_listBasedTestResult;
  }

  web::Element *HitTestResult::TargetElement() const noexcept
  {
    for (web::Node *node = _innerNode.get(); node; node = node->ParentInComposedTree())
    {
      if (auto *element = DynamicDowncast<web::Element>(*node))
      {
        return element;
      }
    }
    return nullptr;
  }

  web::Element *HitTestResult::InnerNonSharedElement() const noexcept
  {
    auto *node = _innerNonSharedNode.get();
    if (!node)
    {
      return nullptr;
    }
    if (auto *element = DynamicDowncast<web::Element>(*node))
    {
      return element;
    }
    return node->ParentElement();
  }
}
