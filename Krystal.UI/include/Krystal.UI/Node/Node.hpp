/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.UI/Config/Config.hpp"
#include "Krystal.UI/Enums/Dimension.hpp"
#include "Krystal.UI/Enums/Direction.hpp"
#include "Krystal.UI/Enums/Edge.hpp"
#include "Krystal.UI/Enums/Errata.hpp"
#include "Krystal.UI/Enums/FlexDirection.hpp"
#include "Krystal.UI/Enums/MeasureMode.hpp"
#include "Krystal.UI/Enums/NodeType.hpp"
#include "Krystal.UI/Enums/PhysicalEdge.hpp"
#include "Krystal.UI/Node/LayoutableChildren.hpp"
#include "Krystal.UI/Node/LayoutResults.hpp"
#include "Krystal.UI/Style/Style.hpp"

namespace Krys::UI
{
  class Node
  {
  private:
    bool _hasNewLayout : 1 = true;
    bool _isReferenceBaseline : 1 = false;
    bool _isDirty : 1 = true;
    bool _alwaysFormsContainingBlock : 1 = false;
    NodeType _nodeType : BitCount<NodeType>() = NodeType::Default;
    void *_context = nullptr;
    MeasureFunc _measureFunc = nullptr;
    BaselineFunc _baselineFunc = nullptr;
    DirtiedFunc _dirtiedFunc = nullptr;
    Style _style;
    LayoutResults _layout;
    size_t _lineIndex = 0;
    size_t _contentsChildrenCount = 0;
    Node *_owner = nullptr;
    List<Node *> _children;
    const Config *_config;
    Array<Style::SizeLength, 2> _processedDimensions {
      {StyleSizeLength::undefined(), StyleSizeLength::undefined()}};

  public:
    using LayoutableChildren = LayoutableChildren<Node>;

    Node() noexcept;

    explicit Node(const Config *config) noexcept;

    Node(Node &&node) noexcept;

    // Does not expose true value semantics, as children are not cloned eagerly.
    // Should we remove this?
    Node(const Node &node) = default;

    // assignment means potential leaks of existing children, or alternatively
    // freeing unowned memory, double free, or freeing stack memory.
    Node &operator=(const Node &) = delete;

    // Getters
    void *GetContext() const
    {
      return _context;
    }

    bool AlwaysFormsContainingBlock() const
    {
      return _alwaysFormsContainingBlock;
    }

    bool GetHasNewLayout() const
    {
      return _hasNewLayout;
    }

    NodeType GetNodeType() const
    {
      return _nodeType;
    }

    bool HasMeasureFunc() const noexcept
    {
      return _measureFunc != nullptr;
    }

    Size Measure(float availableWidth, MeasureMode widthMode, float availableHeight, MeasureMode heightMode);

    bool hasBaselineFunc() const noexcept
    {
      return _baselineFunc != nullptr;
    }

    float Baseline(float width, float height) const;

    float DimensionWithMargin(FlexDirection axis, float widthSize);

    bool IsLayoutDimensionDefined(FlexDirection axis);

    /**
     * Whether the node has a "definite length" along the given axis.
     * https://www.w3.org/TR/css-sizing-3/#definite
     */
    bool HasDefiniteLength(Dimension dimension, float ownerSize)
    {
      auto usedValue = GetProcessedDimension(dimension).resolve(ownerSize);
      return usedValue.isDefined() && usedValue.unwrap() >= 0.0f;
    }

    bool HasErrata(Errata errata) const
    {
      return _config->HasErrata(errata);
    }

    bool HasContentsChildren() const
    {
      return _contentsChildrenCount != 0;
    }

    DirtiedFunc GetDirtiedFunc() const
    {
      return _dirtiedFunc;
    }

    // For Performance reasons passing as reference.
    Style &GetStyle()
    {
      return _style;
    }

    const Style &GetStyle() const
    {
      return _style;
    }

    // For Performance reasons passing as reference.
    LayoutResults &GetLayout()
    {
      return _layout;
    }

    const LayoutResults &GetLayout() const
    {
      return _layout;
    }

    size_t GetLineIndex() const
    {
      return _lineIndex;
    }

    bool IsReferenceBaseline() const
    {
      return _isReferenceBaseline;
    }

    // returns the Node that owns this Node. An owner is used to identify
    // the YogaTree that a Node belongs to. This method will return the parent
    // of the Node when a Node only belongs to one YogaTree or nullptr when
    // the Node is shared between two or more YogaTrees.
    Node *GetOwner() const
    {
      return _owner;
    }

    const List<Node *> &GetChildren() const
    {
      return _children;
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

    const Config *GetConfig() const
    {
      return _config;
    }

    bool IsDirty() const
    {
      return _isDirty;
    }

    Style::SizeLength GetProcessedDimension(Dimension dimension) const
    {
      return _processedDimensions[static_cast<size_t>(dimension)];
    }

    NullableFloat GetResolvedDimension(Direction direction, Dimension dimension, float referenceLength,
                                       float ownerWidth) const
    {
      NullableFloat value = GetProcessedDimension(dimension).resolve(referenceLength);
      if (_style.boxSizing() == BoxSizing::BorderBox)
      {
        return value;
      }

      NullableFloat dimensionPaddingAndBorder =
        NullableFloat {_style.computePaddingAndBorderForDimension(direction, dimension, ownerWidth)};

      return value + (dimensionPaddingAndBorder.HasValue() ? dimensionPaddingAndBorder : NullableFloat {0.0});
    }

    // Setters

    void SetContext(void *context)
    {
      _context = context;
    }

    void SetAlwaysFormsContainingBlock(bool alwaysFormsContainingBlock)
    {
      _alwaysFormsContainingBlock = alwaysFormsContainingBlock;
    }

    void SetHasNewLayout(bool hasNewLayout)
    {
      _hasNewLayout = hasNewLayout;
    }

    void SetNodeType(NodeType nodeType)
    {
      _nodeType = nodeType;
    }

    void SetMeasureFunc(MeasureFunc measureFunc);

    void SetBaselineFunc(BaselineFunc baseLineFunc)
    {
      _baselineFunc = baseLineFunc;
    }

    void SetDirtiedFunc(DirtiedFunc dirtiedFunc)
    {
      _dirtiedFunc = dirtiedFunc;
    }

    void SetStyle(const Style &style)
    {
      _style = style;
    }

    void SetLayout(const LayoutResults &layout)
    {
      _layout = layout;
    }

    void SetLineIndex(size_t lineIndex)
    {
      _lineIndex = lineIndex;
    }

    void SetIsReferenceBaseline(bool isReferenceBaseline)
    {
      _isReferenceBaseline = isReferenceBaseline;
    }

    void SetOwner(Node *owner)
    {
      _owner = owner;
    }

    // TODO: rvalue override for setChildren

    void SetConfig(Config *config);

    void SetDirty(bool isDirty);
    void SetChildren(const std::vector<Node *> &children);
    void SetLayoutLastOwnerDirection(Direction direction);
    void SetLayoutComputedFlexBasis(FloatOptional computedFlexBasis);
    void SetLayoutComputedFlexBasisGeneration(uint32_t computedFlexBasisGeneration);
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
    void MarkDirtyAndPropagate();
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
      _style.setFlexDirection(FlexDirection::Row);
      _style.setAlignContent(Align::Stretch);
    }
  };
}