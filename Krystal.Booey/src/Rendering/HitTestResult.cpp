#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include <algorithm>

namespace krys::boo
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

  HitTestResult::HitTestResult(const HitTestLocation &other) noexcept
      : _hitTestLocation(other), _doublePointInInnerNodeFrame(_hitTestLocation.Point())
  {
  }

  HitTestResult::HitTestResult(const HitTestResult &other) noexcept
      : _hitTestLocation(other._hitTestLocation), _innerNode(ShareRefPtr(other.InnerNode())),
        _innerNonSharedNode(ShareRefPtr(other.InnerNonSharedNode())),
        _doublePointInInnerNodeFrame(other._doublePointInInnerNodeFrame), _localPoint(other.LocalPoint()),
        _innerURLElement(ShareRefPtr(other.URLElement())), _isOverWidget(other.IsOverWidget()),
        _pseudoElementIdentifier(other.GetPseudoElementIdentifier())
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
    _innerURLElement = other._innerURLElement;
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

  void HitTestResult::SetToNonUserAgentShadowAncestor() noexcept
  {
    // There are no user agent shadow trees (form controls' internals) to move out of; shadow roots are only
    // open or closed.
  }

  void HitTestResult::SetInnerNode(dom::Node *node) noexcept
  {
    auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(node);
    _innerNode = ShareRefPtr(pseudoElement ? static_cast<dom::Node *>(pseudoElement->HostElement()) : node);
  }

  void HitTestResult::SetInnerNonSharedNode(dom::Node *node) noexcept
  {
    auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(node);
    _innerNonSharedNode =
      ShareRefPtr(pseudoElement ? static_cast<dom::Node *>(pseudoElement->HostElement()) : node);
  }

  void HitTestResult::SetURLElement(dom::Element *element) noexcept
  {
    _innerURLElement = ShareRefPtr(element);
  }

  LocalFrame *HitTestResult::InnerNodeFrame() const noexcept
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

  LocalFrame *HitTestResult::Frame() const noexcept
  {
    if (_innerNonSharedNode)
    {
      return _innerNonSharedNode->NodeDocument().Frame();
    }

    return nullptr;
  }

  bool HitTestResult::IsOverLink() const noexcept
  {
    return _innerURLElement && css::IsLink(*_innerURLElement);
  }

  template <typename TRect>
  inline HitTestProgress
    HitTestResult::AddNodeToListBasedTestResultCommon(dom::Node *node, const HitTestRequest &request,
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

    // There are no user agent shadow trees whose nodes would be replaced by their hosts.

    auto &nodes = MutableListBasedTestResult();
    if (std::ranges::find_if(nodes, [&](const Ref<dom::Node> &existing) { return existing.get() == node; })
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

  HitTestProgress HitTestResult::AddNodeToListBasedTestResult(dom::Node *node, const HitTestRequest &request,
                                                              const HitTestLocation &locationInContainer,
                                                              const LayoutRect &rect) noexcept
  {
    return AddNodeToListBasedTestResultCommon(node, request, locationInContainer, rect);
  }

  HitTestProgress HitTestResult::AddNodeToListBasedTestResult(dom::Node *node, const HitTestRequest &request,
                                                              const HitTestLocation &locationInContainer,
                                                              const FloatRect &rect) noexcept
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
      _innerURLElement = other._innerURLElement;
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

  dom::Element *HitTestResult::TargetElement() const noexcept
  {
    for (dom::Node *node = _innerNode.get(); node; node = node->ParentInComposedTree())
    {
      if (auto *element = DynamicDowncast<dom::Element>(*node))
      {
        return element;
      }
    }
    return nullptr;
  }

  dom::Element *HitTestResult::InnerNonSharedElement() const noexcept
  {
    auto *node = _innerNonSharedNode.get();
    if (!node)
    {
      return nullptr;
    }
    if (auto *element = DynamicDowncast<dom::Element>(*node))
    {
      return element;
    }
    return node->ParentElement();
  }
}
