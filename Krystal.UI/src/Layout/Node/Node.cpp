#include "Krystal.UI/Layout/Node/Node.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Utils/ToUnderlying.hpp"
#include "Krystal.UI/Layout/Config/Config.hpp"
#include "Krystal.UI/Styles/Helpers/FlexDirection.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>

namespace Krys::UI
{
  Node::Node() : Node(&Config::Defaults())
  {
  }

  Node::Node(const Config *config) : _config(config)
  {
    assert(config != nullptr && "Attempting to construct Node with null config");

    if (config->UseWebDefaults())
    {
      UseWebDefaults();
    }
  }

  Node::Node(Node &&node) noexcept
      : _hasNewLayout(node._hasNewLayout), _isReferenceBaseline(node._isReferenceBaseline),
        _isLayoutDirty(node._isLayoutDirty), _isStyleDirty(node._isStyleDirty),
        _alwaysFormsContainingBlock(node._alwaysFormsContainingBlock), _nodeType(node._nodeType),
        _context(node._context), _measureFunc(node._measureFunc), _baselineFunc(node._baselineFunc),
        _layoutDirtiedFunc(node._layoutDirtiedFunc), _styleDirtiedFunc(node._styleDirtiedFunc),
        _style(std::move(node._style)), _layout(node._layout), _lineIndex(node._lineIndex),
        _contentsChildrenCount(node._contentsChildrenCount), _owner(node._owner),
        _children(std::move(node._children)), _config(node._config),
        _processedDimensions(node._processedDimensions)
  {
    for (auto c : _children)
    {
      c->SetOwner(this);
    }
  }

  Size Node::Measure(float availableWidth, MeasureMode widthMode, float availableHeight,
                     MeasureMode heightMode)
  {
    const auto size = _measureFunc(this, availableWidth, widthMode, availableHeight, heightMode);

    if (Krys::IsUndefined(size.y) || size.y < 0 || Krys::IsUndefined(size.x) || size.x < 0)
    {
      return {MaxOrDefined(0.0f, size.x), MaxOrDefined(0.0f, size.y)};
    }

    return size;
  }

  float Node::Baseline(float width, float height) const
  {
    return _baselineFunc(this, width, height);
  }

  float Node::DimensionWithMargin(const FlexDirection axis, const float widthSize)
  {
    return GetLayout().GetMeasuredDimension(ToDimension(axis)) + _style.ComputeMarginForAxis(axis, widthSize);
  }

  bool Node::IsLayoutDimensionDefined(const FlexDirection axis)
  {
    const float value = GetLayout().GetMeasuredDimension(ToDimension(axis));
    return Krys::IsDefined(value) && value >= 0.0f;
  }

  // Setters

  void Node::SetMeasureFunc(MeasureFunc measureFunc)
  {
    if (measureFunc == nullptr)
    {
      SetNodeType(NodeType::Default);
    }
    else
    {
      assert(_children.empty()
             && "Cannot set measure function: Nodes with measure functions cannot have children.");
      SetNodeType(NodeType::Text);
    }

    _measureFunc = measureFunc;
  }

  void Node::ReplaceChild(Node *child, size_t index)
  {
    auto previousChild = _children[index];
    if (previousChild->GetStyle().GetDisplay() == Display::Contents
        && child->GetStyle().GetDisplay() != Display::Contents)
    {
      _contentsChildrenCount--;
    }
    else if (previousChild->GetStyle().GetDisplay() != Display::Contents
             && child->GetStyle().GetDisplay() == Display::Contents)
    {
      _contentsChildrenCount++;
    }

    _children[index] = child;
  }

  void Node::ReplaceChild(Node *oldChild, Node *newChild)
  {
    if (oldChild->GetStyle().GetDisplay() == Display::Contents
        && newChild->GetStyle().GetDisplay() != Display::Contents)
    {
      _contentsChildrenCount--;
    }
    else if (oldChild->GetStyle().GetDisplay() != Display::Contents
             && newChild->GetStyle().GetDisplay() == Display::Contents)
    {
      _contentsChildrenCount++;
    }

    std::replace(_children.begin(), _children.end(), oldChild, newChild);
  }

  void Node::InsertChild(Node *child, size_t index)
  {
    if (child->GetStyle().GetDisplay() == Display::Contents)
    {
      _contentsChildrenCount++;
    }

    _children.insert(_children.begin() + static_cast<ptrdiff_t>(index), child);
  }

  void Node::SetConfig(Config *config)
  {
    assert(config != nullptr && "Attempting to set a null config on a Node");
    assert(config->UseWebDefaults() == _config->UseWebDefaults()
           && "UseWebDefaults may not be changed after constructing a Node");

    if (ConfigUpdateInvalidatesLayout(*_config, *config))
    {
      MarkLayoutDirtyAndPropagate();
      _layout.ConfigVersion = 0;
    }
    else
    {
      // If the config is functionally the same, then align the configVersion so that we can reuse the layout
      // cache
      _layout.ConfigVersion = config->GetVersion();
    }

    _config = config;
  }

  void Node::SetLayoutDirty(bool isDirty)
  {
    if (static_cast<int>(isDirty) == _isLayoutDirty)
    {
      return;
    }

    _isLayoutDirty = isDirty;

    if (isDirty && (_layoutDirtiedFunc != nullptr))
    {
      _layoutDirtiedFunc(this);
    }
  }

  void Node::SetStyleDirty(bool isDirty)
  {
    if (static_cast<int>(isDirty) == _isStyleDirty)
    {
      return;
    }

    _isStyleDirty = isDirty;

    if (isDirty && (_styleDirtiedFunc != nullptr))
    {
      _styleDirtiedFunc(this);
    }
  }

  void Node::SetChildren(const List<Node *> &children)
  {
    _children = children;

    _contentsChildrenCount = 0;
    for (const auto &child : children)
    {
      if (child->GetStyle().GetDisplay() == Display::Contents)
      {
        _contentsChildrenCount++;
      }
    }
  }

  bool Node::RemoveChild(Node *child)
  {
    auto p = std::find(_children.begin(), _children.end(), child);
    if (p != _children.end())
    {
      if (child->GetStyle().GetDisplay() == Display::Contents)
      {
        _contentsChildrenCount--;
      }

      _children.erase(p);
      return true;
    }
    return false;
  }

  void Node::RemoveChild(size_t index)
  {
    if (_children[index]->GetStyle().GetDisplay() == Display::Contents)
    {
      _contentsChildrenCount--;
    }

    _children.erase(_children.begin() + static_cast<ptrdiff_t>(index));
  }

  void Node::SetLayoutDirection(Direction direction)
  {
    _layout.SetDirection(direction);
  }

  void Node::SetLayoutMargin(float margin, PhysicalEdge edge)
  {
    _layout.SetMargin(edge, margin);
  }

  void Node::SetLayoutBorder(float border, PhysicalEdge edge)
  {
    _layout.SetBorder(edge, border);
  }

  void Node::SetLayoutPadding(float padding, PhysicalEdge edge)
  {
    _layout.SetPadding(edge, padding);
  }

  void Node::SetLayoutLastOwnerDirection(Direction direction)
  {
    _layout.LastOwnerDirection = direction;
  }

  void Node::SetLayoutComputedFlexBasis(const NullableFloat computedFlexBasis)
  {
    _layout.ComputedFlexBasis = computedFlexBasis;
  }

  void Node::SetLayoutPosition(float position, PhysicalEdge edge)
  {
    _layout.SetPosition(edge, position);
  }

  void Node::SetLayoutComputedFlexBasisGeneration(uint16 computedFlexBasisGeneration)
  {
    _layout.ComputedFlexBasisGeneration = computedFlexBasisGeneration;
  }

  void Node::SetLayoutMeasuredDimension(float measuredDimension, Dimension dimension)
  {
    _layout.SetMeasuredDimension(dimension, measuredDimension);
  }

  void Node::SetLayoutHadOverflow(bool hadOverflow)
  {
    _layout.SetHadOverflow(hadOverflow);
  }

  void Node::SetLayoutDimension(float lengthValue, Dimension dimension)
  {
    _layout.SetDimension(dimension, lengthValue);
    _layout.SetRawDimension(dimension, lengthValue);
  }

  // If both left and right are defined, then use left. Otherwise return +left or
  // -right depending on which is defined. Ignore statically positioned nodes as
  // insets do not apply to them.
  float Node::RelativePosition(FlexDirection axis, Direction direction, float axisSize) const
  {
    if (_style.GetPositionType() == PositionType::Static)
    {
      return 0;
    }
    if (_style.IsInlineStartPositionDefined(axis, direction)
        && !_style.IsInlineStartPositionAuto(axis, direction))
    {
      return _style.ComputeInlineStartPosition(axis, direction, axisSize);
    }

    return -1 * _style.ComputeInlineEndPosition(axis, direction, axisSize);
  }

  void Node::SetPosition(const Direction direction, const float ownerWidth, const float ownerHeight)
  {
    /* Root nodes should be always layouted as LTR, so we don't return negative
     * values. */
    const Direction directionRespectingRoot = _owner != nullptr ? direction : Direction::LTR;
    const FlexDirection mainAxis = UI::ResolveDirection(_style.GetFlexDirection(), directionRespectingRoot);
    const FlexDirection crossAxis = UI::ResolveCrossDirection(mainAxis, directionRespectingRoot);

    // In the case of position static these are just 0. See:
    // https://www.w3.org/TR/css-position-3/#valdef-position-static
    const float relativePositionMain =
      RelativePosition(mainAxis, directionRespectingRoot, IsRow(mainAxis) ? ownerWidth : ownerHeight);
    const float relativePositionCross =
      RelativePosition(crossAxis, directionRespectingRoot, IsRow(mainAxis) ? ownerHeight : ownerWidth);

    const auto mainAxisLeadingEdge = InlineStartEdge(mainAxis, direction);
    const auto mainAxisTrailingEdge = InlineEndEdge(mainAxis, direction);
    const auto crossAxisLeadingEdge = InlineStartEdge(crossAxis, direction);
    const auto crossAxisTrailingEdge = InlineEndEdge(crossAxis, direction);

    SetLayoutPosition(
      (_style.ComputeInlineStartMargin(mainAxis, direction, ownerWidth) + relativePositionMain),
      mainAxisLeadingEdge);
    SetLayoutPosition((_style.ComputeInlineEndMargin(mainAxis, direction, ownerWidth) + relativePositionMain),
                      mainAxisTrailingEdge);
    SetLayoutPosition(
      (_style.ComputeInlineStartMargin(crossAxis, direction, ownerWidth) + relativePositionCross),
      crossAxisLeadingEdge);
    SetLayoutPosition(
      (_style.ComputeInlineEndMargin(crossAxis, direction, ownerWidth) + relativePositionCross),
      crossAxisTrailingEdge);
  }

  Style::SizeLength Node::ProcessFlexBasis() const
  {
    Style::SizeLength flexBasis = _style.GetFlexBasis();
    if (!flexBasis.IsAuto() && !flexBasis.IsUndefined())
    {
      return flexBasis;
    }
    if (_style.GetFlex().HasValue() && _style.GetFlex().Value() > 0.0f)
    {
      return _config->UseWebDefaults() ? StyleSizeLength::Auto() : StyleSizeLength::Points(0);
    }
    return StyleSizeLength::Auto();
  }

  NullableFloat Node::ResolveFlexBasis(Direction direction, FlexDirection flexDirection,
                                       float referenceLength, float ownerWidth) const
  {
    NullableFloat value = ProcessFlexBasis().Resolve(referenceLength);
    if (_style.GetBoxSizing() == BoxSizing::BorderBox)
    {
      return value;
    }

    Dimension dim = Dimension(flexDirection);
    NullableFloat dimensionPaddingAndBorder =
      NullableFloat {_style.ComputePaddingAndBorderForDimension(direction, dim, ownerWidth)};

    return value + (dimensionPaddingAndBorder.HasValue() ? dimensionPaddingAndBorder : NullableFloat {0.0});
  }

  void Node::ProcessDimensions()
  {
    for (auto dim : {Dimension::Width, Dimension::Height})
    {
      if (_style.GetMaxDimension(dim).IsDefined()
          && InexactEquals(_style.GetMaxDimension(dim), _style.GetMinDimension(dim)))
      {
        _processedDimensions[ToUnderlying(dim)] = _style.GetMaxDimension(dim);
      }
      else
      {
        _processedDimensions[ToUnderlying(dim)] = _style.GetDimension(dim);
      }
    }
  }

  Direction Node::ResolveDirection(const Direction ownerDirection)
  {
    if (_style.GetDirection() == Direction::Inherit)
    {
      return ownerDirection != Direction::Inherit ? ownerDirection : Direction::LTR;
    }
    else
    {
      return _style.GetDirection();
    }
  }

  void Node::ClearChildren()
  {
    _children.clear();
    _children.shrink_to_fit();
  }

  // Other Methods

  void Node::CloneChildrenIfNeeded()
  {
    size_t i = 0;
    for (Node *&child : _children)
    {
      if (child->GetOwner() != this)
      {
        child = _config->CloneNode(child, this, i);
        child->SetOwner(this);

        if (child->HasContentsChildren()) KRYS_UNLIKELY
        {
          child->CloneContentsChildrenIfNeeded();
        }
      }
      i += 1;
    }
  }

  void Node::CloneContentsChildrenIfNeeded()
  {
    size_t i = 0;
    for (Node *&child : _children)
    {
      if (child->GetStyle().GetDisplay() == Display::Contents && child->GetOwner() != this)
      {
        child = _config->CloneNode(child, this, i);
        child->SetOwner(this);
        child->CloneChildrenIfNeeded();
      }
      i += 1;
    }
  }

  void Node::MarkLayoutDirtyAndPropagate()
  {
    if (!_isLayoutDirty)
    {
      SetLayoutDirty(true);
      SetLayoutComputedFlexBasis(NullableFloat {});
      if (_owner != nullptr)
      {
        _owner->MarkLayoutDirtyAndPropagate();
      }
    }
  }

  void Node::MarkStyleDirtyAndPropagate()
  {
    if (!_isStyleDirty)
    {
      SetStyleDirty(true);
      if (_owner != nullptr)
      {
        _owner->MarkStyleDirtyAndPropagate();
      }
    }
  }

  float Node::ResolveFlexGrow() const
  {
    // Root nodes flexGrow should always be 0
    if (_owner == nullptr)
    {
      return 0.0;
    }
    if (_style.GetFlexGrow().HasValue())
    {
      return _style.GetFlexGrow().Value();
    }
    if (_style.GetFlex().HasValue() && _style.GetFlex().Value() > 0.0f)
    {
      return _style.GetFlex().Value();
    }

    return Style::DefaultFlexGrow;
  }

  float Node::ResolveFlexShrink() const
  {
    if (_owner == nullptr)
    {
      return 0.0;
    }
    if (_style.GetFlexShrink().HasValue())
    {
      return _style.GetFlexShrink().Value();
    }
    if (!_config->UseWebDefaults() && _style.GetFlex().HasValue() && _style.GetFlex().Value() < 0.0f)
    {
      return -_style.GetFlex().Value();
    }
    return _config->UseWebDefaults() ? Style::WebDefaultFlexShrink : Style::DefaultFlexShrink;
  }

  bool Node::IsNodeFlexible()
  {
    return ((_style.GetPositionType() != PositionType::Absolute)
            && (ResolveFlexGrow() != 0 || ResolveFlexShrink() != 0));
  }

  void Node::Reset()
  {
    assert(_children.empty() && "Cannot reset a node which still has children attached");
    assert(_owner == nullptr && "Cannot reset a node still attached to a owner");

    *this = Node {GetConfig()};
  }

}