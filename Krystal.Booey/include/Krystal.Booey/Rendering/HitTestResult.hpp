#pragma once

#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Rendering/HitTestLocation.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class LocalFrame;

  namespace dom
  {
    class Element;
    class Node;
  }

  enum class HitTestProgress
  {
    Stop,
    Continue
  };

  /// @brief What a hit test found: the innermost node under the point, the link element it's in, the point in
  /// its renderer's coordinates, and for list-based tests every node hit.
  /// FIXME: Widgets (isOverWidget's widgets), and the queries about what was hit (selection, spelling,
  /// titles, images, media, links' URLs and targets, editability, dictation) come with widgets, editing,
  /// images, media and navigation.
  class HitTestResult
  {
  public:
    /// @brief The nodes a list-based test hit, in the order they were hit, each once.
    using NodeSet = List<Ref<dom::Node>>;

  private:
    HitTestLocation _hitTestLocation;

    RefPtr<dom::Node> _innerNode;
    RefPtr<dom::Node> _innerNonSharedNode;
    DoublePoint _doublePointInInnerNodeFrame; // The hit-tested point in innerNode frame coordinates.
    // A point in the local coordinate space of _innerNonSharedNode's renderer. Allows us to efficiently
    // determine where inside the renderer we hit on subsequent operations.
    LayoutPoint _localPoint;
    RefPtr<dom::Element> _innerURLElement;
    RefPtr<Scrollbar> _scrollbar;
    // Returns true if we are over a widget (and not in the border/padding area of a RenderWidget for
    // example).
    bool _isOverWidget {false};
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;

    mutable UniquePtr<NodeSet> _listBasedTestResult;

  public:
    /// @brief The scrollbar hit, if any.
    KRYS_NODISCARD Scrollbar *GetScrollbar() const noexcept
    {
      return _scrollbar.get();
    }
    void SetScrollbar(RefPtr<Scrollbar> &&scrollbar) noexcept
    {
      _scrollbar = krys::move(scrollbar);
    }

    HitTestResult() noexcept;
    explicit HitTestResult(const LayoutPoint &point) noexcept;
    explicit HitTestResult(const LayoutRect &rect) noexcept;
    explicit HitTestResult(const HitTestLocation &location) noexcept;
    HitTestResult(const HitTestResult &other) noexcept;
    ~HitTestResult() noexcept;

    HitTestResult &operator=(const HitTestResult &other) noexcept;

    void SetInnerNode(dom::Node *node) noexcept;
    KRYS_NODISCARD dom::Node *InnerNode() const noexcept
    {
      return _innerNode.get();
    }

    void SetInnerNonSharedNode(dom::Node *node) noexcept;
    KRYS_NODISCARD dom::Node *InnerNonSharedNode() const noexcept
    {
      return _innerNonSharedNode.get();
    }

    KRYS_NODISCARD dom::Element *InnerNonSharedElement() const noexcept;

    void SetURLElement(dom::Element *element) noexcept;
    KRYS_NODISCARD dom::Element *URLElement() const noexcept
    {
      return _innerURLElement.get();
    }

    KRYS_NODISCARD bool IsOverWidget() const noexcept
    {
      return _isOverWidget;
    }
    void SetIsOverWidget(bool isOverWidget) noexcept
    {
      _isOverWidget = isOverWidget;
    }

    KRYS_NODISCARD Maybe<style::PseudoElementIdentifier> GetPseudoElementIdentifier() const noexcept
    {
      return _pseudoElementIdentifier;
    }
    void SetPseudoElementIdentifier(Maybe<style::PseudoElementIdentifier> pseudoElementIdentifier) noexcept
    {
      _pseudoElementIdentifier = pseudoElementIdentifier;
    }

    // Forwarded from HitTestLocation
    KRYS_NODISCARD bool IsRectBasedTest() const noexcept
    {
      return _hitTestLocation.IsRectBasedTest();
    }

    /// @brief The hit-tested point in the coordinates of the main frame.
    KRYS_NODISCARD const LayoutPoint &PointInMainFrame() const noexcept
    {
      return _hitTestLocation.Point();
    }
    KRYS_NODISCARD IntPoint RoundedPointInMainFrame() const noexcept
    {
      return RoundedIntPoint(PointInMainFrame());
    }

    /// @brief The hit-tested point in the coordinates of the frame containing the inner node.
    KRYS_NODISCARD LayoutPoint PointInInnerNodeFrame() const noexcept
    {
      return LayoutPoint(_doublePointInInnerNodeFrame);
    }
    KRYS_NODISCARD const DoublePoint &DoublePointInInnerNodeFrame() const noexcept
    {
      return _doublePointInInnerNodeFrame;
    }
    KRYS_NODISCARD LocalFrame *InnerNodeFrame() const noexcept;

    /// @brief The hit-tested point in the coordinates of the inner node's renderer.
    KRYS_NODISCARD const LayoutPoint &LocalPoint() const noexcept
    {
      return _localPoint;
    }
    void SetLocalPoint(const LayoutPoint &point) noexcept;

    /// @brief Moves the inner nodes out of user agent shadow trees, to their hosts.
    void SetToNonUserAgentShadowAncestor() noexcept;

    KRYS_NODISCARD const HitTestLocation &GetHitTestLocation() const noexcept
    {
      return _hitTestLocation;
    }

    KRYS_NODISCARD LocalFrame *Frame() const noexcept;
    KRYS_NODISCARD bool IsOverLink() const noexcept;

    /// @brief Adds @p node to the nodes a list-based test hit.
    /// @return Whether the test can stop: never for a test of every element under the point, else once @p
    /// rect covers the whole area tested.
    HitTestProgress AddNodeToListBasedTestResult(dom::Node *node, const HitTestRequest &request,
                                                 const HitTestLocation &locationInContainer,
                                                 const LayoutRect &rect = LayoutRect()) noexcept;
    HitTestProgress AddNodeToListBasedTestResult(dom::Node *node, const HitTestRequest &request,
                                                 const HitTestLocation &locationInContainer,
                                                 const FloatRect &rect) noexcept;
    /// @brief Takes @p other's inner node if there's none yet, and adds its list of nodes.
    void Append(const HitTestResult &other, const HitTestRequest &request) noexcept;

    KRYS_NODISCARD const NodeSet &ListBasedTestResult() const noexcept;

    KRYS_NODISCARD dom::Node *TargetNode() const noexcept
    {
      return InnerNode();
    }
    /// @brief The inner node, or the nearest element above it.
    KRYS_NODISCARD dom::Element *TargetElement() const noexcept;

  private:
    KRYS_NODISCARD NodeSet &MutableListBasedTestResult() noexcept;

    template <typename TRect>
    HitTestProgress AddNodeToListBasedTestResultCommon(dom::Node *node, const HitTestRequest &request,
                                                       const HitTestLocation &locationInContainer,
                                                       const TRect &rect) noexcept;
  };
}
