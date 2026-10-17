#include "Krystal.Booey/Style/Values/Grid/GridPositionsResolver.hpp"
#include "Krystal.Booey/Rendering/AncestorSubgridIterator.hpp"
#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/Style/GridArea.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPositionSide.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include <algorithm>
#include <cstdlib>

namespace krys::boo::style
{
  namespace
  {
    /// @brief The lines of a grid with a given name, including the ones in its auto repeat tracks.
    class NamedLineCollectionBase
    {
      KRYS_NON_COPY_MOVABLE(NamedLineCollectionBase);

    protected:
      const List<uint32> *_namedLinesIndices {nullptr};
      const List<uint32> *_autoRepeatNamedLinesIndices {nullptr};
      const List<uint32> *_implicitNamedLinesIndices {nullptr};

      List<uint32> _inheritedNamedLinesIndices;

      uint32 _insertionPoint {0};
      uint32 _lastLine {0};
      uint32 _autoRepeatTotalTracks {0};
      uint32 _autoRepeatLines {0};
      uint32 _autoRepeatTrackListLength {0};
      bool _isSubgrid {false};

    public:
      NamedLineCollectionBase(const RenderGrid &initialGrid, const css::CSSOMString &name,
                              GridPositionSide side, bool nameIsAreaName) noexcept;

      KRYS_NODISCARD bool HasNamedLines() const noexcept;
      KRYS_NODISCARD bool HasExplicitNamedLines() const noexcept;
      KRYS_NODISCARD bool Contains(uint32 line) const noexcept;

    protected:
      void EnsureInheritedNamedIndices() noexcept;
    };

    /// @brief The lines of a grid with a given name, including the ones it inherits from the grids it's a
    /// subgrid of.
    class NamedLineCollection : public NamedLineCollectionBase
    {
      KRYS_NON_COPY_MOVABLE(NamedLineCollection);

    public:
      NamedLineCollection(const RenderGrid &initialGrid, const css::CSSOMString &name, GridPositionSide side,
                          bool nameIsAreaName = false) noexcept;

      KRYS_NODISCARD int FirstPosition() const noexcept;

      KRYS_NODISCARD uint32 LastLine() const noexcept;

    private:
      KRYS_NODISCARD int FirstExplicitPosition() const noexcept;
    };

    KRYS_NODISCARD inline bool IsStartSide(GridPositionSide side) noexcept
    {
      return side == GridPositionSide::ColumnStartSide || side == GridPositionSide::RowStartSide;
    }

    KRYS_NODISCARD inline GridTrackSizingDirection DirectionFromSide(GridPositionSide side) noexcept
    {
      return side == GridPositionSide::ColumnStartSide || side == GridPositionSide::ColumnEndSide
               ? GridTrackSizingDirection::Columns
               : GridTrackSizingDirection::Rows;
    }

    KRYS_NODISCARD css::CSSOMString ImplicitNamedGridLineForSide(const css::CSSOMString &lineName,
                                                                 GridPositionSide side) noexcept
    {
      return lineName + (IsStartSide(side) ? u8"-start" : u8"-end");
    }

    KRYS_NODISCARD uint32 ExplicitGridSizeForSide(const RenderGrid &gridContainer,
                                                  GridPositionSide side) noexcept
    {
      return GridPositionsResolver::ExplicitGridCount(gridContainer, DirectionFromSide(side));
    }

    KRYS_NODISCARD inline GridPositionSide TransposedSide(GridPositionSide side) noexcept
    {
      switch (side)
      {
        case GridPositionSide::ColumnStartSide: return GridPositionSide::RowStartSide;
        case GridPositionSide::ColumnEndSide:   return GridPositionSide::RowEndSide;
        case GridPositionSide::RowStartSide:    return GridPositionSide::ColumnStartSide;
        default:                                return GridPositionSide::ColumnEndSide;
      }
    }

    KRYS_NODISCARD Maybe<int> ClampedImplicitLineForArea(const RenderStyle &style,
                                                         const css::CSSOMString &name, int min, int max,
                                                         GridTrackSizingDirection direction,
                                                         bool isStartSide) noexcept
    {
      auto &areas = style.GetGridTemplateAreas().Map.Map;
      auto gridAreaIt = areas.find(name);
      if (gridAreaIt != areas.end())
      {
        auto &gridArea = gridAreaIt->second;
        auto gridSpan = gridArea.Span(direction);
        if (gridSpan.Clamp(min, max))
        {
          return isStartSide ? static_cast<int>(gridSpan.StartLine()) : static_cast<int>(gridSpan.EndLine());
        }
      }
      return null;
    }

    template <typename TMap>
    KRYS_NODISCARD const List<uint32> *FindIndices(const TMap &map, const css::CSSOMString &name) noexcept
    {
      auto iterator = map.find(name);
      return iterator == map.end() ? nullptr : &iterator->second;
    }

    NamedLineCollectionBase::NamedLineCollectionBase(const RenderGrid &initialGrid,
                                                     const css::CSSOMString &name, GridPositionSide side,
                                                     bool nameIsAreaName) noexcept
    {
      css::CSSOMString lineName = nameIsAreaName ? ImplicitNamedGridLineForSide(name, side) : name;
      auto direction = DirectionFromSide(side);
      const auto *grid = &initialGrid;
      const auto *gridContainerStyle = &grid->Style();

      _lastLine = ExplicitGridSizeForSide(*grid, side);

      auto &tracks = gridContainerStyle->GridTemplateList(direction);
      auto &gridLineNames = tracks.NamedLines.Map;
      auto &autoRepeatGridLineNames = tracks.AutoRepeatNamedLines.Map;
      auto &implicitGridLineNames =
        gridContainerStyle->GetGridTemplateAreas().ImplicitNamedGridLines(direction).Map;

      _namedLinesIndices = FindIndices(gridLineNames, lineName);
      _autoRepeatNamedLinesIndices = FindIndices(autoRepeatGridLineNames, lineName);
      _implicitNamedLinesIndices = FindIndices(implicitGridLineNames, lineName);
      _isSubgrid = grid->IsSubgrid(direction);

      _autoRepeatTotalTracks = grid->AutoRepeatCountForDirection(direction);
      _autoRepeatTrackListLength = static_cast<uint32>(tracks.AutoRepeatSizes.size());
      _autoRepeatLines = 0;
      _insertionPoint = tracks.AutoRepeatInsertionPoint;

      if (!_isSubgrid)
      {
        if (tracks.Subgrid)
        {
          // If subgrid was specified, but the grid wasn't able to actually become a subgrid, the used
          // value of the style should be the initial 'none' value.
          _namedLinesIndices = nullptr;
          _autoRepeatNamedLinesIndices = nullptr;
        }
        return;
      }

      if (_implicitNamedLinesIndices)
      {
        // The implicit lines list was created based on the areas specified for the grid areas property, but
        // the subgrid might have inherited fewer tracks than needed to cover the specified area. We want to
        // clamp the specified area down to explicit grid we actually have, and then generate implicit
        // -start/-end lines for the new area.
        krys_debug_assert(_implicitNamedLinesIndices->size() == 1);
        _implicitNamedLinesIndices = &_inheritedNamedLinesIndices;

        // Find the area name that creates the implicit line we're looking for. If the input was an area name,
        // then we can use that, otherwise we need to choose the substring and infer which side the input
        // specified. It's possible for authors to manually name a *-start implicit line name for the end line
        // search, and vice-versa, so we need to remember which side we inferred from the name, separately
        // from the side we're searching for.
        css::CSSOMString areaName = name;
        bool startSide = IsStartSide(side);
        if (!nameIsAreaName)
        {
          size_t suffix = name.find(u8"-start");
          if (suffix == css::CSSOMString::npos)
          {
            suffix = name.find(u8"-end");
            krys_debug_assert(suffix != css::CSSOMString::npos);
            startSide = false;
          }
          else
          {
            startSide = true;
          }
          areaName = name.substr(0, suffix);
        }
        auto implicitLine = ClampedImplicitLineForArea(*gridContainerStyle, areaName, 0,
                                                       static_cast<int>(_lastLine), direction, startSide);
        if (implicitLine)
        {
          _inheritedNamedLinesIndices.push_back(static_cast<uint32>(*implicitLine));
        }
      }

      krys_debug_assert(!_autoRepeatTotalTracks);
      _autoRepeatTrackListLength = static_cast<uint32>(tracks.AutoRepeatOrderedNamedLines.Map.size());
      if (_autoRepeatTrackListLength)
      {
        auto namedLines = static_cast<uint32>(tracks.OrderedNamedLines.Map.size());
        uint32 totalLines = _lastLine + 1;
        if (namedLines < totalLines)
        {
          // auto repeat in a subgrid specifies the line names that should be repeated, not
          // the tracks.
          _autoRepeatLines = (totalLines - namedLines) / _autoRepeatTrackListLength;
          _autoRepeatLines *= _autoRepeatTrackListLength;
        }
      }
    }

    void NamedLineCollectionBase::EnsureInheritedNamedIndices() noexcept
    {
      if (_implicitNamedLinesIndices != &_inheritedNamedLinesIndices)
      {
        if (_implicitNamedLinesIndices)
        {
          _inheritedNamedLinesIndices.insert(_inheritedNamedLinesIndices.end(),
                                             _implicitNamedLinesIndices->begin(),
                                             _implicitNamedLinesIndices->end());
        }
        _implicitNamedLinesIndices = &_inheritedNamedLinesIndices;
      }
    }

    bool NamedLineCollectionBase::Contains(uint32 line) const noexcept
    {
      krys_debug_assert(HasNamedLines());

      if (line > _lastLine)
      {
        return false;
      }

      auto contains = [](const List<uint32> *indices, uint32 line)
      {
        return indices && std::ranges::find(*indices, line) != indices->end();
      };

      if (contains(_implicitNamedLinesIndices, line))
      {
        return true;
      }

      if (!_autoRepeatTrackListLength || line < _insertionPoint)
      {
        return contains(_namedLinesIndices, line);
      }

      if (_isSubgrid)
      {
        if (line >= _insertionPoint + _autoRepeatLines)
        {
          return contains(_namedLinesIndices, line - _autoRepeatLines);
        }

        if (!_autoRepeatLines)
        {
          return contains(_namedLinesIndices, line);
        }

        uint32 autoRepeatIndexInFirstRepetition = (line - _insertionPoint) % _autoRepeatTrackListLength;
        return contains(_autoRepeatNamedLinesIndices, autoRepeatIndexInFirstRepetition);
      }

      krys_debug_assert(_autoRepeatTotalTracks);

      if (line > _insertionPoint + _autoRepeatTotalTracks)
      {
        return contains(_namedLinesIndices, line - (_autoRepeatTotalTracks - 1));
      }

      if (line == _insertionPoint)
      {
        return contains(_namedLinesIndices, line) || contains(_autoRepeatNamedLinesIndices, 0);
      }

      if (line == _insertionPoint + _autoRepeatTotalTracks)
      {
        return contains(_autoRepeatNamedLinesIndices, _autoRepeatTrackListLength)
               || contains(_namedLinesIndices, _insertionPoint + 1);
      }

      uint32 autoRepeatIndexInFirstRepetition = (line - _insertionPoint) % _autoRepeatTrackListLength;
      if (!autoRepeatIndexInFirstRepetition
          && contains(_autoRepeatNamedLinesIndices, _autoRepeatTrackListLength))
      {
        return true;
      }
      return contains(_autoRepeatNamedLinesIndices, autoRepeatIndexInFirstRepetition);
    }

    NamedLineCollection::NamedLineCollection(const RenderGrid &initialGrid, const css::CSSOMString &name,
                                             GridPositionSide side, bool nameIsAreaName) noexcept
        : NamedLineCollectionBase(initialGrid, name, side, nameIsAreaName)
    {
      if (!_lastLine)
      {
        return;
      }
      auto search = GridSpan::TranslatedDefiniteGridSpan(0, _lastLine);
      auto currentSide = side;
      auto direction = DirectionFromSide(currentSide);
      bool initialFlipped = GridLayoutFunctions::IsFlippedDirection(initialGrid, direction);

      // If we're a subgrid, we want to inherit the line names from any ancestor grids.
      if (!initialGrid.IsSubgrid(direction))
      {
        return;
      }
      for (auto &currentAncestorSubgrid : AncestorSubgridIterator(&initialGrid, direction))
      {
        auto *currentAncestorSubgridParent = Downcast<RenderGrid>(currentAncestorSubgrid.Parent());
        auto &currentAncestorStyle = currentAncestorSubgrid.Style();

        // auto-placed subgrids inside a masonry grid do not inherit any line names
        if ((currentAncestorSubgridParent->AreMasonryRows()
             && (currentAncestorStyle.GridItemColumnStart().IsAuto()
                 || currentAncestorStyle.GridItemColumnStart().IsSpan()))
            || (currentAncestorSubgridParent->AreMasonryColumns()
                && (currentAncestorStyle.GridItemRowStart().IsAuto()
                    || currentAncestorStyle.GridItemRowStart().IsSpan())))
        {
          return;
        }
        // Translate our explicit grid set of lines into the coordinate space of the
        // parent grid, adjusting direction/side as needed.
        if (currentAncestorSubgrid.IsHorizontalWritingMode()
            != currentAncestorSubgridParent->IsHorizontalWritingMode())
        {
          currentSide = TransposedSide(currentSide);
        }
        direction = DirectionFromSide(currentSide);

        auto span = currentAncestorSubgridParent->GridSpanForGridItem(currentAncestorSubgrid, direction);
        search.TranslateTo(span, GridLayoutFunctions::IsSubgridReversedDirection(
                                   *currentAncestorSubgridParent, direction, currentAncestorSubgrid));

        auto convertToInitialSpace = [&](uint32 i)
        {
          krys_debug_assert(i >= search.StartLine());
          i -= search.StartLine();
          if (GridLayoutFunctions::IsFlippedDirection(*currentAncestorSubgridParent, direction)
              != initialFlipped)
          {
            krys_debug_assert(_lastLine >= i);
            i = _lastLine - i;
          }
          return i;
        };

        // Create a line collection for the parent grid, and check to see if any of our lines
        // are present. If we find any, add them to a locally stored line name list (with numbering
        // relative to our grid).
        bool appended = false;
        NamedLineCollectionBase parentCollection(*currentAncestorSubgridParent, name, currentSide,
                                                 nameIsAreaName);
        if (parentCollection.HasNamedLines())
        {
          for (uint32 i = search.StartLine(); i <= search.EndLine(); i++)
          {
            if (parentCollection.Contains(i))
            {
              EnsureInheritedNamedIndices();
              appended = true;
              _inheritedNamedLinesIndices.push_back(convertToInitialSpace(i));
            }
          }
        }

        if (nameIsAreaName)
        {
          // We now need to look at the grid areas for the parent (not the implicit
          // lines for the parent!), and insert the ones that intersect as implicit
          // lines (but in our single combined list).
          auto implicitLine = ClampedImplicitLineForArea(
            currentAncestorSubgridParent->Style(), name, static_cast<int>(search.StartLine()),
            static_cast<int>(search.EndLine()), direction, IsStartSide(side));
          if (implicitLine)
          {
            EnsureInheritedNamedIndices();
            appended = true;
            _inheritedNamedLinesIndices.push_back(convertToInitialSpace(static_cast<uint32>(*implicitLine)));
          }
        }

        if (appended)
        {
          // Re-sort m_inheritedNamedLinesIndices
          std::ranges::sort(_inheritedNamedLinesIndices);
        }
      }
    }

    bool NamedLineCollectionBase::HasExplicitNamedLines() const noexcept
    {
      if (_namedLinesIndices)
      {
        return true;
      }
      return _autoRepeatNamedLinesIndices && (!_isSubgrid || _autoRepeatLines);
    }

    bool NamedLineCollectionBase::HasNamedLines() const noexcept
    {
      return HasExplicitNamedLines() || (_implicitNamedLinesIndices && !_implicitNamedLinesIndices->empty());
    }

    uint32 NamedLineCollection::LastLine() const noexcept
    {
      return _lastLine;
    }

    int NamedLineCollection::FirstExplicitPosition() const noexcept
    {
      krys_debug_assert(HasExplicitNamedLines());
      uint32 autoRepeats = _isSubgrid ? _autoRepeatLines : _autoRepeatTotalTracks;

      // If there is no auto repeat(), there must be some named line outside, return the 1st one. Also return
      // it if it precedes the auto-repeat().
      if (_namedLinesIndices && (!autoRepeats || _namedLinesIndices->front() <= _insertionPoint))
      {
        return static_cast<int>(_namedLinesIndices->front());
      }

      // Return the 1st named line inside the auto repeat(), if any.
      if (_autoRepeatNamedLinesIndices)
      {
        return static_cast<int>(_autoRepeatNamedLinesIndices->front() + _insertionPoint);
      }

      // The 1st named line must be after the auto repeat().
      krys_debug_assert(_namedLinesIndices);
      if (_isSubgrid)
      {
        return static_cast<int>(_namedLinesIndices->front() + autoRepeats);
      }
      return static_cast<int>(_namedLinesIndices->front() + autoRepeats - 1);
    }

    int NamedLineCollection::FirstPosition() const noexcept
    {
      krys_debug_assert(HasNamedLines());
      if (!_implicitNamedLinesIndices)
      {
        return FirstExplicitPosition();
      }
      if (!HasExplicitNamedLines())
      {
        return static_cast<int>(_implicitNamedLinesIndices->front());
      }
      return std::min<int>(FirstExplicitPosition(), static_cast<int>(_implicitNamedLinesIndices->front()));
    }

    // https://drafts.csswg.org/css-grid-2/#indefinite-grid-span
    KRYS_NODISCARD bool IsIndefiniteSpan(const GridPosition &initialPosition,
                                         const GridPosition &finalPosition) noexcept
    {
      if (initialPosition.IsAuto())
      {
        return !finalPosition.IsSpan();
      }
      if (finalPosition.IsAuto())
      {
        return !initialPosition.IsSpan();
      }
      return false;
    }

    KRYS_NODISCARD bool HasNamedGridLine(const GridPosition &position) noexcept
    {
      return position.SwitchOn([](const GridPosition::Explicit &explicitPosition)
                               { return explicitPosition.Name.Value != css::CSSOMStringAtom::Null(); },
                               [](const GridPosition::Span &spanPosition)
                               { return spanPosition.Name.Value != css::CSSOMStringAtom::Null(); },
                               [](const css::CustomIdentifier &) { return true; },
                               [](const css::keywords::Auto &) { return false; });
    }

    KRYS_NODISCARD GridPosition SpanPosition(int span) noexcept
    {
      auto spanPosition = GridPosition::Span {};
      spanPosition.Position = GridPosition::Span::PositionType {span};
      return GridPosition {spanPosition};
    }

    KRYS_NODISCARD Pair<GridPosition, GridPosition>
      AdjustGridPositionsFromStyle(const RenderBox &gridItem, GridTrackSizingDirection direction) noexcept
    {
      auto initialPosition = gridItem.Style().GridItemStart(direction);
      auto finalPosition = gridItem.Style().GridItemEnd(direction);

      // We must handle the placement error handling code here instead of in the StyleAdjuster because we
      // don't want to overwrite the specified values.
      if (initialPosition.IsSpan() && finalPosition.IsSpan())
      {
        finalPosition = css::keywords::Auto {};
      }

      // If the grid item has an automatic position and a grid span for a named line in a given dimension,
      // instead treat the grid span as one.
      if (initialPosition.IsAuto() && finalPosition.IsSpan() && HasNamedGridLine(finalPosition))
      {
        finalPosition = SpanPosition(1);
      }
      if (finalPosition.IsAuto() && initialPosition.IsSpan() && HasNamedGridLine(initialPosition))
      {
        initialPosition = SpanPosition(1);
      }

      if (IsIndefiniteSpan(initialPosition, finalPosition))
      {
        auto *renderGrid = DynamicDowncast<RenderGrid>(gridItem);
        if (renderGrid && renderGrid->IsSubgrid(direction))
        {
          // Indefinite span for an item that is subgridded in this axis.
          int lineCount =
            static_cast<int>(gridItem.Style().GridTemplateList(direction).OrderedNamedLines.Map.size());

          if (initialPosition.IsAuto())
          {
            // Set initial position to span <line names - 1>
            initialPosition = SpanPosition(std::max(1, lineCount - 1));
          }
          else
          {
            // Set final position to span <line names - 1>
            finalPosition = SpanPosition(std::max(1, lineCount - 1));
          }
        }
      }

      return {krys::move(initialPosition), krys::move(finalPosition)};
    }

    KRYS_NODISCARD uint32 LookAheadForNamedGridLine(int start, uint32 numberOfLines,
                                                    NamedLineCollection &linesCollection) noexcept
    {
      krys_debug_assert(numberOfLines);

      // Only implicit lines on the search direction are assumed to have the given name, so we can start to
      // look from first line. See: https://drafts.csswg.org/css-grid/#grid-placement-span-int
      uint32 end = static_cast<uint32>(std::max(start, 0));

      if (!linesCollection.HasNamedLines())
      {
        return std::max(end, linesCollection.LastLine() + 1) + numberOfLines - 1;
      }

      for (; numberOfLines; ++end)
      {
        if (end > linesCollection.LastLine() || linesCollection.Contains(end))
        {
          numberOfLines--;
        }
      }

      krys_debug_assert(end);
      return end - 1;
    }

    KRYS_NODISCARD int LookBackForNamedGridLine(int end, uint32 numberOfLines,
                                                NamedLineCollection &linesCollection) noexcept
    {
      krys_debug_assert(numberOfLines);

      // Only implicit lines on the search direction are assumed to have the given name, so we can start to
      // look from last line. See: https://drafts.csswg.org/css-grid/#grid-placement-span-int
      int start = std::min<int>(end, static_cast<int>(linesCollection.LastLine()));

      if (!linesCollection.HasNamedLines())
      {
        return std::min(start, -1) - static_cast<int>(numberOfLines) + 1;
      }

      for (; numberOfLines; --start)
      {
        if (start < 0 || linesCollection.Contains(static_cast<uint32>(start)))
        {
          numberOfLines--;
        }
      }

      return start + 1;
    }

    KRYS_NODISCARD css::CSSOMString NameOf(const css::CustomIdentifier &name) noexcept
    {
      return css::CSSOMString {name.Value.View()};
    }

    KRYS_NODISCARD int ResolveNamedGridLinePositionFromStyle(const RenderGrid &gridContainer,
                                                             const css::CustomIdentifier &name, int position,
                                                             GridPositionSide side) noexcept
    {
      NamedLineCollection linesCollection(gridContainer, NameOf(name), side);

      if (position > 0)
      {
        return static_cast<int>(
          LookAheadForNamedGridLine(0, static_cast<uint32>(std::abs(position)), linesCollection));
      }
      return LookBackForNamedGridLine(static_cast<int>(linesCollection.LastLine()),
                                      static_cast<uint32>(std::abs(position)), linesCollection);
    }

    KRYS_NODISCARD GridSpan DefiniteGridSpanWithNamedLineSpanAgainstOpposite(
      int oppositeLine, int spanPosition, GridPositionSide side,
      NamedLineCollection &linesCollection) noexcept
    {
      int start;
      int end;
      if (side == GridPositionSide::RowStartSide || side == GridPositionSide::ColumnStartSide)
      {
        start =
          LookBackForNamedGridLine(oppositeLine - 1, static_cast<uint32>(spanPosition), linesCollection);
        end = oppositeLine;
      }
      else
      {
        start = oppositeLine;
        end = static_cast<int>(
          LookAheadForNamedGridLine(oppositeLine + 1, static_cast<uint32>(spanPosition), linesCollection));
      }

      return GridSpan::UntranslatedDefiniteGridSpan(start, end);
    }

    KRYS_NODISCARD GridSpan ResolveNamedGridLinePositionAgainstOppositePosition(
      const RenderGrid &gridContainer, int oppositeLine, const css::CustomIdentifier &name, int spanPosition,
      GridPositionSide side) noexcept
    {
      NamedLineCollection linesCollection(gridContainer, NameOf(name), side);

      return DefiniteGridSpanWithNamedLineSpanAgainstOpposite(oppositeLine, spanPosition, side,
                                                              linesCollection);
    }

    KRYS_NODISCARD GridSpan ResolveGridPositionAgainstOppositePosition(const RenderGrid &gridContainer,
                                                                       int oppositeLine,
                                                                       const GridPosition &position,
                                                                       GridPositionSide side) noexcept
    {
      krys_debug_assert(position.ShouldBeResolvedAgainstOppositePosition());

      return position.SwitchOn(
        [&](const css::keywords::Auto &) -> GridSpan
        {
          if (IsStartSide(side))
          {
            return GridSpan::UntranslatedDefiniteGridSpan(oppositeLine - 1, oppositeLine);
          }
          return GridSpan::UntranslatedDefiniteGridSpan(oppositeLine, oppositeLine + 1);
        },
        [&](const GridPosition::Span &spanPosition) -> GridSpan
        {
          if (spanPosition.Name.Value != css::CSSOMStringAtom::Null())
          {
            // span 2 'c' -> we need to find the appropriate grid line before / after our opposite position.
            return ResolveNamedGridLinePositionAgainstOppositePosition(
              gridContainer, oppositeLine, spanPosition.Name, spanPosition.Position.Value, side);
          }

          // 'span 1' is contained inside a single grid track regardless of the direction.
          // That's why the CSS span value is one more than the offset we apply.
          auto positionOffset = spanPosition.Position.Value;
          if (IsStartSide(side))
          {
            return GridSpan::UntranslatedDefiniteGridSpan(oppositeLine - positionOffset, oppositeLine);
          }
          return GridSpan::UntranslatedDefiniteGridSpan(oppositeLine, oppositeLine + positionOffset);
        },
        [&](const GridPosition::Explicit &) -> GridSpan
        {
          krys_debug_assert(false);
          return GridSpan::IndefiniteGridSpan();
        },
        [&](const css::CustomIdentifier &) -> GridSpan
        {
          krys_debug_assert(false);
          return GridSpan::IndefiniteGridSpan();
        });
    }

    KRYS_NODISCARD int ResolveGridPositionFromStyle(const RenderGrid &gridContainer,
                                                    const GridPosition &position,
                                                    GridPositionSide side) noexcept
    {
      return position.SwitchOn(
        [&](const GridPosition::Explicit &explicitPosition) -> int
        {
          krys_debug_assert(explicitPosition.Position.Value != 0);

          if (explicitPosition.Name.Value != css::CSSOMStringAtom::Null())
          {
            return ResolveNamedGridLinePositionFromStyle(gridContainer, explicitPosition.Name,
                                                         explicitPosition.Position.Value, side);
          }

          // Handle <integer> explicit position.
          if (explicitPosition.Position.Value > 0)
          {
            return explicitPosition.Position.Value - 1;
          }

          auto resolvedPosition = static_cast<uint32>(std::abs(explicitPosition.Position.Value) - 1);
          uint32 endOfTrack = ExplicitGridSizeForSide(gridContainer, side);

          return static_cast<int>(endOfTrack) - static_cast<int>(resolvedPosition);
        },
        [&](const css::CustomIdentifier &namedGridAreaPosition) -> int
        {
          // First attempt to match the grid area's edge to a named grid area: if there is a named line with
          // the name
          // ''<custom-ident>-start (for grid-*-start) / <custom-ident>-end'' (for grid-*-end), contributes
          // the first such line to the grid item's placement.
          auto namedGridLine = NameOf(namedGridAreaPosition);
          krys_debug_assert(!namedGridLine.empty());

          NamedLineCollection implicitLines(gridContainer, namedGridLine, side, true);
          if (implicitLines.HasNamedLines())
          {
            return implicitLines.FirstPosition();
          }

          // Otherwise, if there is a named line with the specified name, contributes the first such line to
          // the grid item's placement.
          NamedLineCollection explicitLines(gridContainer, namedGridLine, side);
          if (explicitLines.HasNamedLines())
          {
            return explicitLines.FirstPosition();
          }

          // If none of the above works specs mandate to assume that all the lines in the implicit grid have
          // this name.
          return static_cast<int>(ExplicitGridSizeForSide(gridContainer, side)) + 1;
        },
        [&](const css::keywords::Auto &) -> int
        {
          // 'auto' depends on the opposite position for resolution (e.g. `grid-row: auto / 1`).
          krys_debug_assert(false);
          return 0;
        },
        [&](const GridPosition::Span &) -> int
        {
          // `span` depends on the opposite position for resolution (e.g. `grid-column: span 3 / "myHeader"`).
          krys_debug_assert(false);
          return 0;
        });
    }
  }

  uint32 GridPositionsResolver::ExplicitGridCount(const RenderGrid &gridContainer,
                                                  GridTrackSizingDirection direction) noexcept
  {
    if (gridContainer.IsSubgrid(direction))
    {
      auto &parent = *Downcast<RenderGrid>(gridContainer.Parent());
      auto flowAwareDirection =
        GridLayoutFunctions::FlowAwareDirectionForGridItem(parent, gridContainer, direction);
      return parent.GridSpanForGridItem(gridContainer, flowAwareDirection).IntegerSpan();
    }
    auto &style = gridContainer.Style();
    return std::min<uint32>(
      std::max(static_cast<uint32>(style.GridTemplateList(direction).Sizes.size()
                                   + gridContainer.AutoRepeatCountForDirection(direction)),
               static_cast<uint32>(style.GetGridTemplateAreas().Count(direction))),
      static_cast<uint32>(GridPosition::Max()));
  }

  GridPositionSide GridPositionsResolver::InitialPositionSide(GridTrackSizingDirection direction) noexcept
  {
    return direction == GridTrackSizingDirection::Columns ? GridPositionSide::ColumnStartSide
                                                          : GridPositionSide::RowStartSide;
  }

  GridPositionSide GridPositionsResolver::FinalPositionSide(GridTrackSizingDirection direction) noexcept
  {
    return direction == GridTrackSizingDirection::Columns ? GridPositionSide::ColumnEndSide
                                                          : GridPositionSide::RowEndSide;
  }

  uint32 GridPositionsResolver::SpanSizeForAutoPlacedItem(const RenderBox &gridItem,
                                                          GridTrackSizingDirection direction) noexcept
  {
    auto [initialPosition, finalPosition] = AdjustGridPositionsFromStyle(gridItem, direction);

    // This method will only be used when both positions need to be resolved against the opposite one.
    krys_debug_assert(initialPosition.ShouldBeResolvedAgainstOppositePosition()
                      && finalPosition.ShouldBeResolvedAgainstOppositePosition());

    if (initialPosition.IsAuto() && finalPosition.IsAuto())
    {
      return 1;
    }

    auto &position = initialPosition.IsSpan() ? initialPosition : finalPosition;
    krys_debug_assert(position.IsSpan());

    krys_debug_assert(position.SpanPosition());
    return static_cast<uint32>(position.SpanPosition());
  }

  GridSpan GridPositionsResolver::ResolveGridPositionsFromStyle(const RenderGrid &gridContainer,
                                                                const RenderBox &gridItem,
                                                                GridTrackSizingDirection direction) noexcept
  {
    auto [initialPosition, finalPosition] = AdjustGridPositionsFromStyle(gridItem, direction);

    auto initialSide = InitialPositionSide(direction);
    auto finalSide = FinalPositionSide(direction);

    // We can't get our grid positions without running the auto placement algorithm.
    if (initialPosition.ShouldBeResolvedAgainstOppositePosition()
        && finalPosition.ShouldBeResolvedAgainstOppositePosition())
    {
      return GridSpan::IndefiniteGridSpan();
    }

    if (initialPosition.ShouldBeResolvedAgainstOppositePosition())
    {
      // Infer the position from the final position ('auto / 1' or 'span 2 / 3' case).
      auto endLine = ResolveGridPositionFromStyle(gridContainer, finalPosition, finalSide);
      return ResolveGridPositionAgainstOppositePosition(gridContainer, endLine, initialPosition, initialSide);
    }

    if (finalPosition.ShouldBeResolvedAgainstOppositePosition())
    {
      // Infer our position from the initial position ('1 / auto' or '3 / span 2' case).
      auto startLine = ResolveGridPositionFromStyle(gridContainer, initialPosition, initialSide);
      return ResolveGridPositionAgainstOppositePosition(gridContainer, startLine, finalPosition, finalSide);
    }

    int startLine = ResolveGridPositionFromStyle(gridContainer, initialPosition, initialSide);
    int endLine = ResolveGridPositionFromStyle(gridContainer, finalPosition, finalSide);

    if (startLine > endLine)
    {
      std::swap(startLine, endLine);
    }
    else if (startLine == endLine)
    {
      endLine = startLine + 1;
    }

    return GridSpan::UntranslatedDefiniteGridSpan(startLine, std::max(startLine, endLine));
  }
}
