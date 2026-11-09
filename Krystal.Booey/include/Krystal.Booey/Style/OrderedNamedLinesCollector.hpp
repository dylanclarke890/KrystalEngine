/*
 * Copyright (C) 2004 Zack Rusin <zack@kde.org>
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007 Nicholas Shanks <webkit@nickshanks.com>
 * Copyright (C) 2011 Sencha, Inc. All rights reserved.
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/Extractor/ExtractorState.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridOrderedNamedLinesMap.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTemplateList.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::style
{
  /// @brief Collects the line names of a grid template, in order, for its computed value.
  class OrderedNamedLinesCollector
  {
    KRYS_NON_COPY_MOVABLE(OrderedNamedLinesCollector);

  protected:
    const GridOrderedNamedLinesMap &_orderedNamedGridLines;
    const GridOrderedNamedLinesMap &_orderedNamedAutoRepeatGridLines;

  public:
    OrderedNamedLinesCollector(ExtractorState &, const GridTemplateList &tracks) noexcept
        : _orderedNamedGridLines(tracks.OrderedNamedLines),
          _orderedNamedAutoRepeatGridLines(tracks.AutoRepeatOrderedNamedLines)
    {
    }
    virtual ~OrderedNamedLinesCollector() noexcept = default;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _orderedNamedGridLines.Map.empty() && _orderedNamedAutoRepeatGridLines.Map.empty();
    }

    virtual void CollectLineNamesForIndex(List<css::CSSOMString> &, uint32 index) const noexcept = 0;
    KRYS_NODISCARD virtual int32 NamedGridLineCount() const noexcept
    {
      return static_cast<int32>(_orderedNamedGridLines.Map.size());
    }

  protected:
    enum class NamedLinesType : bool
    {
      NamedLines,
      AutoRepeatNamedLines
    };
    void AppendLines(List<css::CSSOMString> &, uint32 index, NamedLinesType) const noexcept;
  };

  /// @brief Collects the line names of a grid container's template.
  class OrderedNamedLinesCollectorInGridLayout : public OrderedNamedLinesCollector
  {
  private:
    uint32 _insertionPoint {0};
    uint32 _autoRepeatTotalTracks {0};
    uint32 _autoRepeatTrackListLength {0};

  public:
    OrderedNamedLinesCollectorInGridLayout(ExtractorState &state, const GridTemplateList &tracks,
                                           uint32 autoRepeatTracksCount,
                                           uint32 autoRepeatTrackListLength) noexcept
        : OrderedNamedLinesCollector(state, tracks), _insertionPoint(tracks.AutoRepeatInsertionPoint),
          _autoRepeatTotalTracks(autoRepeatTracksCount), _autoRepeatTrackListLength(autoRepeatTrackListLength)
    {
    }

    void CollectLineNamesForIndex(List<css::CSSOMString> &, uint32 index) const noexcept override;
  };

  /// @brief Collects the line names of a subgrid's template.
  class OrderedNamedLinesCollectorInSubgridLayout : public OrderedNamedLinesCollector
  {
  private:
    uint32 _insertionPoint {0};
    uint32 _autoRepeatTotalLineSets {0};
    uint32 _autoRepeatLineSetListLength {0};
    uint32 _totalLines {0};

  public:
    OrderedNamedLinesCollectorInSubgridLayout(ExtractorState &state, const GridTemplateList &tracks,
                                              uint32 totalTracksCount) noexcept
        : OrderedNamedLinesCollector(state, tracks), _insertionPoint(tracks.AutoRepeatInsertionPoint),
          _autoRepeatLineSetListLength(static_cast<uint32>(tracks.AutoRepeatOrderedNamedLines.Map.size())),
          _totalLines(totalTracksCount + 1)
    {
      if (!_autoRepeatLineSetListLength)
      {
        _autoRepeatTotalLineSets = 0;
        return;
      }
      auto named = static_cast<uint32>(tracks.OrderedNamedLines.Map.size());
      if (named >= _totalLines)
      {
        _autoRepeatTotalLineSets = 0;
        return;
      }
      _autoRepeatTotalLineSets = (_totalLines - named) / _autoRepeatLineSetListLength;
      _autoRepeatTotalLineSets *= _autoRepeatLineSetListLength;
    }

    void CollectLineNamesForIndex(List<css::CSSOMString> &, uint32 index) const noexcept override;
    KRYS_NODISCARD int32 NamedGridLineCount() const noexcept override
    {
      return static_cast<int32>(_totalLines);
    }
  };

  inline void OrderedNamedLinesCollector::AppendLines(List<css::CSSOMString> &lineNames, uint32 index,
                                                      NamedLinesType type) const noexcept
  {
    auto &map =
      (type == NamedLinesType::NamedLines ? _orderedNamedGridLines : _orderedNamedAutoRepeatGridLines).Map;
    auto it = map.find(index);
    if (it == map.end())
    {
      return;
    }
    for (auto &name : it->second)
    {
      lineNames.push_back(name);
    }
  }

  inline void
    OrderedNamedLinesCollectorInGridLayout::CollectLineNamesForIndex(List<css::CSSOMString> &lineNamesValue,
                                                                     uint32 i) const noexcept
  {
    krys_debug_assert(!IsEmpty());
    if (!_autoRepeatTrackListLength || i < _insertionPoint)
    {
      AppendLines(lineNamesValue, i, NamedLinesType::NamedLines);
      return;
    }

    krys_debug_assert(_autoRepeatTotalTracks);

    if (i > _insertionPoint + _autoRepeatTotalTracks)
    {
      AppendLines(lineNamesValue, i - (_autoRepeatTotalTracks - 1), NamedLinesType::NamedLines);
      return;
    }

    if (i == _insertionPoint)
    {
      AppendLines(lineNamesValue, i, NamedLinesType::NamedLines);
      AppendLines(lineNamesValue, 0, NamedLinesType::AutoRepeatNamedLines);
      return;
    }

    if (i == _insertionPoint + _autoRepeatTotalTracks)
    {
      AppendLines(lineNamesValue, _autoRepeatTrackListLength, NamedLinesType::AutoRepeatNamedLines);
      AppendLines(lineNamesValue, _insertionPoint + 1, NamedLinesType::NamedLines);
      return;
    }

    uint32 autoRepeatIndexInFirstRepetition = (i - _insertionPoint) % _autoRepeatTrackListLength;
    if (!autoRepeatIndexInFirstRepetition && i > _insertionPoint)
    {
      AppendLines(lineNamesValue, _autoRepeatTrackListLength, NamedLinesType::AutoRepeatNamedLines);
    }
    AppendLines(lineNamesValue, autoRepeatIndexInFirstRepetition, NamedLinesType::AutoRepeatNamedLines);
  }

  inline void OrderedNamedLinesCollectorInSubgridLayout::CollectLineNamesForIndex(
    List<css::CSSOMString> &lineNamesValue, uint32 i) const noexcept
  {
    if (!_autoRepeatLineSetListLength || i < _insertionPoint)
    {
      AppendLines(lineNamesValue, i, NamedLinesType::NamedLines);
      return;
    }

    if (i >= _insertionPoint + _autoRepeatTotalLineSets)
    {
      AppendLines(lineNamesValue, i - _autoRepeatTotalLineSets, NamedLinesType::NamedLines);
      return;
    }

    uint32 autoRepeatIndexInFirstRepetition = (i - _insertionPoint) % _autoRepeatLineSetListLength;
    AppendLines(lineNamesValue, autoRepeatIndexInFirstRepetition, NamedLinesType::AutoRepeatNamedLines);
  }
}
