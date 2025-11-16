#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.UI/Layout/Api/Forward.hpp"
#include "Krystal.UI/Layout/Config/Config.hpp"
#include "Krystal.UI/Layout/Node/LayoutableChildren.hpp"
#include "Krystal.UI/Layout/Node/LayoutResults.hpp"
#include "Krystal.UI/Styles/Enums/Dimension.hpp"
#include "Krystal.UI/Styles/Enums/Direction.hpp"
#include "Krystal.UI/Styles/Enums/Edge.hpp"
#include "Krystal.UI/Styles/Enums/Errata.hpp"
#include "Krystal.UI/Styles/Enums/FlexDirection.hpp"
#include "Krystal.UI/Styles/Enums/MeasureMode.hpp"
#include "Krystal.UI/Styles/Enums/NodeType.hpp"
#include "Krystal.UI/Styles/Enums/PhysicalEdge.hpp"
#include "Krystal.UI/Styles/Style.hpp"

namespace Krys::UI
{
  class Node
  {
    // assignment means potential leaks of existing children, or alternatively freeing unowned memory, double
    // free, or freeing stack memory.
    Node &operator=(const Node &) = delete;

  private:
    bool _hasNewLayout : 1 = true;
    bool _isReferenceBaseline : 1 = false;
    bool _isLayoutDirty : 1 = true;
    bool _isStyleDirty : 1 = true;
    bool _alwaysFormsContainingBlock : 1 = false;
    NodeType _nodeType : BitCount<NodeType>() = NodeType::Default;
    uint16 _lineIndex = 0;
    uint16 _contentsChildrenCount = 0;
    void *_context = nullptr;
    MeasureFunc _measureFunc = nullptr;
    BaselineFunc _baselineFunc = nullptr;
    DirtiedFunc _layoutDirtiedFunc = nullptr;
    DirtiedFunc _styleDirtiedFunc = nullptr;
    Style _style;
    LayoutResults _layout;
    Node *_owner = nullptr;
    List<Node *> _children;
    const Config *_config;
    Array<Style::SizeLength, 2> _processedDimensions {
      {StyleSizeLength::Undefined(), StyleSizeLength::Undefined()}};

  public:
    using LayoutableChildren = LayoutableChildren<Node>;

    Node();

    explicit Node(const Config *config);

    Node(Node &&node) noexcept;

    // Does not expose true value semantics, as children are not cloned eagerly.
    Node(const Node &node) = default;

    Style &GetStyle()
    {
      return _style;
    }

    const Style &GetStyle() const
    {
      return _style;
    }

    void SetStyle(const Style &style)
    {
      _style = style;
    }

    LayoutResults &GetLayout()
    {
      return _layout;
    }

    const LayoutResults &GetLayout() const
    {
      return _layout;
    }

    void SetLayout(const LayoutResults &layout)
    {
      _layout = layout;
    }

    void *GetContext() const
    {
      return _context;
    }

    /// @brief Sets extra data on the node which may be read from during callbacks.
    void SetContext(void *context)
    {
      _context = context;
    }

    bool HasBaselineFunc() const noexcept
    {
      return _baselineFunc != nullptr;
    }

    /// @brief Set a custom function for determining the text baseline for use in baseline alignment.
    void SetBaselineFunc(BaselineFunc baseLineFunc)
    {
      _baselineFunc = baseLineFunc;
    }

    DirtiedFunc GetLayoutDirtiedFunc() const
    {
      return _layoutDirtiedFunc;
    }

    void SetLayoutDirtiedFunc(DirtiedFunc dirtiedFunc)
    {
      _layoutDirtiedFunc = dirtiedFunc;
    }

    DirtiedFunc GetStyleDirtiedFunc() const
    {
      return _styleDirtiedFunc;
    }

    void SetStyleDirtiedFunc(DirtiedFunc dirtiedFunc)
    {
      _styleDirtiedFunc = dirtiedFunc;
    }

    /// @brief Allows providing custom measurements for a leaf node (usually for measuring text).
    /// NodeMarkDirty() must be set if content effecting the measurements of the node changes.
    void SetMeasureFunc(MeasureFunc measureFunc);

    bool HasMeasureFunc() const noexcept
    {
      return _measureFunc != nullptr;
    }

    Size Measure(float availableWidth, MeasureMode widthMode, float availableHeight, MeasureMode heightMode);

    float Baseline(float width, float height) const;

    /// @brief Whether the node will always form a containing block for any descendant. This can happen in
    /// situation where the client implements something like a transform that can affect containing blocks but
    /// is not handled by the layout engine directly.
    bool AlwaysFormsContainingBlock() const
    {
      return _alwaysFormsContainingBlock;
    }

    /// @brief Make it so that this node will always form a containing block for any descendant nodes. This is
    /// useful for when a node has a property outside of of the layout engine that will form a containing
    /// block. For example, transforms or some of the others listed in
    /// https://developer.mozilla.org/en-US/docs/Web/CSS/Containing_block
    void SetAlwaysFormsContainingBlock(bool alwaysFormsContainingBlock)
    {
      _alwaysFormsContainingBlock = alwaysFormsContainingBlock;
    }

    NodeType GetNodeType() const
    {
      return _nodeType;
    }

    void SetNodeType(NodeType nodeType)
    {
      _nodeType = nodeType;
    }

    bool GetHasNewLayout() const
    {
      return _hasNewLayout;
    }

    void SetHasNewLayout(bool hasNewLayout)
    {
      _hasNewLayout = hasNewLayout;
    }

    uint16 GetLineIndex() const
    {
      return _lineIndex;
    }

    void SetLineIndex(uint16 lineIndex)
    {
      _lineIndex = lineIndex;
    }

    bool IsReferenceBaseline() const
    {
      return _isReferenceBaseline;
    }

    void SetIsReferenceBaseline(bool isReferenceBaseline)
    {
      _isReferenceBaseline = isReferenceBaseline;
    }

    /// @brief Returns the Node that owns this Node. An owner is used to identify the Tree that a Node belongs
    /// to. This method will return the parent of the Node when a Node only belongs to one Tree or nullptr
    /// when the Node is shared between two or more Trees.
    Node *GetOwner() const
    {
      return _owner;
    }

    void SetOwner(Node *owner)
    {
      _owner = owner;
    }

    bool IsLayoutDirty() const
    {
      return _isLayoutDirty;
    }

    bool IsStyleDirty() const
    {
      return _isStyleDirty;
    }

    void SetLayoutDirty(bool isDirty);

    void SetStyleDirty(bool isDirty);

    const Config *GetConfig() const
    {
      return _config;
    }

    void SetConfig(Config *config);

    const List<Node *> &GetChildren() const
    {
      return _children;
    }

    void SetChildren(const List<Node *> &children);

    float DimensionWithMargin(FlexDirection axis, float widthSize);

    bool IsLayoutDimensionDefined(FlexDirection axis);

    /// @brief Whether the node has a "definite length" along the given axis.
    /// https://www.w3.org/TR/css-sizing-3/#definite
    bool HasDefiniteLength(Dimension dimension, float ownerSize) const
    {
      auto usedValue = GetProcessedDimension(dimension).Resolve(ownerSize);
      return usedValue.HasValue() && usedValue.Value() >= 0.0f;
    }

    bool HasErrata(Errata errata) const
    {
      return _config->HasErrata(errata);
    }

    bool HasContentsChildren() const
    {
      return _contentsChildrenCount != 0;
    }

    Node *GetChild(size_t index) const
    {
      return _children.at(index);
    }

    size_t GetChildCount() const
    {
      return _children.size();
    }

    LayoutableChildren GetLayoutChildren() const
    {
      return LayoutableChildren(this);
    }

    size_t GetLayoutChildCount() const
    {
      if (_contentsChildrenCount == 0)
      {
        return _children.size();
      }
      else
      {
        size_t count = 0;
        for (auto iter = GetLayoutChildren().begin(); iter != GetLayoutChildren().end(); iter++)
        {
          count++;
        }
        return count;
      }
    }

    Style::SizeLength GetProcessedDimension(Dimension dimension) const
    {
      return _processedDimensions[static_cast<size_t>(dimension)];
    }

    NullableFloat GetResolvedDimension(Direction direction, Dimension dimension, float referenceLength,
                                       float ownerWidth) const
    {
      NullableFloat value = GetProcessedDimension(dimension).Resolve(referenceLength);
      if (_style.GetBoxSizing() == BoxSizing::BorderBox)
      {
        return value;
      }

      NullableFloat dimensionPaddingAndBorder =
        NullableFloat {_style.ComputePaddingAndBorderForDimension(direction, dimension, ownerWidth)};

      return value + (dimensionPaddingAndBorder.HasValue() ? dimensionPaddingAndBorder : NullableFloat {0.0});
    }

    void SetLayoutLastOwnerDirection(Direction direction);
    void SetLayoutComputedFlexBasis(NullableFloat computedFlexBasis);
    void SetLayoutComputedFlexBasisGeneration(uint16 computedFlexBasisGeneration);
    void SetLayoutMeasuredDimension(float measuredDimension, Dimension dimension);
    void SetLayoutHadOverflow(bool hadOverflow);
    void SetLayoutDimension(float lengthValue, Dimension dimension);
    void SetLayoutDirection(Direction direction);
    void SetLayoutMargin(float margin, PhysicalEdge edge);
    void SetLayoutBorder(float border, PhysicalEdge edge);
    void SetLayoutPadding(float padding, PhysicalEdge edge);
    void SetLayoutPosition(float position, PhysicalEdge edge);
    void SetPosition(Direction direction, float ownerWidth, float ownerHeight);

    // Other methods
    Style::SizeLength ProcessFlexBasis() const;
    NullableFloat ResolveFlexBasis(Direction direction, FlexDirection flexDirection, float referenceLength,
                                   float ownerWidth) const;
    void ProcessDimensions();
    Direction ResolveDirection(Direction ownerDirection);
    void ClearChildren();
    /// Replaces the occurrences of oldChild with newChild
    void ReplaceChild(Node *oldChild, Node *newChild);
    void ReplaceChild(Node *child, size_t index);
    void InsertChild(Node *child, size_t index);
    /// Removes the first occurrence of child
    bool RemoveChild(Node *child);
    void RemoveChild(size_t index);

    void CloneChildrenIfNeeded();
    void CloneContentsChildrenIfNeeded();
    void MarkLayoutDirtyAndPropagate();
    void MarkStyleDirtyAndPropagate();
    float ResolveFlexGrow() const;
    float ResolveFlexShrink() const;
    bool IsNodeFlexible();
    void Reset();

  private:
    // Used to allow resetting the node
    Node &operator=(Node &&) noexcept = default;

    float RelativePosition(FlexDirection axis, Direction direction, float axisSize) const;

    void UseWebDefaults()
    {
      _style.SetFlexDirection(FlexDirection::Row);
      _style.SetAlignContent(Align::Stretch);
    }
  };
}