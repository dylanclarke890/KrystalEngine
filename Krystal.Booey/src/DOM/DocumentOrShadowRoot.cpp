#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheetObservableArray.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/RadioButtonGroups.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::dom
{
  DocumentOrShadowRoot::DocumentOrShadowRoot(ContainerNode &rootNode,
                                             RefPtr<html::CustomElementRegistry> &&registry) noexcept
      : _rootNode(rootNode), _customElementRegistry(krys::move(registry)),
        _idTargetObserverRegistry(CreateUnique<dom::IdTargetObserverRegistry>()),
        _radioButtonGroups(CreateUnique<dom::RadioButtonGroups>())
  {
  }

  DocumentOrShadowRoot::~DocumentOrShadowRoot() noexcept = default;

#pragma region DocumentOrShadowRoot Mixin (DOM)

  RefPtr<html::CustomElementRegistry> DocumentOrShadowRoot::CustomElementRegistry() const noexcept
  {
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

  RefPtr<Element> DocumentOrShadowRoot::ActiveElement() const noexcept
  {
    // Let candidate be this's node document's focused area's DOM anchor.
    auto &document = _rootNode.NodeDocument();
    Node *candidate = document.FocusedElement();
    if (!candidate)
    {
      candidate = &document;
    }

    // Set candidate to the result of retargeting candidate against this.
    auto *retargeted = DynamicDowncast<Node>(EventTargetAlgorithms::Retarget(candidate, _rootNode));
    if (!retargeted)
    {
      return nullptr;
    }

    // If candidate's root is not this, then return null.
    if (&TreeQueries::Root(*retargeted) != &_rootNode)
    {
      return nullptr;
    }

    // If candidate is not a Document object, then return candidate.
    if (!Is<Document>(*retargeted))
    {
      return ShareRefPtr(Downcast<Element>(retargeted));
    }

    // If candidate has a body element, then return that body element.
    if (auto body = document.Body())
    {
      return body;
    }

    // If candidate's document element is non-null, then return that document element.
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

    return ShareRefPtr(DynamicDowncast<Element>(node.get()));
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

      elements.push_back(ShareRefPtr(DynamicDowncast<Element>(node.get())));
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
