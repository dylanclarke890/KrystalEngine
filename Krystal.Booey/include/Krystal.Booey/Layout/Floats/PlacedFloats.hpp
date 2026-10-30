#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::layout
{
  /// @brief The floats placed in a block formatting context so far, in placement order.
  ///
  /// PlacedFloats are relative to the BFC's logical top/left.
  /// When floats added by nested IFCs with mismatching inline direcctions (e.g. where BFC is RTL but IFC is
  /// RTL) they get converted as if they had the same inline direction as BFC. What it simply means that
  /// PlacedFloats::Item::isStartPositioned is always relative to BFC, regardless of what it is relative to in
  /// its IFC.
  class PlacedFloats
  {
  public:
    class Item
    {
    public:
      // FIXME: This c'tor is only used by the render tree integation codepath.
      enum class Position : bool
      {
        Start,
        End
      };

    private:
      CheckedPtr<const Box> _layoutBox;
      Position _position;
      BoxGeometry _absoluteBoxGeometry;
      LayoutPoint _localTopLeft;
      RefPtr<const LayoutShape> _shape;
      Maybe<size_t> _placedByLine;

    public:
      Item(Position position, const BoxGeometry &absoluteBoxGeometry, LayoutPoint localTopLeft,
           RefPtr<const LayoutShape> &&shape) noexcept;
      Item(const Box &layoutBox, Position position, const BoxGeometry &absoluteBoxGeometry,
           LayoutPoint localTopLeft, Maybe<size_t> line) noexcept;
      ~Item() noexcept;

      Item(const Item &) noexcept = default;
      Item(Item &&) noexcept = default;
      Item &operator=(const Item &) noexcept = default;
      Item &operator=(Item &&) noexcept = default;

      KRYS_NODISCARD bool IsStartPositioned() const noexcept
      {
        return _position == Position::Start;
      }
      KRYS_NODISCARD bool IsEndPositioned() const noexcept
      {
        return _position == Position::End;
      }

      KRYS_NODISCARD bool IsInFormattingContextOf(const ElementBox &formattingContextRoot) const noexcept;

      KRYS_NODISCARD layout::BoxGeometry GetBoxGeometry() const noexcept;

      KRYS_NODISCARD Rect AbsoluteRectWithMargin() const noexcept
      {
        return BoxGeometry::MarginBoxRect(_absoluteBoxGeometry);
      }
      KRYS_NODISCARD Rect AbsoluteBorderBoxRect() const noexcept
      {
        return BoxGeometry::BorderBoxRect(_absoluteBoxGeometry);
      }
      KRYS_NODISCARD BoxGeometry::HorizontalEdges InlineAxisMargin() const noexcept
      {
        return _absoluteBoxGeometry.HorizontalMargin();
      }
      KRYS_NODISCARD PositionInContextRoot AbsoluteBottom() const noexcept
      {
        return {AbsoluteRectWithMargin().Bottom()};
      }

      KRYS_NODISCARD const LayoutShape *Shape() const noexcept
      {
        return _shape.get();
      }

      KRYS_NODISCARD Maybe<size_t> PlacedByLine() const noexcept
      {
        return _placedByLine;
      }

      KRYS_NODISCARD const Box *LayoutBox() const noexcept
      {
        return _layoutBox.get();
      }
    };

    using List = krys::List<Item>;

  private:
    enum class PositionType : bool
    {
      Start,
      End
    };

    CheckedRef<const ElementBox> _blockFormattingContextRoot;
    List _list;
    EnumSet<PositionType> _positionTypes;
    boo::WritingMode _writingMode;

  public:
    explicit PlacedFloats(const ElementBox &blockFormattingContextRoot) noexcept;

    KRYS_NODISCARD const ElementBox &BlockFormattingContextRoot() const noexcept
    {
      return *_blockFormattingContextRoot;
    }

    KRYS_NODISCARD const List &GetList() const noexcept
    {
      return _list;
    }
    KRYS_NODISCARD const Item *Last() const noexcept
    {
      return GetList().empty() ? nullptr : &_list.back();
    }

    void Add(Item newFloatItem) noexcept;
    bool Remove(const Box &floatBox) noexcept;
    void Clear() noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return GetList().empty();
    }

    KRYS_NODISCARD bool HasStartPositioned() const noexcept
    {
      return _positionTypes.Contains(PositionType::Start);
    }
    KRYS_NODISCARD bool HasEndPositioned() const noexcept
    {
      return _positionTypes.Contains(PositionType::End);
    }

    KRYS_NODISCARD Maybe<LayoutUnit> HighestPositionOnBlockAxis() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      LowestPositionOnBlockAxis(boo::Clear type = boo::Clear::Both) const noexcept;

    void ShrinkToFit() noexcept;
  };
}
