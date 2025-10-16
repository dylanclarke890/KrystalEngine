#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.UI/Algorithm/FlexDirection.hpp"
#include "Krystal.UI/Enums/Align.hpp"
#include "Krystal.UI/Enums/BoxSizing.hpp"
#include "Krystal.UI/Enums/Dimension.hpp"
#include "Krystal.UI/Enums/Direction.hpp"
#include "Krystal.UI/Enums/DisplayType.hpp"
#include "Krystal.UI/Enums/Edge.hpp"
#include "Krystal.UI/Enums/FlexDirection.hpp"
#include "Krystal.UI/Enums/Gutter.hpp"
#include "Krystal.UI/Enums/Justify.hpp"
#include "Krystal.UI/Enums/Overflow.hpp"
#include "Krystal.UI/Enums/PhysicalEdge.hpp"
#include "Krystal.UI/Enums/Position.hpp"
#include "Krystal.UI/Enums/Unit.hpp"
#include "Krystal.UI/Enums/Wrap.hpp"
#include "Krystal.UI/Style/StyleLength.hpp"
#include "Krystal.UI/Style/StyleSizeLength.hpp"
#include "Krystal.UI/Style/StyleValueHandle.hpp"
#include "Krystal.UI/Style/StyleValuePool.hpp"
#include <stdexcept>

namespace Krys::UI
{
  class Style
  {
    using Dimensions = Array<StyleValueHandle, OrdinalCount<Dimension>()>;
    using Edges = Array<StyleValueHandle, OrdinalCount<Edge>()>;
    using Gutters = Array<StyleValueHandle, OrdinalCount<Gutter>()>;

  private:
    Direction _direction : BitCount<Direction>() = Direction::Inherit;
    FlexDirection _flexDirection : BitCount<FlexDirection>() = FlexDirection::Column;
    Justify _justifyContent : BitCount<Justify>() = Justify::FlexStart;
    Align _alignContent : BitCount<Align>() = Align::FlexStart;
    Align _alignItems : BitCount<Align>() = Align::Stretch;
    Align _alignSelf : BitCount<Align>() = Align::Auto;
    PositionType _positionType : BitCount<PositionType>() = PositionType::Relative;
    Wrap _flexWrap : BitCount<Wrap>() = Wrap::NoWrap;
    Overflow _overflow : BitCount<Overflow>() = Overflow::Visible;
    DisplayType _display : BitCount<DisplayType>() = DisplayType::Flex;
    BoxSizing _boxSizing : BitCount<BoxSizing>() = BoxSizing::BorderBox;

    StyleValueHandle _flex {};
    StyleValueHandle _flexGrow {};
    StyleValueHandle _flexShrink {};
    StyleValueHandle _flexBasis {StyleValueHandle::Auto()};
    Edges _margin {};
    Edges _position {};
    Edges _padding {};
    Edges _border {};
    Gutters _gap {};
    Dimensions _dimensions {StyleValueHandle::Auto(), StyleValueHandle::Auto()};
    Dimensions _minDimensions {};
    Dimensions _maxDimensions {};
    StyleValueHandle _aspectRatio {};

    StyleValuePool _pool;

  public:
    using Length = StyleLength;
    using SizeLength = StyleSizeLength;

    static constexpr float DefaultFlexGrow = 0.0f;
    static constexpr float DefaultFlexShrink = 0.0f;
    static constexpr float WebDefaultFlexShrink = 1.0f;

    Direction GetDirection() const
    {
      return _direction;
    }

    void SetDirection(Direction value)
    {
      _direction = value;
    }

    FlexDirection GetFlexDirection() const
    {
      return _flexDirection;
    }

    void SetFlexDirection(FlexDirection value)
    {
      _flexDirection = value;
    }

    Justify GetJustifyContent() const
    {
      return _justifyContent;
    }

    void SetJustifyContent(Justify value)
    {
      _justifyContent = value;
    }

    Align GetAlignContent() const
    {
      return _alignContent;
    }

    void SetAlignContent(Align value)
    {
      _alignContent = value;
    }

    Align GetAlignItems() const
    {
      return _alignItems;
    }

    void SetAlignItems(Align value)
    {
      _alignItems = value;
    }

    Align GetAlignSelf() const
    {
      return _alignSelf;
    }

    void SetAlignSelf(Align value)
    {
      _alignSelf = value;
    }

    PositionType GetPositionType() const
    {
      return _positionType;
    }

    void SetPositionType(PositionType value)
    {
      _positionType = value;
    }

    Wrap GetFlexWrap() const
    {
      return _flexWrap;
    }

    void SetFlexWrap(Wrap value)
    {
      _flexWrap = value;
    }

    Overflow GetOverflow() const
    {
      return _overflow;
    }

    void SetOverflow(Overflow value)
    {
      _overflow = value;
    }

    DisplayType GetDisplay() const
    {
      return _display;
    }

    void SetDisplay(DisplayType value)
    {
      _display = value;
    }

    NullableFloat GetFlex() const
    {
      return _pool.GetNumber(_flex);
    }

    void SetFlex(NullableFloat value)
    {
      _pool.Store(_flex, value);
    }

    NullableFloat GetFlexGrow() const
    {
      return _pool.GetNumber(_flexGrow);
    }

    void SetFlexGrow(NullableFloat value)
    {
      _pool.Store(_flexGrow, value);
    }

    NullableFloat GetFlexShrink() const
    {
      return _pool.GetNumber(_flexShrink);
    }

    void SetFlexShrink(NullableFloat value)
    {
      _pool.Store(_flexShrink, value);
    }

    Style::SizeLength GetFlexBasis() const
    {
      return _pool.GetSize(_flexBasis);
    }

    void SetFlexBasis(Style::SizeLength value)
    {
      _pool.Store(_flexBasis, value);
    }

    Style::Length GetMargin(Edge edge) const
    {
      return _pool.GetLength(_margin[ToUnderlying(edge)]);
    }

    void SetMargin(Edge edge, Style::Length value)
    {
      _pool.Store(_margin[ToUnderlying(edge)], value);
    }

    Style::Length GetPosition(Edge edge) const
    {
      return _pool.GetLength(_position[ToUnderlying(edge)]);
    }

    void SetPosition(Edge edge, Style::Length value)
    {
      _pool.Store(_position[ToUnderlying(edge)], value);
    }

    Style::Length GetPadding(Edge edge) const
    {
      return _pool.GetLength(_padding[ToUnderlying(edge)]);
    }

    void SetPadding(Edge edge, Style::Length value)
    {
      _pool.Store(_padding[ToUnderlying(edge)], value);
    }

    Style::Length GetBorder(Edge edge) const
    {
      return _pool.GetLength(_border[ToUnderlying(edge)]);
    }

    void SetBorder(Edge edge, Style::Length value)
    {
      _pool.Store(_border[ToUnderlying(edge)], value);
    }

    Style::Length GetGap(Gutter gutter) const
    {
      return _pool.GetLength(_gap[ToUnderlying(gutter)]);
    }

    void SetGap(Gutter gutter, Style::Length value)
    {
      _pool.Store(_gap[ToUnderlying(gutter)], value);
    }

    Style::SizeLength GetDimension(Dimension axis) const
    {
      return _pool.GetSize(_dimensions[ToUnderlying(axis)]);
    }

    void SetDimension(Dimension axis, Style::SizeLength value)
    {
      _pool.Store(_dimensions[ToUnderlying(axis)], value);
    }

    Style::SizeLength GetMinDimension(Dimension axis) const
    {
      return _pool.GetSize(_minDimensions[ToUnderlying(axis)]);
    }

    void SetMinDimension(Dimension axis, Style::SizeLength value)
    {
      _pool.Store(_minDimensions[ToUnderlying(axis)], value);
    }

    NullableFloat ResolvedMinDimension(Direction direction, Dimension axis, float referenceLength,
                                       float ownerWidth) const
    {
      NullableFloat value = GetMinDimension(axis).Resolve(referenceLength);
      if (GetBoxSizing() == BoxSizing::BorderBox)
      {
        return value;
      }

      NullableFloat dimensionPaddingAndBorder =
        NullableFloat {ComputePaddingAndBorderForDimension(direction, axis, ownerWidth)};

      return value + (dimensionPaddingAndBorder.HasValue() ? dimensionPaddingAndBorder : NullableFloat {0.0});
    }

    Style::SizeLength GetMaxDimension(Dimension axis) const
    {
      return _pool.GetSize(_maxDimensions[ToUnderlying(axis)]);
    }

    void SetMaxDimension(Dimension axis, Style::SizeLength value)
    {
      _pool.Store(_maxDimensions[ToUnderlying(axis)], value);
    }

    NullableFloat ResolvedMaxDimension(Direction direction, Dimension axis, float referenceLength,
                                       float ownerWidth) const
    {
      NullableFloat value = GetMaxDimension(axis).Resolve(referenceLength);
      if (GetBoxSizing() == BoxSizing::BorderBox)
      {
        return value;
      }

      NullableFloat dimensionPaddingAndBorder =
        NullableFloat {ComputePaddingAndBorderForDimension(direction, axis, ownerWidth)};

      return value + (dimensionPaddingAndBorder.HasValue() ? dimensionPaddingAndBorder : NullableFloat {0.0});
    }

    NullableFloat GetAspectRatio() const
    {
      return _pool.GetNumber(_aspectRatio);
    }

    void SetAspectRatio(NullableFloat value)
    {
      // degenerate aspect ratios act as auto, see:
      // https://drafts.csswg.org/css-sizing-4/#valdef-aspect-ratio-ratio.
      _pool.Store(_aspectRatio, value == 0.0f || IsInf(value.Value()) ? NullableFloat {} : value);
    }

    BoxSizing GetBoxSizing() const
    {
      return _boxSizing;
    }

    void SetBoxSizing(BoxSizing value)
    {
      _boxSizing = value;
    }

    bool HorizontalInsetsDefined() const
    {
      return _position[ToUnderlying(Edge::Left)].IsDefined()
             || _position[ToUnderlying(Edge::Right)].IsDefined()
             || _position[ToUnderlying(Edge::All)].IsDefined()
             || _position[ToUnderlying(Edge::Horizontal)].IsDefined()
             || _position[ToUnderlying(Edge::Start)].IsDefined()
             || _position[ToUnderlying(Edge::End)].IsDefined();
    }

    bool VerticalInsetsDefined() const
    {
      return _position[ToUnderlying(Edge::Top)].IsDefined()
             || _position[ToUnderlying(Edge::Bottom)].IsDefined()
             || _position[ToUnderlying(Edge::All)].IsDefined()
             || _position[ToUnderlying(Edge::Vertical)].IsDefined();
    }

    bool IsFlexStartPositionDefined(FlexDirection axis, Direction direction) const
    {
      return ComputePosition(FlexStartEdge(axis), direction).IsDefined();
    }

    bool IsFlexStartPositionAuto(FlexDirection axis, Direction direction) const
    {
      return ComputePosition(FlexStartEdge(axis), direction).IsAuto();
    }

    bool IsInlineStartPositionDefined(FlexDirection axis, Direction direction) const
    {
      return ComputePosition(InlineStartEdge(axis, direction), direction).IsDefined();
    }

    bool IsInlineStartPositionAuto(FlexDirection axis, Direction direction) const
    {
      return ComputePosition(InlineStartEdge(axis, direction), direction).IsAuto();
    }

    bool IsFlexEndPositionDefined(FlexDirection axis, Direction direction) const
    {
      return ComputePosition(FlexEndEdge(axis), direction).IsDefined();
    }

    bool IsFlexEndPositionAuto(FlexDirection axis, Direction direction) const
    {
      return ComputePosition(FlexEndEdge(axis), direction).IsAuto();
    }

    bool IsInlineEndPositionDefined(FlexDirection axis, Direction direction) const
    {
      return ComputePosition(InlineEndEdge(axis, direction), direction).IsDefined();
    }

    bool IsInlineEndPositionAuto(FlexDirection axis, Direction direction) const
    {
      return ComputePosition(InlineEndEdge(axis, direction), direction).IsAuto();
    }

    float ComputeFlexStartPosition(FlexDirection axis, Direction direction, float axisSize) const
    {
      return ComputePosition(FlexStartEdge(axis), direction).Resolve(axisSize).ValueOrDefault(0.0f);
    }

    float ComputeInlineStartPosition(FlexDirection axis, Direction direction, float axisSize) const
    {
      return ComputePosition(InlineStartEdge(axis, direction), direction)
        .Resolve(axisSize)
        .ValueOrDefault(0.0f);
    }

    float ComputeFlexEndPosition(FlexDirection axis, Direction direction, float axisSize) const
    {
      return ComputePosition(FlexEndEdge(axis), direction).Resolve(axisSize).ValueOrDefault(0.0f);
    }

    float ComputeInlineEndPosition(FlexDirection axis, Direction direction, float axisSize) const
    {
      return ComputePosition(InlineEndEdge(axis, direction), direction)
        .Resolve(axisSize)
        .ValueOrDefault(0.0f);
    }

    float ComputeFlexStartMargin(FlexDirection axis, Direction direction, float widthSize) const
    {
      return ComputeMargin(FlexStartEdge(axis), direction).Resolve(widthSize).ValueOrDefault(0.0f);
    }

    float ComputeInlineStartMargin(FlexDirection axis, Direction direction, float widthSize) const
    {
      return ComputeMargin(InlineStartEdge(axis, direction), direction)
        .Resolve(widthSize)
        .ValueOrDefault(0.0f);
    }

    float ComputeFlexEndMargin(FlexDirection axis, Direction direction, float widthSize) const
    {
      return ComputeMargin(FlexEndEdge(axis), direction).Resolve(widthSize).ValueOrDefault(0.0f);
    }

    float ComputeInlineEndMargin(FlexDirection axis, Direction direction, float widthSize) const
    {
      return ComputeMargin(InlineEndEdge(axis, direction), direction).Resolve(widthSize).ValueOrDefault(0.0f);
    }

    float ComputeFlexStartBorder(FlexDirection axis, Direction direction) const
    {
      return MaxOrDefined(ComputeBorder(FlexStartEdge(axis), direction).Resolve(0.0f).Value(), 0.0f);
    }

    float ComputeInlineStartBorder(FlexDirection axis, Direction direction) const
    {
      return MaxOrDefined(ComputeBorder(InlineStartEdge(axis, direction), direction).Resolve(0.0f).Value(),
                          0.0f);
    }

    float ComputeFlexEndBorder(FlexDirection axis, Direction direction) const
    {
      return MaxOrDefined(ComputeBorder(FlexEndEdge(axis), direction).Resolve(0.0f).Value(), 0.0f);
    }

    float ComputeInlineEndBorder(FlexDirection axis, Direction direction) const
    {
      return MaxOrDefined(ComputeBorder(InlineEndEdge(axis, direction), direction).Resolve(0.0f).Value(),
                          0.0f);
    }

    float ComputeFlexStartPadding(FlexDirection axis, Direction direction, float widthSize) const
    {
      return MaxOrDefined(ComputePadding(FlexStartEdge(axis), direction).Resolve(widthSize).Value(), 0.0f);
    }

    float ComputeInlineStartPadding(FlexDirection axis, Direction direction, float widthSize) const
    {
      return MaxOrDefined(
        ComputePadding(InlineStartEdge(axis, direction), direction).Resolve(widthSize).Value(), 0.0f);
    }

    float ComputeFlexEndPadding(FlexDirection axis, Direction direction, float widthSize) const
    {
      return MaxOrDefined(ComputePadding(FlexEndEdge(axis), direction).Resolve(widthSize).Value(), 0.0f);
    }

    float ComputeInlineEndPadding(FlexDirection axis, Direction direction, float widthSize) const
    {
      return MaxOrDefined(
        ComputePadding(InlineEndEdge(axis, direction), direction).Resolve(widthSize).Value(), 0.0f);
    }

    float ComputeInlineStartPaddingAndBorder(FlexDirection axis, Direction direction, float widthSize) const
    {
      return ComputeInlineStartPadding(axis, direction, widthSize)
             + ComputeInlineStartBorder(axis, direction);
    }

    float ComputeFlexStartPaddingAndBorder(FlexDirection axis, Direction direction, float widthSize) const
    {
      return ComputeFlexStartPadding(axis, direction, widthSize) + ComputeFlexStartBorder(axis, direction);
    }

    float ComputeInlineEndPaddingAndBorder(FlexDirection axis, Direction direction, float widthSize) const
    {
      return ComputeInlineEndPadding(axis, direction, widthSize) + ComputeInlineEndBorder(axis, direction);
    }

    float ComputeFlexEndPaddingAndBorder(FlexDirection axis, Direction direction, float widthSize) const
    {
      return ComputeFlexEndPadding(axis, direction, widthSize) + ComputeFlexEndBorder(axis, direction);
    }

    float ComputePaddingAndBorderForDimension(Direction direction, Dimension dimension, float widthSize) const
    {
      FlexDirection flexDirectionForDimension =
        dimension == Dimension::Width ? FlexDirection::Row : FlexDirection::Column;

      return ComputeFlexStartPaddingAndBorder(flexDirectionForDimension, direction, widthSize)
             + ComputeFlexEndPaddingAndBorder(flexDirectionForDimension, direction, widthSize);
    }

    float ComputeBorderForAxis(FlexDirection axis) const
    {
      return ComputeInlineStartBorder(axis, Direction::LTR) + ComputeInlineEndBorder(axis, Direction::LTR);
    }

    float ComputeMarginForAxis(FlexDirection axis, float widthSize) const
    {
      // The total margin for a given axis does not depend on the direction
      // so hardcoding LTR here to avoid piping direction to this function
      return ComputeInlineStartMargin(axis, Direction::LTR, widthSize)
             + ComputeInlineEndMargin(axis, Direction::LTR, widthSize);
    }

    float ComputeGapForAxis(FlexDirection axis, float ownerSize) const
    {
      auto gap = IsRow(axis) ? ComputeColumnGap() : ComputeRowGap();
      return MaxOrDefined(gap.Resolve(ownerSize).Value(), 0.0f);
    }

    bool FlexStartMarginIsAuto(FlexDirection axis, Direction direction) const
    {
      return ComputeMargin(FlexStartEdge(axis), direction).IsAuto();
    }

    bool FlexEndMarginIsAuto(FlexDirection axis, Direction direction) const
    {
      return ComputeMargin(FlexEndEdge(axis), direction).IsAuto();
    }

    bool operator==(const Style &other) const
    {
      return _direction == other._direction && _flexDirection == other._flexDirection
             && _justifyContent == other._justifyContent && _alignContent == other._alignContent
             && _alignItems == other._alignItems && _alignSelf == other._alignSelf
             && _positionType == other._positionType && _flexWrap == other._flexWrap
             && _overflow == other._overflow && _display == other._display
             && NumbersEqual(_flex, _pool, other._flex, other._pool)
             && NumbersEqual(_flexGrow, _pool, other._flexGrow, other._pool)
             && NumbersEqual(_flexShrink, _pool, other._flexShrink, other._pool)
             && LengthsEqual(_flexBasis, _pool, other._flexBasis, other._pool)
             && LengthsEqual(_margin, _pool, other._margin, other._pool)
             && LengthsEqual(_position, _pool, other._position, other._pool)
             && LengthsEqual(_padding, _pool, other._padding, other._pool)
             && LengthsEqual(_border, _pool, other._border, other._pool)
             && LengthsEqual(_gap, _pool, other._gap, other._pool)
             && LengthsEqual(_dimensions, _pool, other._dimensions, other._pool)
             && LengthsEqual(_minDimensions, _pool, other._minDimensions, other._pool)
             && LengthsEqual(_maxDimensions, _pool, other._maxDimensions, other._pool)
             && NumbersEqual(_aspectRatio, _pool, other._aspectRatio, other._pool);
    }

    bool operator!=(const Style &other) const
    {
      return !(*this == other);
    }

  private:
    static inline bool NumbersEqual(const StyleValueHandle &lhsHandle, const StyleValuePool &lhsPool,
                                    const StyleValueHandle &rhsHandle, const StyleValuePool &rhsPool)
    {
      return (lhsHandle.IsUndefined() && rhsHandle.IsUndefined())
             || (lhsPool.GetNumber(lhsHandle) == rhsPool.GetNumber(rhsHandle));
    }

    static inline bool LengthsEqual(const StyleValueHandle &lhsHandle, const StyleValuePool &lhsPool,
                                    const StyleValueHandle &rhsHandle, const StyleValuePool &rhsPool)
    {
      return (lhsHandle.IsUndefined() && rhsHandle.IsUndefined())
             || (lhsPool.GetLength(lhsHandle) == rhsPool.GetLength(rhsHandle));
    }

    template <size_t N>
    static inline bool LengthsEqual(const std::array<StyleValueHandle, N> &lhs, const StyleValuePool &lhsPool,
                                    const std::array<StyleValueHandle, N> &rhs, const StyleValuePool &rhsPool)
    {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [&](const auto &lhs, const auto &rhs)
                        { return LengthsEqual(lhs, lhsPool, rhs, rhsPool); });
    }

    Style::Length ComputeColumnGap() const
    {
      if (_gap[ToUnderlying(Gutter::Column)].IsDefined())
      {
        return _pool.GetLength(_gap[ToUnderlying(Gutter::Column)]);
      }
      else
      {
        return _pool.GetLength(_gap[ToUnderlying(Gutter::All)]);
      }
    }

    Style::Length ComputeRowGap() const
    {
      if (_gap[ToUnderlying(Gutter::Row)].IsDefined())
      {
        return _pool.GetLength(_gap[ToUnderlying(Gutter::Row)]);
      }
      else
      {
        return _pool.GetLength(_gap[ToUnderlying(Gutter::All)]);
      }
    }

    Style::Length ComputeLeftEdge(const Edges &edges, Direction layoutDirection) const
    {
      if (layoutDirection == Direction::LTR && edges[ToUnderlying(Edge::Start)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Start)]);
      }
      else if (layoutDirection == Direction::RTL && edges[ToUnderlying(Edge::End)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::End)]);
      }
      else if (edges[ToUnderlying(Edge::Left)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Left)]);
      }
      else if (edges[ToUnderlying(Edge::Horizontal)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Horizontal)]);
      }
      else
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::All)]);
      }
    }

    Style::Length ComputeTopEdge(const Edges &edges) const
    {
      if (edges[ToUnderlying(Edge::Top)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Top)]);
      }
      else if (edges[ToUnderlying(Edge::Vertical)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Vertical)]);
      }
      else
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::All)]);
      }
    }

    Style::Length ComputeRightEdge(const Edges &edges, Direction layoutDirection) const
    {
      if (layoutDirection == Direction::LTR && edges[ToUnderlying(Edge::End)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::End)]);
      }
      else if (layoutDirection == Direction::RTL && edges[ToUnderlying(Edge::Start)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Start)]);
      }
      else if (edges[ToUnderlying(Edge::Right)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Right)]);
      }
      else if (edges[ToUnderlying(Edge::Horizontal)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Horizontal)]);
      }
      else
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::All)]);
      }
    }

    Style::Length ComputeBottomEdge(const Edges &edges) const
    {
      if (edges[ToUnderlying(Edge::Bottom)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Bottom)]);
      }
      else if (edges[ToUnderlying(Edge::Vertical)].IsDefined())
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::Vertical)]);
      }
      else
      {
        return _pool.GetLength(edges[ToUnderlying(Edge::All)]);
      }
    }

    Style::Length ComputePosition(PhysicalEdge edge, Direction direction) const
    {
      switch (edge)
      {
        case PhysicalEdge::Left:   return ComputeLeftEdge(_position, direction);
        case PhysicalEdge::Top:    return ComputeTopEdge(_position);
        case PhysicalEdge::Right:  return ComputeRightEdge(_position, direction);
        case PhysicalEdge::Bottom: return ComputeBottomEdge(_position);
      }

      throw std::logic_error("invalid physical edge");
    }

    Style::Length ComputeMargin(PhysicalEdge edge, Direction direction) const
    {
      switch (edge)
      {
        case PhysicalEdge::Left:   return ComputeLeftEdge(_margin, direction);
        case PhysicalEdge::Top:    return ComputeTopEdge(_margin);
        case PhysicalEdge::Right:  return ComputeRightEdge(_margin, direction);
        case PhysicalEdge::Bottom: return ComputeBottomEdge(_margin);
      }

      throw std::logic_error("invalid physical edge");
    }

    Style::Length ComputePadding(PhysicalEdge edge, Direction direction) const
    {
      switch (edge)
      {
        case PhysicalEdge::Left:   return ComputeLeftEdge(_padding, direction);
        case PhysicalEdge::Top:    return ComputeTopEdge(_padding);
        case PhysicalEdge::Right:  return ComputeRightEdge(_padding, direction);
        case PhysicalEdge::Bottom: return ComputeBottomEdge(_padding);
      }

      throw std::logic_error("invalid physical edge");
    }

    Style::Length ComputeBorder(PhysicalEdge edge, Direction direction) const
    {
      switch (edge)
      {
        case PhysicalEdge::Left:   return ComputeLeftEdge(_border, direction);
        case PhysicalEdge::Top:    return ComputeTopEdge(_border);
        case PhysicalEdge::Right:  return ComputeRightEdge(_border, direction);
        case PhysicalEdge::Bottom: return ComputeBottomEdge(_border);
      }

      throw std::logic_error("invalid physical edge");
    }
  };
}