#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/NullableFloat.hpp"
#include "Krystal.UI/Styles/Enums/Align.hpp"
#include "Krystal.UI/Styles/Enums/BoxSizing.hpp"
#include "Krystal.UI/Styles/Enums/Dimension.hpp"
#include "Krystal.UI/Styles/Enums/Direction.hpp"
#include "Krystal.UI/Styles/Enums/Display.hpp"
#include "Krystal.UI/Styles/Enums/Edge.hpp"
#include "Krystal.UI/Styles/Enums/FlexDirection.hpp"
#include "Krystal.UI/Styles/Enums/Gutter.hpp"
#include "Krystal.UI/Styles/Enums/Justify.hpp"
#include "Krystal.UI/Styles/Enums/Overflow.hpp"
#include "Krystal.UI/Styles/Enums/PhysicalEdge.hpp"
#include "Krystal.UI/Styles/Enums/Position.hpp"
#include "Krystal.UI/Styles/Enums/TextAlign.hpp"
#include "Krystal.UI/Styles/Enums/Unit.hpp"
#include "Krystal.UI/Styles/Enums/Wrap.hpp"
#include "Krystal.UI/Styles/Helpers/FlexDirection.hpp"
#include "Krystal.UI/Styles/Values/StyleLength.hpp"
#include "Krystal.UI/Styles/Values/StyleSizeLength.hpp"
#include "Krystal.UI/Styles/Values/StyleValueHandle.hpp"
#include "Krystal.UI/Styles/Values/StyleValuePool.hpp"
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
    Display _display : BitCount<Display>() = Display::Flex;
    BoxSizing _boxSizing : BitCount<BoxSizing>() = BoxSizing::BorderBox;
    TextAlign _textAlign : BitCount<TextAlign>() = {TextAlign::Left};

    StyleValueHandle _flex {};
    StyleValueHandle _flexGrow {};
    StyleValueHandle _flexShrink {};
    StyleValueHandle _flexBasis {StyleValueHandle::Auto()};
    Gutters _gap {};

    Edges _margin {};
    Edges _position {};
    Edges _padding {};
    Edges _border {};

    Dimensions _dimensions {StyleValueHandle::Auto(), StyleValueHandle::Auto()};
    Dimensions _minDimensions {};
    Dimensions _maxDimensions {};
    StyleValueHandle _aspectRatio {};
    StyleValuePool _pool;

    Gfx::ColourbPremultiplied _backgroundColour {Gfx::Colours::Transparent};
    Gfx::ColourbPremultiplied _textColour {Gfx::Colours::Black};
    float _opacity {1.f};
    Array<Gfx::ColourbPremultiplied, 4> _borderColours {Gfx::Colours::Black, Gfx::Colours::Black,
                                                        Gfx::Colours::Black, Gfx::Colours::Black};
    Array<float, 4> _borderWidths {0.f, 0.f, 0.f, 0.f};
    Array<float, 4> _borderRadii {0.f, 0.f, 0.f, 0.f};

    Gfx::FontFamilyHandle _family {};
    float _fontSize {16.f};

  public:
    using Length = StyleLength;
    using SizeLength = StyleSizeLength;

    static constexpr float DefaultFlexGrow = 0.0f;
    static constexpr float DefaultFlexShrink = 0.0f;
    static constexpr float WebDefaultFlexShrink = 1.0f;

    void SetOpacity(float opacity) noexcept
    {
      _opacity = opacity;
    }

    NO_DISCARD float GetOpacity() const noexcept
    {
      return _opacity;
    }

#pragma region Enums

    NO_DISCARD Direction GetDirection() const noexcept
    {
      return _direction;
    }

    void SetDirection(Direction value) noexcept
    {
      _direction = value;
    }

    NO_DISCARD FlexDirection GetFlexDirection() const noexcept
    {
      return _flexDirection;
    }

    void SetFlexDirection(FlexDirection value) noexcept
    {
      _flexDirection = value;
    }

    NO_DISCARD Justify GetJustifyContent() const noexcept
    {
      return _justifyContent;
    }

    void SetJustifyContent(Justify value) noexcept
    {
      _justifyContent = value;
    }

    NO_DISCARD Align GetAlignContent() const noexcept
    {
      return _alignContent;
    }

    void SetAlignContent(Align value) noexcept
    {
      _alignContent = value;
    }

    NO_DISCARD Align GetAlignItems() const noexcept
    {
      return _alignItems;
    }

    void SetAlignItems(Align value) noexcept
    {
      _alignItems = value;
    }

    NO_DISCARD Align GetAlignSelf() const noexcept
    {
      return _alignSelf;
    }

    void SetAlignSelf(Align value) noexcept
    {
      _alignSelf = value;
    }

    NO_DISCARD PositionType GetPositionType() const noexcept
    {
      return _positionType;
    }

    void SetPositionType(PositionType value) noexcept
    {
      _positionType = value;
    }

    NO_DISCARD Wrap GetFlexWrap() const noexcept
    {
      return _flexWrap;
    }

    void SetFlexWrap(Wrap value) noexcept
    {
      _flexWrap = value;
    }

    NO_DISCARD Overflow GetOverflow() const noexcept
    {
      return _overflow;
    }

    void SetOverflow(Overflow value) noexcept
    {
      _overflow = value;
    }

    NO_DISCARD Display GetDisplay() const noexcept
    {
      return _display;
    }

    void SetDisplay(Display value)
    {
      _display = value;
    }

    NO_DISCARD BoxSizing GetBoxSizing() const noexcept
    {
      return _boxSizing;
    }

    void SetBoxSizing(BoxSizing value) noexcept
    {
      _boxSizing = value;
    }

#pragma endregion

#pragma region Flex

    NO_DISCARD NullableFloat GetFlex() const noexcept
    {
      return _pool.GetNumber(_flex);
    }

    void SetFlex(NullableFloat value) noexcept
    {
      _pool.Store(_flex, value);
    }

    NO_DISCARD NullableFloat GetFlexGrow() const noexcept
    {
      return _pool.GetNumber(_flexGrow);
    }

    void SetFlexGrow(NullableFloat value) noexcept
    {
      _pool.Store(_flexGrow, value);
    }

    NO_DISCARD NullableFloat GetFlexShrink() const noexcept
    {
      return _pool.GetNumber(_flexShrink);
    }

    void SetFlexShrink(NullableFloat value) noexcept
    {
      _pool.Store(_flexShrink, value);
    }

    NO_DISCARD Style::SizeLength GetFlexBasis() const noexcept
    {
      return _pool.GetSize(_flexBasis);
    }

    void SetFlexBasis(Style::SizeLength value) noexcept
    {
      _pool.Store(_flexBasis, value);
    }

    NO_DISCARD Style::Length GetGap(Gutter gutter) const noexcept
    {
      return _pool.GetLength(_gap[ToUnderlying(gutter)]);
    }

    void SetGap(Gutter gutter, Style::Length value) noexcept
    {
      _pool.Store(_gap[ToUnderlying(gutter)], value);
    }

#pragma endregion

#pragma region Edges

    NO_DISCARD Style::Length GetMargin(Edge edge) const noexcept
    {
      return _pool.GetLength(_margin[ToUnderlying(edge)]);
    }

    void SetMargin(Edge edge, Style::Length value) noexcept
    {
      _pool.Store(_margin[ToUnderlying(edge)], value);
    }

    NO_DISCARD Style::Length GetPosition(Edge edge) const noexcept
    {
      return _pool.GetLength(_position[ToUnderlying(edge)]);
    }

    void SetPosition(Edge edge, Style::Length value) noexcept
    {
      _pool.Store(_position[ToUnderlying(edge)], value);
    }

    NO_DISCARD Style::Length GetPadding(Edge edge) const noexcept
    {
      return _pool.GetLength(_padding[ToUnderlying(edge)]);
    }

    void SetPadding(Edge edge, Style::Length value) noexcept
    {
      _pool.Store(_padding[ToUnderlying(edge)], value);
    }

    NO_DISCARD Style::Length GetBorder(Edge edge) const noexcept
    {
      return _pool.GetLength(_border[ToUnderlying(edge)]);
    }

    void SetBorder(Edge edge, Style::Length value) noexcept
    {
      _pool.Store(_border[ToUnderlying(edge)], value);
    }

#pragma endregion

#pragma region Dimensions

    NO_DISCARD Style::SizeLength GetDimension(Dimension axis) const noexcept
    {
      return _pool.GetSize(_dimensions[ToUnderlying(axis)]);
    }

    void SetDimension(Dimension axis, Style::SizeLength value) noexcept
    {
      _pool.Store(_dimensions[ToUnderlying(axis)], value);
    }

    NO_DISCARD Style::SizeLength GetMinDimension(Dimension axis) const noexcept
    {
      return _pool.GetSize(_minDimensions[ToUnderlying(axis)]);
    }

    void SetMinDimension(Dimension axis, Style::SizeLength value) noexcept
    {
      _pool.Store(_minDimensions[ToUnderlying(axis)], value);
    }

    NO_DISCARD NullableFloat ResolvedMinDimension(Direction direction, Dimension axis, float referenceLength,
                                                  float ownerWidth) const noexcept
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

    NO_DISCARD Style::SizeLength GetMaxDimension(Dimension axis) const noexcept
    {
      return _pool.GetSize(_maxDimensions[ToUnderlying(axis)]);
    }

    void SetMaxDimension(Dimension axis, Style::SizeLength value) noexcept
    {
      _pool.Store(_maxDimensions[ToUnderlying(axis)], value);
    }

    NullableFloat ResolvedMaxDimension(Direction direction, Dimension axis, float referenceLength,
                                       float ownerWidth) const noexcept
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

    NO_DISCARD NullableFloat GetAspectRatio() const noexcept
    {
      return _pool.GetNumber(_aspectRatio);
    }

    void SetAspectRatio(NullableFloat value) noexcept
    {
      // degenerate aspect ratios act as auto, see:
      // https://drafts.csswg.org/css-sizing-4/#valdef-aspect-ratio-ratio.
      _pool.Store(_aspectRatio, value == 0.0f || IsInf(value.Value()) ? NullableFloat {} : value);
    }

#pragma endregion

#pragma region Background

    NO_DISCARD Gfx::ColourbPremultiplied GetBackgroundColour() const noexcept
    {
      return _backgroundColour;
    }

    void SetBackgroundColour(Gfx::ColourbPremultiplied colour) noexcept
    {
      _backgroundColour = colour;
    }

#pragma endregion

#pragma region Text

    NO_DISCARD Gfx::ColourbPremultiplied GetTextColour() const noexcept
    {
      return _textColour;
    }

    void SetTextColour(Gfx::ColourbPremultiplied colour) noexcept
    {
      _textColour = colour;
    }

    void SetFontFamily(Gfx::FontFamilyHandle family) noexcept
    {
      _family = family;
    }

    NO_DISCARD Gfx::FontFamilyHandle GetFontFamily() const noexcept
    {
      return _family;
    }

    void SetFontSize(float size) noexcept
    {
      assert(size > 0.0f && "Font size must be greater than 0.");
      _fontSize = size;
    }

    NO_DISCARD float GetFontSize() const noexcept
    {
      return _fontSize;
    }

    void SetTextAlign(TextAlign align)
    {
      _textAlign = align;
    }

    NO_DISCARD TextAlign GetTextAlign() const noexcept
    {
      return _textAlign;
    }

#pragma endregion

#pragma region Border

    NO_DISCARD const Array<Gfx::ColourbPremultiplied, 4> &GetBorderColours() const noexcept
    {
      return _borderColours;
    }

    void SetBorderColours(const Array<Gfx::ColourbPremultiplied, 4> &colours) noexcept
    {
      _borderColours = colours;
    }

    NO_DISCARD const Array<float, 4> &GetBorderRadii() const noexcept
    {
      return _borderRadii;
    }

    void SetBorderRadii(const Array<float, 4> &radii)
    {
      _borderRadii = radii;
    }

    NO_DISCARD const Array<float, 4> &GetBorderWidths() const noexcept
    {
      return _borderWidths;
    }

    void SetBorderWidths(const Array<float, 4> &width)
    {
      _borderWidths = width;
    }

#pragma endregion

#pragma region Queries

    NO_DISCARD bool HorizontalInsetsDefined() const noexcept
    {
      return _position[ToUnderlying(Edge::Left)].IsDefined()
             || _position[ToUnderlying(Edge::Right)].IsDefined()
             || _position[ToUnderlying(Edge::All)].IsDefined()
             || _position[ToUnderlying(Edge::Horizontal)].IsDefined()
             || _position[ToUnderlying(Edge::Start)].IsDefined()
             || _position[ToUnderlying(Edge::End)].IsDefined();
    }

    NO_DISCARD bool VerticalInsetsDefined() const noexcept
    {
      return _position[ToUnderlying(Edge::Top)].IsDefined()
             || _position[ToUnderlying(Edge::Bottom)].IsDefined()
             || _position[ToUnderlying(Edge::All)].IsDefined()
             || _position[ToUnderlying(Edge::Vertical)].IsDefined();
    }

    NO_DISCARD bool IsFlexStartPositionDefined(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputePosition(FlexStartEdge(axis), direction).IsDefined();
    }

    NO_DISCARD bool IsFlexStartPositionAuto(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputePosition(FlexStartEdge(axis), direction).IsAuto();
    }

    NO_DISCARD bool IsInlineStartPositionDefined(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputePosition(InlineStartEdge(axis, direction), direction).IsDefined();
    }

    NO_DISCARD bool IsInlineStartPositionAuto(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputePosition(InlineStartEdge(axis, direction), direction).IsAuto();
    }

    NO_DISCARD bool IsFlexEndPositionDefined(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputePosition(FlexEndEdge(axis), direction).IsDefined();
    }

    NO_DISCARD bool IsFlexEndPositionAuto(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputePosition(FlexEndEdge(axis), direction).IsAuto();
    }

    NO_DISCARD bool IsInlineEndPositionDefined(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputePosition(InlineEndEdge(axis, direction), direction).IsDefined();
    }

    NO_DISCARD bool IsInlineEndPositionAuto(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputePosition(InlineEndEdge(axis, direction), direction).IsAuto();
    }

    NO_DISCARD bool IsFlexStartMarginAuto(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputeMargin(FlexStartEdge(axis), direction).IsAuto();
    }

    NO_DISCARD bool IsFlexEndMarginAuto(FlexDirection axis, Direction direction) const noexcept
    {
      return ComputeMargin(FlexEndEdge(axis), direction).IsAuto();
    }

#pragma endregion

#pragma region Computations

    NO_DISCARD float ComputeFlexStartPosition(FlexDirection axis, Direction direction,
                                              float axisSize) const noexcept
    {
      return ComputePosition(FlexStartEdge(axis), direction).Resolve(axisSize).ValueOrDefault(0.0f);
    }

    NO_DISCARD float ComputeInlineStartPosition(FlexDirection axis, Direction direction,
                                                float axisSize) const noexcept
    {
      return ComputePosition(InlineStartEdge(axis, direction), direction)
        .Resolve(axisSize)
        .ValueOrDefault(0.0f);
    }

    NO_DISCARD float ComputeFlexEndPosition(FlexDirection axis, Direction direction,
                                            float axisSize) const noexcept
    {
      return ComputePosition(FlexEndEdge(axis), direction).Resolve(axisSize).ValueOrDefault(0.0f);
    }

    NO_DISCARD float ComputeInlineEndPosition(FlexDirection axis, Direction direction,
                                              float axisSize) const noexcept
    {
      return ComputePosition(InlineEndEdge(axis, direction), direction)
        .Resolve(axisSize)
        .ValueOrDefault(0.0f);
    }

    NO_DISCARD float ComputeFlexStartMargin(FlexDirection axis, Direction direction,
                                            float widthSize) const noexcept
    {
      return ComputeMargin(FlexStartEdge(axis), direction).Resolve(widthSize).ValueOrDefault(0.0f);
    }

    NO_DISCARD float ComputeInlineStartMargin(FlexDirection axis, Direction direction,
                                              float widthSize) const noexcept
    {
      return ComputeMargin(InlineStartEdge(axis, direction), direction)
        .Resolve(widthSize)
        .ValueOrDefault(0.0f);
    }

    NO_DISCARD float ComputeFlexEndMargin(FlexDirection axis, Direction direction,
                                          float widthSize) const noexcept
    {
      return ComputeMargin(FlexEndEdge(axis), direction).Resolve(widthSize).ValueOrDefault(0.0f);
    }

    NO_DISCARD float ComputeInlineEndMargin(FlexDirection axis, Direction direction,
                                            float widthSize) const noexcept
    {
      return ComputeMargin(InlineEndEdge(axis, direction), direction).Resolve(widthSize).ValueOrDefault(0.0f);
    }

    NO_DISCARD float ComputeFlexStartBorder(FlexDirection axis, Direction direction) const noexcept
    {
      return MaxOrDefined(ComputeBorder(FlexStartEdge(axis), direction).Resolve(0.0f).Value(), 0.0f);
    }

    NO_DISCARD float ComputeInlineStartBorder(FlexDirection axis, Direction direction) const noexcept
    {
      return MaxOrDefined(ComputeBorder(InlineStartEdge(axis, direction), direction).Resolve(0.0f).Value(),
                          0.0f);
    }

    NO_DISCARD float ComputeFlexEndBorder(FlexDirection axis, Direction direction) const noexcept
    {
      return MaxOrDefined(ComputeBorder(FlexEndEdge(axis), direction).Resolve(0.0f).Value(), 0.0f);
    }

    NO_DISCARD float ComputeInlineEndBorder(FlexDirection axis, Direction direction) const noexcept
    {
      return MaxOrDefined(ComputeBorder(InlineEndEdge(axis, direction), direction).Resolve(0.0f).Value(),
                          0.0f);
    }

    NO_DISCARD float ComputeFlexStartPadding(FlexDirection axis, Direction direction,
                                             float widthSize) const noexcept
    {
      return MaxOrDefined(ComputePadding(FlexStartEdge(axis), direction).Resolve(widthSize).Value(), 0.0f);
    }

    NO_DISCARD float ComputeInlineStartPadding(FlexDirection axis, Direction direction,
                                               float widthSize) const noexcept
    {
      return MaxOrDefined(
        ComputePadding(InlineStartEdge(axis, direction), direction).Resolve(widthSize).Value(), 0.0f);
    }

    NO_DISCARD float ComputeFlexEndPadding(FlexDirection axis, Direction direction,
                                           float widthSize) const noexcept
    {
      return MaxOrDefined(ComputePadding(FlexEndEdge(axis), direction).Resolve(widthSize).Value(), 0.0f);
    }

    NO_DISCARD float ComputeInlineEndPadding(FlexDirection axis, Direction direction,
                                             float widthSize) const noexcept
    {
      return MaxOrDefined(
        ComputePadding(InlineEndEdge(axis, direction), direction).Resolve(widthSize).Value(), 0.0f);
    }

    NO_DISCARD float ComputeInlineStartPaddingAndBorder(FlexDirection axis, Direction direction,
                                                        float widthSize) const noexcept
    {
      return ComputeInlineStartPadding(axis, direction, widthSize)
             + ComputeInlineStartBorder(axis, direction);
    }

    NO_DISCARD float ComputeFlexStartPaddingAndBorder(FlexDirection axis, Direction direction,
                                                      float widthSize) const noexcept
    {
      return ComputeFlexStartPadding(axis, direction, widthSize) + ComputeFlexStartBorder(axis, direction);
    }

    NO_DISCARD float ComputeInlineEndPaddingAndBorder(FlexDirection axis, Direction direction,
                                                      float widthSize) const noexcept
    {
      return ComputeInlineEndPadding(axis, direction, widthSize) + ComputeInlineEndBorder(axis, direction);
    }

    NO_DISCARD float ComputeFlexEndPaddingAndBorder(FlexDirection axis, Direction direction,
                                                    float widthSize) const noexcept
    {
      return ComputeFlexEndPadding(axis, direction, widthSize) + ComputeFlexEndBorder(axis, direction);
    }

    NO_DISCARD float ComputePaddingAndBorderForDimension(Direction direction, Dimension dimension,
                                                         float widthSize) const noexcept
    {
      FlexDirection flexDirectionForDimension =
        dimension == Dimension::Width ? FlexDirection::Row : FlexDirection::Column;

      return ComputeFlexStartPaddingAndBorder(flexDirectionForDimension, direction, widthSize)
             + ComputeFlexEndPaddingAndBorder(flexDirectionForDimension, direction, widthSize);
    }

    NO_DISCARD float ComputeBorderForAxis(FlexDirection axis) const noexcept
    {
      return ComputeInlineStartBorder(axis, Direction::LTR) + ComputeInlineEndBorder(axis, Direction::LTR);
    }

    NO_DISCARD float ComputeMarginForAxis(FlexDirection axis, float widthSize) const noexcept
    {
      // The total margin for a given axis does not depend on the direction
      // so hardcoding LTR here to avoid piping direction to this function
      return ComputeInlineStartMargin(axis, Direction::LTR, widthSize)
             + ComputeInlineEndMargin(axis, Direction::LTR, widthSize);
    }

    NO_DISCARD float ComputeGapForAxis(FlexDirection axis, float ownerSize) const noexcept
    {
      auto gap = IsRow(axis) ? ComputeColumnGap() : ComputeRowGap();
      return MaxOrDefined(gap.Resolve(ownerSize).Value(), 0.0f);
    }

#pragma endregion

#pragma region Equality

    NO_DISCARD bool operator==(const Style &other) const noexcept
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

    NO_DISCARD bool operator!=(const Style &other) const noexcept
    {
      return !(*this == other);
    }

#pragma endregion

  private:
#pragma region EqualityHelpers

    NO_DISCARD static inline bool NumbersEqual(const StyleValueHandle &lhsHandle,
                                               const StyleValuePool &lhsPool,
                                               const StyleValueHandle &rhsHandle,
                                               const StyleValuePool &rhsPool) noexcept
    {
      return (lhsHandle.IsUndefined() && rhsHandle.IsUndefined())
             || (lhsPool.GetNumber(lhsHandle) == rhsPool.GetNumber(rhsHandle));
    }

    NO_DISCARD static inline bool LengthsEqual(const StyleValueHandle &lhsHandle,
                                               const StyleValuePool &lhsPool,
                                               const StyleValueHandle &rhsHandle,
                                               const StyleValuePool &rhsPool) noexcept
    {
      return (lhsHandle.IsUndefined() && rhsHandle.IsUndefined())
             || (lhsPool.GetLength(lhsHandle) == rhsPool.GetLength(rhsHandle));
    }

    template <size_t N>
    NO_DISCARD static inline bool
      LengthsEqual(const std::array<StyleValueHandle, N> &lhs, const StyleValuePool &lhsPool,
                   const std::array<StyleValueHandle, N> &rhs, const StyleValuePool &rhsPool) noexcept
    {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [&](const auto &lhs, const auto &rhs)
                        { return LengthsEqual(lhs, lhsPool, rhs, rhsPool); });
    }

#pragma endregion

#pragma region ComputationHelpers

    NO_DISCARD Style::Length ComputeColumnGap() const noexcept
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

    NO_DISCARD Style::Length ComputeRowGap() const noexcept
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

    NO_DISCARD Style::Length ComputeLeftEdge(const Edges &edges, Direction layoutDirection) const noexcept
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

    NO_DISCARD Style::Length ComputeTopEdge(const Edges &edges) const noexcept
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

    NO_DISCARD Style::Length ComputeRightEdge(const Edges &edges, Direction layoutDirection) const noexcept
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

    NO_DISCARD Style::Length ComputeBottomEdge(const Edges &edges) const noexcept
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

    NO_DISCARD Style::Length ComputePosition(PhysicalEdge edge, Direction direction) const noexcept
    {
      switch (edge)
      {
        case PhysicalEdge::Left:   return ComputeLeftEdge(_position, direction);
        case PhysicalEdge::Top:    return ComputeTopEdge(_position);
        case PhysicalEdge::Right:  return ComputeRightEdge(_position, direction);
        case PhysicalEdge::Bottom: return ComputeBottomEdge(_position);
      }

      std::unreachable();
    }

    NO_DISCARD Style::Length ComputeMargin(PhysicalEdge edge, Direction direction) const noexcept
    {
      switch (edge)
      {
        case PhysicalEdge::Left:   return ComputeLeftEdge(_margin, direction);
        case PhysicalEdge::Top:    return ComputeTopEdge(_margin);
        case PhysicalEdge::Right:  return ComputeRightEdge(_margin, direction);
        case PhysicalEdge::Bottom: return ComputeBottomEdge(_margin);
      }

      std::unreachable();
    }

    NO_DISCARD Style::Length ComputePadding(PhysicalEdge edge, Direction direction) const noexcept
    {
      switch (edge)
      {
        case PhysicalEdge::Left:   return ComputeLeftEdge(_padding, direction);
        case PhysicalEdge::Top:    return ComputeTopEdge(_padding);
        case PhysicalEdge::Right:  return ComputeRightEdge(_padding, direction);
        case PhysicalEdge::Bottom: return ComputeBottomEdge(_padding);
      }

      std::unreachable();
    }

    NO_DISCARD Style::Length ComputeBorder(PhysicalEdge edge, Direction direction) const noexcept
    {
      switch (edge)
      {
        case PhysicalEdge::Left:   return ComputeLeftEdge(_border, direction);
        case PhysicalEdge::Top:    return ComputeTopEdge(_border);
        case PhysicalEdge::Right:  return ComputeRightEdge(_border, direction);
        case PhysicalEdge::Bottom: return ComputeBottomEdge(_border);
      }

      std::unreachable();
    }

#pragma endregion
  };
}