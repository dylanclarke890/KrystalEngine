#include "Krystal.Booey/Layout/FormattingContexts/Grid/UnplacedGridItem.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::layout
{
  UnplacedGridItem::UnplacedGridItem(const ElementBox &layoutBox, style::GridPosition columnStart,
                                     style::GridPosition columnEnd, style::GridPosition rowStart,
                                     style::GridPosition rowEnd) noexcept
      : _layoutBox(&layoutBox), _columnPosition(krys::move(columnStart), krys::move(columnEnd)),
        _rowPosition(krys::move(rowStart), krys::move(rowEnd))
  {
  }

  int UnplacedGridItem::ExplicitColumnStart() const noexcept
  {
    krys_debug_assert(_columnPosition.first.IsExplicit());
    auto explicitColumnStart = _columnPosition.first.ExplicitPosition();
    // https://www.w3.org/TR/css-grid-1/#line-placement
    // An <integer> value of zero makes the declaration invalid.
    krys_debug_assert(explicitColumnStart);

    // Convert from 1-indexed CSS grid lines to 0-indexed matrix positions.
    return explicitColumnStart > 0 ? explicitColumnStart - 1 : explicitColumnStart;
  }

  size_t UnplacedGridItem::NormalizedColumnStart() const noexcept
  {
    return static_cast<size_t>(ExplicitColumnStart() + static_cast<int>(_columnNormalizationOffset));
  }

  int UnplacedGridItem::ExplicitColumnEnd() const noexcept
  {
    krys_debug_assert(_columnPosition.second.IsExplicit());
    auto explicitColumnEnd = _columnPosition.second.ExplicitPosition();
    // https://www.w3.org/TR/css-grid-1/#line-placement
    // An <integer> value of zero makes the declaration invalid.
    krys_debug_assert(explicitColumnEnd);

    // Convert from 1-indexed CSS grid lines to 0-indexed matrix positions.
    return explicitColumnEnd > 0 ? explicitColumnEnd - 1 : explicitColumnEnd;
  }

  size_t UnplacedGridItem::NormalizedColumnEnd() const noexcept
  {
    return static_cast<size_t>(ExplicitColumnEnd() + static_cast<int>(_columnNormalizationOffset));
  }

  int UnplacedGridItem::ExplicitRowStart() const noexcept
  {
    krys_debug_assert(_rowPosition.first.IsExplicit());
    auto explicitRowStart = _rowPosition.first.ExplicitPosition();
    // https://www.w3.org/TR/css-grid-1/#line-placement
    // An <integer> value of zero makes the declaration invalid.
    krys_debug_assert(explicitRowStart);

    // Convert from 1-indexed CSS grid lines to 0-indexed matrix positions.
    return explicitRowStart > 0 ? explicitRowStart - 1 : explicitRowStart;
  }

  size_t UnplacedGridItem::NormalizedRowStart() const noexcept
  {
    return static_cast<size_t>(ExplicitRowStart() + static_cast<int>(_rowNormalizationOffset));
  }

  int UnplacedGridItem::ExplicitRowEnd() const noexcept
  {
    krys_debug_assert(_rowPosition.second.IsExplicit());
    auto explicitRowEnd = _rowPosition.second.ExplicitPosition();
    // https://www.w3.org/TR/css-grid-1/#line-placement
    // An <integer> value of zero makes the declaration invalid.
    krys_debug_assert(explicitRowEnd);

    // Convert from 1-indexed CSS grid lines to 0-indexed matrix positions.
    return explicitRowEnd > 0 ? explicitRowEnd - 1 : explicitRowEnd;
  }

  size_t UnplacedGridItem::NormalizedRowEnd() const noexcept
  {
    return static_cast<size_t>(ExplicitRowEnd() + static_cast<int>(_rowNormalizationOffset));
  }

  bool UnplacedGridItem::HasDefiniteRowPosition() const noexcept
  {
    return _rowPosition.first.IsExplicit() || _rowPosition.second.IsExplicit();
  }

  bool UnplacedGridItem::HasDefiniteColumnPosition() const noexcept
  {
    return _columnPosition.first.IsExplicit() || _columnPosition.second.IsExplicit();
  }

  bool UnplacedGridItem::HasAutoColumnPosition() const noexcept
  {
    return _columnPosition.first.IsAuto() && _columnPosition.second.IsAuto();
  }

  size_t UnplacedGridItem::ColumnSpanSize() const noexcept
  {
    const auto &firstPosition = _columnPosition.first;
    const auto &secondPosition = _columnPosition.second;

    // Case 1: Both positions are explicit - calculate span size
    if (firstPosition.IsExplicit() && secondPosition.IsExplicit())
    {
      return static_cast<size_t>(ExplicitColumnEnd() - ExplicitColumnStart());
    }

    // Case 2: One position is a span - extract its span size.
    krys_debug_assert(!(firstPosition.IsSpan() && secondPosition.IsSpan()));
    if (firstPosition.IsSpan())
    {
      return static_cast<size_t>(firstPosition.SpanPosition());
    }
    if (secondPosition.IsSpan())
    {
      return static_cast<size_t>(secondPosition.SpanPosition());
    }

    // Default to span 1
    krys_debug_assert(HasAutoColumnPosition());
    return 1;
  }

  Pair<int, int> UnplacedGridItem::DefiniteRowStartEnd() const noexcept
  {
    const auto &startPosition = _rowPosition.first;
    const auto &endPosition = _rowPosition.second;

    if (startPosition.IsExplicit() && endPosition.IsExplicit())
    {
      return {ExplicitRowStart(), ExplicitRowEnd()};
    }

    if (startPosition.IsExplicit() && endPosition.IsSpan())
    {
      return {ExplicitRowStart(), ExplicitRowStart() + endPosition.SpanPosition()};
    }

    if (startPosition.IsSpan() && endPosition.IsExplicit())
    {
      return {ExplicitRowEnd() - startPosition.SpanPosition(), ExplicitRowEnd()};
    }

    if (startPosition.IsExplicit() && endPosition.IsAuto())
    {
      return {ExplicitRowStart(), ExplicitRowStart() + 1};
    }

    if (startPosition.IsAuto() && endPosition.IsExplicit())
    {
      auto explicitEnd = ExplicitRowEnd();
      krys_debug_assert(explicitEnd >= 1);
      return {explicitEnd - 1, explicitEnd};
    }

    krys_debug_assert(false);
    return {0, 0};
  }

  Pair<int, int> UnplacedGridItem::DefiniteColumnStartEnd() const noexcept
  {
    const auto &startPosition = _columnPosition.first;
    const auto &endPosition = _columnPosition.second;

    if (startPosition.IsExplicit() && endPosition.IsExplicit())
    {
      return {ExplicitColumnStart(), ExplicitColumnEnd()};
    }

    if (startPosition.IsExplicit() && endPosition.IsSpan())
    {
      return {ExplicitColumnStart(), ExplicitColumnStart() + endPosition.SpanPosition()};
    }

    if (startPosition.IsSpan() && endPosition.IsExplicit())
    {
      return {ExplicitColumnEnd() - startPosition.SpanPosition(), ExplicitColumnEnd()};
    }

    if (startPosition.IsExplicit() && endPosition.IsAuto())
    {
      return {ExplicitColumnStart(), ExplicitColumnStart() + 1};
    }

    if (startPosition.IsAuto() && endPosition.IsExplicit())
    {
      auto explicitEnd = ExplicitColumnEnd();
      return {explicitEnd - 1, explicitEnd};
    }

    krys_debug_assert(false);
    return {0, 0};
  }

  Pair<size_t, size_t> UnplacedGridItem::NormalizedRowStartEnd() const noexcept
  {
    krys_debug_assert(_hasAppliedGridOffsets);
    auto rowStart = NormalizedRowStart();
    auto rowEnd = NormalizedRowEnd();

    // Handle inverted ranges by swapping start and end
    if (rowEnd < rowStart)
    {
      return {rowEnd, rowStart};
    }

    return {rowStart, rowEnd};
  }

  Pair<size_t, size_t> UnplacedGridItem::NormalizedColumnStartEnd() const noexcept
  {
    krys_debug_assert(_hasAppliedGridOffsets);
    auto columnStart = NormalizedColumnStart();
    auto columnEnd = NormalizedColumnEnd();

    // Handle inverted ranges by swapping start and end
    if (columnEnd < columnStart)
    {
      return {columnEnd, columnStart};
    }

    return {columnStart, columnEnd};
  }

  void UnplacedGridItem::ApplyGridOffsets(size_t rowOffset, size_t columnOffset) noexcept
  {
    krys_debug_assert(!_hasAppliedGridOffsets);
    _rowNormalizationOffset = rowOffset;
    _columnNormalizationOffset = columnOffset;
    _hasAppliedGridOffsets = true;
  }
}
